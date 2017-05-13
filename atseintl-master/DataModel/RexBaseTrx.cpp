//-------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "DataModel/RexBaseTrx.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Billing.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingRequest.h"

namespace tse
{
FALLBACK_DECL(footNotePrevalidationForExc);

RexPricingOptions*
RexBaseTrx::prepareOptions()
{
  RexPricingOptions* options = nullptr;
  _dataHandle.get(options);

  if (options)
  {
    options->setTrx(this);
    _options = options;
  }

  return options;
}


const RexPricingOptions&
RexBaseTrx::getRexOptions() const
{
  return static_cast<RexPricingOptions&>(*_options);
}

const RexPricingRequest&
RexBaseTrx::getRexRequest() const
{
  return static_cast<RexPricingRequest&>(*_request);
}

RexPricingRequest&
RexBaseTrx::getRexRequest()
{
  return static_cast<RexPricingRequest&>(*_request);
}

void
RexBaseTrx::setFareApplicationDT(const DateTime& fareApplicationDT)
{
  dataHandle().setTicketDate(fareApplicationDT);
  _fareApplicationDT = fareApplicationDT;
}

namespace
{

void
fillSeq(std::vector<RexBaseTrx::DateSeqType>& seq,
        const RexDateSeqStatus* tab,
        const DateTime& date)
{
  seq.push_back(RexBaseTrx::DateSeqType(tab[0], date.subtractDays(1)));
  seq.push_back(RexBaseTrx::DateSeqType(tab[1], date.subtractDays(2)));
  seq.push_back(RexBaseTrx::DateSeqType(tab[2], date.addDays(1)));
  seq.push_back(RexBaseTrx::DateSeqType(tab[3], date.addDays(2)));
}

struct Tuple
{
  DateTime date;
  RexDateSeqStatus status[5];
};

} // namespace

void
RexBaseTrx::setupDateSeq()
{
  _tktDateSeq.clear();
  _tktDateSeq.reserve(20);

  const DateTime& reissueDate = lastTktReIssueDT(),
                  commenceDate = _exchangeItin.front()->travelCommenceDate(),
                  originalDate = originalTktIssueDT(), previousExcDate = previousExchangeDT();

  unsigned j = 0;
  std::map<unsigned, Tuple> map;

  Tuple p = { previousExcDate,
              { PREVIOUS_EXCHANGE_DATE, ONE_DAY_BEFORE_PREV_EXC_DATE, TWO_DAYS_BEFORE_PREV_EXC_DATE,
                ONE_DAY_AFTER_PREV_EXC_DATE, TWO_DAYS_AFTER_PREV_EXC_DATE } };

  Tuple o = { originalDate,
              { ORIGINAL_TICKET_DATE, ONE_DAY_BEFORE_ORIG_TKT_DATE, TWO_DAYS_BEFORE_ORIG_TKT_DATE,
                ONE_DAY_AFTER_ORIG_TKT_DATE, TWO_DAYS_AFTER_ORIG_TKT_DATE } };

  Tuple r = { reissueDate,
              { REISSUE_TICKET_DATE, ONE_DAY_BEFORE_REISSUE_TKT_DATE,
                TWO_DAYS_BEFORE_REISSUE_TKT_DATE, ONE_DAY_AFTER_REISSUE_TKT_DATE,
                TWO_DAYS_AFTER_REISSUE_TKT_DATE } };

  Tuple c = { commenceDate,
              { COMMENCE_DATE, ONE_DAY_BEFORE_COMMENCE_DATE, TWO_DAYS_BEFORE_COMMENCE_DATE,
                ONE_DAY_AFTER_COMMENCE_DATE, TWO_DAYS_AFTER_COMMENCE_DATE } };

  if (this->applyReissueExchange() && previousExcDate != DateTime::emptyDate())
  {
    map[j] = p;

    if (originalDate != previousExcDate)
      map[2] = o;

    if (reissueDate != DateTime::emptyDate() && reissueDate != originalDate &&
        reissueDate != previousExcDate)
    {
      j = (map.size() > 1) ? 6 : 4;
      map[j] = r;
    }

    if (commenceDate != DateTime::emptyDate() && commenceDate != originalDate &&
        commenceDate != reissueDate && commenceDate != previousExcDate)
    {
      if (map.size() > 2)
        j = (commenceDate < reissueDate) ? 4 : 8;

      else if ((map.size() > 1 && originalDate == previousExcDate && commenceDate < reissueDate) ||
               map.size() == 1)
        j = 2;
      else
        j = 6;

      map[j] = c;
    }
  }
  else
  {
    map[0] = o;

    if (reissueDate != DateTime::emptyDate() && reissueDate != originalDate)
      map[4] = r;

    if (commenceDate != DateTime::emptyDate() && commenceDate != originalDate &&
        commenceDate != reissueDate)
    {
      j = (map.size() > 1 && commenceDate < reissueDate) ? 2 : 6;
      map[j] = c;
    }
  }

  std::map<unsigned, Tuple>::const_iterator i = map.begin();
  for (; i != map.end(); ++i)
    _tktDateSeq.push_back(DateSeqType(i->second.status[0], i->second.date));
  for (i = map.begin(); i != map.end(); ++i)
    fillSeq(_tktDateSeq, i->second.status + 1, i->second.date);

  _currTktDateSeq = _tktDateSeq.begin();
}

bool
RexBaseTrx::nextTktDateSeq()
{
  ++_currTktDateSeq;
  return (_currTktDateSeq != _tktDateSeq.end());
}

class MatchSeqStatus : public std::unary_function<RexBaseTrx::DateSeqType, bool>
{
public:
  MatchSeqStatus(uint16_t seqStatus) : _seqStatus(seqStatus) {}

