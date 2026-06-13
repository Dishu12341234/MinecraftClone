#include "ItemPool.h"
#include "Properties.h"

// get the item at 'ItemID' 'idx'
Item *ItemPool::getItemAt(uint64_t idx) {
  auto it = items.find(idx);

  if (it == items.end()) {
    return nullptr;
  }

  return it->second;
}

void ItemPool::insertNewItem(uint64_t ID, Item *item) { items[ID] = item; }

void ItemPool::cleanUPItems() {
  for (auto &&[id, item] : items) {
    if (item->itemType == ItemType::BLOCK)
      delete static_cast<BlockTypeProperties *>(item->properties);
    delete item;
  }
  items.clear();
}
