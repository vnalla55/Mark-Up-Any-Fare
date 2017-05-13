// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2009
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

#include "Taxes/LegacyTaxes/TaxJP1_00.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/FareTypeDesignator.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/AdjustTax.h"
#include "Taxes/LegacyTaxes/TaxApply.h"
#include "Taxes/LegacyTaxes/TaxCodeValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

#include <algorithm>
#include <functional>
#include <list>

namespace tse
{
bool
TripTypesValidatorJP1::validateFromTo(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      uint16_t& startIndex,
                                      uint16_t& endIndex)
{
  bool status =
      TripTypesValidator::validateFromTo(trx, taxResponse, taxCodeReg, startIndex, endIndex);

  Itin* itin = taxResponse.farePath()->itin();

  if (taxCodeReg.travelType() == TaxCodeValidator::INTERNATIONAL)
  {
    status = status ? !isReturnToPrecedingPoint(taxCodeReg, itin, startIndex) : status;

    if (status && startIndex<itin->travelSeg().size()-2)
      status = status ? !isReturnToPrecedingPoint(taxCodeReg, itin, startIndex+1) : status;

    if (haveOriginAndTerminationInJp(itin))
    {
      uint16_t prevIndex = startIndex - 1;

      if (!status || !haveOnlyJpSegs(itin, startIndex, endIndex) ||
        !isTerminationPt(itin, prevIndex) || !isTerminationPt(itin, endIndex))
      {
        TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic819);
        status = false;
      }
    }
  }

  return status;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function uint16_t TripTypesValidatorJP1::findTaxStopOverIndex
//
// Description:  This function will find the smallest index of the stopover
//               segment, starting from the startIndex.
//               It considers the transit time (set in tax), open segments
//               and stopover/connection forced by a user.
//               It doesn't consider the mirror images.
//
// @param  PricingTrx - Transaction object, not used. // TODO: get rid of.
// @param  TaxResponse - Tax response to send, contains all segments data.
// @param  TaxCodeReg - Registered tax object, contains all tax conditions.
// @param  startIndex - First TravelSeg to search from.
//
// @return uint16_t - Index of the first stopover segment
//                    starting from startIndex.
//
// </PRE>
// ----------------------------------------------------------------------------
uint16_t
TripTypesValidatorJP1::findTaxStopOverIndex(const PricingTrx& trx,
                                            const TaxResponse& taxResponse,
                                            const TaxCodeReg& taxCodeReg,
                                            uint16_t startIndex)
{
  if (taxCodeReg.nextstopoverrestr() != YES)
    return startIndex;

  std::vector<TravelSeg*>::const_iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin() + startIndex;

  uint16_t index = startIndex;

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++, index++)
  {
    const TravelSeg* travelSeg = *travelSegI;
    if (!travelSeg->isAir())
      return index - 1;

    bool shouldDiscardForcedConx =
        (taxCodeReg.loc1Appl() == LocRestrictionValidator::TAX_ENPLANEMENT ||
         (taxCodeReg.loc2Appl() != LocRestrictionValidator::TAX_DEPLANEMENT &&
          taxCodeReg.loc2Appl() != LocRestrictionValidator::TAX_DESTINATION));
    bool isTransitTimeValid = TransitValidator().validateTransitTime(
        trx, taxResponse, taxCodeReg, index, TransitValidator::SHOULD_CHECK_OPEN);

    if (travelSeg->isForcedStopOver() && travelSeg->isForcedConx() &&
        (index == startIndex || isTransitTimeValid))
      return index; // TODO: convince BA to remove this, will continue instead

    if (!travelSeg->isForcedConx() || shouldDiscardForcedConx)
    {
      if (index != startIndex && !isTransitTimeValid)
      {
        return index - 1;
      }
      else if (travelSeg->isForcedStopOver())
      {
        return index;
      }
    }
  }

  return index - 1;
}

bool
TripTypesValidatorJP1::isTerminationPt(Itin* itin, uint16_t index)
{

  bool isStopOver = true;

  if (index > 0 && index < itin->travelSeg().size() - 1)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(itin->travelSeg()[index]);
    if (!airSeg)
      return true;

    applyForcedQualifierOnStopOver(airSeg, isStopOver);
  }

  return isStopOver;
}