  bool operator()(const RexBaseTrx::DateSeqType& dateSeqType) const
  {
    return (dateSeqType.first == _seqStatus);
  }

private:
  uint16_t _seqStatus;
};

const Loc*
RexBaseTrx::currentSaleLoc() const
{
  if (_request->PricingRequest::salePointOverride().empty())
    return _request->PricingRequest::ticketingAgent()->agentLocation();
  else
    return dataHandle().getLoc(_request->PricingRequest::salePointOverride(), _currentTicketingDT);
}

bool
RexBaseTrx::isSameFareDate(const PaxTypeFare* fare1, const PaxTypeFare* fare2)
{
  return ((fare1 != nullptr) && (fare1->retrievalInfo() != nullptr) && (fare2 != nullptr) &&
          (fare2->retrievalInfo() != nullptr) &&
          ((fare1->retrievalInfo()->_date == fare2->retrievalInfo()->_date) ||
           (fare1->retrievalInfo()->keep() && fare2->retrievalInfo()->keep())));
}

const std::vector<FareTypeTable*>&
RexBaseTrx::getFareTypeTables(const VendorCode& vendor,
                              uint32_t tblItemNo,
                              const DateTime& applicationDate) const
{
  return dataHandle().getFareTypeTable(vendor, tblItemNo, originalTktIssueDT(), applicationDate);
}

void
RexBaseTrx::setupMipDateSeq()
{

  if (excFareCompInfo().empty())
  {
    return;
  } // process only for mip

  std::vector<DateSeqType> mipDateSeq;
  DateTime origTktDate = (*_tktDateSeq.begin()).second;

  std::vector<FareComponentInfo*>::const_iterator fcIter = excFareCompInfo().begin();
  for (; fcIter != excFareCompInfo().end(); fcIter++)
  {
    FareComponentInfo& fci = **fcIter;
    DateTime retrievalDT = fci.vctrInfo()->retrievalDate();
    DateSeqType nextSeqEl(FARE_COMPONENT_DATE, retrievalDT);

    if (retrievalDT == DateTime::emptyDate() || retrievalDT == origTktDate)
    {
      continue;
    }

    if (std::find(mipDateSeq.begin(), mipDateSeq.end(), nextSeqEl) ==
        mipDateSeq.end()) // add unique dates
    {
      mipDateSeq.push_back(nextSeqEl);
    }
  }

  _tktDateSeq.insert(_tktDateSeq.begin() + 1, mipDateSeq.begin(), mipDateSeq.end());
  _currTktDateSeq = _tktDateSeq.begin();
}

void
RexBaseTrx::createExchangePricingTrxForRedirect(bool runForDiagnostic)
{
  dataHandle().get(_exchangePricingTrxForRedirect);
  if (!_exchangePricingTrxForRedirect)
    return;

  if (!_exchangePricingTrxForRedirect->initialize(*this, runForDiagnostic))
  {
    _exchangePricingTrxForRedirect = nullptr;
  }
}

void
RexBaseTrx::setActionCode()
{
  if (!_billing)
    return;
  if (_redirected)
  {
    if (_billing->actionCode().size() > 3)
      _billing->actionCode() = _billing->actionCode().substr(0, 3) + _secondaryExcReqType;
    else
      _billing->actionCode() += _secondaryExcReqType;
  }
  else
  {
    BaseExchangeTrx::setActionCode();
  }
}

namespace
{
class SkipBaseRules : public std::unary_function<uint16_t, bool>
{
public:
  SkipBaseRules(const RexBaseTrx& trx) : _trx(trx) {}

