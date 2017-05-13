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

#include "DataModel/RexPricingTrx.h"

#include "BookingCode/Cat31FareBookingCodeValidator.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConverter.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FlownStatusCheck.h"
#include "Common/Logger.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseUtil.h"
#include "DataModel/CsoPricingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RexPricingRequest.h"
#include "DBAccess/Loc.h"
#include "DBAccess/ReissueSequence.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/XMLConvertUtils.h"

namespace tse
{
FALLBACK_DECL(inactiveNonRefAmountForNonAbacusUsers);
FALLBACK_DECL(fallbackNonRefAmountOptimizationForAbacus);
FALLBACK_DECL(conversionDateSSDSP1154);

namespace
{
ConfigurableValue<bool>
skipCat31AirlineSecurityCheck("REX_FARE_SELECTOR_SVC", "SKIP_CAT_31_AIRLINE_SECURITY_CHECK", false);
ConfigurableValue<bool>
skipCat31SubscriberSecurityCheck("REX_FARE_SELECTOR_SVC",
                                 "SKIP_CAT_31_SUBSCRIBER_SECURITY_CHECK",
                                 false);
}
Logger
RexPricingTrx::_logger("atseintl.RexPricing.RexPricingTrx");
Logger
RexPricingTrx::_faresInfoLogger("atseintl.RexPricing.RexPricingTrx.FareInfo");

FareBytesData::FareBytesData(ProcessTagInfo* pti)
{
  add(pti);
}

RexPricingTrx::RexPricingTrx()
{
  _reqType = AUTOMATED_REISSUE;
  _excTrxType = PricingTrx::AR_EXC_TRX;
}

const DateTime&
RexPricingTrx::originalTktIssueDT() const
{
  return _trxPhase == RexBaseTrx::PRICE_NEWITIN_PHASE ? BaseExchangeTrx::originalTktIssueDT()
                                                      : _originalTktIssueDT;
}

void
FareBytesData::add(ProcessTagInfo* pti)
{
  _processTags.insert(pti);

  if (!pti->reissueSequence()->orig())
  {
    _fareRulesApply = _fareTrfNumberApply = _fareClassCodeApply = _fareNormalSpecialApply =
        _excludePrivateApply = _owrtApply = _fareAmountApply = _sameApply = false;

    return;
  }

  const ReissueSequenceW& seq = *pti->reissueSequence();

  if (_fareRulesApply && _fareRulesApply && seq.ruleInd() == NOT_APPLICABLE)
    _fareRulesApply = false;

  if (_fareTrfNumberApply && seq.ruleTariffNo() == 0)
    _fareTrfNumberApply = false;

  if (_fareClassCodeApply && seq.fareTypeInd() == NOT_APPLICABLE)
    _fareClassCodeApply = false;

  if (_fareNormalSpecialApply && seq.normalspecialInd() == NOT_APPLICABLE)
    _fareNormalSpecialApply = false;

  if (_excludePrivateApply && seq.excludePrivate() == NOT_APPLICABLE)
    _excludePrivateApply = false;

  if (_owrtApply && seq.owrt() == ALL_WAYS)
    _owrtApply = false;

  if (_fareAmountApply && seq.fareAmtInd() == NOT_APPLICABLE)
    _fareAmountApply = false;

  if (_sameApply && seq.sameInd() == NOT_APPLICABLE)
    _sameApply = false;

  if (_fareAmountApply && pti->processTag() != REISSUE_DOWN_TO_LOWER_FARE)
    _processTagsForAmount = true;
}

ROEDateSetter::ROEDateSetter(PricingTrx& pricingTrx, FarePath& fp)
  : _pricingTrx(pricingTrx), _fp(fp)
{
  if (UNLIKELY(
          RexPricingTrx::isRexTrxAndNewItin(_pricingTrx) &&
          static_cast<RexBaseTrx&>(_pricingTrx).applyReissueExchange() &&
          !static_cast<RexBaseTrx&>(_pricingTrx).newItinSecondROEConversionDate().isEmptyDate()))
  {
    RexBaseTrx& rexBaseTrx = static_cast<RexBaseTrx&>(_pricingTrx);
    if (useHistoricalRoeDate())
    {
      const DateTime& historicalDate = rexBaseTrx.previousExchangeDT().isEmptyDate()
                                           ? rexBaseTrx.originalTktIssueDT()
                                           : rexBaseTrx.previousExchangeDT();
      if (rexBaseTrx.newItinROEConversionDate() != historicalDate)
      {
        rexBaseTrx.useSecondROEConversionDate() = true;
        fp.useSecondRoeDate() = true;
        _recalculate = true;
      }
    }
    else
    {
      if (rexBaseTrx.newItinROEConversionDate() != rexBaseTrx.currentTicketingDT())
      {
        rexBaseTrx.useSecondROEConversionDate() = true;
        fp.useSecondRoeDate() = true;
        _recalculate = true;
      }
    }
  }
}

ROEDateSetter::~ROEDateSetter()
{
  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(_pricingTrx)))
  {
    RexBaseTrx& rexBaseTrx = static_cast<RexBaseTrx&>(_pricingTrx);
    rexBaseTrx.useSecondROEConversionDate() = false;
  }
}