bool
TripTypesValidatorJP1::haveOnlyJpSegs(Itin* itin, uint16_t startIndex, uint16_t endIndex)
{
  std::vector<TravelSeg*>::iterator travelSegI = itin->travelSeg().begin() + startIndex;
  std::vector<TravelSeg*>::iterator travelSegIEnd = itin->travelSeg().begin() + endIndex;

  for (; travelSegI != travelSegIEnd + 1; travelSegI++)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);
    if (!airSeg)
      return false;

    if (airSeg->destination()->nation() != JAPAN || airSeg->origin()->nation() != JAPAN)
      return false;
  }

  return true;
}

bool
TripTypesValidatorJP1::haveOriginAndTerminationInJp(Itin* itin)
{
  const AirSeg* airSegFirst = dynamic_cast<const AirSeg*>(itin->travelSeg().front());
  const AirSeg* airSegLast = dynamic_cast<const AirSeg*>(itin->travelSeg().back());

  if (!airSegFirst || !airSegLast)
    return false;

  return (airSegFirst->origin()->nation() == JAPAN && airSegLast->destination()->nation() == JAPAN);
}

void
TripTypesValidatorJP1::applyForcedQualifierOnStopOver(const AirSeg* airSeg, bool& isStopOver)
{
  if (airSeg->isForcedStopOver())
    isStopOver = true;
  else if (airSeg->isForcedConx())
    isStopOver = false;
}

//APO-30072 - checks if returning to preceding point
bool
TripTypesValidatorJP1::isReturnToPrecedingPoint (const TaxCodeReg& taxCodeReg, Itin* itin, uint16_t index) const
{
  //there is no previous segment for 0
  if (!index)
    return false;

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(itin->travelSeg()[index]);
  if (!airSeg)
      return false;

  const AirSeg* airSegPrev = dynamic_cast<const AirSeg*>(itin->travelSeg()[index-1]);
  if (!airSegPrev)
    return false;

  if (airSeg->destination() != airSegPrev->origin())
    return false;

  //default 24h or defined in transit
  int nExpectedReturnTime = 24*60*60;
  if (!taxCodeReg.restrictionTransit().empty())
  {
    const TaxRestrictionTransit& restrictTransit = taxCodeReg.restrictionTransit().front();
    nExpectedReturnTime = restrictTransit.transitHours() * 60;
    nExpectedReturnTime += restrictTransit.transitMinutes();
    nExpectedReturnTime *= 60;
  }

  const int64_t nTime = DateTime::diffTime(airSeg->arrivalDT(), airSegPrev->departureDT());

  if (nTime>nExpectedReturnTime)
    return false;

  return true;
}

log4cxx::LoggerPtr
TaxJP1_00::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxJP1_00"));

const std::string TaxJP1_00::_allowedFareTypeFilter = "RU/RR/FU/FR/JU/JR/BU/BR/WU/WR/EU/ER";

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
TaxJP1_00::TaxJP1_00() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
TaxJP1_00::~TaxJP1_00() {}

namespace
{

class IsTravelSegInFareUsage
{
private:
  FareUsage* fu;

public:
  IsTravelSegInFareUsage(FareUsage* fareUsage) : fu(fareUsage) {};

  bool operator()(TravelSeg* ts) const
  {
    return (std::find_if(fu->travelSeg().begin(),
                         fu->travelSeg().end(),
                         std::bind2nd(std::equal_to<TravelSeg*>(), ts)) != fu->travelSeg().end());
  }
};

const char WPNCS_ON = 'T';
const char WPNCS_OFF = 'F';
const bool CHANGE_TRAVEL_DATE = true;
const bool DONT_CHANGE_TRAVEL_DATE = false;
const bool IGNORE_CABIN_CHECK = true;
const bool DONT_IGNORE_CABIN_CHECK = false;

}

bool
TaxJP1_00::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)