  bool operator()(int rule) const
  {
    switch (rule)
    {
    case RuleConst::DAY_TIME_RULE:
    case RuleConst::SEASONAL_RULE:
    case RuleConst::FLIGHT_APPLICATION_RULE:
    case RuleConst::MINIMUM_STAY_RULE:
    case RuleConst::MAXIMUM_STAY_RULE:
    case RuleConst::BLACKOUTS_RULE:
    case RuleConst::ACCOMPANIED_PSG_RULE:
    case RuleConst::TRAVEL_RESTRICTIONS_RULE:
    case RuleConst::HIP_RULE:
    case RuleConst::TICKET_ENDORSMENT_RULE:
    case RuleConst::CHILDREN_DISCOUNT_RULE:
    case RuleConst::TOUR_DISCOUNT_RULE:
    case RuleConst::AGENTS_DISCOUNT_RULE:
    case RuleConst::OTHER_DISCOUNT_RULE:
    case RuleConst::MISC_FARE_TAG:
      return true;
    }
    return false;
  }

private:
  const RexBaseTrx& _trx;
};
}

void
RexBaseTrx::skipRulesOnExcItin(std::vector<uint16_t>& categorySequence) const
{
  if (_trxPhase != RexBaseTrx::PRICE_NEWITIN_PHASE)
    categorySequence.erase(
        remove_if(categorySequence.begin(), categorySequence.end(), SkipBaseRules(*this)),
        categorySequence.end());
}

const DateTime
RexBaseTrx::adjustToCurrentUtcZone(const DateTime& dateToAdjust, const Loc* locOfAdjust) const
{
  if (!locOfAdjust)
    return dateToAdjust;

  short utcoffset(0);
  if (LocUtil::getUtcOffsetDifference(*currentSaleLoc(),
                                      *locOfAdjust,
                                      utcoffset,
                                      dataHandle(),
                                      currentTicketingDT(),
                                      dateToAdjust))
    return dateToAdjust + Minutes(utcoffset);

  return DateTime::emptyDate();
}

const Percent*
RexBaseTrx::getExcDiscountPercentage(const FareMarket& fareMarket) const
{
  return TrxUtil::getDiscountPercentage(fareMarket, getRexRequest().excDiscounts(), false);
}

const DiscountAmount*
RexBaseTrx::getExcDiscountAmount(const FareMarket& fareMarket) const
{
  return TrxUtil::getDiscountAmount(fareMarket, getRexRequest().excDiscounts());
}

const Percent*
RexBaseTrx::getExcDiscountPercentage(const PricingUnit& pricingUnit) const
{
  return TrxUtil::getDiscountPercentage(pricingUnit, getRexRequest().excDiscounts(), false);
}

const DiscountAmount*
RexBaseTrx::getExcDiscountAmount(const PricingUnit& pricingUnit) const
{
  return TrxUtil::getDiscountAmount(pricingUnit, getRexRequest().excDiscounts());
}

const CurrencyCode&
RexBaseTrx::exchangeItinCalculationCurrency() const
{
  return exchangeItin().front()->calcCurrencyOverride().empty()
             ? exchangeItin().front()->calculationCurrency()
             : exchangeItin().front()->calcCurrencyOverride();
}

Money
RexBaseTrx::convertCurrency(const Money& source, const CurrencyCode& targetCurr, bool rounding)
    const
{
  return ExchangeUtil::convertCurrency(*this, source, targetCurr, rounding);
}

void
RexBaseTrx::setupFootNotePrevalidation()
{
  _footNotePrevalidationAllowed = (trxPhase() == RexBaseTrx::PRICE_NEWITIN_PHASE) &&
                                  !fallback::footNotePrevalidationForExc(this);
}

} // tse
