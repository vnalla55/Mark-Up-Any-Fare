//-------------------------------------------------------------------
//
//  File:
//  Created:     April 16, 2007
//  Authors:     Artur Krezel
//
//  Updates:
//
//  Copyright Sabre 2007
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "RexPricing/RexFareSelector.h"

#include "Common/Assert.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/Money.h"
#include "Common/Thread/ThreadPoolFactory.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TSEException.h"
#include "DataModel/ExcItin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/Diag23XCollector.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/BooleanFlagResetter.h"
#include "Fares/FareCollectorOrchestrator.h"
#include "Fares/RoutingController.h"
#include "RexPricing/PrepareRexFareRules.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Server/TseServer.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackCat31KeepWholeFareSetOnExcFM);
FALLBACK_DECL(azPlusUp);
FALLBACK_DECL(azPlusUpExc);
FALLBACK_DECL(reworkTrxAborter);
FALLBACK_DECL(diag23XVarianceFix);
FALLBACK_DECL(allow100PExcDiscounts);
FALLBACK_DECL(excDiscountsFixDivideByZero);

RexFareSelector::RexFareSelector(RexBaseTrx& trx)
  : _trx(trx),
    _subjectCategory((trx.excTrxType() == PricingTrx::AF_EXC_TRX)
                         ? RuleConst::VOLUNTARY_REFUNDS_RULE
                         : RuleConst::VOLUNTARY_EXCHANGE_RULE),
    _lastChanceForVariance(false),
    _discountOnNewItinFound(false),
    _selectorVCTR(trx),
    _selectorBasic(trx, _preSelectedFares),
    _selectorVariance(trx, _preSelectedFares),
    _selectorDiscounted(trx, _preSelectedFares),
    _selectorHip(trx),
    _areAllFCsProcessed(false)
{
}

Logger
RexFareSelector::_logger("atseintl.RexPricing.RexFareSelector");

namespace
{

class FCThreadTask : public TseCallableTrxTask
{
public:
  FCThreadTask(RexFareSelector& rfs, RexBaseTrx& trx, FareCompInfo& fc)
    : _rfs(rfs), _trx(trx), _fc(fc), _error(ErrorResponseException::NO_ERROR)
  {
    TseCallableTrxTask::trx(&_trx);
    TseCallableTrxTask::desc("RETRIEVE VCTR TASK");
  }

  void performTask() override
  {
    try { _rfs.retrieveVCTRTask(_fc); }
    catch (ErrorResponseException& ere) { _error = ere; }
    catch (...)
    {
      _error = ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION,
                                      "UNKNOWN ERROR DURING PRICING");
    }
  }

  const ErrorResponseException& error() const { return _error; }

private:
  RexFareSelector& _rfs;
  RexBaseTrx& _trx;
  FareCompInfo& _fc;
  ErrorResponseException _error;
};

class RoutingValid
{
public:
  void operator()(PaxTypeFare& ptf)
  {
    if (!ptf.isRoutingValid())
      ptf.setRoutingValid(true);
  }
};

typedef RAIIImpl<PaxTypeFare, RoutingValid> RAIISetRoutingValid;

} // namespace

uint32_t
RexFareSelector::getActiveThreads()
{
  if (!ThreadPoolFactory::isMetricsEnabled())
    return 0;

  return ThreadPoolFactory::getNumberActiveThreads(TseThreadingConst::REX_FARE_SELECTOR_TASK);
}

namespace
{
// Class responsible for matching of fare markets
// (we can't just use comparision operator of FareMarket class)
// because we need to have custom matching.
class MatchFareMarket : public std::unary_function<const FareMarket*, bool>
{
public:
  MatchFareMarket(const FareMarket* fareMarket) : _fareMarket(fareMarket) {}

  bool operator()(const FareMarket* fareMarket) const
  {
    if (_fareMarket == fareMarket)
      return false;

    if (_fareMarket->travelSeg() != fareMarket->travelSeg())
      return false;

    return true;
  }

protected:
  const FareMarket* _fareMarket;
};

void
setInitialFaresStatus(std::vector<PaxTypeFare*>& fares,
                      const unsigned int category,
                      const bool catIsValid)
{
  std::vector<PaxTypeFare*>::const_iterator faresIter = fares.begin();

  for (; faresIter != fares.end(); ++faresIter)
    (*faresIter)->setCategoryValid(category, catIsValid);
}

} // end of namespace

