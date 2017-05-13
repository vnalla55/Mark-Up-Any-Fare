// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/CurrencyConversionFacade.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/GoverningCarrier.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AncRequest.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RepricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/FareTypeMatrix.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"
#include "Taxes/LegacyFacades/CurrencyServiceV2.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyFacades/RepricingServiceV2.h"
#include "Taxes/LegacyFacades/TaxPointUtil.h"
#include "Taxes/Common/LocRestrictionValidator3601.h"
#include "Taxes/Common/PartialTaxableFare.h"
#include "Taxes/Common/TaxUtility.h"

#include <boost/range/adaptor/reversed.hpp>

namespace tse
{

namespace
{
Logger
logger("atseintl.AtpcoTaxes.RepricingServiceV2");
}

namespace
{
class IsTravelSegInFareUsage
{
public:
  IsTravelSegInFareUsage(const FareUsage* fareUsage) : _fareUsage(fareUsage) {};

  bool operator()(const TravelSeg* ts) const
  {
    return (std::find(_fareUsage->travelSeg().begin(),
                      _fareUsage->travelSeg().end(),
                      ts) != _fareUsage->travelSeg().end());
  }

private:
  const FareUsage* _fareUsage;
};

bool
isValidForValidatingCarrier(const PricingTrx& trx,
                            const PaxTypeFare& ptf,
                            const CarrierCode& vcxr)
{
  if (!trx.isValidatingCxrGsaApplicable())
    return true;

  const std::vector<CarrierCode>& ptfVcxrList = ptf.validatingCarriers();
  if (!ptfVcxrList.empty() &&
      std::find(ptfVcxrList.begin(), ptfVcxrList.end(), vcxr) == ptfVcxrList.end())
  {
    return false;
  }

  return true;
}

const PaxTypeFare*
findPaxTypeFareForCarrier(const PricingTrx& trx,
                          const Itin* itin,
                          const FarePath* farePath,
                          const FareUsage* fareUsage,
                          const std::vector<TravelSeg*>& travelSegFMVec,
                          const CarrierCode& marketingOrOperatingCarrier,
                          const CarrierCode& validatingCarrier)
{
  const FareMarket* fareMarket = TrxUtil::getFareMarket(trx,
                                                        marketingOrOperatingCarrier,
                                                        travelSegFMVec,
                                                        fareUsage->paxTypeFare()->retrievalDate(),
                                                        itin);
  if (!fareMarket)
    return nullptr;

  const PaxTypeBucket* paxTypeCortege = fareMarket->paxTypeCortege(farePath->paxType());
  if (!paxTypeCortege)
    return nullptr;

  for (const auto& paxTypeFare : paxTypeCortege->paxTypeFare())
  {
    if (fareUsage->isInbound() && paxTypeFare->directionality() == FROM)
      continue;

    if (fareUsage->isOutbound() && paxTypeFare->directionality() == TO)
      continue;

    if (!paxTypeFare->isValid() || !isValidForValidatingCarrier(trx, *paxTypeFare, validatingCarrier))
      continue;

    // TODO: This condition is present in ATP, but not in Legacy; clarify with ATP when regression tests are resumed
    if (fareUsage->paxTypeFare()->fare()->fareClass() != paxTypeFare->fare()->fareClass())
      continue;

    return paxTypeFare;
  }

  return nullptr;
}

const char WPNCS_ON = 'T';
const char WPNCS_OFF = 'F';
const bool CHANGE_TRAVEL_DATE = true;
const bool DONT_CHANGE_TRAVEL_DATE = false;
const bool IGNORE_CABIN_CHECK = true;
const bool DONT_IGNORE_CABIN_CHECK = false;
const NationCode BAHAMAS_CODE("BS");
const char* DOMESTIC_FARE_BASIS = "SDOM";
}

RepricingServiceV2::RepricingServiceV2(PricingTrx& trx, const tax::V2TrxMappingDetails* v2Mapping)
  : _trx(trx), _v2Mapping(v2Mapping)
{
}

FarePath*
RepricingServiceV2::getFarePathFromItinId(tax::type::Index itinId) const
{
  if (_v2Mapping == nullptr)
    return nullptr;

  for (const auto mapping : _v2Mapping->_itinFarePathMapping)
  {
    if (std::get<3>(mapping) == itinId)
      return std::get<1>(mapping);
  }

  return nullptr;
}

void
RepricingServiceV2::convertCurrency(MoneyAmount& taxableAmount,
                                    FarePath& farePath,
                                    CurrencyCode& paymentCurrency) const
{
  DateTime dummyDateTime;
  CurrencyServiceV2 currencyService(_trx, dummyDateTime);
  tax::type::MoneyAmount convertedAmount = 0;

  try
  {
    convertedAmount = currencyService.convert(taxableAmount,
                                              farePath.baseFareCurrency(),
                                              farePath.calculationCurrency(),
                                              paymentCurrency,
                                              CurrencyConversionRequest::FARES,
                                              farePath.itin()->useInternationalRounding());
  }
  catch(const std::runtime_error& exception)
  {
    LOG4CXX_ERROR(logger, exception.what());
  }

  taxableAmount = tax::amountToDouble(convertedAmount);
}

bool
RepricingServiceV2::isUsOrBufferZone(const Loc* loc) const
{
  if (loc == nullptr)
    return false;
  return (taxUtil::checkLocCategory(*loc) != taxUtil::OTHER ||
          (taxUtil::soldInUS(_trx) && taxUtil::isBufferZone(*loc)));
}

bool
RepricingServiceV2::isGroundTransportation(const Itin& itin,
                                           const uint16_t startIndex,
                                           const uint16_t endIndex) const
{
  TravelSeg* travelSeg;
  for (uint16_t i = startIndex; i <= endIndex; ++i)
  {
    travelSeg = itin.travelSeg()[i];
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
    if (!airSeg || airSeg->equipmentType() == TGV || airSeg->equipmentType() == ICE ||
        airSeg->equipmentType() == BOAT || airSeg->equipmentType() == LMO ||
        airSeg->equipmentType() == BUS || airSeg->equipmentType() == TRAIN)
      continue;

    return false;
  }

  return true;
}

RepricingTrx*
RepricingServiceV2::getRepricingTrx(std::vector<TravelSeg*>& tvlSeg,
                                    FMDirection fmDirectionOverride) const
{
  RepricingTrx* rpTrx = nullptr;

  try
  {
    bool overrideTicketingAgent = (dynamic_cast<AncRequest*>(_trx.getRequest()) != nullptr);
    rpTrx = TrxUtil::reprice(_trx,
                             tvlSeg,
                             fmDirectionOverride,
                             false, // skipRuleValidation
                             nullptr, // carrierOverride
                             nullptr, // globalDirectionOverride
                             "", // extraPaxType
                             true, // retrieveFbrFares
                             true, // retrieveNegFares
                             'I', // wpncsFlagIndicator
                             0, // optionsFareFamilyType
                             false, // useCurrentDate
                             false, // privateFareCheck
                             overrideTicketingAgent); // overrideTicketingAgent
  }
  catch (const ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(logger,
                  "RepricingServiceV2: Exception during repricing with " << ex.code() << " - "
                                                                         << ex.message());
    return nullptr;
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "RepricingServiceV2: Unknown exception during repricing");
    return nullptr;
  }