{
  TripTypesValidatorJP1 tripTypesValidator;
  return tripTypesValidator.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

void
TaxJP1_00::taxCreate(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t travelSegStartIndex,
                     uint16_t travelSegEndIndex)
{
  LOG4CXX_DEBUG(_logger, "New TaxJP1_00 new implementation");

  _failCode = TaxDiagnostic::NO_TAX_ADDED;

  LOG4CXX_DEBUG(_logger,
                "TaxJP1_00::findDomesticPart on segs: " << travelSegStartIndex << "-"
                                                        << travelSegEndIndex);

  TravelSeg* travelSeg = taxResponse.farePath()->itin()->travelSeg()[travelSegStartIndex];

  if (travelSeg == nullptr)
    return;

  std::list<TravelSeg*> domesticTravelSegVec;

  std::vector<TravelSeg*>::iterator travelSegIter =
      taxResponse.farePath()->itin()->travelSeg().begin() + travelSegStartIndex;
  std::vector<TravelSeg*>::iterator travelSegEndIter =
      taxResponse.farePath()->itin()->travelSeg().end(); // + travelSegEndIndex;

  for (uint16_t travelSegIndex = travelSegStartIndex;
       travelSegIndex <= travelSegEndIndex && travelSegIter != travelSegEndIter;
       travelSegIter++, travelSegIndex++)
  {
    LOG4CXX_DEBUG(_logger,
                  "TaxJP1_00: Checking segment ["
                      << travelSegIndex << "] segNo: " << (*travelSegIter)->pnrSegment() << " "
                      << (*travelSegIter)->origin()->loc() << "-"
                      << (*travelSegIter)->destination()->loc());

    if (isTravelSegWhollyJapan(*travelSegIter))
    {
      domesticTravelSegVec.push_back(*travelSegIter);

      uint16_t segmentOrder = taxResponse.farePath()->itin()->segmentOrder(*travelSegIter);

      _travelSegStartIndex =
          segmentOrder < _travelSegStartIndex ? segmentOrder : _travelSegStartIndex;
      _travelSegEndIndex =
          segmentOrder > _travelSegEndIndex ? segmentOrder : _travelSegEndIndex;
    }
  }

  if (domesticTravelSegVec.empty())
  {
    LOG4CXX_DEBUG(_logger, "TaxJP1_00: NO DOMESTIC SEGMENTS - return");
    return;
  }
  else if (LOG4CXX_UNLIKELY(IS_DEBUG_ENABLED(_logger)))
  {
    int iS = 0;
    for (const auto elem : domesticTravelSegVec)
    {
      LOG4CXX_DEBUG(_logger,
                    "TaxJP1_00: DOMESTIC SEGMENT[" << iS << "] segNo: " << elem->pnrSegment() << " "
                                                   << elem->origin()->loc() << "-"
                                                   << elem->destination()->loc());

      iS++;
    }
  }

  CurrencyCode paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if (!trx.getOptions()->currencyOverride().empty())
    paymentCurrency = trx.getOptions()->currencyOverride();
  LOG4CXX_DEBUG(_logger, "TaxJP1_00: payment currency " << paymentCurrency);

  _taxableFare = _taxAmount = 0;

  for (std::list<TravelSeg*>::iterator it = domesticTravelSegVec.begin();
       it != domesticTravelSegVec.end() && !domesticTravelSegVec.empty();)
  {
    FareUsage* fareUsage = locateWhollyDomesticFareUsage(taxResponse.farePath(), (*it));

    if (fareUsage)
    {
      MoneyAmount taxableFare = 0;

      if (fareUsage->paxTypeFare()->isForeignDomestic()) // get percentage and drop all dom segments
                                                         // priced with this fare
      {
        taxableFare = fareUsage->paxTypeFare()->totalFareAmount();

        LOG4CXX_DEBUG(_logger,
                      "TaxJP1_00: Get 5% from fare amount: "
                          << taxableFare << " Seqno: " << (*it)->pnrSegment() << " "
                          << (*it)->origin()->loc() << "-" << (*it)->destination()->loc() << "["
                          << _travelSegStartIndex << "-" << _travelSegEndIndex << "]");

        applyPartialAmount(trx, taxResponse, taxCodeReg, taxableFare, paymentCurrency, *it);

        // drop other segments with this fare
        IsTravelSegInFareUsage isInTravelSeg(fareUsage);

        LOG4CXX_DEBUG(_logger, "TaxJP1_00: remove similar segments...");
        domesticTravelSegVec.remove_if(isInTravelSeg);
        it = domesticTravelSegVec.begin();
      }
      else if ((*it)->isAir())
      {
        // try to reprice:

        if (!TrxUtil::isPricingTaxRequest(&trx) && !domesticTravelSegVec.empty())
        {
          bool bFareFound = findRepricedFare(trx,
                                             taxResponse.paxTypeCode(),
                                             *fareUsage,
                                             it,
                                             DONT_CHANGE_TRAVEL_DATE,
                                             WPNCS_OFF,
                                             taxableFare,
                                             taxResponse.farePath()->itin()->travelDate(),
                                             IGNORE_CABIN_CHECK,
                                             "TaxJP1_00: repricing trx is ready opt 1 ",
                                             "TaxJP1_00: no repricing trx opt 1");

          if (!bFareFound) // WPNCS
          {
            bFareFound = findRepricedFare(trx,
                                          taxResponse.paxTypeCode(),
                                          *fareUsage,
                                          it,
                                          DONT_CHANGE_TRAVEL_DATE,
                                          WPNCS_ON,
                                          taxableFare,
                                          taxResponse.farePath()->itin()->travelDate(),
                                          DONT_IGNORE_CABIN_CHECK,
                                          "TaxJP1_00: repricing trx is ready opt 2 ",
                                          "TaxJP1_00: no repricing trx opt 2");
          }
          if (!bFareFound) // change travel date
          {
            bFareFound = findRepricedFare(trx,
                                          taxResponse.paxTypeCode(),
                                          *fareUsage,
                                          it,
                                          CHANGE_TRAVEL_DATE,
                                          WPNCS_OFF,
                                          taxableFare,
                                          taxResponse.farePath()->itin()->travelDate(),
                                          IGNORE_CABIN_CHECK,
                                          "TaxJP1_00: repricing trx is ready opt 3 ",
                                          "TaxJP1_00: no repricing trx opt 3");
          }
          if (!bFareFound) // change date WPNCS
          {
            bFareFound = findRepricedFare(trx,
                                          taxResponse.paxTypeCode(),
                                          *fareUsage,
                                          it,
                                          CHANGE_TRAVEL_DATE,
                                          WPNCS_ON,
                                          taxableFare,
                                          taxResponse.farePath()->itin()->travelDate(),
                                          DONT_IGNORE_CABIN_CHECK,
                                          "TaxJP1_00: repricing trx is ready opt 4 ",
                                          "TaxJP1_00: no repricing trx opt 4");
          }

          if ((!bFareFound) && (trx.getOptions()->isPrivateFares())) // PV
          {
            bFareFound = findRepricedFare(trx,
                                          taxResponse.paxTypeCode(),
                                          *fareUsage,
                                          it,
                                          DONT_CHANGE_TRAVEL_DATE,
                                          WPNCS_OFF,
                                          taxableFare,
                                          taxResponse.farePath()->itin()->travelDate(),
                                          IGNORE_CABIN_CHECK,
                                          "TaxJP1_00: repricing trx is ready opt 5 ",
                                          "TaxJP1_00: no repricing trx opt 5",
                                          true);
          }

          if ( !bFareFound && trx.getOptions()->isPrivateFares() ) // PV + WPNCS
          {
            bFareFound = findRepricedFare(trx,
                                          taxResponse.paxTypeCode(),
                                          *fareUsage,
                                          it,
                                          DONT_CHANGE_TRAVEL_DATE,
                                          WPNCS_ON,
                                          taxableFare,
                                          taxResponse.farePath()->itin()->travelDate(),
                                          DONT_IGNORE_CABIN_CHECK,
                                          "TaxJP1_00: repricing trx is ready opt 6 ",
                                          "TaxJP1_00: no repricing trx opt 6",
                                          true);
          }
        }

        applyPartialAmount(trx, taxResponse, taxCodeReg, taxableFare, paymentCurrency, *it);

        domesticTravelSegVec.erase(it);
        it = domesticTravelSegVec.begin();
      }
    }
    else // attempt to repriece all
    {
      ++it; // increment if all no domestic fare or repricing
    }
  }
  LOG4CXX_DEBUG(_logger, "TaxJP1_00: Apply tax...");

  _failCode = TaxDiagnostic::NONE;
  _thruTotalFare = _taxableFare;
  _paymentCurrency = paymentCurrency;
  _travelSegStartIndex = travelSegStartIndex;
  _travelSegEndIndex = travelSegEndIndex;

  Money decMoney(_paymentCurrency);
  _paymentCurrencyNoDec = decMoney.noDec(trx.ticketingDate());
}

bool
TaxJP1_00::findRepricedFare(PricingTrx& trx,
                            const PaxTypeCode& paxTypeCode,
                            FareUsage& fareUsage,
                            std::list<TravelSeg*>::iterator& travelSegIter,
                            bool changeDate,
                            Indicator wpncsFlagIndicator,
                            MoneyAmount& taxableFare,
                            const DateTime& travelDate,
                            bool ignoreCabinCheck,
                            std::string repricingTrxReadyMessage,
                            std::string noRepricingTrxMessage,
                            bool privateFareCheck /* = false*/)
{
  RepricingTrx* retrx = nullptr;

  std::vector<TravelSeg*> segs;
  fillSegmentForRepricing(trx, fareUsage, travelSegIter, changeDate, segs);
  retrx = getRepricingTrx(
      trx, segs, wpncsFlagIndicator, getMappedPaxTypeCode(paxTypeCode), privateFareCheck);

  if (retrx)
  {
    LOG4CXX_DEBUG(_logger, repricingTrxReadyMessage << retrx);
    if (getAmountFromRepricedFare(paxTypeCode,
                                  retrx,
                                  (*travelSegIter),
                                  taxableFare,
                                  fareUsage.paxTypeFare()->cabin(),
                                  travelDate,
                                  ignoreCabinCheck))
      return true;
  }
  else
  {
    LOG4CXX_DEBUG(_logger, noRepricingTrxMessage);
  }

  return false;
}

void
TaxJP1_00::setDepartureAndArrivalDates(AirSeg* ts, PricingTrx& trx)
{
  int64_t diff = DateTime::diffTime(ts->arrivalDT(), ts->departureDT());

  ts->departureDT() = trx.ticketingDate();
  ts->arrivalDT() = ts->departureDT().addSeconds(diff);
  ts->pssDepartureDate() = ts->departureDT().dateToSqlString();
  ts->pssArrivalDate() = ts->arrivalDT().dateToSqlString();

  ts->earliestDepartureDT() = ts->departureDT();
  ts->latestDepartureDT() = ts->departureDT();
  ts->earliestArrivalDT() = ts->arrivalDT();
  ts->latestArrivalDT() = ts->arrivalDT();
}

void
TaxJP1_00::fillSegmentForRepricing(PricingTrx& trx,
                                   FareUsage& fareUsage,
                                   std::list<TravelSeg*>::iterator& travelSegIter,
                                   bool changeDate,
                                   std::vector<TravelSeg*>& segs)
{
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegIter);
  AirSeg* ts = trx.dataHandle().create<AirSeg>();
  *ts = *airSeg;
  getBkgCodeReBooked(&fareUsage, (*travelSegIter), ts);

  if (changeDate)
    setDepartureAndArrivalDates(ts, trx);

  segs.clear();
  segs.push_back(ts);
}

