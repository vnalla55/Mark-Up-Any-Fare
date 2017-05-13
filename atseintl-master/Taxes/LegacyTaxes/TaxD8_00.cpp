//---------------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxD8_00.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/AdjustTax.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"

using namespace tse;
using namespace std;

Logger
TaxD8_00::_logger("atseintl.Taxes.TaxD8_00");

namespace
{
class IsTravelSegInFareUsage
{
private:
  const FareUsage* fu;

public:
  IsTravelSegInFareUsage(const FareUsage* fareUsage) : fu(fareUsage) {};

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

const NationCode MalaysiaCode("MY");

} //namespace


bool
TaxD8_00::isDomesticSegment(TravelSeg* travelSeg) const
{
  if (!travelSeg->isAir())
    return false;

  return (travelSeg->origin()->nation() == MalaysiaCode &&
      travelSeg->destination()->nation() == MalaysiaCode);
}

void
TaxD8_00::taxCreate(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg,
                    uint16_t travelSegStartIndex,
                    uint16_t travelSegEndIndex)
{
  Tax::taxCreate(trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex);

  LOG4CXX_DEBUG(_logger, "TaxD8_00::taxCreate");
  _failCode = TaxDiagnostic::NO_TAX_ADDED;

  std::list<TravelSeg*> domesticTravelSegVec;

  //check if tax on domestic part is required
  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  locIt->setSkipHidden(true);
  locIt->toSegmentNo(0);
  locIt->setStopHours(24);

  bool bD8 = false;
  while (!locIt->isEnd())
  {
    bool bDomesticSeg = false;
    if (locIt->hasPrevious())
    {
      if (isDomesticSegment(locIt->prevSeg()))
      {
        domesticTravelSegVec.push_back(locIt->prevSeg());
        bDomesticSeg = true;

        //store min-max segment with D8 tax
        uint16_t segmentOrder = taxResponse.farePath()->itin()->segmentOrder(locIt->prevSeg());
        _travelSegStartIndex =
            segmentOrder < _travelSegStartIndex ? segmentOrder : _travelSegStartIndex;
        _travelSegEndIndex =
            segmentOrder > _travelSegEndIndex ? segmentOrder : _travelSegEndIndex;
      }

      LOG4CXX_DEBUG(_logger, "TaxD8_00: Checking segment [segNo: " << locIt->prevSegNo() << "] "
                            << locIt->prevSeg()->origin()->loc() << "-"
                            << locIt->prevSeg()->destination()->loc() <<
                            (bDomesticSeg ? " DOMESTIC" : " INTERNATIONAL"));

      if (!bD8 && locIt->hasNext())
      {
        if (!bDomesticSeg)
          bDomesticSeg = isDomesticSegment(locIt->nextSeg());

        //if prev or next is domestic (Malyasia) then check stopover condition
        if (bDomesticSeg)
        {
          bD8 = locIt->isStop();
          if (bD8 && LOG4CXX_UNLIKELY(IS_DEBUG_ENABLED(_logger)))
          {
            LOG4CXX_DEBUG(_logger,
                          "TaxD8_00: D8 should apply due to stopover between: "
                              << locIt->prevSeg()->origin()->loc() << "-"
                              << locIt->nextSeg()->destination()->loc()
                              << " segNo:" << locIt->prevSegNo());
          }

          if (!bD8) // check if the same carrier
          {
            if (locIt->prevSeg()->isAir() && locIt->nextSeg()->isAir())
            {
              const AirSeg* travelSegP = static_cast<const AirSeg*>(locIt->prevSeg());
              const AirSeg* travelSegN = static_cast<const AirSeg*>(locIt->nextSeg());
              if (travelSegN && travelSegN && travelSegP->carrier() != travelSegN->carrier())
              {
                bD8 = true;
                if (LOG4CXX_UNLIKELY(IS_DEBUG_ENABLED(_logger)))
                {
                  LOG4CXX_DEBUG(_logger,
                                "TaxD8_00: D8 should apply due to diff carrier: "
                                    << travelSegP->carrier() << "<>" << travelSegN->carrier()
                                    << " segNo:" << locIt->prevSegNo());
                } // if (LOG4CXX_UNLIKELY(IS_DEBUG_ENABLED(_logger)))
              } // if (travelSegN&& travelSegN && travelSegP->carrier()!=travelSegN->carrier())
            } // if (locIt->prevSeg()->isAir() && locIt->nextSeg()->isAir())
          } // if (!bD8)
        } // if (bDomesticSeg)
      } // if (!bD8 && locIt->hasNext())
    } // if (locIt->hasPrevious())
    locIt->next();
  } //while (!locIt->isEnd())

  LOG4CXX_DEBUG(_logger, "TaxD8_00: payment currency " << _paymentCurrency);

  _failCode = TaxDiagnostic::NONE;
  _taxAmount = _taxableFare = _taxableBaseFare = 0;

  if (!bD8)
  {
    LOG4CXX_DEBUG(_logger, "TaxD8_00: TaxFare amount is not required");
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "TaxD8_00: Calculate TaxFare amount");

    for (std::list<TravelSeg*>::iterator it = domesticTravelSegVec.begin();
         it != domesticTravelSegVec.end() && !domesticTravelSegVec.empty();)
    {
      MoneyAmount taxableFare = 0;

      const FareUsage* fareUsage = locateFare(taxResponse.farePath(), (*it));
      if (fareUsage)
      {
        if (fareUsage->paxTypeFare()->isForeignDomestic()) // get percentage and drop all dom segments
                                                           // priced with this fare
        {
          taxableFare = fareUsage->totalFareAmount();

          LOG4CXX_DEBUG(_logger, "TaxD8_00: Whole domestic fare found: "
                                 << taxableFare << " Seqno: " << (*it)->pnrSegment() << " "
                                 << (*it)->origin()->loc() << "-" << (*it)->destination()->loc());

          applyPartialAmount(trx, taxResponse, taxCodeReg, taxableFare, _paymentCurrency, *it);

          // drop other segments with this fare
          IsTravelSegInFareUsage isInTravelSeg(fareUsage);
          domesticTravelSegVec.remove_if(isInTravelSeg);

          it = domesticTravelSegVec.begin();
        }
        else if ((*it)->isAir())
        {
          // try to reprice:
          if (!TrxUtil::isPricingTaxRequest(&trx))
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
                                             "TaxD8_00: repricing trx is ready opt 1 ",
                                             "TaxD8_00: no repricing trx opt 1");

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
                                             "TaxD8_00: repricing trx is ready opt 2 ",
                                             "TaxD8_00: no repricing trx opt 2");
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
                                             "TaxD8_00: repricing trx is ready opt 3 ",
                                             "TaxD8_00: no repricing trx opt 3");
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
                                             "TaxD8_00: repricing trx is ready opt 4 ",
                                             "TaxD8_00: no repricing trx opt 4");
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
                                             "TaxD8_00: repricing trx is ready opt 5 ",
                                             "TaxD8_00: no repricing trx opt 5",
                                             true);
           }
           if (!bFareFound && trx.getOptions()->isPrivateFares()) // PV + WPNCS
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
                                             "TaxD8_00: repricing trx is ready opt 6 ",
                                             "TaxD8_00: no repricing trx opt 6",
                                             true);
            }
          } //if (!TrxUtil::isPricingTaxRequest(&trx))

          applyPartialAmount(trx, taxResponse, taxCodeReg, taxableFare, _paymentCurrency, *it);
          domesticTravelSegVec.erase(it);
          it = domesticTravelSegVec.begin();
        } //else if ((*it)->isAir())
        else
        {
          domesticTravelSegVec.erase(it);
          it = domesticTravelSegVec.begin();
        }
      }
      else
      {
        domesticTravelSegVec.erase(it);
        it = domesticTravelSegVec.begin();
      }
    } //for
  }


}