  return rpTrx;
}

RepricingTrx*
RepricingServiceV2::getRepricingTrx(std::vector<TravelSeg*>& tvlSeg,
                                    Indicator wpncsFlagIndicator,
                                    const PaxTypeCode& extraPaxType,
                                    const bool privateFareCheck) const
{
  RepricingTrx* rpTrx = nullptr;

  try
  {
    bool overrideTicketingAgent = (dynamic_cast<AncRequest*>(_trx.getRequest()) != nullptr);
    rpTrx = TrxUtil::reprice(_trx,
                             tvlSeg,
                             FMDirection::UNKNOWN,
                             false, // skipRuleValidation
                             nullptr, // carrierOverride
                             nullptr, // globalDirectionOverride
                             extraPaxType,
                             false, // retrieveFbrFares
                             false, // retrieveNegFares
                             wpncsFlagIndicator,
                             ' ', // optionsFareFamilyType
                             false, // useCurrentDate
                             privateFareCheck,
                             overrideTicketingAgent);
  }
  catch (const ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(logger,
                  "RepricingServiceV2: Exception during repricing with " << ex.code() << " - "
                                                                         << ex.message());
    return nullptr;
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "RepricingServiceV2: Unknown exception during repricing");
    return nullptr;
  }

  return rpTrx;
}

bool
RepricingServiceV2::getNetAmountForLCT(FareUsage* fareUsage, MoneyAmount& netAmount) const
{
  bool netAmountKnown = false;

  if (!fareUsage)
    return false;

  if (!_trx.getRequest())
    return false;
  const Agent* agent = _trx.getRequest()->ticketingAgent();

  if (!agent)
    return false;

  if (!agent->agentTJR())
    return false;

  if (agent->agentTJR()->pricingApplTag3() != 'Y')
    return false;

  if (!fareUsage->paxTypeFare())
    return false;

  PaxTypeFare& ptFare = *(fareUsage->paxTypeFare());
  bool IsTypeLCT = ptFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF &&
                   (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE ||
                    ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
                    ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD);

  const NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;
  if (fareUsage->paxTypeFare()->isNegotiated())
  {
    negPaxTypeFare = fareUsage->paxTypeFare()->getNegRuleData();

    if (negPaxTypeFare && negPaxTypeFare->nucNetAmount() > 0)
    {
      netAmountKnown = true;
    }
  }

  if (IsTypeLCT && netAmountKnown)
  {
    netAmount = negPaxTypeFare->netAmount();
  }
  return netAmountKnown;
}

uint16_t
RepricingServiceV2::calculateMiles(FarePath& farePath,
                                   std::vector<TravelSeg*>& travelSegs,
                                   const Loc* origin,
                                   const Loc* destination) const
{
  // TODO: Consider possibility of  using MileageServiceV2 when
  // MileageGetterV2::getSingleGlobalDir() is implemented fully
  if (origin == nullptr || destination == nullptr)
    return 0;

  GlobalDirection gd = GlobalDirection::XX;
  DateTime travelDate = farePath.itin()->travelDate();
  GlobalDirectionFinderV2Adapter::getGlobalDirection(&_trx, travelDate, travelSegs, gd);
  if (gd == GlobalDirection::XX)
  {
    LOG4CXX_DEBUG(logger, "GlobalDirection Not Located");
    return 0;
  }

  PricingRequest* request = _trx.getRequest();

  if (request)
  {
    return LocUtil::getTPM(*origin, *destination, gd, request->ticketingDT(), _trx.dataHandle());
  }
  else
  {
    LOG4CXX_DEBUG(logger, "PricingRequest was NULL");
    return 0;
  }
}

