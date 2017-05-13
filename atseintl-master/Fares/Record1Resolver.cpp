#include "Fares/Record1Resolver.h"

#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "DataModel/Billing.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag204Collector.h"
#include "Fares/FareUtil.h"
#include "Fares/FareTypeMatcher.h"
#include "Rules/RuleUtil.h"

namespace tse
{
FALLBACK_DECL(fallbackAPO37838Record1EffDateCheck);

const MoneyAmount Record1Resolver::CalcMoney::NO_AMT = -1.0;

static Logger
_logger("atseintl.Fares.Record1Resolver");

Record1Resolver::Record1Resolver(PricingTrx& trx, const Itin& itin, FareMarket& fareMarket) :
    _trx(trx),
    _fareMarket(fareMarket),
    _travelDate(ItinUtil::getTravelDate(itin.travelSeg())),
    _calcMoney(trx, _ccFacade, itin)
{
  _travelDate = trx.adjustedTravelDate(_travelDate);

  // For FareDisplay Transactions
  _fdTrx = dynamic_cast<FareDisplayTrx*>(&_trx);
}

bool
Record1Resolver::resolveR1s(const Fare& fare, std::vector<PaxTypeFare*>& resultingPTFs)
{
  return resolveFareClassApp(fare, resultingPTFs);
}

bool
Record1Resolver::resolveFareClassApp(const Fare& fare, std::vector<PaxTypeFare*>& ptFares)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FC RESOLVE FCA");
#endif
  // retrieve a vector of FareClassApp candidates
  // spr37838 filter record 1 by travel date
  if(!fallback::fallbackAPO37838Record1EffDateCheck(&_trx))
      return resolveFareClassAppByTravelDT(fare, ptFares);

  // this section of the code to be removed when fallback is removed.
  const std::vector<const FareClassAppInfo*>& fareClassAppList = _trx.dataHandle().getFareClassApp(
      fare.vendor(), fare.carrier(), fare.tcrRuleTariff(), fare.ruleNumber(), fare.fareClass());

  Diag204Collector* diag204 = Diag204Collector::getDiag204(_trx);

  // FareDataManager sorts these records by sequence number
  // scan the vector and find the first match

  std::vector<const FareClassAppInfo*>::const_iterator i = fareClassAppList.begin();
  std::vector<const FareClassAppInfo*>::const_iterator ie = fareClassAppList.end();

  if (i == ie)
  {
    LOG4CXX_DEBUG(RuleUtil::_dataErrLogger,
                  "NO RECORD1 FOUND FOR " << fare.vendor() << " " << fare.carrier() << " "
                                          << fare.tcrRuleTariff() << " " << fare.ruleNumber()
                                          << " FARE:" << fare.fareClass());

    if (UNLIKELY(diag204))
    {
      diag204->writeNoRecord1(_trx, fare, _fareMarket);
    }
  }

  for (; i != ie; ++i)
  {
    const FareClassAppInfo* fcaInfo = *i;

    if (UNLIKELY(fcaInfo == nullptr))
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeNoFCA(_trx, fare, _fareMarket);
      }
      continue;
    }

    if (fcaInfo->_unavailTag == UNAVAILABLE && !isFdTrx() &&
        !commandPricingFare(fare.fareClass())) // Allow R1 unavailable when doing command pricing
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeNoMatchFareClass(_trx, fare, _fareMarket);
      }
      continue;
    }

    // match Locations 1 & 2

    bool flipGeo = false;

    if (!matchLocation(fare, flipGeo, *fcaInfo))
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeNoMatchLocation(_trx, fare, _fareMarket, *fcaInfo);
      }
      continue;
    }

    // match OW/RT indicator

    if (UNLIKELY(((fare.owrt() == ONE_WAY_MAY_BE_DOUBLED) || (fare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)) &&
        (fcaInfo->_owrt != ONE_WAY_MAY_BE_DOUBLED)))
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeFareCanNotDouble(_trx, fare, _fareMarket);
      }
      continue;
    }
    else if ((fare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) &&
             (fcaInfo->_owrt != ROUND_TRIP_MAYNOT_BE_HALVED))
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeFareCanNotBeHalved(_trx, fare, _fareMarket);
      }
      continue;
    }

    // match routing number

    if (!fcaInfo->_routingNumber.empty())
    {
      if (fare.routingNumber() != fcaInfo->_routingNumber)
      {
        if (UNLIKELY(diag204))
        {
          diag204->writeNoMatchRoutingNumber(_trx, fare, _fareMarket, *fcaInfo);
        }
        continue;
      }
    }

    // match Footnotes
    if (!RuleUtil::matchFareFootnote(
            fcaInfo->_footnote1, fcaInfo->_footnote2, fare.footNote1(), fare.footNote2()))
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeNoFootnoteMatch(_trx, fare, _fareMarket, *fcaInfo);
      }
      continue;
    }

    if (LIKELY(resolveFareClassAppSeg(fare, ptFares, flipGeo, *fcaInfo)))
    {
      if (UNLIKELY(diag204))
      {
        diag204->flushMsg();
      }
      return true;
    }
  }

  // Diagnostic204, print out all record 1 we tried
  if (UNLIKELY(diag204))
    diag204->flushMsg();

  return false;
}