bool
ROEDateSetter::useHistoricalRoeDate() const
{
  bool containsCurrent = false;
  bool containsHistorical = false;

  for (const PricingUnit* pricingUnit : _fp.pricingUnit())
  {
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      if (fareUsage->paxTypeFare()->retrievalFlag() & FareMarket::RetrievCurrent)
        containsCurrent = true;
      else
        containsHistorical = true;

      if (containsCurrent && containsHistorical)
        return _fp.isReissue();
    }
  }

  return containsHistorical;
}

void
ROEDateSetter::reCalculateNucAmounts()
{
  if (UNLIKELY(_recalculate))
  {
    _fp.setTotalNUCAmount(0);
    std::vector<PricingUnit*>::const_iterator puI, puE;
    std::vector<FareUsage*>::const_iterator fuI, fuE;
    for (puI = _fp.pricingUnit().begin(), puE = _fp.pricingUnit().end(); puI != puE; ++puI)
    {
      (*puI)->setTotalPuNucAmount(0.0);
      for (fuI = (*puI)->fareUsage().begin(), fuE = (*puI)->fareUsage().end(); fuI != fuE; fuI++)
      {
        (*puI)->setTotalPuNucAmount((*puI)->getTotalPuNucAmount() + (*fuI)->totalFareAmount());
      }
      _fp.increaseTotalNUCAmount((*puI)->getTotalPuNucAmount());
    }
  }
}

RaiiRexFarePathPlusUps::RaiiRexFarePathPlusUps(PricingTrx& trx, FarePath& fp, bool& valid)
  : _rexBaseTrx(
        (trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX)
            ? static_cast<RexBaseTrx*>(&trx)
            : nullptr),
    _valid(valid)
{
  if (UNLIKELY(_rexBaseTrx))
  {
    if (_rexBaseTrx->excTrxType() == PricingTrx::AR_EXC_TRX &&
        _rexBaseTrx->trxPhase() != RexBaseTrx::PRICE_NEWITIN_PHASE)
    {
      if (boost::indeterminate(
              static_cast<const RexPricingTrx&>(*_rexBaseTrx).excTktNonRefundable()))
        determineNonRefundable(fp);

      else
        _nonRefundable = static_cast<const RexPricingTrx&>(*_rexBaseTrx).excTktNonRefundable();
    }
  }
}

bool
RaiiRexFarePathPlusUps::plusUpsNeeded() const
{
  if (LIKELY(!_rexBaseTrx || _rexBaseTrx->trxPhase() == RexBaseTrx::PRICE_NEWITIN_PHASE))
    return true;

  if (_rexBaseTrx->isPlusUpCalculationNeeded())
  {
    if (_rexBaseTrx->excTrxType() == PricingTrx::AF_EXC_TRX || boost::indeterminate(_nonRefundable))
      return true;
  }

  return false;
}

RaiiRexFarePathPlusUps::~RaiiRexFarePathPlusUps()
{
  if (UNLIKELY(
          _valid && _rexBaseTrx && _rexBaseTrx->excTrxType() == PricingTrx::AR_EXC_TRX &&
          _rexBaseTrx->trxPhase() != RexBaseTrx::PRICE_NEWITIN_PHASE &&
          boost::indeterminate(static_cast<RexPricingTrx&>(*_rexBaseTrx).excTktNonRefundable()) &&
          !(boost::indeterminate(_nonRefundable))))
    static_cast<RexPricingTrx&>(*_rexBaseTrx).setExcTktNonRefundable(_nonRefundable);
}

void
RaiiRexFarePathPlusUps::determineNonRefundable(const FarePath& fp)
{
  bool firstFUNonRefundable = fp.pricingUnit().front()->fareUsage().front()->isNonRefundable();

  for (const PricingUnit* pu : fp.pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      if (firstFUNonRefundable != fu->isNonRefundable())
      {
        _nonRefundable = boost::indeterminate;
        return;
      }
    }
  }

  _nonRefundable = firstFUNonRefundable;
}