bool
RepricingServiceV2::locateOpenJaw(FarePath& farePath) const
{
  if (_trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX || _trx.getTrxType() == PricingTrx::IS_TRX)
    return false;

  Itin* itin = farePath.itin();
  if (itin == nullptr)
    return false;

  if (itin->travelSeg().empty())
    return false;

  TravelSeg* travelSegBack = itin->travelSeg().back();
  TravelSeg* travelSegFront = itin->travelSeg().front();

  if (travelSegBack == nullptr || travelSegFront == nullptr)
    return false;

  if (travelSegFront->origin() == nullptr || travelSegBack->destination() == nullptr)
    return false;

  if (!LocUtil::isUS(*(travelSegFront->origin())) ||
      LocUtil::isUSTerritoryOnly(*(travelSegFront->origin())))
    return false;

  if (!LocUtil::isUS(*(travelSegBack->destination())) ||
      LocUtil::isUSTerritoryOnly(*(travelSegBack->destination())))
    return false;

  if (travelSegFront->origin()->loc() == travelSegBack->destination()->loc())
    return false;

  bool intPoint = false;
  for(TravelSeg* travelSeg : itin->travelSeg())
  {
    if (!isUsOrBufferZone(travelSeg->origin()) ||
        !isUsOrBufferZone(travelSeg->destination()))
      intPoint = true;

    if ((travelSeg->origin()->area() != IATA_AREA1) ||
        (travelSeg->origin()->subarea() == IATA_SUB_AREA_14()))
    {
      return false;
    }

    if ((travelSeg->destination()->area() != IATA_AREA1) ||
        (travelSeg->destination()->subarea() == IATA_SUB_AREA_14()))
    {
      return false;
    }
  }
  if (!intPoint)
    return false;

  uint16_t startToEndMiles = calculateMiles(
      farePath, itin->travelSeg(), travelSegFront->origin(), travelSegBack->destination());

  LocRestrictionValidator3601 locRestrictionValidator;
  if ((!locRestrictionValidator.fareBreaksFound()))
    locRestrictionValidator.findFareBreaks(farePath);
  locRestrictionValidator.findFarthestPoint(_trx, *(farePath.itin()), 0);
  TravelSeg* travelSegFar = itin->travelSeg()[locRestrictionValidator.getFarthestSegIndex()];

  uint16_t sideMiles = calculateMiles(
      farePath, itin->travelSeg(), travelSegFront->origin(), travelSegFar->destination());

  if (sideMiles < startToEndMiles && sideMiles != 0)
    return true;

  sideMiles = calculateMiles(
      farePath, itin->travelSeg(), travelSegFar->destination(), travelSegBack->destination());

  if (sideMiles < startToEndMiles && sideMiles != 0)
    return true;

  return false;
}

bool
RepricingServiceV2::subtractPartialFare(FarePath& farePath,
                                        const MoneyAmount& totalFareAmount,
                                        MoneyAmount& taxableAmount,
                                        FareUsage& fareUsage,
                                        std::vector<TravelSeg*>& travelSegsEx,
                                        CurrencyCode& paymentCurrency) const
{
  if (travelSegsEx.size() == 0)
    return true;
  PartialTaxableFare partialFareLocator;
  partialFareLocator.fareUsage() = &fareUsage;
  partialFareLocator.thruTotalFare() = totalFareAmount;
  partialFareLocator.taxablePartialFare() = taxableAmount;
  partialFareLocator.paymentCurrency() = paymentCurrency;
  std::set<CarrierCode> govCxrs;
  GoverningCarrier govCxrSel(&_trx);
  if(_trx.isIataFareSelectionApplicable())
  {
    TravelSeg* dummy = nullptr;
    govCxrSel.selectFirstCrossingGovCxr(fareUsage.travelSeg(), govCxrs, FMDirection::UNKNOWN, dummy);
  }
  else
    govCxrSel.getGoverningCarrier(fareUsage.travelSeg(), govCxrs);
  const FareMarket* fareMarket = TrxUtil::getFareMarket(
      _trx, *(govCxrs.begin()), travelSegsEx, fareUsage.paxTypeFare()->retrievalDate());
  if (!fareMarket)
  {
    if (!TrxUtil::isPricingTaxRequest(&_trx))
    {
      RepricingTrx* reTrx =
          getRepricingTrx(travelSegsEx,
                          fareUsage.paxTypeFare()->directionality() == TO ? FMDirection::INBOUND
                                                                          : FMDirection::UNKNOWN);
      if (!reTrx)
      {
        LOG4CXX_WARN(logger, "RepricingServiceV2: no repricing trx");
        return false;
      }
      LOG4CXX_DEBUG(logger, "RepricingServiceV2: repricing trx is ready");
      fareMarket = TrxUtil::getFareMarket(
          *reTrx, *(govCxrs.begin()), travelSegsEx, fareUsage.paxTypeFare()->retrievalDate());
      if (!fareMarket)
      {
        LOG4CXX_DEBUG(logger, "RepricingServiceV2: repricing - TrxUtil::getFareMarket failed");
        return false;
      }

      if (!partialFareLocator.appliedFare(*reTrx, farePath, fareMarket))
      {
        LOG4CXX_DEBUG(logger,
                      "RepricingServiceV2: repricing - partialFareLocator.appliedFare failed");
        return false;
      }

      LOG4CXX_DEBUG(logger, "RepricingServiceV2: applied repriced fare");
    }
    else
      return false;
  }
  else
  {
    if (!partialFareLocator.appliedFare(_trx, farePath, fareMarket))
      return false;
  }
  taxableAmount = partialFareLocator.taxablePartialFare();
  return true;
}

