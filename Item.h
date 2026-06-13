#ifndef ITEM_H
#define ITEM_H

#include <cstdint>
#include <string>

enum struct ItemType {
  BLOCK,
  WEAPON,
  TOOL,
};

class Item {
private:
  uint64_t itemID;
  std::string itemUITexturePath;
  ItemType itemType;

  void *properties;
  friend class HelloTriangleApplication;
  friend class ItemPool;

public:
  Item(uint64_t itemID, std::string itemUITexturePath, ItemType type,
       void *properties)
      : itemID(itemID), itemUITexturePath(itemUITexturePath), itemType(type),
        properties(properties) {};
  ~Item() = default;
};

#endif
