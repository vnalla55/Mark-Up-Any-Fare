#include "BrandingService/BrandResponseItem.h"

namespace tse
{

void
BrandResponseItem::clear()
{
  _campaignCode = "";
  _brandCode = "";
  _bookingCodeVec.clear();
  _includedFClassVec.clear();
  _excludedFClassVec.clear();
}
};