const FareUsage*
TaxD8_00::locateFare(const FarePath* farePath, const TravelSeg* travelSegIn) const
{
  for (const PricingUnit* pricingUnit : farePath->pricingUnit())
  {
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      for (const TravelSeg* travelSeg : fareUsage->travelSeg())
      {
        if (farePath->itin()->segmentOrder(travelSeg) ==
            farePath->itin()->segmentOrder(travelSegIn))
        {
          LOG4CXX_DEBUG(_logger, "TaxD8_00: Fareusage: " << fareUsage->totalFareAmount());
          return fareUsage;
        }
      }
    }
  }

  return nullptr;
}

void
TaxD8_00::applyPartialAmount(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             MoneyAmount taxableFare,
                             const CurrencyCode& paymentCurrency,
                             const TravelSeg* travelSeg)
{
  taxableFare = convertCurrency(trx, taxResponse, taxableFare, paymentCurrency);

  Percent discPercent = trx.getRequest()->discountPercentage(travelSeg->segmentOrder());
  if (discPercent >= 0 && discPercent <= 100)
  {
    LOG4CXX_DEBUG(_logger,
                  "TaxD8_00: discount " << discPercent << " on taxable fare: " << taxableFare);
    taxableFare *= (1.0 - discPercent / 100.0);
  }

  MoneyAmount taxAmount = taxableFare * taxCodeReg.taxAmt();
  LOG4CXX_DEBUG(_logger, "TaxD8_00: tax amount: " << taxableFare << " converted: " << taxAmount);

  _paymentCurrency = paymentCurrency;

  MoneyAmount newTaxAmount = AdjustTax::applyAdjust(
      trx, taxResponse, taxAmount, _paymentCurrency, taxCodeReg, _calculationDetails);

  if (newTaxAmount)
    taxAmount = newTaxAmount;

  _thruTotalFare = _taxableFare;

  MoneyAmount keepTaxableFare = _taxableFare;
  MoneyAmount keepTaxAmount = _taxAmount;

  _taxableFare = taxableFare;
  _taxAmount = taxAmount;

  doTaxRound(trx, taxCodeReg);

  LOG4CXX_DEBUG(_logger,
                "TaxD8_00: tax amount:(rounded) " << _taxableFare << " converted: " << _taxAmount);

  _taxableFare += keepTaxableFare;
  _taxAmount += keepTaxAmount;
}