void
RexPricingTrx::prepareRequest()
{
  RexPricingRequest* request = nullptr;
  _dataHandle.get(request);
  if (request)
  {
    request->setTrx(this);
    _request = request;
  }
}

void
RexPricingTrx::set(const RexBaseTrx::RequestTypes& reqTypes)
{
  _reqType = reqTypes.first;

  if (reqTypes.second == PARTIAL_EXCHANGE || reqTypes.second == FULL_EXCHANGE ||
      reqTypes.second == TAG_10_EXCHANGE)
    _secondaryExcReqType = reqTypes.second;
}

void
RexPricingTrx::setUpSkipSecurityForExcItin()
{
  _skipSecurityForExcItin = getRequest()->ticketingAgent()->tvlAgencyPCC().empty()
                                ? skipCat31AirlineSecurityCheck.getValue()
                                : skipCat31SubscriberSecurityCheck.getValue();
}

const DateTime&
RexPricingTrx::getRuleApplicationDate(const CarrierCode& govCarrier) const
{
  return dataHandle().getVoluntaryChangesConfig(
      govCarrier, currentTicketingDT(), getCurrTktDateSeq());
}

void
RexPricingTrx::setTktValidityDate()
{
  if (!_exchangeItin.empty())
  {
    ExcItin* itin = _exchangeItin.front();

    DateTime originalBasis;

    FlownStatusCheck check(*itin);
    if (check.isTotallyUnflown())
    {
      if (applyReissueExchange() && !previousExchangeDT().isEmptyDate())
        originalBasis = _previousExchangeDT;
      else
        originalBasis = _originalTktIssueDT;
    }
    else if (check.isPartiallyFlown())
    {
      originalBasis = itin->travelSeg().front()->departureDT();
    }
    else
    {
      return;
    }

    originalBasis = originalBasis.addYears(1);
    itin->setTktValidityDate(originalBasis.year(), originalBasis.month(), originalBasis.day());
  }
}

const PenaltyFee*
RexPricingTrx::getPenaltyFee(const PaxTypeFare* ptf,
                             const VoluntaryChangesInfo* voluntaryChangeInfo) const
{
  if (ptf == nullptr || voluntaryChangeInfo == nullptr)
    return nullptr;

  PenaltyFeeMap::const_iterator iter = _penaltyFees.find(std::make_pair(ptf, voluntaryChangeInfo));
  if (iter != _penaltyFees.end())
    return iter->second;

  return nullptr;
}

bool
RexPricingTrx::matchFareRetrievalDate(const FareMarket& fm)
{
  if (_analyzingExcItin)
    return true;

  // Enhance later
  return true;
}

bool
RexPricingTrx::matchFareRetrievalDate(const PaxTypeFare& paxTypeFare)
{
  if (_analyzingExcItin || _dataHandle.useTLS())
    return true;

  if ((paxTypeFare.retrievalInfo() != nullptr) &&
      (paxTypeFare.retrievalInfo()->_date == _fareApplicationDT))
    return true;

  return false;
}

void
RexPricingTrx::createPricingTrxForCSO(bool runForDiagnostic)
{
  dataHandle().get(_pricingTrxForCSO);
  if (!_pricingTrxForCSO)
    return;
  try
  {
    if (_pricingTrxForCSO->initialize(*this, runForDiagnostic))
      return;
  }
  catch (const ErrorResponseException& ex)
  {
    _csoPricingErrorCode = ex.code();
    _csoPricingErrorMsg = ex.message();
    LOG4CXX_WARN(RexPricingTrx::_logger,
                 "Exception:" << ex.message() << " - Cancel and start over pricing failed");
  }
  catch (const std::exception& e)
  {
    _csoPricingErrorCode = ErrorResponseException::UNKNOWN_EXCEPTION;
    _csoPricingErrorMsg = e.what();
    LOG4CXX_WARN(RexPricingTrx::_logger,
                 "Exception:" << e.what() << " - Cancel and start over pricing failed");
  }
  catch (...)
  {
    _csoPricingErrorMsg = "UNKNOWN EXCEPTION - CANCEL AND START OVER PRICING FAILED";
    _csoPricingErrorCode = ErrorResponseException::UNKNOWN_EXCEPTION;
    LOG4CXX_WARN(RexPricingTrx::_logger, _csoPricingErrorMsg);
  }

  _pricingTrxForCSO = nullptr;
}