RepricingTrx*
TaxJP1_00::getRepricingTrx(PricingTrx& trx,
                           std::vector<TravelSeg*>& tvlSeg,
                           Indicator wpncsFlagIndicator,
                           const PaxTypeCode& extraPaxType,
                           const bool privateFareCheck)
{
  RepricingTrx* rpTrx = nullptr;

  try
  {
    rpTrx = TrxUtil::reprice(trx,
                             tvlSeg,
                             FMDirection::UNKNOWN,
                             false,
                             nullptr,
                             nullptr,
                             extraPaxType,
                             false,
                             false,
                             wpncsFlagIndicator,
                             ' ',
                             false,
                             privateFareCheck);
  }
  catch (const ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(_logger,
                  "TaxJP1_00: Exception during repricing with " << ex.code() << " - "
                                                                << ex.message());
    return nullptr;
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "TaxJP1_00: Unknown exception during repricing");
    return nullptr;
  }

  return rpTrx;
}

bool
TaxJP1_00::isTravelSegWhollyJapan(TravelSeg* travelSeg)
{
  if (!dynamic_cast<const AirSeg*>(travelSeg))
    return false;

  const Loc& org = *(travelSeg->origin());
  const Loc& dest = *(travelSeg->destination());

  if (!(LocUtil::isJapan(org) && LocUtil::isJapan(dest)))
    return false;

  return true;
}