Diag23XCollector*
RexFareSelector::getActiveDiag()
{
  DCFactory* factory = DCFactory::instance();
  Diag23XCollector* diagPtr = dynamic_cast<Diag23XCollector*>(factory->create(_trx));

  if (diagPtr != nullptr)
  {
    diagPtr->enable(Diagnostic231, Diagnostic233);

    if (diagPtr->isActive())
      return diagPtr;
  }

  return nullptr;
}

void
RexFareSelector::displayFareCompInfo(Diag23XCollector& diag, FareCompInfo& fc)
{
  diag << fc;

  if (!fallback::diag23XVarianceFix(&_trx))
  {
    diag << "USING STRATEGY ";
    switch (chooseStrategy(fc))
    {
    case Strategy::VCTR:
      diag << "VCTR";
      break;
    case Strategy::BASIC:
      diag << "BASIC";
      break;
    case Strategy::VARIANCE:
      diag << "VARIANCE";
      break;
    case Strategy::DISCOUNTED:
      diag << "DISCOUNTED";
      break;
    case Strategy::HIP:
      diag << "HIP";
      break;
    default:
      TSE_ASSERT(!"invalid Strategy value");
    }
    diag << "\n\n";
  }

  if (fc.matchedFares().empty())
    diag << "UNABLE TO MATCH FARES\n";
  else
  {
    diag.printSeparator();
    for (const FareCompInfo::MatchedFare& fare : fc.getMatchedFares())
    {
      diag << fare;
    }
  }

  if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX && _trx.isPlusUpCalculationNeeded())
    diag << '\n';

  diag.flushMsg();
}

void
RexFareSelector::displayDiagnostics()
{
  Diag23XCollector* diag = getActiveDiag();

  if (diag)
  {
    diag->parseQualifiers(_trx);

    diag->printHeader();
    *diag << _trx << '\n';

    std::vector<FareCompInfo*>::iterator fcIter = _trx.exchangeItin()[0]->fareComponent().begin(),
                                         fcIterEnd = _trx.exchangeItin()[0]->fareComponent().end();

    for (; fcIter != fcIterEnd; ++fcIter)
      displayFareCompInfo(*diag, **fcIter);
  }
}

Service*
RexFareSelector::getServicePointer(const std::string& serviceName) const
{
  Service* to = Global::service("TO_SVC");
  TSE_ASSERT(to != nullptr);

  TseServer& mainSrv = to->server();

  Service* svc = mainSrv.service(serviceName);
  TSE_ASSERT(svc != nullptr);

  return svc;
}

bool
RexFareSelector::runFareCollector(Service& fareCollector) const try
{
  // unvalidating Fare Collector service for fare markets which didn't match any fare
  std::vector<FareCompInfo*>::iterator fcIter = _trx.exchangeItin()[0]->fareComponent().begin(),
                                       fcIterEnd = _trx.exchangeItin()[0]->fareComponent().end();
  for (; fcIter != fcIterEnd; ++fcIter)
  {
    FareCompInfo& fc = **fcIter;
    if (!fc.fareMarket()->serviceStatus().isSet(FareMarket::RexFareSelector))
      fc.fareMarket()->serviceStatus().clear(FareMarket::FareCollector);
    fc.fareMarket()->travelDate() = _trx.adjustedTravelDate(fc.fareMarket()->travelDate());
    fc.fareMarket()->failCode() = tse::ErrorResponseException::NO_ERROR;
  }
  return fareCollector.process(_trx);
}
catch (...) { return false; }

RexFareSelector::Strategy
RexFareSelector::chooseStrategy(FareCompInfo& fc) const
{
  if (fc.hasVCTR())
    return Strategy::VCTR;

  if (fc.hipIncluded() && !_lastChanceForVariance)
    return Strategy::HIP;

  if (_discountOnNewItinFound)
    return Strategy::DISCOUNTED;

  if (_lastChanceForVariance && fc.fareCalcFareAmt() > EPSILON)
    return Strategy::VARIANCE;

  return Strategy::BASIC;
}

const RexFareSelectorStrategy&
RexFareSelector::getSelector(Strategy strategy) const
{
  switch (strategy)
  {
  case Strategy::VCTR:
    return _selectorVCTR;
  case Strategy::BASIC:
    return _selectorBasic;
  case Strategy::VARIANCE:
    return _selectorVariance;
  case Strategy::DISCOUNTED:
    return _selectorDiscounted;
  case Strategy::HIP:
    return _selectorHip;
  default:
    TSE_ASSERT(!"invalid Strategy value");
  }
}