void
RexPricingTrx::calculateNonrefundableAmountForValidFarePaths()
{
  if (exchangeItin().front()->farePath().empty())
    return;
  if (!exchangeItin().front()->farePath().front())
    return;

  const FarePath& excFarePath = *exchangeItin().front()->farePath().front();
  FarePath* fPath[2] = {lowestRebookedFarePath(), lowestBookedFarePath()};

  for (FarePath* fp : fPath)
  {
    if (!fp)
      continue;

    ExchangeUtil::RaiiProcessingDate conversionDateSetter(
        *this, *fp, fallback::conversionDateSSDSP1154(this));

    if (isPlusUpCalculationNeeded() && fp->isExcTicketHigher())
    {
      NonRefundableUtil nru(*this);
      nru.calculateNonRefundableAmount(*fp);
    }
    else
      fp->calculateNonrefundableAmount(excFarePath, *this);

    fp->updateTktEndorsement();
  }
}

bool
RexPricingTrx::repriceWithSameFareDate()
{
  return (!_repriceWithDiffDates && _trxPhase == RexBaseTrx::PRICE_NEWITIN_PHASE);
}

void
RexPricingTrx::buildFareInfoMessage(const std::set<FareMarket::FareRetrievalFlags>& fareTypes,
                                    std::string& result) const
{
  std::set<FareMarket::FareRetrievalFlags>::const_iterator iter = fareTypes.begin();
  const std::set<FareMarket::FareRetrievalFlags>::const_iterator iterEnd = fareTypes.end();

  for (; iter != iterEnd; ++iter)
  {
    result += FareMarket::fareRetrievalFlagToStr(*iter);
  }
}

void
RexPricingTrx::logFaresInfo() const
{
  std::set<FareMarket::FareRetrievalFlags> fareTypes;

  std::vector<Itin*>::const_iterator itinIter = _itin.begin();
  const std::vector<Itin*>::const_iterator itinIterE = _itin.end();

  for (; itinIter != itinIterE; ++itinIter)
  {
    const Itin* itin = *itinIter;
    std::vector<FareMarket*>::const_iterator fmIter = itin->fareMarket().begin();
    const std::vector<FareMarket*>::const_iterator fmIterE = itin->fareMarket().end();

    for (; fmIter != fmIterE; ++fmIter)
    {
      fareTypes.insert((*fmIter)->retrievalFlag());
    }

    std::string result;
    buildFareInfoMessage(fareTypes, result);

    LOG4CXX_INFO(_faresInfoLogger,
                 "Retrieved fare types: " << result
                                          << " - TXN CLIENT ID: " << _billing->clientTransactionID()
                                          << " - TXN ID: " << _billing->transactionID());
  }
}

void
RexPricingTrx::getFareTypes(FarePath& farePath, std::set<FareMarket::FareRetrievalFlags>& fareTypes)
    const
{
  std::vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puIterEnd = farePath.pricingUnit().end();

  std::vector<FareUsage*>::const_iterator fuIter;
  std::vector<FareUsage*>::const_iterator fuIterEnd;

  for (; puIter != puIterEnd; ++puIter)
  {
    fuIter = (*puIter)->fareUsage().begin();
    fuIterEnd = (*puIter)->fareUsage().end();

    for (; fuIter != fuIterEnd; ++fuIter)
      fareTypes.insert((*fuIter)->paxTypeFare()->retrievalFlag());
  }
}

void
RexPricingTrx::logLowestResultInfo(FarePath& farePath, const std::string& solutType) const
{
  std::set<FareMarket::FareRetrievalFlags> fareTypes;

  getFareTypes(farePath, fareTypes);

  std::string result;
  buildFareInfoMessage(fareTypes, result);

  LOG4CXX_INFO(_faresInfoLogger,
               "Lowest " << solutType << " result fare types: " << result
                         << " - TXN CLIENT ID: " << _billing->clientTransactionID()
                         << " - TXN ID: " << _billing->transactionID());
}

void
RexPricingTrx::logLowestResultInfo() const
{
  if (_lowestRebookedFarePath)
  {
    logLowestResultInfo(*_lowestRebookedFarePath, "rebook");
  }

  if (_lowestBookedFarePath)
  {
    logLowestResultInfo(*_lowestBookedFarePath, "book");
  }
}