FareUsage*
TaxJP1_00::locateWhollyDomesticFareUsage(FarePath* farePath, TravelSeg* travelSeg)
{
  std::vector<PricingUnit*>::const_iterator pricingUnitI;
  std::vector<FareUsage*>::iterator fareUsageI;
  std::vector<TravelSeg*>::const_iterator travelSegFuI;

  for (pricingUnitI = farePath->pricingUnit().begin();
       pricingUnitI != farePath->pricingUnit().end();
       pricingUnitI++)
  {
    for (fareUsageI = (*pricingUnitI)->fareUsage().begin();
         fareUsageI != (*pricingUnitI)->fareUsage().end();
         fareUsageI++)
    {
      bool bWhollyJapan = true;
      bool bHasTravelSeg = false;

      LOG4CXX_DEBUG(_logger, "TaxJP1_00: Checking fareusage: " << (*fareUsageI)->totalFareAmount());

      for (travelSegFuI = (*fareUsageI)->travelSeg().begin();
           travelSegFuI != (*fareUsageI)->travelSeg().end();
           travelSegFuI++)
      {
        // skip this fare usage if it has a international travel seg
        if (!isTravelSegWhollyJapan(*travelSegFuI))
          bWhollyJapan = false;

        if (farePath->itin()->segmentOrder(*travelSegFuI) ==
            farePath->itin()->segmentOrder(travelSeg))
          bHasTravelSeg = true;
      }

      LOG4CXX_DEBUG(_logger,
                    "TaxJP1_00: Fareusage: " << (*fareUsageI)->totalFareAmount()
                                             << " wholly japan: " << bWhollyJapan
                                             << " has travel seg " << bHasTravelSeg);

      if (bHasTravelSeg && bWhollyJapan)
        return *fareUsageI;
      else if (bHasTravelSeg)
        return *fareUsageI; // segment already checked..
    }
  }
  return nullptr;
}