bool
Record1Resolver::resolveFareClassAppByTravelDT(const Fare& fare, std::vector<PaxTypeFare*>& ptFares)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FC RESOLVE FCA");
#endif

  // retrieve a vector of FareClassApp candidates filtering by travel date
  const DateTime& travelDT = (_trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX) ?
      _travelDate : _fareMarket.travelDate();

  const std::vector<const FareClassAppInfo*>& fareClassAppList =
      _trx.dataHandle().getFareClassAppByTravelDT(fare.vendor(), fare.carrier(),
              fare.tcrRuleTariff(),fare.ruleNumber(), fare.fareClass(), travelDT);

  Diag204Collector* diag204 = Diag204Collector::getDiag204(_trx);

  // FareDataManager sorts these records by sequence number
  // scan the vector and find the first match

  std::vector<const FareClassAppInfo*>::const_iterator i = fareClassAppList.begin();
  std::vector<const FareClassAppInfo*>::const_iterator ie = fareClassAppList.end();

  if (i == ie)
  {
    LOG4CXX_DEBUG(RuleUtil::_dataErrLogger,
                  "NO RECORD1 FOUND FOR " << fare.vendor() << " " << fare.carrier() << " "
                                          << fare.tcrRuleTariff() << " " << fare.ruleNumber()
                                          << " FARE:" << fare.fareClass());

    if (UNLIKELY(diag204))
    {
      diag204->writeNoRecord1(_trx, fare, _fareMarket);
    }
  }

  for (; i != ie; ++i)
  {
    const FareClassAppInfo* fcaInfo = *i;

    if (fcaInfo == nullptr)
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeNoFCA(_trx, fare, _fareMarket);
      }
      continue;
    }

    // commandPricingFare = (fare.fareClass() == _fareMarket.fareBasisCode() &&
    // _fareMarket.fbcUsage() == COMMAND_PRICE_FBC);

    if (fcaInfo->_unavailTag == UNAVAILABLE && !isFdTrx() &&
        !commandPricingFare(fare.fareClass())) // Allow R1 unavailable when doing command pricing
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeNoMatchFareClass(_trx, fare, _fareMarket);
      }
      continue;
    }

    // match Locations 1 & 2

    bool flipGeo = false;

    if (!matchLocation(fare, flipGeo, *fcaInfo))
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeNoMatchLocation(_trx, fare, _fareMarket, *fcaInfo);
      }
      continue;
    }

    // match OW/RT indicator

    if (((fare.owrt() == ONE_WAY_MAY_BE_DOUBLED) || (fare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)) &&
        (fcaInfo->_owrt != ONE_WAY_MAY_BE_DOUBLED))
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeFareCanNotDouble(_trx, fare, _fareMarket);
      }
      continue;
    }

    else if ((fare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) &&
             (fcaInfo->_owrt != ROUND_TRIP_MAYNOT_BE_HALVED))
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeFareCanNotBeHalved(_trx, fare, _fareMarket);
      }
      continue;
    }

    // match routing number

    if (!fcaInfo->_routingNumber.empty())
    {
      if (fare.routingNumber() != fcaInfo->_routingNumber)
      {
        if (UNLIKELY(diag204))
        {
          diag204->writeNoMatchRoutingNumber(_trx, fare, _fareMarket, *fcaInfo);
        }
        continue;
      }
    }

    // match Footnotes
    if (!RuleUtil::matchFareFootnote(
            fcaInfo->_footnote1, fcaInfo->_footnote2, fare.footNote1(), fare.footNote2()))
    {
      if (UNLIKELY(diag204))
      {
        diag204->writeNoFootnoteMatch(_trx, fare, _fareMarket, *fcaInfo);
      }
      continue;
    }

    if (resolveFareClassAppSeg(fare, ptFares, flipGeo, *fcaInfo))
    {
      if (UNLIKELY(diag204))
      {
        diag204->flushMsg();
      }
      return true;
    }
  }

  // Diagnostic204, print out all record 1 we tried

  if (UNLIKELY(diag204))
  {
    diag204->flushMsg();
  }

  return false;
}

bool
Record1Resolver::resolveFareClassAppSeg(const Fare& fare,
                                       std::vector<PaxTypeFare*>& ptFares,
                                       bool flipGeo,
                                       const FareClassAppInfo& fcaInfo)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FC RESOLVE FCAS");