bool
RexPricingTrx::isNeedDetermineBaseFareForExcFarePath(uint16_t itinIndex)
{
  if (exchangeItin().front()->farePath().empty())
    return false;
  if (!exchangeItin().front()->farePath().front())
    return false;
  if (processTagPermutations(itinIndex).empty())
    return false;

  bool need = false;
  FarePath* fPath[2] = {lowestRebookedFarePath(), lowestBookedFarePath()};
  for (int i = 0; i < 2; ++i)
    if (fPath[i])
    {
      ExchangeUtil::RaiiProcessingDate conversionDateSetter(
          *this, *fPath[i], fallback::conversionDateSSDSP1154(this));
      fPath[i]->isExcTicketHigher() = false;

      if (ProcessTagPermutation::requiresComparisonOfExcAndNewNonRef(
              fPath[i]->lowestFee31Perm()->getEndorsementByte()))
      {
        if (isPlusUpCalculationNeeded())
        {
          NonRefundableUtil nru(*this);

          fPath[i]->isExcTicketHigher() =
              nru.excNonRefundableNucAmt() > fPath[i]->getNonrefundableAmountInNUC(*this);
        }
        else
        {
          MoneyAmount amt = fPath[i]->getNonrefundableAmountInNUC(*this);
          MoneyAmount excAmt = 0.0;
          excAmt = exchangeItin().front()->getNonRefAmountInNUC().value();

          fPath[i]->isExcTicketHigher() = excAmt > amt;
        }
      }

      need = need || fPath[i]->isExcTicketHigher();
    }
  return need;
}

class AddFlownSegRetrievalFlag : public std::unary_function<TravelSeg*, void>
{
public:
  AddFlownSegRetrievalFlag(std::map<int16_t, FareMarket::FareRetrievalFlags>& map,
                           FareMarket::FareRetrievalFlags& flag)
    : _map(map), _flag(flag)
  {
  }

  void operator()(TravelSeg* tvlSeg) { _map.insert(std::make_pair(tvlSeg->segmentOrder(), _flag)); }

  std::map<int16_t, FareMarket::FareRetrievalFlags>& _map;
  FareMarket::FareRetrievalFlags& _flag;
};

void
RexPricingTrx::getFlownSegRetrievalMap(
    std::map<int16_t, FareMarket::FareRetrievalFlags>& flownSegRetrievalMap)
{
  std::map<const FareMarket*, FareMarket::FareRetrievalFlags>::iterator excFlownFmIter =
      _excFlownFcFareAppl.begin();
  for (; excFlownFmIter != _excFlownFcFareAppl.end(); ++excFlownFmIter)
  {
    const FareMarket* excFlownFm = excFlownFmIter->first;

    for_each(excFlownFm->travelSeg().begin(),
             excFlownFm->travelSeg().end(),
             AddFlownSegRetrievalFlag(flownSegRetrievalMap, excFlownFmIter->second));
  }
}

void
RexPricingTrx::setAdjustedTravelDate()
{
  if (!TrxUtil::isAdjustRexTravelDateEnabled())
  {
    _adjustedTravelDate = _travelDate;
    return;
  }
  bool hasUnflown(false);
  for (std::vector<TravelSeg*>::const_iterator i = _travelSeg.begin(); i != _travelSeg.end(); i++)
  {
    if ((*i)->departureDT() > _currentTicketingDT && (*i)->segmentType() != Arunk)
    {
      _adjustedTravelDate = (*i)->departureDT();
      hasUnflown = true;
      break;
    }
  }

  if (!hasUnflown)
  {
    _adjustedTravelDate = _currentTicketingDT;
  }
}

const DateTime&
RexPricingTrx::adjustedTravelDate(const DateTime& travelDate) const
{
  return (_excTrxType == PricingTrx::AR_EXC_TRX && TrxUtil::isAdjustRexTravelDateEnabled())
             ? std::max(_dataHandle.ticketDate(), travelDate)
             : travelDate;
}

const DateTime&
RexPricingTrx::travelDate() const
{
  return adjustedTravelDate(_adjustedTravelDate);
}

void
RexPricingTrx::setAnalyzingExcItin(const bool isAnalyzingExcItin)
{
  _workingItin = isAnalyzingExcItin ? (std::vector<Itin*>*)(&_exchangeItin) : &_itin;
  _analyzingExcItin = isAnalyzingExcItin;
  _options->fbcSelected() = isAnalyzingExcItin;
  if (isAnalyzingExcItin)
  {
    dataHandle().setTicketDate(_originalTktIssueDT);
    _fareApplicationDT = _originalTktIssueDT;
  }
  else
  {
    dataHandle().setTicketDate(_currentTicketingDT);
    _fareApplicationDT = _currentTicketingDT;
  }
}