MoneyAmount
RepricingServiceV2::calculatePartAmount(FarePath& farePath,
                                        uint16_t startIndex,
                                        uint16_t endIndex,
                                        uint16_t fareBreakEnd,
                                        FareUsage& fareUsage) const
{
  CurrencyCode paymentCurrency = _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  if (!_trx.getOptions()->currencyOverride().empty())
  {
    paymentCurrency = _trx.getOptions()->currencyOverride();
  }
  bool openJaw = (startIndex == 0 && locateOpenJaw(farePath) && taxUtil::soldInUS(_trx));

  if (!farePath.itin())
    return 0;
  Itin& itin = *(farePath.itin());
  uint16_t fareBreakStart =
      static_cast<uint16_t>(itin.segmentOrder(fareUsage.travelSeg().front()) - 1);
  MoneyAmount totalFareAmount = fareUsage.totalFareAmount();
  convertCurrency(totalFareAmount, farePath, paymentCurrency);
  if (itin.travelSeg().empty())
    return 0;

  TravelSeg* travelSegStart = itin.travelSeg()[startIndex];
  TravelSeg* travelSegEnd = itin.travelSeg()[endIndex];
  if (travelSegStart == nullptr || travelSegEnd == nullptr)
    return 0;

  if (startIndex == fareBreakStart && endIndex == fareBreakEnd &&
      isUsOrBufferZone(travelSegStart->origin()) && isUsOrBufferZone(travelSegEnd->destination()))
  {
    getNetAmountForLCT(&fareUsage, totalFareAmount);
    return totalFareAmount;
  }
  else
  {
    MoneyAmount taxableAmount = totalFareAmount;
    const Loc* loc1 = travelSegStart->origin();
    const Loc* loc2 = travelSegEnd->destination();
    if ((isUsOrBufferZone(loc1) && isUsOrBufferZone(loc2)) || openJaw)
    {
      std::vector<TravelSeg*> travelSegsEx;
      travelSegsEx.insert(travelSegsEx.begin(),
                          itin.travelSeg().begin() + fareBreakStart,
                          itin.travelSeg().begin() + startIndex);
      if (subtractPartialFare(
              farePath, totalFareAmount, taxableAmount, fareUsage, travelSegsEx, paymentCurrency))
      {
        travelSegsEx.clear();
        travelSegsEx.insert(travelSegsEx.begin(),
                            itin.travelSeg().begin() + endIndex + 1,
                            itin.travelSeg().begin() + fareBreakEnd + 1);
        if (subtractPartialFare(
                farePath, totalFareAmount, taxableAmount, fareUsage, travelSegsEx, paymentCurrency))
        {
          return taxableAmount;
        }
      }
    }
    if (!openJaw)
    {
      if (!isUsOrBufferZone(loc1))
      {
        for(const Loc* hiddenStop : travelSegStart->hiddenStops())
        {
          if (isUsOrBufferZone(hiddenStop))
          {
            loc1 = hiddenStop;
            break;
          }
        }
      }
      if (!isUsOrBufferZone(loc2))
      {
        for (const Loc* hiddenStop : boost::adaptors::reverse(travelSegEnd->hiddenStops()))
        {
          if (isUsOrBufferZone(hiddenStop))
          {
            loc2 = hiddenStop;
            break;
          }
        }
      }
    }
    std::vector<TravelSeg*> travelSegs;
    travelSegs.insert(travelSegs.begin(),
                      itin.travelSeg().begin() + fareBreakStart,
                      itin.travelSeg().begin() + fareBreakEnd + 1);
    if (travelSegs.front() == nullptr || travelSegs.back() == nullptr)
      return 0;

    const Loc* locf1 = travelSegs.front()->origin();
    const Loc* locf2 = travelSegs.back()->destination();
    uint16_t localMiles = calculateMiles(farePath, travelSegs, loc1, loc2);
    if (localMiles > 0)
    {
      uint16_t thruMiles = calculateMiles(farePath, travelSegs, locf1, locf2);
      if (thruMiles > 0)
      {
        if (localMiles < thruMiles)
        {
          getNetAmountForLCT(&fareUsage, totalFareAmount);
          taxableAmount = totalFareAmount * localMiles / thruMiles;
        }
        return taxableAmount;
      }
    }
    return totalFareAmount;
  }
}

PaxTypeCode
RepricingServiceV2::getMappedPaxTypeCode(const PaxTypeCode& paxTypeCode,
                                         const PaxTypeCode& defaultPaxTypeCode) const
{
  if (paxTypeCode == JCB || paxTypeCode == JNN)
    return ADULT;

  return defaultPaxTypeCode;
}

const std::vector<PaxTypeFare*>*
RepricingServiceV2::locatePaxTypeFare(const FareMarket* fareMarketReTrx,
                                      const PaxTypeCode& paxTypeCode) const
{
  for (const PaxTypeBucket& paxTypeCortege : fareMarketReTrx->paxTypeCortege())
  {
    if (paxTypeCortege.requestedPaxType()->paxType() == paxTypeCode)
      return &(paxTypeCortege.paxTypeFare());
  }

  return nullptr;
}