namespace
{

inline bool
needSelectionProcess(const RexBaseTrx& trx, const FareCompInfo& fc)
{
  return fc.getMatchedFares().empty();
}

} // namespace

void
RexFareSelector::retrieveVCTRTask(FareCompInfo& fc)
{
  LOG4CXX_INFO(_logger, "Entered RexFareSelector::retrieveVCTRTask()");

  if (&fc == nullptr || fc.fareMarket() == nullptr)
  {
    LOG4CXX_ERROR(_logger,
                  "RexFareSelector::retrieveVCTRTask() - Empty fare market in FareCompInfo");
    return;
  }

  if (fc.hasVCTR() && (_lastChanceForVariance || _discountOnNewItinFound))
  {
    _areAllFCsProcessed = false;
    return;
  }

  if (!fallback::azPlusUp(&_trx) && !fallback::azPlusUpExc(&_trx) && !fc.isDscProcessed())
  {
    updateFCAmount(fc);
    fc.setIsDscProcessed();
  }

  const RexFareSelectorStrategy& selector = getSelector(chooseStrategy(fc));

  // Select fares for primary market
  selector.process(fc);

  // Check if no fares where selected and if secondary market is available
  if (needSelectionProcess(_trx, fc))
  {
    LOG4CXX_INFO(_logger, "RexFareSelector::retrieveVCTRTask() - no fares for primary fare market");
    if (fc.secondaryFareMarket() != nullptr)
    {
      // Select fares for secondary market
      LOG4CXX_INFO(_logger, "RexFareSelector::retrieveVCTRTask() - trying secondary fare market");
      std::swap(fc.fareMarket(), fc.secondaryFareMarket());
      selector.process(fc);

      fc.secondaryFMprocessed() = true;

      if (needSelectionProcess(_trx, fc))
      {
        _areAllFCsProcessed = false;
        std::swap(fc.fareMarket(), fc.secondaryFareMarket());
        LOG4CXX_INFO(_logger,
                     "RexFareSelector::retrieveVCTRTask() - no fares for secondary market also");
      }
    }
    else
      _areAllFCsProcessed = false;
  }
  LOG4CXX_INFO(_logger, "Leaving RexFareSelector::retrieveVCTRTask()");
}

void
RexFareSelector::storeVCTR()
{
  if (_trx.getCurrTktDateSeqStatus() != FARE_COMPONENT_DATE)
  {
    return;
  } // store only for mip dates

  if (_vctrStorage.empty())
  {
    _vctrStorage.resize(_trx.excFareCompInfo().size());
  }
  std::vector<FareCompInfo*>& fcVector = _trx.exchangeItin()[0]->fareComponent();
  TSE_ASSERT(fcVector.size() == _trx.excFareCompInfo().size());

  for (size_t index = 0; index < fcVector.size(); ++index)
  {

    FareCompInfo& fc = *fcVector[index];
    FareComponentInfo& fci = *_trx.excFareCompInfo()[index];

    _vctrStorage[index].first = fc.VCTR();
    _vctrStorage[index].second = fc.hasVCTR();

    fc.VCTR() = fci.vctrInfo()->vctr();
    fc.hasVCTR() = true;
  }
}

void
RexFareSelector::restoreVCTR()
{
  if (_trx.getCurrTktDateSeqStatus() != FARE_COMPONENT_DATE) // restore only for mip dates
  {
    return;
  }

  std::vector<FareCompInfo*>& fcVector = _trx.exchangeItin()[0]->fareComponent();
  TSE_ASSERT(fcVector.size() == _vctrStorage.size());

  for (size_t index = 0; index < fcVector.size(); ++index)
  {
    FareCompInfo& fc = *fcVector[index];

    fc.VCTR() = _vctrStorage[index].first;
    fc.hasVCTR() = _vctrStorage[index].second;
  }
}