void
RexPricingTrx::getRec2Cat10WithVariousRetrieveDates(
    std::vector<MergedFareMarket*>& mergedFareMarketVect, GetRec2Cat10Function getRec2Cat10)
{
  bool needRetrieveCurrent = needRetrieveCurrentFare();

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
    else
      needRetrieveCurrent = true;
  }

  if (needRetrieveKeepFare())
  {
    std::set<DateTime> keepFareDates;

    NewItinKeepFareMap::iterator keepFareMapToNewFmIter;
    for (keepFareMapToNewFmIter = newItinKeepFares().begin();
         keepFareMapToNewFmIter != newItinKeepFares().end();
         ++keepFareMapToNewFmIter)
    {
      const PaxTypeFare* excItinFare = keepFareMapToNewFmIter->first;
      FareMarket* newItinFm = keepFareMapToNewFmIter->second;
      if (excItinFare == nullptr || newItinFm == nullptr)
        continue;

      const DateTime& retrievalDate = excItinFare->retrievalDate();

      if ((((retrievalDate == originalTktIssueDT()) && needRetrieveHistoricalFare()) ||
           ((retrievalDate == exchangeItin().front()->travelCommenceDate()) &&
            needRetrieveTvlCommenceFare())))
        continue;

      if (keepFareDates.find(retrievalDate) == keepFareDates.end())
      {
        setFareApplicationDT(retrievalDate);
        getRec2Cat10(*this, mergedFareMarketVect);
        keepFareDates.insert(retrievalDate);
      }
    }
  }

  setFareApplicationDT(currentTicketingDT());

  if (needRetrieveCurrent)
  {
    getRec2Cat10(*this, mergedFareMarketVect);
  }
}

void
RexPricingTrx::filterFareMarketByFlownFareAppl()
{
  std::map<int16_t, FareMarket::FareRetrievalFlags> flownSegRetrievalMap;
  getFlownSegRetrievalMap(flownSegRetrievalMap);

  std::vector<Itin*>::iterator itinIter = itin().begin();

  for (; itinIter != itin().end(); ++itinIter)
  {
    Itin& itin = **itinIter;
    std::vector<FareMarket*>& fareMarkets = itin.fareMarket();
    std::vector<FareMarket*>::iterator iter = fareMarkets.begin();
    for (; iter != fareMarkets.end(); ++iter)
    {
      FareMarket& fm = **iter;
      if (fm.travelSeg().front()->unflown())
        continue;

      std::vector<TravelSeg*>::iterator tvlSegIter = fm.travelSeg().begin();
      for (; tvlSegIter != fm.travelSeg().end(); ++tvlSegIter)
      {
        if ((*tvlSegIter)->unflown())
          break;

        std::map<int16_t, FareMarket::FareRetrievalFlags>::iterator flagIter =
            flownSegRetrievalMap.find(itin.segmentOrder(*tvlSegIter));
        if ((flagIter != flownSegRetrievalMap.end()) && fm.retrievalInfo())
        {
          FareMarket::FareRetrievalFlags fmFlag = fm.retrievalInfo()->_flag;
          if (!((flagIter->second & fmFlag) || ((flagIter->second & FareMarket::RetrievKeep) &&
                                                (fmFlag == FareMarket::RetrievNone))))
          {
            fm.setBreakIndicator(true);
            break;
          }
        }
      }
    }
  }
}

void
RexPricingTrx::initalizeForRedirect(std::vector<FareMarket*>& fm, std::vector<TravelSeg*>& ts) const
{
  fm = fareMarket();
  ts = travelSeg();
}

void
RexPricingTrx::insert(ProcessTagPermutation& perm)
{
  if (perm.tag1StopYonly())
  {
    if (!tag1PricingSvcCallStatus())
      setTag1PricingSvcCallStatus() = TAG1PERMUTATION;

    processTagPermutations().push_back(&perm);
  }

  else if (tag1PricingSvcCallStatus() == NONE)
    processTagPermutations().push_back(&perm);

  else
  {
    setTag1PricingSvcCallStatus() = NONE;
    processTagPermutations().push_back(&perm);
    setTag1PricingSvcCallStatus() = TAG1PERMUTATION;
  }
}

bool
RexPricingTrx::separateTag7Permutations()
{
  std::vector<ProcessTagPermutation*> tag7perms;

  setTag1PricingSvcCallStatus() = NONE;
  std::remove_copy_if(processTagPermutations().begin(),
                      processTagPermutations().end(),
                      std::back_inserter(tag7perms),
                      std::not1(std::mem_fun(&ProcessTagPermutation::hasTag7only)));

  if (tag7perms.empty())
    return false;

  setTag1PricingSvcCallStatus() = TAG7PERMUTATION;
  processTagPermutations().swap(tag7perms);

  markFareRetrievalMethodCurrent(true);

  return true;
}

