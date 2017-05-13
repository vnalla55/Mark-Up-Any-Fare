#include "DataModel/FBRPaxTypeFareRuleData.h"

#include "DBAccess/DataHandle.h"

namespace tse
{

bool
FBRPaxTypeFareRuleData::getBaseFarePrimeBookingCode(std::vector<BookingCode>& bookingCodeVec) const
{
  bookingCodeVec.clear();
  for (const auto& elem : _baseFareBookingCode)
  {
    if (!elem.empty())
    {
      bookingCodeVec.push_back(elem);
    }
  }
  return (!bookingCodeVec.empty());
}

void
FBRPaxTypeFareRuleData::setBaseFarePrimeBookingCode(
    std::vector<BookingCode>& baseFareBookingCodeVec)
{
  if (UNLIKELY(baseFareBookingCodeVec.size() == 0))
  {
    return;
  }
  for (uint16_t i = 0; i < baseFareBookingCodeVec.size(); ++i)
  {
    if (LIKELY(!baseFareBookingCodeVec[i].empty()))
    {
      _baseFareBookingCode[i] = baseFareBookingCodeVec[i];
    }
  }
}

FBRPaxTypeFareRuleData*
FBRPaxTypeFareRuleData::clone(DataHandle& dataHandle) const
{
  FBRPaxTypeFareRuleData* cloneObj = nullptr;
  dataHandle.get(cloneObj);

  copyTo(*cloneObj);

  return cloneObj;
}

void
FBRPaxTypeFareRuleData::copyTo(FBRPaxTypeFareRuleData& cloneObj) const
{
  // Clone base
  PaxTypeFareRuleData::copyTo(cloneObj);

  cloneObj._fbrApp = _fbrApp;
  cloneObj._baseFareBookingCodeTblItemNo = _baseFareBookingCodeTblItemNo;
  for (int i = 0; i < FareClassAppSegInfo::BK_CODE_SIZE; ++i)
    cloneObj._baseFareBookingCode[i] = _baseFareBookingCode[i];
  cloneObj._isR8LocationSwapped = _isR8LocationSwapped;
  cloneObj._isSpanishResidence = _isSpanishResidence;
  cloneObj._baseFareInfoBookingCodes = _baseFareInfoBookingCodes;
  cloneObj._isBaseFareAvailBkcMatched = _isBaseFareAvailBkcMatched;
}

} // tse