#endif

  // iterate through this vector of FareClassAppSegInfo records
  // and find the perfect match.
  size_t startFareCount = 0;
  startFareCount = ptFares.size();

  Diag204Collector* diag204 = Diag204Collector::getDiag204(_trx);

  const FareTypeMatrix* ftm = getFareTypeMatrix(fcaInfo._fareType);

  if (UNLIKELY(ftm == nullptr))
  {
    if (UNLIKELY(diag204))
    {
      diag204->writeFareTypeNotRetrieved(_trx, fare, _fareMarket, fcaInfo);
      diag204->flushMsg();
    }
    // If we couldnt resolve the fare type matrix, all the records in this sequence will fail!
    LOG4CXX_ERROR(_logger, "DataHandle.resolveFareTypeMatrix() cannot retrieve fare type");
    LOG4CXX_ERROR(_logger, "  faretype: " << fcaInfo._fareType);
    return false;
  }

  FareClassAppSegInfoList::const_iterator fcasI = fcaInfo._segs.begin();
  FareClassAppSegInfoList::const_iterator fcasJ = fcaInfo._segs.end();
  const FareClassAppSegInfo* fcasLast = fcaInfo._segs.back();

  bool cloneFareNeeded = false;
  const FareClassAppSegInfo* prevFcasWithMatchedTbl990 = nullptr;
  PaxTypeFare* prevPTFWithMatchedTbl990 = nullptr;

  PricingOptions* secondAction = _trx.getOptions();

  bool isFareTypePricing = secondAction->isFareFamilyType();
  FareTypeMatcher fareTypeMatch(_trx);

  for (; fcasI != fcasJ; ++fcasI)
  {
    const FareClassAppSegInfo* fcas = *fcasI;

    // match directional indicator

    if (UNLIKELY(FareUtil::failFareClassAppSegDirectioanlity(*fcas, _trx, flipGeo)))
      continue;

    // table 994 match
    if (UNLIKELY(fcas->_overrideDateTblNo != 0))
    {
      DateTime travelDate;
      RuleUtil::getFirstValidTvlDT(travelDate, _fareMarket.travelSeg(), true);

      if (travelDate.isOpenDate())
      {
        // use first travelSeg departure date(+1 logic), setby PricingModelMap
        travelDate = _fareMarket.travelSeg().front()->departureDT();
      }
      DateTime bookingDate;

      RuleUtil::getLatestBookingDate(bookingDate, _fareMarket.travelSeg());

      if (!RuleUtil::validateDateOverrideRuleItem(_trx,
                                                  fcas->_overrideDateTblNo,
                                                  fare.vendor(),
                                                  travelDate,
                                                  _trx.getRequest()->ticketingDT(),
                                                  bookingDate))
      {
        continue;
      }
    }

    // table990 match
    bool isYYFareWithTbl990 =
        (fcas->_carrierApplTblItemNo != 0 && fcaInfo._carrier == INDUSTRY_CARRIER);

    if (UNLIKELY(isYYFareWithTbl990))
    {

      // need to check if this is BookingCode overflown FareClassAppSegInfo
      if (prevFcasWithMatchedTbl990 != nullptr &&
          prevFcasWithMatchedTbl990->_carrierApplTblItemNo == fcas->_carrierApplTblItemNo)
      {
        if (isForBkgCdsOverFlown(prevFcasWithMatchedTbl990, fcas))
        {
          if (!prevPTFWithMatchedTbl990->fcasOfOverFlownBkgCds())
          {
            std::vector<const FareClassAppSegInfo*>* fcasVec = nullptr;
            _trx.dataHandle().get(fcasVec);
            if (fcasVec != nullptr)
            {
              fcasVec->push_back(fcas);
              prevPTFWithMatchedTbl990->fcasOfOverFlownBkgCds() = fcasVec;
            }
          }
          else
          {
            prevPTFWithMatchedTbl990->fcasOfOverFlownBkgCds()->push_back(fcas);
          }

          continue; // no need to create a new PTFare
        }
      }
      else
      {
        if (!_fareMarket.findFMCxrInTbl(_trx.getTrxType(),
                                        _trx.dataHandle().getCarrierApplication(
                                            fare.vendor(), fcas->_carrierApplTblItemNo)))
        {
          // if this is the last AppSeg and we did not have match on 990 before
          // we will need to create this PTFare, otherwise we do not
          if (prevFcasWithMatchedTbl990 != nullptr || fcasLast != fcas)
            continue;
        }
      }
    }

    PaxTypeFare* ptFare = nullptr;
    if (cloneFareNeeded)
    {
      Fare* newFare = fare.clone(_trx.dataHandle());
      ptFare = createPaxTypeFare(const_cast<Fare*>(newFare),
                                 const_cast<FareClassAppInfo*>(&fcaInfo),
                                 const_cast<FareClassAppSegInfo*>(fcas));
    }
    else
    {
      ptFare = createPaxTypeFare(const_cast<Fare*>(&fare),
                                 const_cast<FareClassAppInfo*>(&fcaInfo),
                                 const_cast<FareClassAppSegInfo*>(fcas));
    }
    if (UNLIKELY(ptFare == nullptr))
      continue;

    cloneFareNeeded = true; // from second time, we need to clone new Fare obj

    ptFare->cabin() = ftm->cabin();
    ptFare->fareTypeDesignator() = ftm->fareTypeDesig();
    ptFare->fareTypeApplication() = ftm->fareTypeAppl();

    //@TODO: implement the Secondary action code validaion for the Pricing Entries
    if (UNLIKELY(secondAction->isNormalFare() &&
        !ptFare->isNormal())) // Need NL fare but paxtypefare is not Normal
    {
      ptFare->fare()->setFailBySecondaryActionCode();
      //@todo should there be a continue here?
    }

    // WPT/{NL,EX,IT} entry
    if (UNLIKELY(isFareTypePricing && !fareTypeMatch(ptFare)))
    {
      ptFare->fare()->setFailBySecondaryActionCode();
    }

    if (UNLIKELY(fcaInfo._unavailTag == UNAVAILABLE && fare.fareClass() == _fareMarket.fareBasisCode()))
    {
      ptFare->cpFailedStatus().set(PaxTypeFare::PTFF_R1UNAVAIL);
    }

    // Fix for EXPEDIA - we need brand status vector filled
    if (ptFare->fareMarket() != nullptr)
      ptFare->getMutableBrandStatusVec().assign(ptFare->fareMarket()->brandProgramIndexVec().size(),
          std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));

    ptFares.push_back(ptFare);
    if (isYYFareWithTbl990)
    {
      prevPTFWithMatchedTbl990 = ptFare;
      prevFcasWithMatchedTbl990 = fcas;
    }

    if (PricingTrx::IS_TRX == _trx.getTrxType() && fcas->_paxType.equalToConst("JCB") &&
        false == _fareMarket.isJcb())
    {
      _fareMarket.setJcb(true);
    }

  } // end for_fcasI

  if (LIKELY(ptFares.size() != startFareCount))
    return true; // We must have added at least one

  if (UNLIKELY(diag204))
  {
    diag204->writeNoMatchOnPaxTypes(_trx, fare, _fareMarket);
    diag204->flushMsg();
  }

  return false;
}

bool
Record1Resolver::resolveFareClassAppESV(const Fare& fare, PaxTypeFare*& ptFare)
{
  if (true == _trx.getCreateFareClassAppInfoBF())
  {
    return buildFareClassAppInfoBF(fare, ptFare);
  }

  return false;
}