bool
RexPricingTrx::shouldCallPricingSvcSecondTime()
{
  if (lowestRebookedFarePath() || lowestBookedFarePath())
    return separateTag7Permutations();

  setTag1PricingSvcCallStatus() = NONE;
  return !processTagPermutations().empty();
}

void
RexPricingTrx::prepareForSecondPricingSvcCall()
{
  lowestRebookedFarePath() = nullptr;
  lowestBookedFarePath() = nullptr;
  curNewItin()->farePath().clear();
  _reissuePricingErrorCode = ErrorResponseException::NO_ERROR;
  _reissuePricingErrorMsg = "";
}

void
RexPricingTrx::validSolutionsPlacement(std::pair<FarePath*, FarePath*> rebookedBooked)
{
  if (!rebookedBooked.first && !rebookedBooked.second)
    return;

  switchIfCheaperOrEqual(rebookedBooked.first, lowestRebookedFarePath());
  switchIfCheaperOrEqual(rebookedBooked.second, lowestBookedFarePath());

  switch (curNewItin()->farePath().size())
  {
  case 0:
    if (rebookedBooked.first)
      curNewItin()->farePath().push_back(rebookedBooked.first);
    if (rebookedBooked.second)
      curNewItin()->farePath().push_back(rebookedBooked.second);
    break;

  case 1:
    if (curNewItin()->farePath().front()->rebookClassesExists())
    {
      switchIfCheaperOrEqual(rebookedBooked.first, curNewItin()->farePath().front());
      if (rebookedBooked.second)
        curNewItin()->farePath().push_back(rebookedBooked.second);
    }
    else
    {
      switchIfCheaperOrEqual(rebookedBooked.second, curNewItin()->farePath().front());
      if (rebookedBooked.first)
        curNewItin()->farePath().insert(curNewItin()->farePath().begin(), rebookedBooked.first);
    }
    break;

  case 2:
    switchIfCheaperOrEqual(rebookedBooked.first, curNewItin()->farePath().front());
    switchIfCheaperOrEqual(rebookedBooked.second, curNewItin()->farePath().back());
    break;
  }

  disableRebookedIfGreaterEqualThanBooked();
}

void
RexPricingTrx::disableRebookedIfGreaterEqualThanBooked()
{
  if (lowestRebookedFarePath() && lowestBookedFarePath())
  {
    _reissuePricingErrorCode = ErrorResponseException::NO_ERROR;
    _reissuePricingErrorMsg = "";

    if (!FarePath::Less()(lowestRebookedFarePath(), lowestBookedFarePath()))
      lowestRebookedFarePath() = nullptr;
  }
}

void
RexPricingTrx::switchIfCheaperOrEqual(FarePath* candidate, FarePath*& current)
{
  if ((candidate && !current) || (candidate && current && !FarePath::Less()(current, candidate)))
    current = candidate;
}

const SeasonalityDOW*
RexPricingTrx::getSeasonalityDOW(const VendorCode& vendor,
                                 uint32_t tblItemNo,
                                 const DateTime& applicationDate) const
{
  return dataHandle().getSeasonalityDOW(vendor, tblItemNo, originalTktIssueDT(), applicationDate);
}

bool
RexPricingTrx::expndKeepSameNation(const Loc& loc1, const Loc& loc2)
{
  return loc1.nation() == loc2.nation() ||
         (LocUtil::isDomesticUSCA(loc1) && LocUtil::isDomesticUSCA(loc2)) ||
         (LocUtil::isRussianGroup(loc1) && LocUtil::isRussianGroup(loc2));
}

namespace
{
class GreaterRetrievalDate
    : public std::binary_function<const RexPricingTrx::ExpndKeepMap::value_type&,
                                  const RexPricingTrx::ExpndKeepMap::value_type&,
                                  bool>
{
public:
  bool operator()(const RexPricingTrx::ExpndKeepMap::value_type& vt1,
                  const RexPricingTrx::ExpndKeepMap::value_type& vt2) const
  {
    return vt1.second->retrievalDate() < vt2.second->retrievalDate();
  }
};
}

const PaxTypeFare*
RexPricingTrx::getKeepPtf(const FareMarket& fareMarket) const
{
  RexPricingTrx::NewItinKeepFareMap::const_iterator found = std::find_if(
      newItinKeepFares().begin(),
      newItinKeepFares().end(),
      boost::bind(&RexPricingTrx::NewItinKeepFareMap::value_type::second, _1) == &fareMarket);

  if (found != newItinKeepFares().end())
    return found->first;

  std::pair<RexPricingTrx::ExpndKeepMapI, RexPricingTrx::ExpndKeepMapI> range =
      expndKeepMap().equal_range(&fareMarket);

  if (range.first == range.second)
    return nullptr;

  RexPricingTrx::ExpndKeepMapI maxDate =
      std::max_element(range.first, range.second, GreaterRetrievalDate());

  return maxDate->second;
}

