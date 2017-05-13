//-------------------------------------------------------------------
//  Copyright Sabre 2011
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
#include "FreeBagService/ChargesDataStrategy.h"

#include "Common/FreeBaggageUtil.h"
#include "Common/FreqFlyerUtils.h"
#include "Common/Logger.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxType.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/BaggageOcValidationAdapter.h"
#include "FreeBagService/BaggageTextUtil.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "FreeBagService/ChargesUtil.h"
#include "Util/Algorithm/Bitset.h"
#include "Util/Algorithm/Iterator.h"
#include "Util/IteratorRange.h"

#include <boost/lexical_cast.hpp>

namespace tse
{
static Logger
logger("atseintl.FreeBagService.ChargesDataStrategy");

void
ChargesDataStrategy::processBaggageTravel(BaggageTravel* baggageTravel,
                                          const BaggageTravelInfo& bagInfo,
                                          const CheckedPoint& furthestCheckedPoint,
                                          BaggageTripType btt,
                                          Diag852Collector* dc) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " - entering");
  if (!baggageTravel->_processCharges)
    return;

  const uint32_t requestedBagPieces = _trx.getBaggagePolicy().getRequestedBagPieces();
  const uint32_t firstChargedPiece =
      FreeBaggageUtil::calcFirstChargedPiece(baggageTravel->_allowance);

  if (firstChargedPiece >= requestedBagPieces)
    return;

  std::vector<const SubCodeInfo*> subCodes;
  retrieveS5Records(baggageTravel, btt.isUsDot(), subCodes);
  if (TrxUtil::isFrequentFlyerTierActive(_trx) && !_trx.getRequest()->isSpecificAgencyText())
  {
    if (!subCodes.empty())
    {
      baggageTravel->setCarrier(subCodes.front()->carrier());
      baggageTravel->setFreqFlyerTierLevel(freqflyerutils::determineFreqFlyerTierLevel(
          dc,
          baggageTravel->paxType()->freqFlyerTierWithCarrier(),
          baggageTravel->getCarrier(),
          &_trx));
    }
  }
  retrieveCharges(baggageTravel, bagInfo, subCodes, furthestCheckedPoint, btt.isUsDot(), dc);

  if (shouldDisplayChargesDiagnostic(baggageTravel, bagInfo, dc))
  {
    dc->printChargesHeader();
    for (uint32_t bagNo = 0; bagNo < requestedBagPieces; ++bagNo)
      dc->printCharge(baggageTravel->_charges[bagNo], bagNo);
  }
}

void
ChargesDataStrategy::retrieveS5Records(const BaggageTravel* baggageTravel,
                                       bool isUsDot,
                                       std::vector<const SubCodeInfo*>& subCodes) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  const OptionalServicesInfo* allowanceS7 = getAllowanceS7(baggageTravel);
  CarrierCode carrier;

  if (allowanceS7)
    carrier = allowanceS7->carrier();
  else
    carrier =
        isUsDot
            ? static_cast<const AirSeg*>(baggageTravel->_carrierTravelSeg)->marketingCarrierCode()
            : static_cast<const AirSeg*>(*baggageTravel->_MSSJourney)->marketingCarrierCode();

  subCodes = ChargesUtil::retrieveS5(_trx, *baggageTravel->itin(), carrier);
}

void
ChargesDataStrategy::retrieveS5Records(
    const CarrierCode& carrier,
    std::vector<const SubCodeInfo*>& subCodes,
    const boost::function<bool(const SubCodeInfo* const subCodeInfo)>& checkSubCodeInfo) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " - entering carrier=" << carrier << ", subCodes.size()=" << subCodes.size());
  FreeBaggageUtil::S5RecordsRetriever(checkSubCodeInfo, carrier, _trx).get(subCodes);
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " - exiting subCodes.size()=" << subCodes.size());
}

void
ChargesDataStrategy::retrieveCharges(BaggageTravel* baggageTravel,
                                     const BaggageTravelInfo& bagInfo,
                                     const std::vector<const SubCodeInfo*> subCodes,
                                     const CheckedPoint& furthestCheckedPoint,
                                     bool isUsDot,
                                     Diag852Collector* dc) const
{
  if (IS_DEBUG_ENABLED(logger))
  {
    std::ostringstream dbgMsg;
    dbgMsg << __LOG4CXX_FUNC__ << " - entering: ";
    dbgMsg << "subCodes=[" << subCodes.size() << "]{";
    for (auto subCode : subCodes)
      dbgMsg << "{" << subCode->serviceGroup() << "," << subCode->serviceSubGroup() << "},";

    LOG4CXX_DEBUG(logger, dbgMsg.str());
  }
  if (!subCodes.empty())
    baggageTravel->_chargeS5Available = true;

  ChargeVector& charges = baggageTravel->_chargeVector;

  for (const SubCodeInfo* subCodeInfo : subCodes)
    matchS7s(baggageTravel, bagInfo, subCodeInfo, furthestCheckedPoint, isUsDot, dc, charges);

  findLowestCharges(baggageTravel, bagInfo, charges, dc);
}