PaxTypeFare*
Record1Resolver::createPaxTypeFare(Fare* fare, FareClassAppInfo* fcai, FareClassAppSegInfo* fcasi)
{
  PaxType* paxType = nullptr;

  if (LIKELY(fcasi != nullptr && fare != nullptr))
  {
    if (!fcasi->_paxType.empty())

    {
      paxType = const_cast<PaxType*>(
          PaxTypeUtil::isAnActualPaxInTrx(_trx, fare->carrier(), fcasi->_paxType));
    }
    else
    {
      paxType = const_cast<PaxType*>(PaxTypeUtil::isAnActualPaxInTrx(_trx, fare->carrier(), ADULT));
    }

    if (paxType == nullptr)
    {
      if (fcasi->_paxType.empty() || fcasi->_paxType == ADULT)
      {
        paxType = const_cast<PaxType*>(PaxTypeUtil::isAnActualPaxInTrx(_trx, fare->carrier(), JCB));
      }
      else if (UNLIKELY(_trx.getTrxType() == PricingTrx::ESV_TRX && fcasi->_paxType == JCB))
      {
        paxType =
            const_cast<PaxType*>(PaxTypeUtil::isAnActualPaxInTrx(_trx, fare->carrier(), ADULT));
      }
    }
  }

  if (UNLIKELY(paxType == nullptr && (_trx.getOptions() != nullptr && _trx.getOptions()->isWeb()) &&
      (fcasi != nullptr && fcasi->_paxType == NEG)))
  {
    paxType = _trx.paxType().front();
  }

  if (UNLIKELY(paxType == nullptr && (_trx.getOptions() != nullptr && _trx.getOptions()->fbcSelected())))
  {
    paxType = _trx.paxType().front();
  }

  if (paxType == nullptr && (fcasi != nullptr && (fcasi->_paxType.empty() || fcasi->_paxType == ADULT)) &&
      (fcai != nullptr && (fcai->_displayCatType == RuleConst::SELLING_FARE ||
                     fcai->_displayCatType == RuleConst::NET_SUBMIT_FARE ||
                     fcai->_displayCatType == RuleConst::NET_SUBMIT_FARE_UPD)))
  {
    paxType = _trx.paxType().front();
  }

  PaxTypeFare* ptFare = nullptr;
  _trx.dataHandle().get(ptFare);

  // lint --e{413}
  ptFare->initialize(fare, paxType, &_fareMarket, _trx);

  if (LIKELY(fcai != nullptr))
  {
    ptFare->fareClassAppInfo() = fcai;
  }

  if (LIKELY(fcasi != nullptr))
  {
    ptFare->fareClassAppSegInfo() = fcasi;
  }

  try { _calcMoney.adjustPTF(*ptFare); } //  try
  //         Put in a catch and
  //           invalidate the fare if the fare is in an invalid Currency
  catch (tse::ErrorResponseException& ex)
  {
    if (ex.code() != tse::ErrorResponseException::MISSING_NUC_RATE)
    {
      throw;
    }

    // Set so that Pricing/FQ diag 200 can show why it is failing.
    LOG4CXX_ERROR(_logger, "Exception in FareController::createPaxTypeFare: " << ex.what());
    LOG4CXX_ERROR(_logger,
                  " Carrier " << ptFare->carrier() << " " << ptFare->market1() << ptFare->market2()
                              << " Fareclass " << ptFare->fareClass());
    ptFare->setInvalidFareCurrency(); // Set PTF Currency invalid.
  } // catch

  // Create FareDisplayInfo in PaxtypeFare for Fare Display
  if (UNLIKELY(isFdTrx()))
  {
    if (!FareDisplayUtil::initFareDisplayInfo(_fdTrx, *ptFare))
    {
      LOG4CXX_ERROR(_logger, "createPaxTypeFares - Unable to init FareDisplayInfo");
    }
  }

  return ptFare;
}

bool
Record1Resolver::buildFareClassAppInfoBF(const Fare& fare, PaxTypeFare*& ptFare)
{
  FareClassAppInfo* pFareClassAppInfo = createFareClassAppInfoBF(fare);
  FareClassAppSegInfo* pFareClassAppSegInfo = createFareClassAppSegInfoBF(fare);

  if (nullptr == pFareClassAppInfo)
  {
    LOG4CXX_ERROR(_logger,
                  "FareController::buildFareClassAppInfoBF - FareClassAppInfo object is NULL.");
    return false;
  }

  if (nullptr == pFareClassAppSegInfo)
  {
    LOG4CXX_ERROR(_logger,
                  "FareController::buildFareClassAppInfoBF - FareClassAppSegInfo object is NULL.");
    return false;
  }

  ptFare = createPaxTypeFare(const_cast<Fare*>(&fare), pFareClassAppInfo, pFareClassAppSegInfo);

  if (nullptr == ptFare)
  {
    LOG4CXX_ERROR(_logger, "FareController::buildFareClassAppInfoBF - PaxTypeFare object is NULL.");
    return false;
  }

  return true;
}

FareClassAppInfo*
Record1Resolver::createFareClassAppInfoBF(const Fare& fare) const
{
  FareClassAppInfo* fcAppInfo = nullptr;
  _trx.dataHandle().get(fcAppInfo);
  if (fcAppInfo != nullptr)
  {
    fcAppInfo->_vendor = fare.vendor();
    fcAppInfo->_carrier = fare.carrier();
    fcAppInfo->_ruleTariff = fare.tcrRuleTariff();
    fcAppInfo->_ruleNumber = fare.ruleNumber();
    fcAppInfo->_fareClass = fare.fareClass();
    fcAppInfo->_effectiveDate = fare.effectiveDate();
    fcAppInfo->_expirationDate = fare.expirationDate();
    fcAppInfo->_location1Type = LOCTYPE_CITY;
    fcAppInfo->_location1 = fare.origin();
    fcAppInfo->_location2Type = LOCTYPE_CITY;
    fcAppInfo->_location2 = fare.destination();
    fcAppInfo->_owrt = fare.owrt();
    fcAppInfo->_routingNumber = fare.routingNumber();
    fcAppInfo->_fareType = fare.fareInfo()->getFareType(); //"EU";
    //
    fcAppInfo->_seasonType = ' ';
    fcAppInfo->_dowType = ' ';
    fcAppInfo->_pricingCatType = ' ';
    fcAppInfo->_displayCatType = ' ';
    fcAppInfo->_inhibit = 'N';
    fcAppInfo->_unavailTag = 'N';
    fcAppInfo->_seqNo = 1;
    fcAppInfo->_segCount = 1;
  }
  return fcAppInfo;
}