void
RexFareSelector::processAllFareCompInfos()
{
  typedef std::shared_ptr<FCThreadTask> TaskPtr;
  typedef std::vector<TaskPtr> TaskVector;
  TaskVector tasks;

  TseRunnableExecutor taskExecutor(TseThreadingConst::REX_FARE_SELECTOR_TASK);

  std::vector<FareCompInfo*>& fcVector = _trx.exchangeItin()[0]->fareComponent();

  storeVCTR();

  _areAllFCsProcessed = true;

  for (const auto elem : fcVector)
  {
    FareCompInfo& fc = *elem;

    if (fc.fareMarket()->serviceStatus().isSet(FareMarket::RexFareSelector) == false)
    {
      setInitialFaresStatus(fc.fareMarket()->allPaxTypeFare(), _subjectCategory, false);

      tasks.push_back(TaskPtr(new FCThreadTask(*this, _trx, fc)));

      taskExecutor.execute(*tasks.back());
    }
  } // for each fare component

  taskExecutor.wait();

  restoreVCTR();

  if (_trx.aborter())
  {
    if (fallback::reworkTrxAborter(&_trx))
      _trx.aborter()->setAbortOnHurry(false);
    else
      _trx.setAbortOnHurry(false);
  }

  RuleControllerWithChancelor<FareMarketRuleController> ruleController(VolunExcPrevalidation);
  if (_trx.excTrxType() == PricingTrx::AF_EXC_TRX)
    ruleController.categorySequence().clear();

  ruleController.categorySequence().push_back(_subjectCategory);

  PrepareRexFareRules prepareRexFareRules(_trx, &ruleController);

  for (FareCompInfo* fc : fcVector)
  {
    if (fc->fareMarket()->serviceStatus().isSet(FareMarket::RexFareRulePreparor))
      continue;

    if (fc->getMatchedFares().empty())
      continue;

    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX && _trx.isPlusUpCalculationNeeded())
    {
      for (FareCompInfo::MatchedFare& fare : fc->matchedFares())
      {
        initializeMatchedFare(fare.get());

        fare.get()->setCategoryValid(_subjectCategory, true);
        prepareRexFareRules.process(fare.get());
      }
    }
    else
    {
      // remove routing call for both AR/AF together with fallback!
      RoutingController routingController(_trx);
      TravelRoute travelRoute;
      TravelRoute travelRouteTktOnly;
      travelRoute.travelRouteTktOnly() = &travelRouteTktOnly;
      RoutingInfos routingInfos;

      for (FareCompInfo::MatchedFare& fare : fc->matchedFares())
      {
        initializeMatchedFare(fare.get());

        RAIISetRoutingValid rsrv(*fare.get());
        routingController.process(*fare.get(), travelRoute, routingInfos);
        if (routingController.isSpecialRouting(*fare.get()) &&
            routingController.needSpecialRouting(*fare.get()))
        {
          std::vector<PaxTypeFare*> fares(1, fare.get());
          routingController.processSpecialRouting(fares, routingInfos, travelRoute);
        }

        fare.get()->setCategoryValid(_subjectCategory, true);
        prepareRexFareRules.process(fare.get());
      }
    }

    fc->fareMarket()->serviceStatus().set(FareMarket::RexFareRulePreparor);
  }
}

void
RexFareSelector::initializeMatchedFare(PaxTypeFare* ptf)
{
  if (ptf != nullptr)
  {
    FareMarket::RetrievalInfo* info = nullptr;
    _trx.dataHandle().get(info);
    info->_date = _trx.dataHandle().ticketDate();
    info->_flag = (FareMarket::FareRetrievalFlags)FareMarket::RetrievKeep;
    ptf->retrievalInfo() = info;
  }
}

void
RexFareSelector::invalidateClones(const FareMarket& originalFm,
                                  std::vector<FareMarket*>::const_iterator dupFmIter,
                                  const std::vector<FareMarket*>::const_iterator exchFmIterEnd)
{
  MatchFareMarket mfm(&originalFm);
  for (; dupFmIter != exchFmIterEnd; ++dupFmIter)
    if (mfm(*dupFmIter))
      const_cast<FareMarket*>(*dupFmIter)->setBreakIndicator(true);
}

