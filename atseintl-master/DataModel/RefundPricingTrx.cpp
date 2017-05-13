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

#include "DataModel/RefundPricingTrx.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingRequest.h"
#include "DataModel/RexPricingOptions.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/XMLConvertUtils.h"

#include <functional>

namespace tse
{
namespace
{
Logger
logger("atseintl.DataModel.RefundPricingTrx");
ConfigurableValue<bool>
skipCat33AirlineSecurityCheck("REX_FARE_SELECTOR_SVC", "SKIP_CAT_33_AIRLINE_SECURITY_CHECK", false);
ConfigurableValue<bool>
skipCat33SubscriberSecurityCheck("REX_FARE_SELECTOR_SVC",
                                 "SKIP_CAT_33_SUBSCRIBER_SECURITY_CHECK",
                                 false);
}

void
RefundPricingTrx::prepareRequest()
{
  RefundPricingRequest* request = nullptr;
  _dataHandle.get(request);
  if (request)
  {
    request->setTrx(this);
    _request = request;
  }
}

void
RefundPricingTrx::set(const RexBaseTrx::RequestTypes& reqTypes)
{
  _reqType = reqTypes.first;

  if (reqTypes.second == AGENT_PRICING_MASK)
    _secondaryExcReqType = reqTypes.second;
}

void
RefundPricingTrx::setUpSkipSecurityForExcItin()
{
  _skipSecurityForExcItin = getRequest()->ticketingAgent()->tvlAgencyPCC().empty()
                                ? skipCat33AirlineSecurityCheck.getValue()
                                : skipCat33SubscriberSecurityCheck.getValue();
}

const DateTime&
RefundPricingTrx::getRuleApplicationDate(const CarrierCode& govCarrier) const
{
  return dataHandle().getVoluntaryRefundsConfig(
      govCarrier, currentTicketingDT(), getCurrTktDateSeq());
}

bool
RefundPricingTrx::repriceWithSameFareDate()
{
  return _trxPhase == RexBaseTrx::PRICE_NEWITIN_PHASE;
}

void
RefundPricingTrx::setAnalyzingExcItin(const bool isAnalyzingExcItin)
{
  _workingItin = isAnalyzingExcItin ? (std::vector<Itin*>*)(&_exchangeItin) : &_itin;
  _analyzingExcItin = isAnalyzingExcItin;
  _options->fbcSelected() = isAnalyzingExcItin;
}

std::vector<TravelSeg*>&
RefundPricingTrx::travelSeg()
{
  return _fullRefund ? _exchangeItin.front()->travelSeg() : _travelSeg;
}

const std::vector<TravelSeg*>&
RefundPricingTrx::travelSeg() const
{
  return _fullRefund ? _exchangeItin.front()->travelSeg() : _travelSeg;
}

void
RefundPricingTrx::insertOption(const PaxTypeFare* ptf, const VoluntaryRefundsInfo* rec3)
{
  _refundOptions.insert(std::make_pair(ptf, rec3));
}

size_t
RefundPricingTrx::refundOptions(const PaxTypeFare* ptf)
{
  return _refundOptions.count(ptf);
}

namespace
{
struct HasRepriceInd
{
  HasRepriceInd(Indicator ind) : _repriceInd(ind) {}
  bool operator()(const RefundPermutation* perm) const
  {
    return perm->repriceIndicator() == _repriceInd;
  }

private:
  Indicator _repriceInd;
};
}

std::vector<FareMarket::RetrievalInfo*>
RefundPricingTrx::getPermutationsRetrievalInfo() const
{
  typedef FareMarket::RetrievalInfo RetrievalInfo;
  std::vector<RetrievalInfo*> info;
  if (std::find_if(_permutations.begin(),
                   _permutations.end(),
                   HasRepriceInd(RefundPermutation::HISTORICAL_TICKET_BASED)) !=
      _permutations.end())
    info.push_back(
        RetrievalInfo::construct(*this, originalTktIssueDT(), FareMarket::RetrievHistorical));
  if (std::find_if(_permutations.begin(),
                   _permutations.end(),
                   HasRepriceInd(RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED)) !=
      _permutations.end())
    info.push_back(RetrievalInfo::construct(
        *this, _exchangeItin.front()->travelCommenceDate(), FareMarket::RetrievTvlCommence));
  return info;
}

void
RefundPricingTrx::getRec2Cat10WithVariousRetrieveDates(
    std::vector<MergedFareMarket*>& mergedFareMarketVect, GetRec2Cat10Function getRec2Cat10)
{
  if (needRetrieveHistoricalFare())
  {
    setFareApplicationDT(originalTktIssueDT());
    getRec2Cat10(*this, mergedFareMarketVect);
  }

  if (needRetrieveTvlCommenceFare())
  {
    DateTime& commenceDate = exchangeItin().front()->travelCommenceDate();
    if (commenceDate != DateTime::emptyDate())
    {
      setFareApplicationDT(commenceDate);
      getRec2Cat10(*this, mergedFareMarketVect);
    }
  }
  setFareApplicationDT(originalTktIssueDT());
}

namespace
{
struct CloneFareMarket
{
  CloneFareMarket(DataHandle& dh) : _dh(dh) {}