FareClassAppSegInfo*
Record1Resolver::createFareClassAppSegInfoBF(const Fare& fare) const
{
  FareClassAppSegInfo* fcAppSegInfo = nullptr;
  _trx.dataHandle().get(fcAppSegInfo);
  if (fcAppSegInfo != nullptr)
  {
    fcAppSegInfo->_directionality = DIR_IND_NOT_DEFINED;
    fcAppSegInfo->_bookingCodeTblItemNo = 0;
    fcAppSegInfo->_paxType = fare.fareInfo()->getPaxType(); // "ADT" or "JCB"
    fcAppSegInfo->_seqNo = 1;
    // insert booking codes
    const std::vector<BookingCode>* pBookingCodes = fare.fareInfo()->getBookingCodes(_trx);
    if (pBookingCodes != nullptr)
    {
      size_t sz(pBookingCodes->size());
      for (size_t i = 0; i < sz && i < 8; ++i)
      {
        fcAppSegInfo->_bookingCode[i] = (*pBookingCodes)[i];
      }
    }
  }
  return fcAppSegInfo;
}

bool
Record1Resolver::commandPricingFare(const FareClassCode& fbc) const
{
  return _fareMarket.fbcUsage() == COMMAND_PRICE_FBC && fbc == _fareMarket.fareBasisCode();
}

bool
Record1Resolver::matchLocation(const Fare& fare, bool& flipGeo, const FareClassAppInfo& fcaInfo)
    const
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(_trx, "FC MATCHLOC");
#endif
  flipGeo = false;

  GeoTravelType geoTvlType = _fareMarket.geoTravelType();

  // First see if they match the original direction

  const Loc* locMarket1 = getLoc(fare.origin());
  const Loc* locMarket2 = getLoc(fare.destination());

  if (UNLIKELY((locMarket1 == nullptr) || (locMarket2 == nullptr)))
  {
    return false;
  }

  if ((fcaInfo._location1Type == UNKNOWN_LOC ||
       LocUtil::isInLoc(*locMarket1,
                        fcaInfo._location1Type,
                        fcaInfo._location1,
                        fare.vendor(),
                        RESERVED,
                        LocUtil::RECORD1_2_6,
                        geoTvlType,
                        EMPTY_STRING(),
                        _trx.getRequest()->ticketingDT())) &&
      (fcaInfo._location2Type == UNKNOWN_LOC || LocUtil::isInLoc(*locMarket2,
                                                                 fcaInfo._location2Type,
                                                                 fcaInfo._location2,
                                                                 fare.vendor(),
                                                                 RESERVED,
                                                                 LocUtil::RECORD1_2_6,
                                                                 geoTvlType,
                                                                 EMPTY_STRING(),
                                                                 _trx.getRequest()->ticketingDT())))
  {
    return true;
  }

  // Now see if they match by flipping them

  if ((fcaInfo._location2Type == UNKNOWN_LOC ||
       LocUtil::isInLoc(*locMarket1,
                        fcaInfo._location2Type,
                        fcaInfo._location2,
                        fare.vendor(),
                        RESERVED,
                        LocUtil::RECORD1_2_6,
                        geoTvlType,
                        EMPTY_STRING(),
                        _trx.getRequest()->ticketingDT())) &&
      (fcaInfo._location1Type == UNKNOWN_LOC || LocUtil::isInLoc(*locMarket2,
                                                                 fcaInfo._location1Type,
                                                                 fcaInfo._location1,
                                                                 fare.vendor(),
                                                                 RESERVED,
                                                                 LocUtil::RECORD1_2_6,
                                                                 geoTvlType,
                                                                 EMPTY_STRING(),
                                                                 _trx.getRequest()->ticketingDT())))
  {
    flipGeo = true;
    return true;
  }

  return false;
}

bool
Record1Resolver::isForBkgCdsOverFlown(const FareClassAppSegInfo* prevFcas,
                                     const FareClassAppSegInfo* thisFcas)
{
  return (prevFcas->_paxType == thisFcas->_paxType);
}

const Loc*
Record1Resolver::getLoc(const LocCode& code) const
{
  const std::map<LocCode, const Loc*>::const_iterator i = _locCache.find(code);
  if (i != _locCache.end())
  {
    return i->second;
  }

  const Loc* const res = _trx.dataHandle().getLoc(code, _travelDate);
  _locCache.insert(std::pair<LocCode, const Loc*>(code, res));
  return res;
}

const FareTypeMatrix*
Record1Resolver::getFareTypeMatrix(const FareType& type) const
{
  const std::map<FareType, const FareTypeMatrix*>::const_iterator i =
      _fareTypeMatrixCache.find(type);
  if (i != _fareTypeMatrixCache.end())
  {
    return i->second;
  }

  const FareTypeMatrix* const res = _trx.dataHandle().getFareTypeMatrix(type, _travelDate);
  _fareTypeMatrixCache.insert(std::pair<FareType, const FareTypeMatrix*>(type, res));
  return res;
}

Record1Resolver::CalcMoney::CalcMoney(PricingTrx& trx, CurrencyConversionFacade& ccf, const Itin& itin)
  : Money(NUC),
    _trx(trx),
    _itinMoney(itin.calculationCurrency()),
    _rexSecondRoeItinMoney(itin.calculationCurrency()),
    _ccf(ccf),
    _isIntl(itin.useInternationalRounding()),
    _cache(trx.dataHandle())
{
  if (UNLIKELY((trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
       trx.excTrxType() == PricingTrx::AR_EXC_TRX) &&
      !itin.calcCurrencyOverride().empty() && itin.calcCurrencyOverride() != NUC &&
      itin.calcCurrencyOverride() != USD && trx.billing() && trx.billing()->partitionID() == "WN"))
  {
    _excCurrOverrideNotNuc = true;
  }
  else
    _excCurrOverrideNotNuc = false;
}