void
RepricingServiceV2::getBkgCodeReBooked(const FareUsage* fareUsage,
                                       TravelSeg* travelSegRef,
                                       TravelSeg* travelSegClone) const
{
  for (uint16_t iTravelSeg = 0; iTravelSeg < fareUsage->travelSeg().size(); iTravelSeg++)
  {
    if (fareUsage->travelSeg()[iTravelSeg] == travelSegRef && !fareUsage->segmentStatus().empty() &&
        !fareUsage->segmentStatus()[iTravelSeg]._bkgCodeReBook.empty() &&
        fareUsage->segmentStatus()[iTravelSeg]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
    {
      travelSegClone->setBookingCode(fareUsage->segmentStatus()[iTravelSeg]._bkgCodeReBook);
      travelSegClone->bookedCabin() = fareUsage->segmentStatus()[iTravelSeg]._reBookCabin;
      break;
    }
  }
}

bool
RepricingServiceV2::isTravelSegWhollyDomestic(const TravelSeg* travelSeg,
                                              const NationCode& nation) const
{
  if (!dynamic_cast<const AirSeg*>(travelSeg))
    return false;

  return
      (travelSeg->origin()->nation() == nation) && (travelSeg->destination()->nation() == nation);
}

const FareUsage*
RepricingServiceV2::locateFareUsage(const FarePath& farePath, const TravelSeg* travelSeg) const
{
  for(PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for(FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      for(TravelSeg* fareUsageTravelSeg : fareUsage->travelSeg())
      {
        if (farePath.itin()->segmentOrder(fareUsageTravelSeg) ==
            farePath.itin()->segmentOrder(travelSeg))
          return fareUsage;
      }
    }
  }

  return nullptr;
}

void
RepricingServiceV2::setDepartureAndArrivalDates(AirSeg* ts) const
{
  int64_t diff = DateTime::diffTime(ts->arrivalDT(), ts->departureDT());

  ts->departureDT() = _trx.ticketingDate();
  ts->arrivalDT() = ts->departureDT().addSeconds(diff);
  ts->pssDepartureDate() = ts->departureDT().dateToSqlString();
  ts->pssArrivalDate() = ts->arrivalDT().dateToSqlString();

  ts->earliestDepartureDT() = ts->departureDT();
  ts->latestDepartureDT() = ts->departureDT();
  ts->earliestArrivalDT() = ts->arrivalDT();
  ts->latestArrivalDT() = ts->arrivalDT();
}

void
RepricingServiceV2::fillSegmentForRepricing(const FareUsage& fareUsage,
                                            TravelSeg* travelSeg,
                                            bool changeDate,
                                            std::vector<TravelSeg*>& segs) const
{
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
  AirSeg* ts = _trx.dataHandle().create<AirSeg>();
  *ts = *airSeg;
  getBkgCodeReBooked(&fareUsage, travelSeg, ts);

  if (changeDate)
    setDepartureAndArrivalDates(ts);

  segs.clear();
  segs.push_back(ts);
}

bool
RepricingServiceV2::getAmountFromRepricedFare(const PaxTypeCode& paxTypeCode,
                                              RepricingTrx& retrx,
                                              MoneyAmount& taxableFare,
                                              const CabinType& orgTravelSegCabin,
                                              const DateTime& travelDate,
                                              bool bIgnoreCabinCheck) const
{
  const std::vector<PaxTypeFare*>* paxTypeFare = nullptr;

  paxTypeFare =
      locatePaxTypeFare(retrx.fareMarket()[0], getMappedPaxTypeCode(paxTypeCode, paxTypeCode));
  if (!paxTypeFare)
    return false;

  for(PaxTypeFare* fare : *paxTypeFare)
  {
    const FareTypeMatrix* fareTypeMatrix =
        retrx.dataHandle().getFareTypeMatrix(fare->fcaFareType(), travelDate);

    if ((fareTypeMatrix != nullptr &&
         (!fareTypeMatrix->fareTypeDesig().isFTDAddon() &&
          fareTypeMatrix->fareTypeDisplay() == 'N' && fareTypeMatrix->fareTypeAppl() == 'N' &&
          (fareTypeMatrix->restrInd() == 'R' || fareTypeMatrix->restrInd() == 'U'))) &&
        (bIgnoreCabinCheck || fare->cabin() == orgTravelSegCabin) &&
        fare->isForeignDomestic() && fare->directionality() == FROM &&
        fare->owrt() != ROUND_TRIP_MAYNOT_BE_HALVED && fare->isNormal() &&
        (fare->isValidForPricing() ||
         (fare->isSoftPassed() && fare->isRoutingValid() &&
          (fare->bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED) ||
           !fare->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL)))))
    {
      taxableFare = fare->totalFareAmount();

      return true;
    }
  }

  return false;
}

bool
RepricingServiceV2::findRepricedFare(const PaxTypeCode& paxTypeCode,
                                     const FareUsage& fareUsage,
                                     TravelSeg* travelSeg,
                                     bool changeDate,
                                     Indicator wpncsFlagIndicator,
                                     MoneyAmount& taxableFare,
                                     const DateTime& travelDate,
                                     bool ignoreCabinCheck,
                                     bool privateFareCheck /* = false*/) const
{
  RepricingTrx* retrx = nullptr;

  std::vector<TravelSeg*> segs;
  fillSegmentForRepricing(fareUsage, travelSeg, changeDate, segs);
  retrx = getRepricingTrx(
      segs, wpncsFlagIndicator, getMappedPaxTypeCode(paxTypeCode), privateFareCheck);

  if (retrx)
  {
    if (getAmountFromRepricedFare(paxTypeCode,
                                  *retrx,
                                  taxableFare,
                                  fareUsage.paxTypeFare()->cabin(),
                                  travelDate,
                                  ignoreCabinCheck))
      return true;
  }

  return false;
}

void
RepricingServiceV2::applyPartialAmount(FarePath& farePath,
                                       MoneyAmount& taxableFare,
                                       CurrencyCode& paymentCurrency,
                                       TravelSeg* travelSeg) const
{
  convertCurrency(taxableFare, farePath, paymentCurrency);

  Percent discPercent = _trx.getRequest()->discountPercentage(travelSeg->segmentOrder());
  if (discPercent >= 0 && discPercent <= 100)
  {
    taxableFare *= (1.0 - discPercent / 100.0);
  }
}

