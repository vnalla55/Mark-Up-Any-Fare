// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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

#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsValidators.h"

#include "Common/Assert.h"
#include "Common/Global.h"
#include "Common/GoverningCarrier.h"
#include "Common/ShoppingUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TseConsts.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/FareMarket.h"
#include "DataModel/InterlineTicketCarrierData.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/Diag910Collector.h"
#include "Diagnostic/DiagManager.h"

#include <algorithm>
#include <iterator>
#include <set>

namespace tse
{
namespace fos
{

VITAValidatorFOS::VITAValidatorFOS(ShoppingTrx& trx)
  : _trx(trx), _diag910(nullptr), _interlineTicketCxrData(nullptr)
{
  if (_trx.getRequest()->processVITAData() &&
      _trx.getOptions()->validateTicketingAgreement() &&
      !_trx.isValidatingCxrGsaApplicable())
  {
    if (_trx.diagnostic().diagnosticType() == Diagnostic910 &&
        _trx.diagnostic().diagParamMapItem("DD") == "VITA")
    {
      _diag910 = dynamic_cast<Diag910Collector*>(DCFactory::instance()->create(_trx));
      _diag910->activate();
    }

    if (InterlineTicketCarrier::isPriceInterlineActivated(trx))
    {
      _trx.dataHandle().get(_interlineTicketCxrData);
    }
    else if (_diag910)
    {
      *_diag910 << "IET PRICING IS NOT ACTIVE\n";
      _diag910->flushMsg();
    }
  }
  _resultsPerCxrMap.clear();
}

bool
VITAValidatorFOS::
operator()(SopsCombination& sopVec)
{
  bool validationResult(true);
  if (_interlineTicketCxrData)
  {
    Itin* itin = _trx.dataHandle().create<Itin>();
    for (size_t legIdx = 0; legIdx < sopVec.size(); ++legIdx)
    {
      const Itin* sopItin = _trx.legs()[legIdx].sop()[sopVec[legIdx]].itin();
      itin->travelSeg().insert(
          itin->travelSeg().end(), sopItin->travelSeg().begin(), sopItin->travelSeg().end());
    }
    ValidatingCarrierUpdater valCxrUpdater(_trx);
    const std::vector<int>& tempSopVec = sopVec;
    valCxrUpdater.update(*itin, false, &tempSopVec);

    std::string validationMessage;
    validationResult = _interlineTicketCxrData->validateInterlineTicketCarrierAgreement(
        _trx, itin->validatingCarrier(), itin->travelSeg(), &validationMessage);
    addValidationResult(sopVec, itin, validationResult, validationMessage);
  }
  return validationResult;
}

void
VITAValidatorFOS::addValidationResult(SopsCombination& sopVec,
                                      Itin* itin,
                                      bool validationResult,
                                      const std::string& validationMessage)
{
  if (_diag910 && itin)
  {
    ValidationResultInfo validationInfo(sopVec, itin, validationResult, validationMessage);
    _resultsPerCxrMap[itin->validatingCarrier()].push_back(validationInfo);
  }
}

void
VITAValidatorFOS::printDiagnostics()
{
  if (_diag910 && _interlineTicketCxrData && !_resultsPerCxrMap.empty())
  {
    for (ResultPerCarrierMap::value_type& value : _resultsPerCxrMap)
    {
      const CarrierCode& validatingCarrier = value.first;
      (*_diag910) << _interlineTicketCxrData->getInterlineCarriersForValidatingCarrier(
                         _trx, validatingCarrier);

      for (ValidationResultInfo& validationResultInfo : value.second)
      {
        _diag910->printVITAData(_trx,
                                validationResultInfo._sopVec,
                                validationResultInfo._itin->travelSeg(),
                                validationResultInfo._itin->validatingCarrier(),
                                validationResultInfo._validationResult,
                                false,
                                validationResultInfo._validationMessage);
      }
    }
    _diag910->flushMsg();
  }
}

void
Validator::printDiagnostics()
{
  _vitaValidator.printDiagnostics();
}

bool
Validator::isValidSolution(SopsCombination& sopVec)
{
  if (!isValidSolutionCommon(sopVec))
    return false;

  return checkConnectingFlights(sopVec) && checkConnectingCities(sopVec) &&
         checkDirectFlight(sopVec);
}

bool
Validator::isValidSolutionCommon(SopsCombination& sopVec)
{
  if (_trx.onlineSolutionsOnly())
  {
    if (!ShoppingUtil::isOnlineSolution(_trx, sopVec))
      return false;
  }
  if (_trx.interlineSolutionsOnly())
  {
    if (ShoppingUtil::isOnlineSolution(_trx, sopVec) && checkOnlineFlightsCombination(sopVec))
      return false;
  }

  return _vitaValidator(sopVec) && checkMinConnectionTime(sopVec) &&
         ShoppingUtil::isCabinClassValid(_trx, sopVec) &&
         ShoppingUtil::checkForcedConnection(sopVec, _trx) &&
         (_trx.getRequest()->cxrOverride() == BLANK_CODE ||
          ShoppingUtil::checkOverridedSegment(sopVec, _trx));
}

bool
Validator::checkMinConnectionTime(SopsCombination& sopVec)
{
  return ShoppingUtil::checkMinConnectionTime(_trx.getOptions(), sopVec, _trx.legs());
}

bool
Validator::checkOnlineFlightsCombination(const SopsCombination& sopVec)
{
  std::vector<TravelSeg*> itinerarySegments;
  for (size_t legIdx = 0; legIdx < sopVec.size(); ++legIdx)
  {
    std::vector<TravelSeg*>& tvlSegs = _trx.legs()[legIdx].sop()[sopVec[legIdx]].itin()->travelSeg();
    itinerarySegments.insert(itinerarySegments.end(), tvlSegs.begin(), tvlSegs.end());
  }
  return TravelSegUtil::isTravelSegVecOnline(itinerarySegments);
}

bool
Validator::checkConnectingFlights(SopsCombination& sopVec, size_t noOfSegments)
{
  for (size_t legNo = 0; legNo < sopVec.size(); ++legNo)
  {
    const int sopNo = sopVec[legNo];
    const ShoppingTrx::SchedulingOption& sop = _trx.legs()[legNo].sop()[sopNo];

    if (sop.itin()->travelSeg().size() > noOfSegments)
      return true;
  }
  return false;
}

ShapeSolution::Type
AltDatesValidator::validateSolutionShape(SOPCollections& sopsCollection, SopsCombination& sopVec)
{
  SOPMap::iterator outboundSopIt =
      sopsCollection.getSopsByLegDetails()[0].find(SOPWrapper(sopVec[0]));
  SOPMap::iterator inboundSopIt =
      sopsCollection.getSopsByLegDetails()[1].find(SOPWrapper(sopVec[1]));
  TSE_ASSERT(outboundSopIt != sopsCollection.getSopsByLegDetails()[0].end());
  TSE_ASSERT(inboundSopIt != sopsCollection.getSopsByLegDetails()[1].end());

  // at this point we need to now only if it's snowmand or not
  if (isSnowman(outboundSopIt->second, inboundSopIt->second))
    return ShapeSolution::VALID_SNOWMAN;
  return ShapeSolution::VALID;
}

bool
AltDatesValidator::checkDirectFlight(SopsCombination& sopVec)
{
  for (size_t legNo = 0; legNo < sopVec.size(); ++legNo)
  {
    const int sopNo = sopVec[legNo];
    const ShoppingTrx::SchedulingOption& sop = _trx.legs()[legNo].sop()[sopNo];
    if (sop.itin()->travelSeg().size() == 1)
      return true;
  }
  return false;
}

bool
AltDatesValidator::isSnowman(const SOPDetailsVec& outbDetails, const SOPDetailsVec& inbDetails)
{
  for (const SOPDetails& outboundComb : outbDetails)
  {
    for (const SOPDetails& inboundComb : inbDetails)
    {
      if (outboundComb._tvlSegPortions[0].back()->destAirport() ==
          inboundComb._tvlSegPortions[0].back()->destAirport())
        return true;
    }
  }
  return false;
}

Validator*
SoloFlightOnlySolutionsValidators::getValidator(ShoppingTrx& trx, ValidatorType type)
{
  switch (type)
  {
  case ALTDATES:
    return &trx.dataHandle().safe_create<AltDatesValidator>(trx);
  case COMMON:
  default:
    return &trx.dataHandle().safe_create<Validator>(trx);
  }
}

} // namespace fos
} // namespace tse