void
Record1Resolver::CalcMoney::configForCat25()
{
  _isNucSelectAllowed = false;
}

void
Record1Resolver::CalcMoney::setCurrency(const CurrencyCode& newCur)
{
  setCode(newCur);
  value() = NO_AMT;
}
void
Record1Resolver::CalcMoney::setRT(bool newRT)
{
  _isRT = newRT;
}

void
Record1Resolver::CalcMoney::getFromPTF(PaxTypeFare& paxTypeFare, bool doNotChkNonIATARounding)
{
  if (!doNotChkNonIATARounding)
    _applyNonIATARounding = paxTypeFare.applyNonIATARounding(_trx);

  setRT(paxTypeFare.isRoundTrip());
  setCurrency(paxTypeFare.currency()); // Fare native currency
  bool rexSecondAmtNeeded =
      RexPricingTrx::isRexTrxAndNewItin(_trx) &&
      static_cast<RexPricingTrx&>(_trx).applyReissueExchange() &&
      !static_cast<RexBaseTrx&>(_trx).newItinSecondROEConversionDate().isEmptyDate();

  if (_isRT)
  {
    _itinMoney.value() = paxTypeFare.nucOriginalFareAmount();
    value() = paxTypeFare.originalFareAmount();
    if (UNLIKELY(rexSecondAmtNeeded))
      _rexSecondRoeItinMoney.value() = paxTypeFare.fare()->rexSecondNucOriginalFareAmount();
  }
  else
  {
    _itinMoney.value() =
        paxTypeFare.nucFareAmount(); // FareAmount in converted currency, maynot NUC.
    value() = paxTypeFare.fareAmount(); // FareAmount in local currency
    if (UNLIKELY(rexSecondAmtNeeded))
      _rexSecondRoeItinMoney.value() = paxTypeFare.rexSecondNucFareAmount();
  }
}

void
Record1Resolver::CalcMoney::calcFareAmount()
{
  // don't overwite if calc could find native currency
  if (UNLIKELY(this->value() <= NO_AMT))
  {
    _ccf.convertCalc(*this,
                     _itinMoney,
                     _trx,
                     _isIntl,
                     CurrencyConversionRequest::OTHER,
                     false,
                     nullptr,
                     &_cache,
                     _trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX);
  }
}

MoneyAmount
Record1Resolver::CalcMoney::fareAmount()
{
  calcFareAmount();
  return value();
}

void
Record1Resolver::CalcMoney::setFareAmount(MoneyAmount fareAmt)
{
  value() = fareAmt;
  _itinMoney.value() = NO_AMT;
}

void
Record1Resolver::CalcMoney::putIntoPTF(PaxTypeFare& paxTypeFare, FareInfo& fi)
{
  if (_itinMoney.value() <= NO_AMT)
  {
    adjust(paxTypeFare.fareMarket()->direction() == FMDirection::OUTBOUND);
  }

  bool rexSecondAmtNeeded =
      RexPricingTrx::isRexTrxAndNewItin(_trx) &&
      static_cast<RexPricingTrx&>(_trx).applyReissueExchange() &&
      !static_cast<RexBaseTrx&>(_trx).newItinSecondROEConversionDate().isEmptyDate();

  paxTypeFare.nucOriginalFareAmount() = _itinMoney.value();
  if (UNLIKELY(rexSecondAmtNeeded))
    paxTypeFare.fare()->rexSecondNucOriginalFareAmount() = _rexSecondRoeItinMoney.value();
  fi.currency() = this->code();
  fi.originalFareAmount() = this->value();

  if (_isRT && !_trx.getOptions()->isRtw())
  {
    fi.fareAmount() = this->value() / 2.0;
    CurrencyUtil::halveNUCAmount(_itinMoney.value());
    if (UNLIKELY(rexSecondAmtNeeded))
      CurrencyUtil::halveNUCAmount(_rexSecondRoeItinMoney.value());
  }
  else
  {
    fi.fareAmount() = this->value();
  }

  paxTypeFare.nucFareAmount() = _itinMoney.value();
  if (UNLIKELY(rexSecondAmtNeeded))
    paxTypeFare.rexSecondNucFareAmount() = _rexSecondRoeItinMoney.value();
}

void
Record1Resolver::CalcMoney::setupConverionDateForHistorical()
{
  RexBaseTrx* rexBaseTrx = dynamic_cast<RexBaseTrx*>(&_trx);

  if(rexBaseTrx)
  {
    bool useCurrentDate = false; // Historical
    if (rexBaseTrx->curNewItin()->exchangeReissue() == EXCHANGE)
    {
      if (rexBaseTrx->needRetrieveCurrentFare())
        useCurrentDate = true; // current
    }
    else // Reissue
    {
      if (rexBaseTrx->needRetrieveCurrentFare() && !rexBaseTrx->needRetrieveHistoricalFare() &&
          !rexBaseTrx->needRetrieveTvlCommenceFare() && !rexBaseTrx->needRetrieveKeepFare())
        useCurrentDate = true; // current
    }
    if (useCurrentDate)
    {
      rexBaseTrx->newItinROEConversionDate() = rexBaseTrx->currentTicketingDT();
    }
    else
    {
      if (!rexBaseTrx->previousExchangeDT().isEmptyDate())
      {
        rexBaseTrx->newItinROEConversionDate() = rexBaseTrx->previousExchangeDT();
      }
      else
      {
        rexBaseTrx->newItinROEConversionDate() = rexBaseTrx->originalTktIssueDT();
      }
    }

    if (rexBaseTrx->needRetrieveCurrentFare() &&
        (rexBaseTrx->needRetrieveHistoricalFare() || rexBaseTrx->needRetrieveTvlCommenceFare() ||
            rexBaseTrx->needRetrieveKeepFare()))
    {
      if (useCurrentDate)
      {
        if (!rexBaseTrx->previousExchangeDT().isEmptyDate())
        {
          rexBaseTrx->newItinSecondROEConversionDate() = rexBaseTrx->previousExchangeDT();
        }
        else
        {
          rexBaseTrx->newItinSecondROEConversionDate() = rexBaseTrx->originalTktIssueDT();
        }
      }
      else
        rexBaseTrx->newItinSecondROEConversionDate() = rexBaseTrx->currentTicketingDT();
    }
  }
}