void
ChargesDataStrategy::matchS7s(BaggageTravel* baggageTravel,
                              const BaggageTravelInfo& bagInfo,
                              const SubCodeInfo* s5,
                              const CheckedPoint& furthestCheckedPoint,
                              bool isUsDot,
                              Diag852Collector* dc,
                              ChargeVector& charges) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  if (s5)
  {
    bool displayDiag =
        shouldDisplayDiagnostic(baggageTravel, bagInfo, s5->serviceSubTypeCode(), dc);

    if (displayDiag)
    {
      const bool defer = isChargesCarrierOverridden() ? false : baggageTravel->_defer;

      printS7ProcessingContext(
          baggageTravel, bagInfo, s5, isUsDot, dc, defer, isChargesCarrierOverridden());
    }
    displayDiag = displayDiag && shouldDisplayS7Diagnostic(dc);

    matchS7s(*baggageTravel, s5, furthestCheckedPoint, isUsDot, (displayDiag ? dc : nullptr), charges);
  }
}

void
ChargesDataStrategy::matchS7s(BaggageTravel& baggageTravel,
                              const SubCodeInfo* s5,
                              const CheckedPoint& furthestCheckedPoint,
                              bool isUsDot,
                              Diag852Collector* dc,
                              ChargeVector& charges) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  BaggageOcValidationAdapter::matchS7ChargesRecords(
      *s5, baggageTravel, furthestCheckedPoint, dc, charges);
}

bool
ChargesDataStrategy::matchOccurrence(const OptionalServicesInfo* s7, int32_t bagNo) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  return FreeBaggageUtil::matchOccurrence(*s7, bagNo);
}

void
ChargesDataStrategy::findLowestCharges(BaggageTravel* bt,
                                       const BaggageTravelInfo& bagInfo,
                                       ChargeVector& charges,
                                       Diag852Collector* dc) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);

  const uint32_t firstChargedPiece = FreeBaggageUtil::calcFirstChargedPiece(bt->_allowance);
  const uint32_t requestedPieces = _trx.getBaggagePolicy().getRequestedBagPieces();

  if (firstChargedPiece >= requestedPieces)
    return;

  bt->_charges.fill(nullptr);

  for (BaggageCharge* bc : charges)
  {
    for (uint32_t bagNo = firstChargedPiece; bagNo < requestedPieces; ++bagNo)
    {
      if (!bc->matchedBag(bagNo))
        continue;
      ChargesUtil::selectForPricing(bt->_charges[bagNo], *bc);
    }
  }
}

bool
ChargesDataStrategy::shouldDisplayDiagnostic(BaggageTravel* baggageTravel,
                                             const BaggageTravelInfo& bagInfo,
                                             const ServiceSubTypeCode& subTypeCode,
                                             const Diag852Collector* dc) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  return shouldDisplayChargesDiagnostic(baggageTravel, bagInfo, dc) &&
         (!shouldDisplayS7Diagnostic(dc) || (dc->subTypeCode() == subTypeCode));
}

bool
ChargesDataStrategy::shouldDisplayS7Diagnostic(const Diag852Collector* dc) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  return dc->diagType() == Diag852Collector::STACTIVE ||
      dc->diagType() == Diag852Collector::FAACTIVE;
}

bool
ChargesDataStrategy::shouldDisplayChargesDiagnostic(BaggageTravel* baggageTravel,
                                                    const BaggageTravelInfo& bagInfo,
                                                    const Diag852Collector* dc) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  return checkFareLineAndCheckedPortion(baggageTravel, bagInfo, dc) &&
         dc->hasDisplayChargesOption() && !dc->isDisplayCarryOnAllowanceOption() &&
         !dc->isDisplayCarryOnChargesOption() && !dc->isDisplayEmbargoesOption();
}

void
ChargesDataStrategy::printS7ProcessingContext(BaggageTravel* baggageTravel,
                                              const BaggageTravelInfo& bagInfo,
                                              const SubCodeInfo* s5,
                                              bool isUsDot,
                                              Diag852Collector* dc,
                                              bool defer,
                                              bool isCarrierOverride) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  const bool useMark = BaggageTextUtil::isMarketingCxrUsed(_trx, isUsDot, defer);
  dc->printS7ProcessingContext(
      _trx, baggageTravel, isUsDot, bagInfo.bagIndex() + 1, useMark, s5, defer, isCarrierOverride);
}

const OptionalServicesInfo*
ChargesDataStrategy::getAllowanceS7(const BaggageTravel* baggageTravel) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  return baggageTravel->_allowance ? baggageTravel->_allowance->optFee() : nullptr;
}

bool
ChargesDataStrategy::allSegmentsOnTheSameCarrier(const Itin& itinerary) const
{
  return ChargesUtil::isOnlineNonCodeShare(itinerary);
}

} // tse
