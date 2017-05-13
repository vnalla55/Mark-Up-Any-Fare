#include "Common/TseConsts.h"
#include "DBAccess/SubCodeInfo.h"
#include "FreeBagService/test/S7Builder.h"
namespace tse
{
S7Builder&
S7Builder::withAllowanceMatched(const CarrierCode& carrier, const Indicator& notAvailNoChargeInd)
{
  _s7->carrier() = carrier;
  _s7->vendor() = ATPCO_VENDOR_CODE;
  _s7->frequentFlyerStatus() = 0;
  _s7->serviceFeesAccountCodeTblItemNo() = 0;
  _s7->serviceFeesTktDesigTblItemNo() = 0;
  _s7->serviceFeesSecurityTblItemNo() = 0;
  _s7->publicPrivateInd() = 'J';
  _s7->sectorPortionInd() = ' ';
  _s7->stopCnxDestInd() = ' ';
  _s7->cabin() = ' ';
  _s7->serviceFeesResBkgDesigTblItemNo() = 0;
  _s7->serviceFeesCxrResultingFclTblItemNo() = 0;
  _s7->resultServiceFeesTktDesigTblItemNo() = 0;
  _s7->ruleTariffInd() = "";
  _s7->notAvailNoChargeInd() = 'F';
  _s7->seqNo() = 1;
  _s7->notAvailNoChargeInd() = notAvailNoChargeInd;

  return *this;
}
}