void
Record1Resolver::CalcMoney::adjust(bool isOutbound)
{
  // convert the other way (to itinMoney from this)

  if (UNLIKELY(_trx.excTrxType() == PricingTrx::PORT_EXC_TRX && _itinMoney.code() != "NUC"))
  {
    // exception will be dealed at root use cache
    _ccf.convert(_itinMoney, *this, _trx, false, CurrencyConversionRequest::NO_ROUNDING);
  }
  else
  {
    setupConverionDateForHistorical();
    _ccf.convertCalc(
        _itinMoney, *this, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, nullptr, &_cache);
  }

  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(_trx) &&
      static_cast<RexPricingTrx&>(_trx).applyReissueExchange() &&
      !static_cast<RexBaseTrx&>(_trx).newItinSecondROEConversionDate().isEmptyDate()))
  {
    _ccf.setUseSecondRoeDate(true);
    _rexSecondRoeItinMoney = _itinMoney;
    _ccf.convertCalc(_rexSecondRoeItinMoney,
                     *this,
                     _trx,
                     _isIntl,
                     CurrencyConversionRequest::OTHER,
                     false,
                     nullptr,
                     &_cache);
    _ccf.setUseSecondRoeDate(false);
  }
}

// does put & get for creating base fares.   Don't have NUC, fareInfo has fareAmount
//
void
Record1Resolver::CalcMoney::adjustPTF(PaxTypeFare& paxTypeFare)
{
  _isRT = paxTypeFare.isRoundTrip();

  bool rexSecondAmtNeeded =
      RexPricingTrx::isRexTrxAndNewItin(_trx) &&
      static_cast<RexPricingTrx&>(_trx).applyReissueExchange() &&
      !static_cast<RexBaseTrx&>(_trx).newItinSecondROEConversionDate().isEmptyDate();

  if (paxTypeFare.isConstructed() && _itinMoney.code().equalToConst("NUC"))
  {
    _itinMoney.value() = paxTypeFare.fare()->constructedFareInfo()->constructedNucAmount();
    paxTypeFare.nucOriginalFareAmount() = _itinMoney.value();
    if (UNLIKELY(rexSecondAmtNeeded))
    {
      _rexSecondRoeItinMoney.value() =
          paxTypeFare.fare()->constructedFareInfo()->constructedSecondNucAmount();
      paxTypeFare.fare()->rexSecondNucOriginalFareAmount() = _rexSecondRoeItinMoney.value();
    }
  }
  else
    try
    {
      setCode(paxTypeFare.currency());
      value() = paxTypeFare.originalFareAmount();
      adjust(paxTypeFare.fareMarket()->direction() == FMDirection::OUTBOUND);
      paxTypeFare.nucOriginalFareAmount() = _itinMoney.value();
      if (UNLIKELY(rexSecondAmtNeeded))
        paxTypeFare.fare()->rexSecondNucOriginalFareAmount() = _rexSecondRoeItinMoney.value();
    }
  catch (tse::ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(_logger, "Exception in FareController::CalcMoney::adjustPTFNew: " << ex.what());

    throw;
  }

  if (_isRT && !_trx.getOptions()->isRtw())
  {
    CurrencyUtil::halveNUCAmount(_itinMoney.value());
    if (UNLIKELY(rexSecondAmtNeeded))
      CurrencyUtil::halveNUCAmount(_rexSecondRoeItinMoney.value());
  }

  paxTypeFare.nucFareAmount() = _itinMoney.value();
  if (UNLIKELY(rexSecondAmtNeeded))
    paxTypeFare.rexSecondNucFareAmount() = _rexSecondRoeItinMoney.value();
}

/* ret =0 fail, =1 used cur1, =2 used cur2 */
int
Record1Resolver::CalcMoney::pickAmt(Money& native,
                                   Money& nuc,
                                   const MoneyAmount amt1,
                                   const CurrencyCode cur1,
                                   const MoneyAmount amt2,
                                   const CurrencyCode cur2)
{
  int ret = 0;
  if (cur1 == native.code())
  {
    native.value() = amt1;
    _ccf.convertCalc(
        nuc, native, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, nullptr, &_cache);
    ret = 1;
  }
  else if (cur2 == native.code())
  {
    native.value() = amt2;
    _ccf.convertCalc(
        nuc, native, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, nullptr, &_cache);
    ret = 2;
  }
  else if (amt1 == 0.0)
  {
    native.value() = 0.0;
    nuc.value() = 0.0;
    ret = 1;
  }
  else if (_isNucSelectAllowed)
  {
    Money m1(amt1, cur1);
    _ccf.convertCalc(nuc, m1, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, nullptr, &_cache);
    ret = 1;
    if (!cur2.empty())
    {
      Money m2(amt2, cur2);
      Money nuc2(nuc.code());
      _ccf.convertCalc(
          nuc2, m2, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, nullptr, &_cache);
      if (nuc2.value() < nuc.value())
      {
        nuc.value() = nuc2.value();
        ret = 2;
      }
    }

    _ccf.convertCalc(
        native, nuc, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, nullptr, &_cache);

  } // endif - NUC compare

  return ret;
}

