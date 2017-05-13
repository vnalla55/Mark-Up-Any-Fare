// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "FreeBagService/CarryOnAllowanceDataStrategy.h"

#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/BaggageAllowanceValidator.h"
#include "FreeBagService/BaggageOcValidationAdapter.h"
#include "FreeBagService/BaggageTravelInfo.h"


#include <iostream>

namespace tse
{

namespace
{
struct CheckS5 : std::unary_function<const SubCodeInfo*, bool>
{
  bool operator()(const SubCodeInfo* s5) const
  {
    return s5->fltTktMerchInd() == CARRY_ON_ALLOWANCE && s5->serviceSubTypeCode().equalToConst("0LN");
  }
};
}

CarryOnAllowanceDataStrategy::CarryOnAllowanceDataStrategy(PricingTrx& trx) : DataStrategyBase(trx)
{
}

CarryOnAllowanceDataStrategy::~CarryOnAllowanceDataStrategy() {}

void
CarryOnAllowanceDataStrategy::processBaggageTravel(BaggageTravel* baggageTravel,
                                                   const BaggageTravelInfo& bagInfo,
                                                   const CheckedPoint& furthestCheckedPoint,
                                                   BaggageTripType btt,
                                                   Diag852Collector* dc) const
{
  CarrierCode carrier = static_cast<const AirSeg*>(*baggageTravel->_MSS)->operatingCarrierCode();
  
  const SubCodeInfo* s5 = retrieveS5Record(carrier);

  if (s5)
  {
    OCFees& allowance = matchS7(baggageTravel, bagInfo, s5, furthestCheckedPoint, btt.isUsDot(), dc);
    baggageTravel->_allowance = &allowance;
  }
}

const SubCodeInfo*
CarryOnAllowanceDataStrategy::retrieveS5Record(const CarrierCode& carrier) const
{
  CheckS5 checkS5;

  for (const SubCodeInfo* subCodeInfo :
       DataStrategyBase::retrieveS5Records(ATPCO_VENDOR_CODE, carrier))
  {
    if (checkS5(subCodeInfo))
      return subCodeInfo;
  }

  for (const SubCodeInfo* subCodeInfo :
       DataStrategyBase::retrieveS5Records(MERCH_MANAGER_VENDOR_CODE, carrier))
  {
    if (checkS5(subCodeInfo))
      return subCodeInfo;
  }

  return nullptr;
}

OCFees&
CarryOnAllowanceDataStrategy::matchS7(BaggageTravel* baggageTravel,
                                      const BaggageTravelInfo& bagInfo,
                                      const SubCodeInfo* s5,
                                      const CheckedPoint& furthestCheckedPoint,
                                      bool isUsDot,
                                      Diag852Collector* dc) const
{
  bool displayDiag = shouldDisplayDiagnostic(baggageTravel, bagInfo, dc);

  if (displayDiag)
    printS7ProcessingContext(baggageTravel, bagInfo, s5, isUsDot, dc);

  return BaggageOcValidationAdapter::matchS7CarryOnAllowanceRecord(
      *s5, *baggageTravel, furthestCheckedPoint, (displayDiag ? dc : nullptr));
}

void
CarryOnAllowanceDataStrategy::printS7ProcessingContext(BaggageTravel* baggageTravel,
                                                       const BaggageTravelInfo& bagInfo,
                                                       const SubCodeInfo* s5,
                                                       bool isUsDot,
                                                       Diag852Collector* dc) const
{
  dc->printS7ProcessingCarryOnContext(_trx, baggageTravel, bagInfo.bagIndex(), s5);
}

bool
CarryOnAllowanceDataStrategy::shouldDisplayDiagnostic(BaggageTravel* baggageTravel,
                                                      const BaggageTravelInfo& bagInfo,
                                                      const Diag852Collector* dc) const
{
  return checkFareLineAndCheckedPortion(baggageTravel, bagInfo, dc) &&
         dc->isDisplayCarryOnAllowanceOption();
}
}
