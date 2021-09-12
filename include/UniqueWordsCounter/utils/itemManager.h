#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "tbb/concurrent_queue.h"

namespace UniqueWordsCounter::Utils
{
// clang-format off
template <typename Item, typename ItemAllocator=std::allocator<Item>>
    requires(
        std::is_same_v<Item, typename std::decay<Item>::type> &&
        !std::is_pointer_v<Item>
    )
class ItemManager
// clang-format on
{
public:
    using item_type           = Item;
    using item_allocator_type = ItemAllocator;

    explicit ItemManager(size_t channelsNum = 1ULL) : _channelsNum{ channelsNum }
    {
        if (channelsNum == 0)
            throw std::runtime_error{ "Number of channels should be positive" };

        // prevent false sharing
        _pendingItems.reserve(channelsNum);
        for (auto i = size_t{}; i < channelsNum; ++i)
            _pendingItems.emplace_back(new (std::align_val_t{ 64 })
                                           tbb::concurrent_bounded_queue<item_type *>);
    }

    ItemManager(const ItemManager &) = delete;
    ItemManager &operator=(const ItemManager &) = delete;

    ItemManager(ItemManager &&) = delete;
    ItemManager &operator=(ItemManager &&) = delete;

    ~ItemManager()
    {
        if (!_wasDeathPillAdded)
            unsafe_addDeathPillToAllChannels();

        while (true)
        {
            item_type *current{};
            while (!_ownerItems.pop(current)) {}
            if (current == nullptr)
            {
                break;
            }
            else
            {
                std::destroy_at(current);
                _allocator.deallocate(current, 1ULL);
            }
        }
    }

    class ItemGuard
    {
    public:
        ItemGuard(const ItemGuard &) = delete;
        ItemGuard &operator=(const ItemGuard &) = delete;

        ItemGuard(ItemGuard &&) = default;
        ItemGuard &operator=(ItemGuard &&) = delete;

        ~ItemGuard()
        {
            if (!isDeathPill())
                _manager._availableItems.push(_item);
        }

        bool       isDeathPill() const { return _item == nullptr; }
        item_type *operator->() { return _item; }
        item_type &operator*() { return *_item; }

    private:
        friend class ItemManager;
        ItemGuard(ItemManager &manager, size_t channelIndex) : _manager{ manager }
        {
            while (!_manager._pendingItems[channelIndex]->pop(_item)) {}
            if (isDeathPill())
                _manager._pendingItems[channelIndex]->push(_item);
        }

    private:
        ItemManager &_manager;
        item_type *  _item{};
    };

    [[nodiscard]] item_type *reuse()
    {
        if (item_type * result; _availableItems.try_pop(result))
            return result;
        return nullptr;
    }

    template <typename... Args>
    [[nodiscard]] item_type *allocate(Args &&...args)
    {
        auto item = _allocator.allocate(1ULL);
        std::construct_at(item, std::forward<Args>(args)...);

        _ownerItems.push(item);

        return item;
    }

    void setPending(item_type &item, size_t channel = 0ULL)
    {
        checkChannelIndex(channel);
        _pendingItems[channel]->push(&item);
    }

    void unsafe_addDeathPillToAllChannels()
    {
        _wasDeathPillAdded = true;
        for (size_t channel{}; channel < channelsNum(); ++channel)
            _pendingItems[channel]->push(nullptr);
        _ownerItems.push(nullptr);
    }

    [[nodiscard]] ItemGuard retrievePending(size_t channel = 0ULL)
    {
        checkChannelIndex(channel);
        return ItemGuard{ *this, channel };
    }

    [[nodiscard]] inline size_t channelsNum() const noexcept { return _channelsNum; }

private:
    inline void checkChannelIndex(size_t channel) const
    {
        using namespace std::string_literals;
        if (channel >= channelsNum())
            throw std::runtime_error{ "Invalid channel: "s + std::to_string(channel) };
    }

private:
    tbb::concurrent_bounded_queue<item_type *> _ownerItems;

    const size_t _channelsNum;
    std::vector<std::unique_ptr<tbb::concurrent_bounded_queue<item_type *>>>
        _pendingItems;

    tbb::concurrent_bounded_queue<item_type *> _availableItems;

    bool _wasDeathPillAdded{};

    item_allocator_type _allocator{};
};

}    // namespace UniqueWordsCounter::Utils