void
Record1Resolver::CalcMoney::doPercent(const MoneyAmount percent)
{
  if (UNLIKELY(_applyNonIATARounding))
  {
    _itinMoney.setApplyNonIATARounding();
    this->setApplyNonIATARounding();
  }


  value() *= (percent / 100.0);
  _ccf.round(*this, _trx, _isIntl);
  CurrencyUtil::truncateNonNUCAmount(value(), this->noDec());

  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
      _itinMoney.code() == NUC && !_isIntl))
  {
    _ccf.convertCalc(
        _itinMoney, *this, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, 0, &_cache);
  }
  else
  {
    _itinMoney.value() *= (percent / 100.0);
    if (UNLIKELY(_excCurrOverrideNotNuc))
      CurrencyUtil::truncateNonNUCAmount(_itinMoney.value(), this->noDec());
    else
      _ccf.round(_itinMoney, _trx, _isIntl);
  }

  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(_trx) &&
      static_cast<RexPricingTrx&>(_trx).applyReissueExchange() &&
      !static_cast<RexBaseTrx&>(_trx).newItinSecondROEConversionDate().isEmptyDate()))
  {
    _rexSecondRoeItinMoney.value() *= (percent / 100.0);
    if (_excCurrOverrideNotNuc)
      CurrencyUtil::truncateNonNUCAmount(_rexSecondRoeItinMoney.value(), this->noDec());
    else
      _ccf.round(_rexSecondRoeItinMoney, _trx, _isIntl);
  }
}

int
Record1Resolver::CalcMoney::doAdd(const MoneyAmount amt1,
                                 const CurrencyCode cur1,
                                 const MoneyAmount amt2,
                                 const CurrencyCode cur2)
{
  Money temp(this->code());
  Money tempNuc(_itinMoney.code());

  bool addToSecondNucAmt =
      RexPricingTrx::isRexTrxAndNewItin(_trx) &&
      static_cast<RexPricingTrx&>(_trx).applyReissueExchange() &&
      !static_cast<RexBaseTrx&>(_trx).newItinSecondROEConversionDate().isEmptyDate();

  int ret = pickAmt(temp, tempNuc, amt1, cur1, amt2, cur2);
  if (UNLIKELY(((ret == 1) && (cur1 == temp.code()) && (cur1 == _itinMoney.code())) ||
      ((ret == 2) && (cur2 == temp.code()) && (cur2 == _itinMoney.code()))))
  {
    _itinMoney.value() += temp.value();
    if (addToSecondNucAmt)
      _rexSecondRoeItinMoney.value() += temp.value();
    this->value() += temp.value(); // Add with amount in Native currency
  }
  else if (LIKELY(ret != 0))
  {
    _itinMoney.value() += tempNuc.value();
    if (UNLIKELY(addToSecondNucAmt))
      _rexSecondRoeItinMoney.value() += tempNuc.value();
    this->value() += temp.value(); // Add with amount in Native currency
  }

  return ret;
}

int
Record1Resolver::CalcMoney::doMinus(const MoneyAmount amt1,
                                   const CurrencyCode cur1,
                                   const MoneyAmount amt2,
                                   const CurrencyCode cur2)
{
  Money temp(this->code());
  Money tempNuc(_itinMoney.code());

  bool minusSecondNucAmt =
      RexPricingTrx::isRexTrxAndNewItin(_trx) &&
      static_cast<RexPricingTrx&>(_trx).applyReissueExchange() &&
      !static_cast<RexBaseTrx&>(_trx).newItinSecondROEConversionDate().isEmptyDate();

  int ret = pickAmt(temp, tempNuc, amt1, cur1, amt2, cur2);
  if (UNLIKELY(((ret == 1) && (cur1 == temp.code()) && (cur1 == _itinMoney.code())) ||
      ((ret == 2) && (cur2 == temp.code()) && (cur2 == _itinMoney.code()))))
  {
    _itinMoney.value() -= temp.value();
    if (minusSecondNucAmt)
      _rexSecondRoeItinMoney.value() -= temp.value();
    this->value() -= temp.value(); // Minus with amount in Native currency
  }
  else if (LIKELY(ret != 0))
  {
    _itinMoney.value() -= tempNuc.value();
    if (UNLIKELY(minusSecondNucAmt))
      _rexSecondRoeItinMoney.value() -= tempNuc.value();
    this->value() -= temp.value(); // Minus with amount in Native currency
  }
  if (LIKELY(_rexSecondRoeItinMoney.value() < EPSILON))
    _rexSecondRoeItinMoney.value() = 0.0;

  return ret;
}

// must initialze first [ e.g. getFromPTF() or setCurrency()/setRT ]
int
Record1Resolver::CalcMoney::getFromSpec(const MoneyAmount amt1,
                                       const CurrencyCode cur1,
                                       const MoneyAmount amt2,
                                       const CurrencyCode cur2)
{
  int ret = pickAmt(*this, _itinMoney, amt1, cur1, amt2, cur2);

  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(_trx) &&
               static_cast<RexPricingTrx&>(_trx).applyReissueExchange() &&
               !static_cast<RexBaseTrx&>(_trx).newItinSecondROEConversionDate().isEmptyDate()))
  {
    if (((ret == 1) && (cur1 == this->code()) && (cur1 == _itinMoney.code())) ||
        ((ret == 2) && (cur2 == this->code()) && (cur2 == _itinMoney.code())))
    {
      _rexSecondRoeItinMoney.value() = this->value();
    }
    else if (ret != 0)
    {
      _rexSecondRoeItinMoney.value() = _itinMoney.value();
    }
  }
  return ret;
}

Record1Resolver::CalcMoney&
Record1Resolver::CalcMoney::
operator=(const Record1Resolver::CalcMoney& rhs)
{
  if (UNLIKELY(&rhs == this))
    return *this;
  _amount = rhs.value();
  _currencyCode = rhs.code();
  _isNuc = rhs._isNuc;
  _itinMoney.value() = rhs._itinMoney.value();
  _rexSecondRoeItinMoney.value() = rhs._rexSecondRoeItinMoney.value();
  _isRT = rhs._isRT;
  _isNucSelectAllowed = rhs._isNucSelectAllowed;
  // other values from ctor should already be the same
  return *this;
} // lint !e1539

}
