//-------------------------------------------------------------------
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "FreeBagService/CarryOnChargesDataStrategy.h"

#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/BaggageOcValidationAdapter.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "ServiceFees/OCFees.h"

namespace tse
{

namespace
{
struct CheckSubCodeInfo : std::unary_function<const SubCodeInfo*, bool>
{
  bool _isInterLine;

  CheckSubCodeInfo(bool isInterLine) : _isInterLine(isInterLine) {}

  bool operator()(const SubCodeInfo* subCodeInfo) const
  {
    bool result =
        (subCodeInfo->fltTktMerchInd() == BAGGAGE_CHARGE) && (subCodeInfo->serviceSubGroup().equalToConst("CY"));

    if (_isInterLine)
      result = result && subCodeInfo->industryCarrierInd() == 'I';

    return result;
  }
};

struct CheckSubCodeInfo_old : std::unary_function<const SubCodeInfo*, bool>
{
  bool operator()(const SubCodeInfo* subCodeInfo) const
  {
    return (subCodeInfo->fltTktMerchInd() == BAGGAGE_CHARGE) && (subCodeInfo->serviceSubGroup().equalToConst("CY"));
  }
};
}

// when emdInfoMap is not set, Emd validation will not be performed
CarryOnChargesDataStrategy::CarryOnChargesDataStrategy(PricingTrx& trx, boost::optional<const EmdInterlineAgreementInfoMap&> emdInfoMap)
  : AncillaryChargesDataStrategy(trx, emdInfoMap)
{
}

CarryOnChargesDataStrategy::~CarryOnChargesDataStrategy() {}

void
CarryOnChargesDataStrategy::processBaggageTravel(BaggageTravel* baggageTravel,
                                                 const BaggageTravelInfo& bagInfo,
                                                 const CheckedPoint& furthestCheckedPoint,
                                                 BaggageTripType btt,
                                                 Diag852Collector* dc) const
{
  const OCFees* allowance = baggageTravel->_allowance;
  if (allowance &&
      // deliberate assignment
      (baggageTravel->_processCharges = shouldProcessCharges(allowance->optFee())))
  {
    AncillaryChargesDataStrategy::processBaggageTravel(
        baggageTravel, bagInfo, furthestCheckedPoint, btt, dc);
  }
}

void
CarryOnChargesDataStrategy::retrieveS5Records(const BaggageTravel* baggageTravel,
                                              std::vector<const SubCodeInfo*>& subCodes) const
{
  if (TrxUtil::isBaggage302GlobalDisclosureActivated(_trx))
  {
    ChargesDataStrategy::retrieveS5Records(
        getS5CarrierCode(baggageTravel),
        subCodes,
        CheckSubCodeInfo(!allSegmentsOnTheSameCarrier(*baggageTravel->itin())));
  }
  else
  {
    ChargesDataStrategy::retrieveS5Records(
        getS5CarrierCode(baggageTravel), subCodes, CheckSubCodeInfo_old());
  }
}

bool
CarryOnChargesDataStrategy::shouldDisplayChargesDiagnostic(BaggageTravel* baggageTravel,
                                                           const BaggageTravelInfo& bagInfo,
                                                           const Diag852Collector* dc) const
{
  return checkFareLineAndCheckedPortion(baggageTravel, bagInfo, dc) &&
         dc->isDisplayCarryOnChargesOption();
}

bool
CarryOnChargesDataStrategy::shouldProcessCharges(const OptionalServicesInfo* s7) const
{
  return !s7 || (s7->freeBaggagePcs() >= 0 && s7->freeBaggagePcs() <= 1);
}

void
CarryOnChargesDataStrategy::printS7ProcessingContext(BaggageTravel* baggageTravel,
                                                     const BaggageTravelInfo& bagInfo,
                                                     const SubCodeInfo* s5,
                                                     bool isUsDot,
                                                     Diag852Collector* dc,
                                                     bool /*defer*/,
                                                     bool /*isCarrierOverride*/) const
{
  dc->printS7ProcessingCarryOnContext(_trx, baggageTravel, bagInfo.bagIndex(), s5);
}

CarrierCode
CarryOnChargesDataStrategy::getS5CarrierCode(const BaggageTravel* baggageTravel) const
{
  return static_cast<const AirSeg*>(*baggageTravel->_MSS)->operatingCarrierCode();
}

void
CarryOnChargesDataStrategy::matchS7s(BaggageTravel& baggageTravel,
                                     const SubCodeInfo* s5,
                                     const CheckedPoint& furthestCheckedPoint,
                                     bool isUsDot,
                                     Diag852Collector* dc,
                                     ChargeVector& charges) const
{
  BaggageOcValidationAdapter::matchS7CarryOnChargesRecords(
      *s5, baggageTravel, furthestCheckedPoint, dc, charges);
}

bool
CarryOnChargesDataStrategy::shouldDisplayEmdDaignostic(const Diag852Collector* dc) const
{
  return dc->isDisplayCarryOnChargesOption();
}
} // tse