MoneyAmount
TaxJP1_00::convertCurrency(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           MoneyAmount moneyAmount,
                           CurrencyCode& paymentCurrency)
{
  CurrencyConversionFacade ccFacade;

  Money targetMoney(paymentCurrency);
  targetMoney.value() = 0;

  if (taxResponse.farePath()->calculationCurrency() != taxResponse.farePath()->baseFareCurrency())
  {
    Money targetMoneyOrigination(taxResponse.farePath()->baseFareCurrency());
    targetMoneyOrigination.value() = 0;

    Money sourceMoneyCalculation(moneyAmount, taxResponse.farePath()->calculationCurrency());

    if (!ccFacade.convert(targetMoneyOrigination,
                          sourceMoneyCalculation,
                          trx,
                          false,
                          CurrencyConversionRequest::TAXES))
    {
      LOG4CXX_WARN(_logger,
                   "Currency Convertion Failure To Convert: "
                       << taxResponse.farePath()->calculationCurrency() << " To "
                       << taxResponse.farePath()->baseFareCurrency());
    }

    LOG4CXX_DEBUG(_logger,
                  "TaxJP1_00: convertion calculation currency (s/t): "
                      << sourceMoneyCalculation << "/" << targetMoneyOrigination);

    moneyAmount = targetMoneyOrigination.value();
  }

  if (taxResponse.farePath()->baseFareCurrency() != paymentCurrency)
  {
    Money sourceMoney(moneyAmount, taxResponse.farePath()->baseFareCurrency());

    if (!ccFacade.convert(targetMoney, sourceMoney, trx, false, CurrencyConversionRequest::TAXES))
    {
      LOG4CXX_WARN(_logger,
                   "Currency Convertion Failure To Convert: "
                       << taxResponse.farePath()->baseFareCurrency() << " To " << paymentCurrency);
    }
    LOG4CXX_DEBUG(_logger,
                  "TaxJP1_00: convertion payment currency (s/t): " << sourceMoney << "/"
                                                                   << targetMoney);

    moneyAmount = targetMoney.value();
  }

  return moneyAmount;
}

