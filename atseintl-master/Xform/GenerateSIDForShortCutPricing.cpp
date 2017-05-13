#include "Xform/GenerateSIDForShortCutPricing.h"

#include "Common/ShoppingUtil.h"
#include "DataModel/ShoppingTrx.h"

#include <vector>

namespace tse
{

void
GenerateSIDForShortCutPricing::process(const Itin*)
{
  for (uint16_t i = 0; i < _sops.size(); ++i)
  {
    Node(_writer, "SID").convertAttr("Q14", i).convertAttr(
        "Q15", ShoppingUtil::findSopId(*_trx, i, _sops[i]));
  }
}
}
