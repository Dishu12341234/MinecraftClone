#ifndef ITEMPOOL_H
#define ITEMPOOL_H

#include "Item.h"
#include <cstdint>
#include <unordered_map>

class ItemPool {
private:
  std::unordered_map<uint64_t, Item *> items;

public:
  Item *getItemAt(uint64_t idx);
  void insertNewItem(uint64_t itemID, Item *item);

  std::unordered_map<uint64_t, Item *> getItems_p() { return items; };

  void cleanUPItems();
};

#endif
