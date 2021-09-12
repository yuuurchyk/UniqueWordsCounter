#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "tbb/concurrent_queue.h"

namespace UniqueWordsCounter::Utils
{
// clang-format off
template <typename T>
    requires(
        std::is_same_v<T, typename std::decay<T>::type> &&
        !std::is_pointer_v<T>
    )
class ItemManager
// clang-format on
{
public:
    using item_type = T;

    ItemManager() = default;
    ItemManager(std::function<void(item_type &)> itemCleanupFn)
        : _itemCleanupFn{ std::move(itemCleanupFn) }
    {
    }

    ItemManager(const ItemManager &) = delete;
    ItemManager &operator=(const ItemManager &) = delete;

    ItemManager(ItemManager &&) = delete;
    ItemManager &operator=(ItemManager &&) = delete;

    ~ItemManager() = default;

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
            {
                if (const auto &fn = _manager._itemCleanupFn; fn != nullptr)
                    fn(*_item);
                _manager._availableItems.push(_item);
            }
        }

        bool       isDeathPill() const { return _item == nullptr; }
        item_type *operator->() { return _item; }
        item_type &operator*() { return *_item; }

    private:
        friend class ItemManager;
        ItemGuard(ItemManager &manager) : _manager{ manager }
        {
            while (!_manager._pendingItems.pop(_item)) {}
            if (isDeathPill())
                _manager._pendingItems.push(_item);
        }

    private:
        ItemManager &_manager;
        item_type *  _item{};
    };

    template <typename... Args>
    item_type *allocate(Args &&...args)
    {
        if (item_type * result{}; _availableItems.try_pop(result))
            return result;

        std::scoped_lock lck{ _itemsOwnerMutex };
        _itemsOwner.emplace_back(new item_type{ std::forward<Args>(args)... });

        return _itemsOwner.back().get();
    }
    void setPending(item_type *item) { _pendingItems.push(item); }

    void addDeathPill()
    {
        _wasDeathPillAdded = true;
        _pendingItems.push(nullptr);
    }

    ItemGuard retrievePending() { return ItemGuard{ *this }; }

private:
    std::mutex                              _itemsOwnerMutex;
    std::vector<std::unique_ptr<item_type>> _itemsOwner;

    tbb::concurrent_bounded_queue<item_type *> _pendingItems;
    tbb::concurrent_bounded_queue<item_type *> _availableItems;

    bool _wasDeathPillAdded{};

    const std::function<void(item_type &)> _itemCleanupFn{};
};

}    // namespace UniqueWordsCounter::Utils