tax::type::MoneyAmount
RepricingServiceV2::getSimilarDomesticFare(const tax::type::Index& taxPointBegin,
                                           const tax::type::Index& taxPointEnd,
                                           const tax::type::Index& itinId,
                                           bool& fareFound) const
{
  fareFound = false;

  // Convert data to V2 form
  FarePath* farePath = getFarePathFromItinId(itinId);
  if (farePath == nullptr)
    return 0;

  Itin* itin = farePath->itin();
  if (itin == nullptr)
    return 0;

  const TaxResponse* taxResponse = nullptr;
  for(TaxResponse* response : itin->getTaxResponses())
    if (response->farePath() == farePath)
      taxResponse = response;

  if (taxResponse == nullptr)
    return 0;

  if (itin->travelSeg().empty())
    return 0;

  uint16_t travelSegStartIndex = 0;
  uint16_t travelSegEndIndex = 0;
  TaxPointUtil::setTravelSegIndices(taxPointBegin, taxPointEnd, *itin, travelSegStartIndex, travelSegEndIndex);

  TravelSeg* travelSeg = itin->travelSeg()[travelSegStartIndex];
  if (travelSeg == nullptr)
    return 0;

  if (TrxUtil::isPricingTaxRequest(&_trx))
    return 0;

  const FareUsage* fareUsage = locateFareUsage(*farePath, travelSeg);
  if (fareUsage == nullptr)
    return 0;

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
  if (!airSeg)
    return 0;

  if (!fareUsage->paxTypeFare()->isForeignDomestic())
  {
    std::vector<TravelSeg*> travelSegFMVec{travelSeg};

    // Search for marketing carrier fares
    const PaxTypeFare* paxTypeFare = findPaxTypeFareForCarrier(_trx,
                                                               itin,
                                                               farePath,
                                                               fareUsage,
                                                               travelSegFMVec,
                                                               airSeg->marketingCarrierCode(),
                                                               taxResponse->validatingCarrier());

    if (paxTypeFare == nullptr && airSeg->marketingCarrierCode() != airSeg->operatingCarrierCode())
    {
      // If no fares for marketing carrier, try operating carrier
      paxTypeFare = findPaxTypeFareForCarrier(_trx,
                                              itin,
                                              farePath,
                                              fareUsage,
                                              travelSegFMVec,
                                              airSeg->operatingCarrierCode(),
                                              taxResponse->validatingCarrier());

    }

    if (paxTypeFare == nullptr)
      return 0;

    MoneyAmount totalFareAmount = paxTypeFare->totalFareAmount();
    CurrencyCode paymentCurrency = _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
    if (!_trx.getOptions()->currencyOverride().empty())
    {
      paymentCurrency = _trx.getOptions()->currencyOverride();
    }

    convertCurrency(totalFareAmount, *farePath, paymentCurrency);
    fareFound = true;
    return tax::doubleToAmount(totalFareAmount);
  }

  return 0;
}

tax::type::MoneyAmount
RepricingServiceV2::getBahamasSDOMFare(const tax::type::Index& taxPointBegin,
                                       const tax::type::Index& taxPointEnd,
                                       const tax::type::Index& itinId) const
{
  // Convert data to V2 form
  FarePath* farePath = getFarePathFromItinId(itinId);
  if (farePath == nullptr)
  {
    throw std::logic_error("Can't find FarePath for calculation!");
  }
  Itin* itin = farePath->itin();
  if (itin == nullptr)
  {
    throw std::logic_error("Can't find Itin for calculation");
  }

  const TaxResponse* taxResponse = nullptr;
  for(TaxResponse* response : itin->getTaxResponses())
    if (response->farePath() == farePath)
      taxResponse = response;

  CurrencyCode paymentCurrency = _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  if (!_trx.getOptions()->currencyOverride().empty())
  {
    paymentCurrency = _trx.getOptions()->currencyOverride();
  }

  const PaxTypeCode paxTypeCode = (taxResponse != nullptr) ? taxResponse->paxTypeCode() : "";

  uint16_t travelSegStartIndex = 0;
  uint16_t travelSegEndIndex = 0;
  TaxPointUtil::setTravelSegIndices(taxPointBegin, taxPointEnd, *itin, travelSegStartIndex, travelSegEndIndex);

  for (uint16_t segIndex = travelSegStartIndex; segIndex <= travelSegEndIndex; ++segIndex)
  {
    TravelSeg* travelSeg = itin->travelSeg()[segIndex];

    const FareUsage* fareUsage = locateFareUsage(*farePath, travelSeg);
    if (fareUsage)
    {
      bool isDomesticFare = true;
      for(const TravelSeg* seg : fareUsage->travelSeg())
      {
        if (!seg->isAir())
          continue;

        if (seg->origin()->nation() != BAHAMAS_CODE ||
            seg->destination()->nation() != BAHAMAS_CODE)
          isDomesticFare = false;
      }

      MoneyAmount sdomFareAmount = 0;
      const PaxTypeFare* sdomFare = getDomesticBahamasFare(*travelSeg, fareUsage, paxTypeCode);
      if (sdomFare != nullptr)
      {
        sdomFareAmount = taxUtil::convertCurrency(_trx,
                                                  sdomFare->totalFareAmount(),
                                                  paymentCurrency,
                                                  farePath->calculationCurrency(),
                                                  farePath->baseFareCurrency(),
                                                  CurrencyConversionRequest::FARES,
                                                  itin->useInternationalRounding());
      }

      if (isDomesticFare)
      {
        MoneyAmount fareAmount = taxUtil::convertCurrency(_trx,
                                                          fareUsage->totalFareAmount(),
                                                          paymentCurrency,
                                                          farePath->calculationCurrency(),
                                                          farePath->baseFareCurrency(),
                                                          CurrencyConversionRequest::FARES,
                                                          itin->useInternationalRounding());

        return tax::doubleToAmount(std::max(fareAmount, sdomFareAmount));
      }
      else
      {
        return tax::doubleToAmount(sdomFareAmount);
      }
    }
  }

  return tax::doubleToAmount(0);
}

