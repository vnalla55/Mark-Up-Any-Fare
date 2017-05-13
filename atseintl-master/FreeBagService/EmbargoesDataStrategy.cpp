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

#include "FreeBagService/EmbargoesDataStrategy.h"

#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/BaggageOcValidationAdapter.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "ServiceFees/OCFees.h"


namespace tse
{

namespace
{

struct CheckS5_old : std::unary_function<const SubCodeInfo*, bool>
{
  bool operator()(const SubCodeInfo* s5) const
  {
    return s5->fltTktMerchInd() == BAGGAGE_EMBARGO && s5->concur() == 'X';
  }
};

struct CheckS5 : std::unary_function<const SubCodeInfo*, bool>
{
  bool _isInterLine;

  CheckS5(bool isInterLine) : _isInterLine(isInterLine) {}

  bool operator()(const SubCodeInfo* s5) const
  {
    bool result = s5->fltTktMerchInd() == BAGGAGE_EMBARGO && s5->concur() == 'X';

    if (_isInterLine)
      result = result && s5->industryCarrierInd() == 'I';

    return result;
  }
};
}

EmbargoesDataStrategy::EmbargoesDataStrategy(PricingTrx& trx) : AncillaryChargesDataStrategy(trx) {}

EmbargoesDataStrategy::~EmbargoesDataStrategy() {}

void
EmbargoesDataStrategy::processBaggageTravel(BaggageTravel* baggageTravel,
                                            const BaggageTravelInfo& bagInfo,
                                            const CheckedPoint& furthestCheckedPoint,
                                            BaggageTripType btt,
                                            Diag852Collector* dc) const
{
  std::vector<const SubCodeInfo*> subCodes;
  retrieveS5Records(baggageTravel, subCodes);

  for (const SubCodeInfo* subCodeInfo : subCodes)
  {
    OCFees& embargo =
        matchS7(baggageTravel, bagInfo, subCodeInfo, furthestCheckedPoint, btt.isUsDot(), dc);

    if (embargo.optFee())
      baggageTravel->_embargoVector.push_back(&embargo);
  }
}

void
EmbargoesDataStrategy::retrieveS5Records(const BaggageTravel* baggageTravel,
                                         std::vector<const SubCodeInfo*>& subCodes) const
{
  if (TrxUtil::isBaggage302GlobalDisclosureActivated(_trx))
  {
    ChargesDataStrategy::retrieveS5Records(
        static_cast<const AirSeg*>(*baggageTravel->_MSS)->operatingCarrierCode(),
        subCodes,
        CheckS5(!allSegmentsOnTheSameCarrier(*baggageTravel->itin())));
  }
  else
  {
    ChargesDataStrategy::retrieveS5Records(
        static_cast<const AirSeg*>(*baggageTravel->_MSS)->operatingCarrierCode(),
        subCodes,
        CheckS5_old());
  }
}

OCFees&
EmbargoesDataStrategy::matchS7(BaggageTravel* baggageTravel,
                               const BaggageTravelInfo& bagInfo,
                               const SubCodeInfo* s5,
                               const CheckedPoint& furthestCheckedPoint,
                               bool isUsDot,
                               Diag852Collector* dc) const
{
  bool displayDiag = false;

  if (dc)
  {
    displayDiag = shouldDisplayDiagnostic(baggageTravel, bagInfo, s5->serviceSubTypeCode(), dc);

    if (displayDiag)
      printS7ProcessingContext(baggageTravel, bagInfo, s5, isUsDot, dc);

    displayDiag &= shouldDisplayS7Diagnostic(dc);
  }

  return BaggageOcValidationAdapter::matchS7EmbargoRecord(
      *s5, *baggageTravel, furthestCheckedPoint, (displayDiag ? dc : nullptr));
}

void
EmbargoesDataStrategy::printS7ProcessingContext(BaggageTravel* baggageTravel,
                                                const BaggageTravelInfo& bagInfo,
                                                const SubCodeInfo* s5,
                                                bool isUsDot,
                                                Diag852Collector* dc) const
{
  dc->printS7ProcessingCarryOnContext(_trx, baggageTravel, bagInfo.bagIndex(), s5);
}

bool
EmbargoesDataStrategy::shouldDisplayDiagnostic(BaggageTravel* baggageTravel,
                                               const BaggageTravelInfo& bagInfo,
                                               const ServiceSubTypeCode& subTypeCode,
                                               const Diag852Collector* dc) const
{
  return shouldDisplayDiagnostic(baggageTravel, bagInfo, dc) &&
         (!shouldDisplayS7Diagnostic(dc) || (dc->subTypeCode() == subTypeCode));
}

bool
EmbargoesDataStrategy::shouldDisplayDiagnostic(BaggageTravel* baggageTravel,
                                               const BaggageTravelInfo& bagInfo,
                                               const Diag852Collector* dc) const
{
  return checkFareLineAndCheckedPortion(baggageTravel, bagInfo, dc) &&
         dc->isDisplayEmbargoesOption();
}
}