void
RexFareSelector::selectSecondaryFareMarket()
{
  const std::vector<FareMarket*>& exchangeFareMarket = _trx.exchangeItin()[0]->fareMarket();
  const std::vector<FareMarket*>::const_iterator exchFmIterEnd = exchangeFareMarket.end();

  std::vector<FareCompInfo*>::const_iterator fcIter =
      _trx.exchangeItin()[0]->fareComponent().begin();
  const std::vector<FareCompInfo*>::const_iterator fcIterEnd =
      _trx.exchangeItin()[0]->fareComponent().end();

  // Iterate through all fare components
  for (; fcIter != fcIterEnd; ++fcIter)
  {
    FareCompInfo* fc = *fcIter;

    bool checkSecondaryFm = false;
    if ( _trx.isIataFareSelectionApplicable() )
    {
      checkSecondaryFm = isSecondaryFareMarket( *fc );
    }
    else if ( fc->fareMarket()->travelSeg().size() > 1 )
    {
      TravelSegAnalysis travelAnalysis;
      checkSecondaryFm =
          travelAnalysis.selectTravelBoundary(fc->fareMarket()->travelSeg()) == Boundary::AREA_21;
    }

    // Fare market travel type has to be foreign domestic
    if (fc->fareMarket()->geoTravelType() == GeoTravelType::ForeignDomestic || checkSecondaryFm)
    {
      // Try to find duplicate faremarket in rest of exchange itin but with different carrier
      std::vector<FareMarket*>::const_iterator dupFmIter = std::find_if(
          exchangeFareMarket.begin(), exchFmIterEnd, MatchFareMarket(fc->fareMarket()));

      if (dupFmIter != exchFmIterEnd)
      {
        // If found, assign is as a secondary faremarket in current fare component
        fc->secondaryFareMarket() = *dupFmIter;
        fc->secondaryFareMarket()->fareCompInfo() = fc;

        // If the exchange governing carrier is given then we want to make sure the primary
        // fare market is set accordingly. This way, we have a better chance of gathering the
        // correct fares and matching the first time, reducing the possibility of trying to
        // match with the secondary fare market.
        if ( _trx.isIataFareSelectionApplicable() )
        {
          if ( fc->hasVCTR() && fc->VCTR().carrier() == fc->secondaryFareMarket()->governingCarrier() )
            std::swap( fc->fareMarket(), fc->secondaryFareMarket() );
        }

        // temporary solution! : invalidate rest of clones
        invalidateClones(*fc->fareMarket(), ++dupFmIter, exchFmIterEnd);
      }
    }
  }
}

bool
RexFareSelector::isSecondaryFareMarket(const FareCompInfo& fci) const
{
  bool isSecondaryFareMarket = (fci.fareMarket()->travelSeg().size() > 1);
  if ( isSecondaryFareMarket )
  {
    const Boundary travelBoundary =
        TravelSegAnalysis::selectTravelBoundary( fci.fareMarket()->travelSeg() );
    isSecondaryFareMarket =
        ((Boundary::USCA == travelBoundary) || (Boundary::EXCEPT_USCA == travelBoundary)) ? false
                                                                                          : true;
  }

  return isSecondaryFareMarket;
}

void
RexFareSelector::setCalculationCurrencyNoDecForExcItin() const
{
  ExcItin& itin = *_trx.exchangeItin()[0];
  const CurrencyCode& currencyCode = itin.calcCurrencyOverride().empty()
                                         ? itin.calculationCurrency()
                                         : itin.calcCurrencyOverride();

  if (currencyCode == NUC)
    itin.calculationCurrencyNoDec() = 2;
  else
  {
    const Currency* currency = nullptr;
    currency = _trx.dataHandle().getCurrency( currencyCode );
    itin.calculationCurrencyNoDec() = currency ? currency->noDec() : 2;
  }
}

bool
RexFareSelector::checkIfNewItinHasDiscount()
{
  if (TrxUtil::newDiscountLogic(_trx))
    return _discountOnNewItinFound = !(_trx.getRequest()->getDiscountAmountsNew().empty() &&
                                       _trx.getRequest()->getDiscountPercentagesNew().empty());
  else
    return _discountOnNewItinFound = !(_trx.getRequest()->getDiscountAmounts().empty() &&
                                       _trx.getRequest()->getDiscountPercentages().empty());
}

void
RexFareSelector::processDateSequence()
{
  _trx.resetCurrTktDateSeqStatus();
  do
  {
    _trx.dataHandle().setTicketDate(_trx.getCurrTktDateSeq());
    _trx.ticketingDate() = _trx.getCurrTktDateSeq();
    processAllFareCompInfos();
  } while (_trx.nextTktDateSeq() && !_areAllFCsProcessed);
}