MoneyAmount
TaxD8_00::convertCurrency(PricingTrx& trx,
                          const TaxResponse& taxResponse,
                          MoneyAmount moneyAmount,
                          const  CurrencyCode& paymentCurrency) const
{
  CurrencyConversionFacade ccFacade;

  Money targetMoney(paymentCurrency);
  targetMoney.value() = 0;

  if (taxResponse.farePath()->calculationCurrency() != taxResponse.farePath()->baseFareCurrency())
  {
    Money targetMoneyOrigination(taxResponse.farePath()->baseFareCurrency());
    targetMoneyOrigination.value() = 0;

    Money sourceMoneyCalculation(moneyAmount, taxResponse.farePath()->calculationCurrency());

    if (paymentCurrency!=taxResponse.farePath()->baseFareCurrency())
      ccFacade.setRoundFare(false);

    if (!ccFacade.convert(targetMoneyOrigination,
                          sourceMoneyCalculation,
                          trx,
                          false,
                          CurrencyConversionRequest::TAXES))
    {
      LOG4CXX_WARN(_logger, "Currency Convertion Failure To Convert: "
                            << taxResponse.farePath()->calculationCurrency() << " To "
                             << taxResponse.farePath()->baseFareCurrency());
    }

    LOG4CXX_DEBUG(_logger, "TaxD8_00: convertion calculation currency (s/t): "
                           << sourceMoneyCalculation << "/" << targetMoneyOrigination);

    moneyAmount = targetMoneyOrigination.value();
  }

  if (taxResponse.farePath()->baseFareCurrency() != paymentCurrency)
  {
    ccFacade.setRoundFare(true);

    Money sourceMoney(moneyAmount, taxResponse.farePath()->baseFareCurrency());

    if (!ccFacade.convert(targetMoney, sourceMoney, trx, false, CurrencyConversionRequest::TAXES))
    {
      LOG4CXX_WARN(_logger, "Currency Convertion Failure To Convert: "
                            << taxResponse.farePath()->baseFareCurrency() << " To " << paymentCurrency);
    }
    LOG4CXX_DEBUG(_logger, "TaxD8_00: convertion payment currency (s/t): "
                           << sourceMoney << "/" << targetMoney);

    moneyAmount = targetMoney.value();
  }

  return moneyAmount;
}

