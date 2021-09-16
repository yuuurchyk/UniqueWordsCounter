#pragma once

#include "itemManager.h"

template <typename Item>
UniqueWordsCounter::Utils::ItemManager<Item>::~ItemManager()
{
    item_type *item{};
    while (_availableItems.try_pop(item))
        delete item;

    while (true)
    {
        while (!_pendingItems.pop(item))
            std::this_thread::yield;

        if (item == nullptr)
            return;
        else
            delete item;
    }
}

template <typename Item>
auto UniqueWordsCounter::Utils::ItemManager<Item>::reuse() -> item_type *
{
    if (item_type * result; _availableItems.try_pop(result))
        return result;
    else
        return nullptr;
}

template <typename Item>
auto UniqueWordsCounter::Utils::ItemManager<Item>::setPending(item_type &item) -> void
{
    _pendingItems.push(&item);
}

template <typename Item>
auto UniqueWordsCounter::Utils::ItemManager<Item>::retrievePending() -> ItemGuard
{
    item_type *item{};

    while (!_pendingItems.pop(item))
        std::this_thread::yield();

    if (item == nullptr) [[unlikely]]
        _pendingItems.push(nullptr);

    return ItemGuard{ *this, item };
}

template <typename Item>
auto UniqueWordsCounter::Utils::ItemManager<Item>::addDeathPill() -> void
{
    _pendingItems.push(nullptr);
}

template <typename Item>
UniqueWordsCounter::Utils::ItemManager<Item>::ItemGuard::~ItemGuard()
{
    if (!isDeathPill()) [[likely]]
        _manager._availableItems.push(_item);
}