bool
TaxJP1_00::getAmountFromRepricedFare(const PaxTypeCode& paxTypeCode,
                                     RepricingTrx* retrx,
                                     TravelSeg* travelSeg,
                                     MoneyAmount& taxableFare,
                                     const CabinType& orgTravelSegCabin,
                                     const DateTime& travelDate,
                                     bool bIgnoreCabinCheck)
{

  const std::vector<PaxTypeFare*>* paxTypeFare = nullptr;

  paxTypeFare = locatePaxTypeFare(
      retrx->fareMarket()[0],
      getMappedPaxTypeCode(paxTypeCode, paxTypeCode));
  if (!paxTypeFare)
    return false;

  std::vector<PaxTypeFare*>::const_iterator fare = paxTypeFare->begin();
  std::vector<PaxTypeFare*>::const_iterator fareEnd = paxTypeFare->end();

  for (; fare != fareEnd; fare++)
  {

    const FareTypeMatrix* fareTypeMatrix =
        retrx->dataHandle().getFareTypeMatrix((*fare)->fcaFareType(), travelDate);

    std::string fareBasis = (*fare)->createFareBasis(retrx, false);

    if (LOG4CXX_UNLIKELY(IS_DEBUG_ENABLED(_logger)))
    {
      std::ostringstream os;

      if (fareTypeMatrix != nullptr)
      {
        os << "T/";
        os << (!fareTypeMatrix->fareTypeDesig().isFTDAddon() ? "NA/" : "na/");
        os << (fareTypeMatrix->fareTypeDisplay() == 'N' ? "N/" : "n/");
        os << (fareTypeMatrix->fareTypeAppl() == 'N' ? "A/" : "a/");
        os << ((fareTypeMatrix->restrInd() == 'R' || fareTypeMatrix->restrInd() == 'U') ? "I "
                                                                                        : "i ");
      }
      else
        os << "t/--/-/-/- ";

      Money m((*fare)->fareAmount(), (*fare)->currency());

      os << (bIgnoreCabinCheck || (*fare)->cabin() == orgTravelSegCabin ? "C/" : "c/");
      os << ((*fare)->isForeignDomestic() ? "FD/" : "fd/");
      os << ((*fare)->directionality() == FROM ? "F/" : "f/");
      os << ((*fare)->owrt() != ROUND_TRIP_MAYNOT_BE_HALVED ? "R/" : "r/");
      os << ((*fare)->isNormal() ? "N/" : "n/");
      os << ((*fare)->isValidForPricing() ? "P/" : "p/");
      os << ((*fare)->isSoftPassed() ? "S/" : "s/");
      os << ((*fare)->isRoutingValid() ? "RV/" : "rv/");
      os << ((*fare)->isCategoryValid(3) ? "3/" : "x/");
      os << ((*fare)->isCategorySoftPassed(3) ? "s3/" : "sx/");
      os << ((*fare)->bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED) ||
                     !(*fare)->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL)
                 ? "B "
                 : "b ");

      os << (*fare)->ruleNumber() << " ";
      os << m.toString();

      LOG4CXX_DEBUG(_logger, "TaxJP1_00: repricing trx fare: " << fareBasis << ": " << os.str());
    }

    if ((fareTypeMatrix != nullptr &&
         (!fareTypeMatrix->fareTypeDesig().isFTDAddon() &&
          fareTypeMatrix->fareTypeDisplay() == 'N' && fareTypeMatrix->fareTypeAppl() == 'N' &&
          (fareTypeMatrix->restrInd() == 'R' || fareTypeMatrix->restrInd() == 'U'))) &&
        (bIgnoreCabinCheck || (*fare)->cabin() == orgTravelSegCabin) &&
        (*fare)->isForeignDomestic() && (*fare)->directionality() == FROM &&
        (*fare)->owrt() != ROUND_TRIP_MAYNOT_BE_HALVED && (*fare)->isNormal() &&
        ((*fare)->isValidForPricing() ||
         ((*fare)->isSoftPassed() && (*fare)->isRoutingValid() &&
          ((*fare)->bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED) ||
           !(*fare)->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL)))))
    {
      taxableFare = (*fare)->totalFareAmount();

      Money m((*fare)->fareAmount(), (*fare)->currency());

      LOG4CXX_DEBUG(_logger,
                    "TaxJP1_00: Get 5% from repriced fare amount: "
                        << taxableFare << "/" << m.toString() << " farebasis: " << fareBasis
                        << " Seqno: " << travelSeg->pnrSegment() << " "
                        << travelSeg->origin()->loc() << "-" << travelSeg->destination()->loc()
                        << "[" << _travelSegStartIndex << "-" << _travelSegEndIndex << "]");

      return true;
    }
  }

  return false;
}