  FareMarket* operator()(FareMarket* sourceFm) const
  {
    FareMarket* fm = nullptr;
    _dh.get(fm);
    sourceFm->clone(*fm);
    return fm;
  }

protected:
  DataHandle& _dh;
};
}

void
RefundPricingTrx::prepareNewFareMarkets()
{
  std::vector<FareMarket::RetrievalInfo*> retrievalInfo = getPermutationsRetrievalInfo();

  typedef std::vector<FareMarket::RetrievalInfo*>::const_iterator riIt;
  for (riIt i = retrievalInfo.begin(); i != retrievalInfo.end(); ++i)
    _fareRetrievalFlags.set((*i)->_flag);

  std::vector<FareMarket*>& fm = newItin().front()->fareMarket();

  typedef std::vector<FareMarket*>::const_iterator It;
  std::vector<FareMarket*> clonedFm;
  switch (retrievalInfo.size())
  {
  case 2:
    clonedFm.reserve(fm.size());
    std::transform(
        fm.begin(), fm.end(), std::back_inserter(clonedFm), CloneFareMarket(dataHandle()));
    for (It i = clonedFm.begin(); i != clonedFm.end(); ++i)
      (*i)->retrievalInfo() = retrievalInfo[1];
  case 1:
    for (It i = fm.begin(); i != fm.end(); ++i)
      (*i)->retrievalInfo() = retrievalInfo[0];
  }
  if (!clonedFm.empty())
    fm.insert(fm.end(), clonedFm.begin(), clonedFm.end());
}

void
RefundPricingTrx::setFullRefundWinningPermutation(const RefundPermutation& perm)
{
  if (!_fullRefundWinningPermutation ||
      _fullRefundWinningPermutation->overallPenalty(*this) > perm.overallPenalty(*this))
    _fullRefundWinningPermutation = &perm;
}

void
RefundPricingTrx::initalizeForRedirect(std::vector<FareMarket*>& fm, std::vector<TravelSeg*>& ts)
    const
{
  fm = newItin().front()->fareMarket();
  ts = newItin().front()->travelSeg();
}

namespace
{
class SkipRefundPlusUpRules : public std::unary_function<uint16_t, bool>
{
public:
  bool operator()(uint16_t rule)
  {
    switch (rule)
    {
    case RuleConst::STOPOVER_RULE:
    case RuleConst::TRANSFER_RULE:
      return true;
    case RuleConst::SURCHARGE_RULE:
      return _skipCat12;
    }
    return false;
  }

  SkipRefundPlusUpRules(bool skipCat12) : _skipCat12(skipCat12) {}

private:
  bool _skipCat12;
};

} // namespace

void
RefundPricingTrx::skipRulesOnExcItin(std::vector<uint16_t>& categorySequence) const
{
  RexBaseTrx::skipRulesOnExcItin(categorySequence);

  if (_trxPhase != RexBaseTrx::PRICE_NEWITIN_PHASE && _arePenaltiesAndFCsEqualToSumFromFareCalc)
    categorySequence.erase(
        remove_if(categorySequence.begin(),
                  categorySequence.end(),
                  // if vector contain any surcharges - do not remove cat12 from categorySequence
                  SkipRefundPlusUpRules(_exchangeOverrides.surchargeOverride().empty())),
        categorySequence.end());
}

Money
RefundPricingTrx::convertCurrency(const Money& source, const CurrencyCode& targetCurr) const
{
  const CurrencyCode& baseFareCurr =
      static_cast<const RexPricingOptions&>(*_options).excBaseFareCurrency();

  // X-_-X || X-X-X
  if (source.code() == targetCurr && (baseFareCurr.empty() || baseFareCurr == targetCurr))
    return source;

  Money target(targetCurr);
  CurrencyConversionFacade currConvFacade;

  // X-_-Y || X-Y-Y || X-X-Y
  if (source.code() != targetCurr &&
      (baseFareCurr.empty() || baseFareCurr == targetCurr || baseFareCurr == source.code()))
  {
    currConvFacade.convert(target,
                           source,
                           const_cast<RefundPricingTrx&>(*this),
                           exchangeItin().front()->useInternationalRounding());
    return target;
  }

  Money transferTarget(baseFareCurr);

  // finally X-Y-X || X-Y-Z
  currConvFacade.convert(transferTarget,
                         source,
                         const_cast<RefundPricingTrx&>(*this),
                         exchangeItin().front()->useInternationalRounding());

  currConvFacade.convert(target,
                         transferTarget,
                         const_cast<RefundPricingTrx&>(*this),
                         exchangeItin().front()->useInternationalRounding());

  return target;
}

void
RefundPricingTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  std::string tmpResponse(ere.message());
  if (ere.code() > 0 && ere.message().empty())
  {
    tmpResponse = "UNKNOWN EXCEPTION";
  }
  PricingResponseFormatter formatter;
  response = formatter.formatResponse(tmpResponse, false, *this, nullptr, ere.code());
}

bool
RefundPricingTrx::convert(std::string& response)
{
  XMLConvertUtils::tracking(*this);
  if (!taxRequestToBeReturnedAsResponse().empty())
  {
    response = taxRequestToBeReturnedAsResponse();
    return true;
  }

  LOG4CXX_DEBUG(logger, "Doing RefundPricingTrx response");
  response = XMLConvertUtils::rexPricingTrxResponse(*this);
  LOG4CXX_DEBUG(logger, "response: " << response);

  return true;
}

} // tse namespace