const PaxTypeFare*
RepricingServiceV2::getDomesticBahamasFare(TravelSeg& seg,
                                           const FareUsage* fareUsage,
                                           const PaxTypeCode& paxTypeCode) const
{
  std::vector<FareMarket*> retFareMarket;
  PricingTrx* trxTmp = getFareMarkets(fareUsage->paxTypeFare()->retrievalDate(),
                                      seg,
                                      paxTypeCode,
                                      retFareMarket);

  if (retFareMarket.empty())
  {
    return nullptr;
  }

  const PaxTypeFare* domesticFare = nullptr;
  for (const FareMarket* fareMarket: retFareMarket)
  {
    const std::vector<PaxTypeFare*>* paxTypeFare = taxUtil::locatePaxTypeFare(fareMarket, paxTypeCode);
    if (!paxTypeFare)
    {
      continue;
    }

    for (const PaxTypeFare* fare: *paxTypeFare)
    {
      std::string fareBasis = fare->createFareBasis(trxTmp, false);

      if (!fare->isValid() && fareBasis != DOMESTIC_FARE_BASIS)
        continue;

      Directionality dir = fareUsage->isInbound() ? TO : FROM;

      if (fareUsage->isInbound() && fareUsage->paxTypeFare()->directionality() == FROM)
        dir = FROM;

      if (fare->directionality() != dir)
      {
        continue;
      }

      if (fareBasis == DOMESTIC_FARE_BASIS)
      {
        domesticFare = fare;
        break;
      }

      if (domesticFare)
      {
        continue;
      }

      if (!fare->cabin().isEconomyClass())
      {
        continue;
      }

      if (!domesticFare)
      {
        domesticFare = fare;
      }

    } //for (const PaxTypeFare* fare: *paxTypeFare)

    if (domesticFare)
    {
      break;
    }

  } //for (FareMarket* fareMarket: retFareMarket)

  return domesticFare;
}

PricingTrx*
RepricingServiceV2::getFareMarkets(const DateTime& dateFare,
                                   TravelSeg& seg,
                                   const PaxTypeCode& paxTypeCode,
                                   std::vector<FareMarket*>& vFareMarkets) const
{
  PricingTrx* trxTmp = &_trx;

  std::vector<TravelSeg*> tvlSeg = {&seg};
  TrxUtil::getFareMarket(_trx, tvlSeg, dateFare, vFareMarkets);
  if (vFareMarkets.empty())
  {
    if (!TrxUtil::isPricingTaxRequest(&_trx))
    {
      try
      {
        trxTmp = TrxUtil::reprice(_trx,
                                  tvlSeg,
                                  FMDirection::UNKNOWN,
                                  false, // skipRuleValidation
                                  nullptr, // carrierOverride
                                  nullptr, // globalDirectionOverride
                                  paxTypeCode, // extraPaxType
                                  false, // retrieveFbrFares
                                  false, // retrieveNegFares
                                  WPNCS_OFF,
                                  ' ', // optionsFareFamilyType
                                  false, // useCurrentDate
                                  false); //private fare
      }
      catch (...)
      {
      }

      if (!trxTmp)
      {
        return trxTmp;
      }

      TrxUtil::getFareMarket( *trxTmp, tvlSeg, dateFare, vFareMarkets);
    }
  }

  return nullptr;
}