void
TaxJP1_00::getBkgCodeReBooked(FareUsage* fareUsage,
                              TravelSeg* travelSegRef,
                              TravelSeg* travelSegClone)
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

void
TaxJP1_00::applyPartialAmount(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              MoneyAmount& taxableFare,
                              CurrencyCode& paymentCurrency,
                              TravelSeg* travelSeg)
{
  taxableFare = convertCurrency(trx, taxResponse, taxableFare, paymentCurrency);

  Percent discPercent = trx.getRequest()->discountPercentage(travelSeg->segmentOrder());
  if (discPercent >= 0 && discPercent <= 100)
  {
    LOG4CXX_DEBUG(_logger,
                  "TaxJP1_00: discount " << discPercent << " on taxable fare: " << taxableFare);
    taxableFare *= (1.0 - discPercent / 100.0);
  }

  MoneyAmount taxAmount = taxableFare * taxCodeReg.taxAmt();
  LOG4CXX_DEBUG(_logger, "TaxJP1_00: tax amount: " << taxableFare << " converted: " << taxAmount);

  _paymentCurrency = paymentCurrency;

  MoneyAmount newTaxAmount = AdjustTax::applyAdjust(
      trx, taxResponse, taxAmount, _paymentCurrency, taxCodeReg, _calculationDetails);

  if (newTaxAmount)
    taxAmount = newTaxAmount;

  _failCode = TaxDiagnostic::NONE;
  _thruTotalFare = _taxableFare;

  MoneyAmount keepTaxableFare = _taxableFare;
  MoneyAmount keepTaxAmount = _taxAmount;

  _taxableFare = taxableFare;
  _taxAmount = taxAmount;

  doTaxRound(trx, taxCodeReg);

  LOG4CXX_DEBUG(_logger,
                "TaxJP1_00: tax amount:(rounded) " << _taxableFare << " converted: " << _taxAmount);

  _taxableFare += keepTaxableFare;
  _taxAmount += keepTaxAmount;
}

const std::vector<PaxTypeFare*>*
TaxJP1_00::locatePaxTypeFare(const FareMarket* fareMarketReTrx, const PaxTypeCode& paxTypeCode)
{
  const std::vector<PaxTypeBucket>& paxTypeCortege = fareMarketReTrx->paxTypeCortege();
  std::vector<PaxTypeBucket>::const_iterator paxTypeCortegeI = paxTypeCortege.begin();

  for (; paxTypeCortegeI != fareMarketReTrx->paxTypeCortege().end(); ++paxTypeCortegeI)
  {
    if ((*paxTypeCortegeI).requestedPaxType()->paxType() == paxTypeCode)
      return &(*paxTypeCortegeI).paxTypeFare();
  }

  return nullptr;
}

const PaxTypeCode
TaxJP1_00::getMappedPaxTypeCode(const PaxTypeCode& paxTypeCode,
                                const PaxTypeCode& defaultPaxTypeCode)
{
  if (paxTypeCode == JCB || paxTypeCode == JNN)
    return ADULT;

  return defaultPaxTypeCode;
}
}