void
RexFareSelector::process()
{
  // initial checks
  if (_trx.exchangeItin().empty())
  {
    LOG4CXX_ERROR(_logger, "RexFareSelector::process() - Empty exchange Itin vector");
    return;
  }
  if ((_trx.exchangePaxType() == nullptr || _trx.exchangePaxType()->paxType().empty()) &&
      (_trx.paxType().empty() || _trx.paxType()[0] == nullptr || _trx.paxType()[0]->paxType().empty()))
  {
    LOG4CXX_ERROR(_logger, "RexFareSelector::process() - Empty exchange PaxType for RexBaseTrx");
    return;
  }
  if (_trx.exchangeItin()[0] == nullptr || _trx.exchangeItin()[0]->fareComponent().empty())
  {
    LOG4CXX_ERROR(_logger,
                  "RexFareSelector::process() - Empty exchange Itin->fareComponent vector");
    return;
  }

  setCalculationCurrencyNoDecForExcItin();

  // Try to find secondary market for foreign domestic fare components
  selectSecondaryFareMarket();

  _trx.setupDateSeq();
  _trx.setupMipDateSeq();
  _trx.resetCurrTktDateSeqStatus();

  _lastChanceForVariance = false;

  _trx.setFareApplicationDT(_trx.getCurrTktDateSeq());
  processAllFareCompInfos();

  Service* fareCollector = getServicePointer("FARESC_SVC");

  while (_trx.nextTktDateSeq() && !_areAllFCsProcessed)
  {
    _trx.setFareApplicationDT(_trx.getCurrTktDateSeq());
    runFareCollector(*fareCollector);
    processAllFareCompInfos();
  }

  _lastChanceForVariance = true;

  if (!_areAllFCsProcessed)
    processDateSequence();

  if (!_areAllFCsProcessed && checkIfNewItinHasDiscount())
    processDateSequence();

  _trx.setFareApplicationDT(_trx.originalTktIssueDT());

  _preSelectedFares.clear();

  displayDiagnostics();

  if (!_trx.isPlusUpCalculationNeeded())
  {
    const RexBaseRequest& rexBaseRequest = static_cast<const RexBaseRequest&>(*_trx.getRequest());

    if (!fallback::fallbackCat31KeepWholeFareSetOnExcFM(&_trx) &&
        rexBaseRequest.currentTicketingAgent()->abacusUser() && _areAllFCsProcessed &&
        _trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    {
      for (FareCompInfo* fci : _trx.exchangeItin()[0]->fareComponent())
        fci->loadOtherFares(static_cast<RexPricingTrx&>(_trx));
    }
  }
}

void
RexFareSelector::updateFCAmount(FareCompInfo& fc)
{
  if (const Percent* percent = _trx.getExcDiscountPercentage(*fc.fareMarket()))
  {
    if (!fallback::allow100PExcDiscounts(&_trx))
    {
      if (*percent < 100.0 || fallback::excDiscountsFixDivideByZero(&_trx))
      {
        const double divisor = 1.0 - (*percent) / 100.0;
        fc.fareCalcFareAmt() /= divisor;
        fc.tktFareCalcFareAmt() /= divisor;
      }
      else if (*percent > 100.0)
      {
        throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                     "DISCOUNT HIGHER THAN 100 PERCENT NOT POSSIBLE");
      }
    }
    else
    {
      if (*percent >= 100.0)
        throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                     "PENALTY HIGHER/EQUAL 100 PERCENT NOT POSSIBLE");

      fc.fareCalcFareAmt() /= 1.0 - (*percent) / 100.0;
      fc.tktFareCalcFareAmt() /= 1.0 - (*percent) / 100.0;
    }
  }

  if (const DiscountAmount* discountAmount = _trx.getExcDiscountAmount(*fc.fareMarket()))
  {
    CurrencyCode currency = _trx.exchangeItin().front()->calculationCurrency();
    MoneyAmount markupAmt =
        CurrencyUtil::convertMoneyAmount(std::abs(discountAmount->amount),
                                         discountAmount->currencyCode,
                                         currency,
                                         _trx,
                                         CurrencyConversionRequest::NO_ROUNDING);
    fc.fareCalcFareAmt() += std::copysign(markupAmt, discountAmount->amount);

    if (currency != _trx.exchangeItin().front()->calcCurrencyOverride())
    {
      currency = _trx.exchangeItin().front()->calcCurrencyOverride();
      markupAmt = CurrencyUtil::convertMoneyAmount(std::abs(discountAmount->amount),
                                                   discountAmount->currencyCode,
                                                   currency,
                                                   _trx,
                                                   CurrencyConversionRequest::NO_ROUNDING);
    }

    fc.tktFareCalcFareAmt() += std::copysign(markupAmt, discountAmount->amount);
  }
}

} //tse