bool
TaxD8_00::findRepricedFare(PricingTrx& trx,
                           const PaxTypeCode& paxTypeCode,
                           const FareUsage& fareUsage,
                           std::list<TravelSeg*>::iterator& travelSegIter,
                           bool changeDate,
                           Indicator wpncsFlagIndicator,
                           MoneyAmount& taxableFare,
                           const DateTime& travelDate,
                           bool ignoreCabinCheck,
                           const char* repricingTrxReadyMessage,
                           const char* noRepricingTrxMessage,
                           bool privateFareCheck /* = false*/) const
{
  RepricingTrx* retrx = nullptr;

  std::vector<TravelSeg*> segs;
  fillSegmentForRepricing(trx, fareUsage, travelSegIter, changeDate, segs);
  retrx = getRepricingTrx(trx, segs, wpncsFlagIndicator, paxTypeCode, privateFareCheck);

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
TaxD8_00::fillSegmentForRepricing(PricingTrx& trx,
                                  const FareUsage& fareUsage,
                                  std::list<TravelSeg*>::iterator& travelSegIter,
                                  bool changeDate,
                                  std::vector<TravelSeg*>& segs) const
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
TaxD8_00::getRepricingTrx(PricingTrx& trx,
                          std::vector<TravelSeg*>& tvlSeg,
                          Indicator wpncsFlagIndicator,
                          const PaxTypeCode& extraPaxType,
                          const bool privateFareCheck) const
{
  try
  {
    return TrxUtil::reprice(trx,
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
    LOG4CXX_ERROR(_logger, "TaxD8_00: Exception during repricing with "
                           << ex.code() << " - " << ex.message());
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "TaxD8_00: Unknown exception during repricing");
  }

  return nullptr;
}

bool
TaxD8_00::getAmountFromRepricedFare(const PaxTypeCode& paxTypeCode,
                                    RepricingTrx* retrx,
                                    const TravelSeg* travelSeg,
                                    MoneyAmount& taxableFare,
                                    const CabinType& orgTravelSegCabin,
                                    const DateTime& travelDate,
                                    bool bIgnoreCabinCheck) const
{

  const std::vector<PaxTypeFare*>* paxTypeFare = nullptr;

  paxTypeFare = locatePaxTypeFare(retrx->fareMarket()[0],paxTypeCode);
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

      LOG4CXX_DEBUG(_logger, "TaxD8_00: repricing trx fare: " << fareBasis << ": " << os.str());
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

      LOG4CXX_DEBUG(_logger, "TaxD8_00: Get repriced fare amount: "
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
TaxD8_00::getBkgCodeReBooked(const FareUsage* fareUsage,
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

void
TaxD8_00::setDepartureAndArrivalDates(AirSeg* ts, const PricingTrx& trx) const
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

const std::vector<PaxTypeFare*>*
TaxD8_00::locatePaxTypeFare(const FareMarket* fareMarketReTrx, const PaxTypeCode& paxTypeCode) const
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
