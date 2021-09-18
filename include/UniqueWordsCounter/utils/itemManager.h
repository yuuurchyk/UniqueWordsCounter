#pragma once

#include <thread>
#include <type_traits>

#include "tbb/concurrent_queue.h"

namespace UniqueWordsCounter::Utils
{
template <typename Item>
class ItemManager
{
    static_assert(std::is_same_v<Item, typename std::decay<Item>::type>,
                  "CV qualified types are not allowed as managed items");
    static_assert(!std::is_pointer_v<Item>, "Pointers are not allowed as managed items");

public:
    using item_type = Item;

    ItemManager() = default;
    ~ItemManager();

    class ItemGuard
    {
    public:
        [[nodiscard]] bool isDeathPill() const noexcept { return _item == nullptr; }
        item_type *        operator->() { return _item; }
        item_type &        operator*() { return *_item; }

        ItemGuard(ItemGuard &&) = default;

        ~ItemGuard();

    private:
        friend class ItemManager;
        ItemGuard(ItemManager &manager, item_type *item)
            : _manager{ manager }, _item{ item }
        {
        }

        ItemGuard(const ItemGuard &) = delete;
        ItemGuard &operator=(const ItemGuard &) = delete;

        ItemGuard &operator=(ItemGuard &&) = delete;

        ItemManager &_manager;
        item_type *  _item;
    };

    void                    setPending(item_type &);
    [[nodiscard]] ItemGuard retrievePending();

    [[nodiscard]] item_type *reuse();

    void addDeathPill();

private:
    ItemManager(const ItemManager &) = delete;
    ItemManager &operator=(const ItemManager &) = delete;

    ItemManager(ItemManager &&) = delete;
    ItemManager &operator=(ItemManager &&) = delete;

private:
    tbb::concurrent_bounded_queue<item_type *> _pendingItems;
    tbb::concurrent_bounded_queue<item_type *> _availableItems;
};

}    // namespace UniqueWordsCounter::Utils

#include "itemManagerImpl.h"