tax::type::MoneyAmount
RepricingServiceV2::getFareFromFareList(const tax::type::Index& taxPointBegin,
                                        const tax::type::Index& taxPointEnd,
                                        const tax::type::Index& itinId) const
{
  // Convert data to V2 form
  FarePath* farePath = getFarePathFromItinId(itinId);
  if (farePath == nullptr)
  {
    throw std::logic_error("Can't find FarePath for calculation!");
  }
  Itin* itin = farePath->itin();
  if (itin == nullptr)
  {
    throw std::logic_error("Can't find Itin for calculation");
  }

  uint16_t travelSegStartIndex = 0;
  uint16_t travelSegEndIndex = 0;
  TaxPointUtil::setTravelSegIndices(taxPointBegin, taxPointEnd, *itin, travelSegStartIndex, travelSegEndIndex);

  // Calculate
  TravelSeg* travelSeg = itin->travelSeg()[travelSegStartIndex];
  if (travelSeg == nullptr)
    return 0;

  const NationCode& nation = itin->travelSeg()[travelSegStartIndex]->origin()->nation();
  std::list<TravelSeg*> domesticTravelSegList;
  for (uint16_t travelSegIndex = travelSegStartIndex;
       travelSegIndex <= travelSegEndIndex;
       travelSegIndex++)
  {
    if (isTravelSegWhollyDomestic(itin->travelSeg()[travelSegIndex], nation))
    {
      domesticTravelSegList.push_back(itin->travelSeg()[travelSegIndex]);
    }
  }

  if (domesticTravelSegList.empty())
  {
    return 0;
  }

  CurrencyCode paymentCurrency = _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  if (!_trx.getOptions()->currencyOverride().empty())
  {
    paymentCurrency = _trx.getOptions()->currencyOverride();
  }

  MoneyAmount totalTaxableFare = 0;

  for (std::list<TravelSeg*>::iterator domesticTravelSegIter = domesticTravelSegList.begin();
       domesticTravelSegIter != domesticTravelSegList.end() && !domesticTravelSegList.empty();)
  {
    const FareUsage* fareUsage = locateFareUsage(*farePath, *domesticTravelSegIter);

    if (fareUsage)
    {
      MoneyAmount taxableFare = 0;
      if (fareUsage->paxTypeFare()->isForeignDomestic())
      {
        taxableFare = fareUsage->paxTypeFare()->totalFareAmount();
        applyPartialAmount(*farePath, taxableFare, paymentCurrency, *domesticTravelSegIter);
        totalTaxableFare += taxableFare;

        // drop other segments with this fare
        IsTravelSegInFareUsage isInTravelSeg(fareUsage);

        domesticTravelSegList.remove_if(isInTravelSeg);
        domesticTravelSegIter = domesticTravelSegList.begin();
      }
      else if ((*domesticTravelSegIter)->isAir())
      {
        // try to reprice:
        if (!TrxUtil::isPricingTaxRequest(&_trx) && !domesticTravelSegList.empty())
        {
          // filling taxableFare
          bool fareFound = findRepricedFare(farePath->paxType()->paxType(),
                                            *fareUsage,
                                            *domesticTravelSegIter,
                                            DONT_CHANGE_TRAVEL_DATE,
                                            WPNCS_OFF,
                                            taxableFare,
                                            itin->travelDate(),
                                            IGNORE_CABIN_CHECK);

          if (!fareFound) // WPNCS
          {
            fareFound = findRepricedFare(farePath->paxType()->paxType(),
                                         *fareUsage,
                                         *domesticTravelSegIter,
                                         DONT_CHANGE_TRAVEL_DATE,
                                         WPNCS_ON,
                                         taxableFare,
                                         itin->travelDate(),
                                         DONT_IGNORE_CABIN_CHECK);
          }

          if (!fareFound) // change travel date
          {
            fareFound = findRepricedFare(farePath->paxType()->paxType(),
                                         *fareUsage,
                                         *domesticTravelSegIter,
                                         CHANGE_TRAVEL_DATE,
                                         WPNCS_OFF,
                                         taxableFare,
                                         itin->travelDate(),
                                         IGNORE_CABIN_CHECK);
          }

          if (!fareFound) // change date WPNCS
          {
            fareFound = findRepricedFare(farePath->paxType()->paxType(),
                                         *fareUsage,
                                         *domesticTravelSegIter,
                                         CHANGE_TRAVEL_DATE,
                                         WPNCS_ON,
                                         taxableFare,
                                         itin->travelDate(),
                                         DONT_IGNORE_CABIN_CHECK);
          }

          if ((!fareFound) && (_trx.getOptions()->isPrivateFares())) // PV
          {
            fareFound = findRepricedFare(farePath->paxType()->paxType(),
                                         *fareUsage,
                                         *domesticTravelSegIter,
                                         DONT_CHANGE_TRAVEL_DATE,
                                         WPNCS_OFF,
                                         taxableFare,
                                         itin->travelDate(),
                                         IGNORE_CABIN_CHECK,
                                         true);
          }

          if (!fareFound && (_trx.getOptions()->isPrivateFares())) // PV + WPNCS
          {
            fareFound = findRepricedFare(farePath->paxType()->paxType(),
                                         *fareUsage,
                                         *domesticTravelSegIter,
                                         DONT_CHANGE_TRAVEL_DATE,
                                         WPNCS_ON,
                                         taxableFare,
                                         itin->travelDate(),
                                         DONT_IGNORE_CABIN_CHECK,
                                         true);

          }
        }

        applyPartialAmount(*farePath, taxableFare, paymentCurrency, *domesticTravelSegIter);
        totalTaxableFare += taxableFare;

        domesticTravelSegList.erase(domesticTravelSegIter);
        domesticTravelSegIter = domesticTravelSegList.begin();
      }
    }
    else // !fareUsage: try to reprice next segment
    {
      ++domesticTravelSegIter;
    }
  }

  return tax::doubleToAmount(totalTaxableFare);
}

tax::type::MoneyAmount
RepricingServiceV2::getFareUsingUSDeductMethod(const tax::type::Index& taxPointBegin,
                                               const tax::type::Index& taxPointEnd,
                                               const tax::type::Index& itinId) const
{
  // Convert data to V2 form
  FarePath* farePath = getFarePathFromItinId(itinId);
  if (farePath == nullptr)
  {
    throw std::logic_error("Can't find FarePath for calculation!");
  }
  std::map<uint16_t, FareUsage*> fareBreaks;
  taxUtil::findFareBreaks(fareBreaks, *farePath);
  Itin* itin = farePath->itin();
  if (itin == nullptr)
  {
    throw std::logic_error("Can't find Itin for calculation");
  }

  uint16_t travelSegStartIndex = 0;
  uint16_t travelSegEndIndex = 0;
  TaxPointUtil::setTravelSegIndices(taxPointBegin, taxPointEnd, *itin, travelSegStartIndex, travelSegEndIndex);

  // Calculate fare amount
  MoneyAmount taxableFare = 0;
  MoneyAmount taxableAmount = 0;
  uint16_t startIndex;
  uint16_t endIndex;
  for (startIndex = travelSegStartIndex; startIndex <= travelSegEndIndex; startIndex = endIndex + 1)
  {
    for (endIndex = startIndex;
         endIndex < travelSegEndIndex && fareBreaks.find(endIndex) == fareBreaks.end();
         ++endIndex)
      ;

    std::map<uint16_t, FareUsage*>::const_iterator fbI;
    uint16_t fareBreakIndex;
    for (fareBreakIndex = endIndex; fareBreakIndex < itin->travelSeg().size(); ++fareBreakIndex)
    {
      fbI = fareBreaks.find(fareBreakIndex);
      if (fbI != fareBreaks.end())
      {
        FareUsage* fareUsage = fbI->second;
        if (itin->segmentOrder(fareUsage->travelSeg().front()) - 1 <= startIndex)
          break;

        fbI = fareBreaks.end();
      }
    }

    if (fbI != fareBreaks.end())
    {
      if (fbI->second)
      {
        taxableAmount =
            calculatePartAmount(*farePath, startIndex, endIndex, fareBreakIndex, *(fbI->second));
      }
    }
    else
    {
      taxableAmount = farePath->getTotalNUCAmount();
    }

    if (isGroundTransportation(*itin, startIndex, endIndex))
      taxableAmount = 0;

    taxableFare += taxableAmount;
  }

  return tax::doubleToAmount(taxableFare);
}
}