bool
RexPricingTrx::isFareMarketNeededForMinFares(const FareMarket& fm) const
{
  const Itin& nItin = *newItin().front();

  if (!fm.travelSeg().back()->stopOver() && fm.travelSeg().back() != nItin.travelSeg().back())
    return false;

  std::vector<FareMarket*>::const_iterator fmIter = nItin.fareMarket().begin();
  const std::vector<FareMarket*>::const_iterator fmIterEnd = nItin.fareMarket().end();

  for (; fmIter != fmIterEnd; ++fmIter)
  {
    const FareMarket& possibleParentFM = **fmIter;
    const PaxTypeFare* keepPtf;

    if (possibleParentFM.travelSeg().size() <= fm.travelSeg().size() ||
        (keepPtf = getKeepPtf(possibleParentFM)) == nullptr)
      continue;

    std::vector<TravelSeg*>::const_iterator pos = std::search(possibleParentFM.travelSeg().begin(),
                                                              possibleParentFM.travelSeg().end(),
                                                              fm.travelSeg().begin(),
                                                              fm.travelSeg().end());

    if (pos != possibleParentFM.travelSeg().end())
      return true;
  }

  return false;
}

std::pair<const DateTime*, const Loc*>
RexPricingTrx::getLocAndDateForAdjustment() const
{
  const RexBaseRequest& rexBaseRequest = static_cast<const RexBaseRequest&>(*getRequest());
  const DateTime* dateToAdjust = nullptr;
  const Loc* locOfAdjust = nullptr;
  // subsequent exchange
  if (!_previousExchangeDT.isEmptyDate())
  {
    dateToAdjust = &_previousExchangeDT;

    if (rexBaseRequest.prevTicketIssueAgent() &&
        !rexBaseRequest.prevTicketIssueAgent()->agentCity().empty())
      locOfAdjust = rexBaseRequest.prevTicketIssueAgent()->agentLocation();
  }

  // first exchange
  else
  {
    dateToAdjust = &_originalTktIssueDT;

    if (rexBaseRequest.getOriginalTicketAgentLocation())
      locOfAdjust = rexBaseRequest.getOriginalTicketAgentLocation();

    else if (rexBaseRequest.prevTicketIssueAgent() &&
             !rexBaseRequest.prevTicketIssueAgent()->agentCity().empty())
      locOfAdjust = rexBaseRequest.prevTicketIssueAgent()->agentLocation();
  }

  return std::make_pair(dateToAdjust, locOfAdjust);
}

const DateTime
RexPricingTrx::adjustToCurrentUtcZone() const
{
  const std::pair<const DateTime*, const Loc*>& timeAndLoc = getLocAndDateForAdjustment();

  if (!timeAndLoc.first->historicalIncludesTime())
    return *timeAndLoc.first;

  return RexBaseTrx::adjustToCurrentUtcZone(*timeAndLoc.first, timeAndLoc.second);
}

bool
RexPricingTrx::isPlusUpCalculationNeeded() const
{
  const RexBaseRequest& rexBaseRequest = static_cast<const RexBaseRequest&>(*getRequest());

  return rexBaseRequest.currentTicketingAgent()->abacusUser()
             ? true
             : !fallback::inactiveNonRefAmountForNonAbacusUsers(this);
}

void
RexPricingTrx::setExcTktNonRefundable(boost::tribool status)
{
  const RexBaseRequest& rexBaseRequest = static_cast<const RexBaseRequest&>(*getRequest());

  if (rexBaseRequest.currentTicketingAgent()->abacusUser() &&
      fallback::fallbackNonRefAmountOptimizationForAbacus(this))
    return;

  _excTktNonRefundable = status;
}

void
RexPricingTrx::convert(tse::ErrorResponseException& ere, std::string& response)
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
RexPricingTrx::convert(std::string& response)
{
  XMLConvertUtils::tracking(*this);
  if (!taxRequestToBeReturnedAsResponse().empty())
  {
    response = taxRequestToBeReturnedAsResponse();
    return true;
  }

  LOG4CXX_DEBUG(_logger, "Doing RexPricingTrx response");
  response = XMLConvertUtils::rexPricingTrxResponse(*this);
  LOG4CXX_DEBUG(_logger, "response: " << response);

  return true;
}

} // tse namespace
