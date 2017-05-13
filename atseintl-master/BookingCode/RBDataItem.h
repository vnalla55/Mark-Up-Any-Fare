#pragma once

#include "DataModel/FareDisplayTrx.h"

namespace tse
{

/* RBDataItem is the result from ATSE that is converted to XML request for RTG.*/

class RBDataItem
{

public:
  RBDataItem() : _vendor(EMPTY_STRING()) {}

  RBDataItem* getRBDataItem(const FareDisplayTrx& _trx)
  {
    RBDataItem* item(nullptr);
    _trx.dataHandle().get(item);
    return item;
  }

  uint32_t _itemNo = 0;
  uint32_t _seqNo = 0;
  uint32_t _segNo = 0;
  std::string _vendor;
};
}
