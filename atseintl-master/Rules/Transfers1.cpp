//-------------------------------------------------------------------
//
//  File:        Transfers1.cpp
//  Created:     May 23, 2006
//  Authors:     Andrew Ahmad
//
//  Description: Version 2 of Transfers processing code. This
//               second version is required for processing the
//               new Cat-09 record format as mandated by ATPCO.
//
//
//  Copyright Sabre 2004-2006
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

#include "Rules/Transfers1.h"

#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/RtwUtil.h"
#include "Common/TruePaxType.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/SamePoint.h"
#include "DBAccess/SurfaceTransfersInfo.h"
#include "DBAccess/TransfersInfo1.h"
#include "DBAccess/TransfersInfoSeg1.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/TransfersInfoWrapper.h"
#include "Rules/TransfersStopoversUtil.h"
#include "Rules/TransfersTravelSegWrapper.h"
#include "Rules/TSIGateway.h"
#include "Util/BranchPrediction.h"

#include <algorithm>

namespace tse
{
FALLBACK_DECL(cat9RecurringSegFcScope);
FALLBACK_DECL(fallbackapo37432Cat9BlankIORecurSeg);
FALLBACK_DECL(cat9FixMaxTransfers)

static Logger
logger("atseintl.Rules.Transfers1");

const std::string Transfers1::NUM_TRANSFERS_UNLIMITED = "XX";

void
Transfers1::TransfersInfoSegMarkup::initialize(const TransfersInfoSeg1* trInfoSeg)
{
  if (LIKELY(trInfoSeg))
  {
    _trInfoSeg = trInfoSeg;

    if ((_trInfoSeg->primeOnline() != ALLOW_BLANK) || (_trInfoSeg->sameOnline() != ALLOW_BLANK) ||
        (_trInfoSeg->primeInterline() != ALLOW_BLANK) ||
        (_trInfoSeg->otherInterline() != ALLOW_BLANK))
    {
      _checkTransferType = true;
    }

    Indicator inOutInd = _trInfoSeg->outInPortion();
    if (inOutInd != Transfers1::SEG_INOUT_BLANK)
    {
      _checkInOut = true;

      if (inOutInd == Transfers1::SEG_INOUT_OUT)
      {
        _inOutOut = true;
      }
      else if (inOutInd == Transfers1::SEG_INOUT_IN)
      {
        _inOutIn = true;
      }
      else if (inOutInd == Transfers1::SEG_INOUT_EITHER)
      {
        _inOutEither = true;
      }
      else if (LIKELY(inOutInd == Transfers1::SEG_INOUT_BOTH))
      {
        _inOutBoth = true;
      }
    }

    std::string noTransfers = trInfoSeg->noTransfersPermitted();

    if (noTransfers.empty() || (noTransfers == Transfers1::NUM_TRANSFERS_UNLIMITED))
    {
      _numTransfersUnlimited = true;
    }
    else
    {
      _numTransfers = atoi(noTransfers.c_str());
    }
  }
}

Record3ReturnTypes
Transfers1::validate(PricingTrx& trx,
                     const RuleItemInfo* rule,
                     const FarePath& farePath,
                     const PricingUnit& pricingUnit,
                     const FareUsage& fareUsage)
{
  LOG4CXX_INFO(logger, "Entering Transfers1::validate()");

  const TransfersInfoWrapper* trInfoWrapper = dynamic_cast<const TransfersInfoWrapper*>(rule);

  if (UNLIKELY(!trInfoWrapper))
  {
    LOG4CXX_ERROR(logger, "Rules.Transfers1::validate(): trInfoWrapper = 0");
    LOG4CXX_INFO(logger, "Leaving Transfers1::validate()");
    return tse::FAIL;
  }

  const TransfersInfo1* trInfo = trInfoWrapper->trInfo();

  if (UNLIKELY(!trInfo))
  {
    LOG4CXX_ERROR(logger, "Rules.Transfers1::validate(): trInfo = 0");
    LOG4CXX_INFO(logger, "Leaving Transfers1::validate()");
    return tse::FAIL;
  }

  const PaxTypeFare* ptf = fareUsage.paxTypeFare();
  const FareMarket* fm = nullptr;

  if (LIKELY(ptf))
  {
    fm = fareUsage.paxTypeFare()->fareMarket();
  }

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  bool diagEnabled = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic309 &&
               diagPtr->parseFareMarket(trx, *fm)))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(trx);
    diagPtr->enable(Diagnostic309);
    diagEnabled = true;
    printDiagHeader(trx, *diagPtr, *trInfoWrapper, *trInfo, fm);
  }

  Record3ReturnTypes ret = tse::SKIP;

  if (UNLIKELY(!checkPreconditions(*trInfo, ret, diagPtr)))
  {
    if (UNLIKELY(diagPtr))
    {
      diagPtr->flushMsg();
    }
    LOG4CXX_INFO(logger, "Leaving Transfers1::validate()");
    return ret;
  }

  ProcessingResult res = Transfers1::CONTINUE;
  std::string failReason;

  // Processing order (current FU first)
  TravelSegMarkupContainer tsmProcOrder;
  TravelSegMarkupContainer tsmPUOrder;

  // if R3 directionality is '3' or '4' we skiped surface sector validation on FC level
  // ned to do this now

  bool needValidateSurfaces = _needSurfValidationForFP || ptf->puRuleValNeeded();
  needValidateSurfaces |= trInfoWrapper->isRelationOrExists();

  if (needValidateSurfaces)
  {
    res = processSurfaceSectorRestrictions(trx, *trInfo, *fm, diagEnabled ? &failReason : nullptr);
  }

  if (LIKELY((res == Transfers1::PASS) || (res == Transfers1::CONTINUE)))
  {
    if (LIKELY(initializeTravelSegMarkupContainer(
                  trx, tsmProcOrder, tsmPUOrder, pricingUnit, fareUsage, *trInfoWrapper, farePath.itin())))
    {
      res = processTransfers(trx,
                             tsmProcOrder,
                             tsmPUOrder,
                             *trInfoWrapper,
                             &farePath,
                             &pricingUnit,
                             fm,
                             ptf,
                             failReason,
                             &fareUsage);

      if (res == Transfers1::PASS || res == Transfers1::CONTINUE)
      {
        processSurcharges(trx, tsmPUOrder, *trInfoWrapper, *trInfo, fareUsage, farePath);
      }
    }
    else
    {
      failReason = "INTERNAL PROCESSING ERROR";
    }
  }
  if (UNLIKELY(diagEnabled))
  {
    printDiagnostics(*diagPtr, *trInfo, tsmPUOrder, pricingUnit);
  }

  if (res == Transfers1::FAIL)
  {
    ret = tse::FAIL;
  }
  else if (UNLIKELY(res == Transfers1::PASS))
  {
    ret = tse::PASS;
  }
  else if (LIKELY(res == Transfers1::CONTINUE))
  {
    ret = tse::PASS;
  }
  else if (res == Transfers1::SKIP)
  {
    ret = tse::SKIP;
  }
  else if (res == Transfers1::STOP)
  {
    ret = tse::FAIL;
  } // keep it?

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "TRANSFERS : ";

    if (ret == tse::PASS)
      (*diagPtr) << "PASS - NOT FINAL VALIDATION";
    else if (ret == tse::SKIP)
      (*diagPtr) << "SKIP";
    else if (ret == tse::FAIL)
      (*diagPtr) << "FAIL - " << failReason;
    else if (ret == tse::STOP)
      (*diagPtr) << "STOP - " << failReason;

    (*diagPtr) << std::endl;

    diagPtr->flushMsg();
  }

  LOG4CXX_INFO(logger, "Leaving Transfers1::validate()");
  return ret;
}

Record3ReturnTypes
Transfers1::validate(PricingTrx& trx,
                     Itin& itin,
                     const PaxTypeFare& fare,
                     const RuleItemInfo* rule,
                     const FareMarket& fareMarket)
{
  return validate(trx, itin, fare, rule, fareMarket, false);
}

Record3ReturnTypes
Transfers1::validate(PricingTrx& trx,
                     Itin& itin,
                     const PaxTypeFare& fare,
                     const RuleItemInfo* rule,
                     const FareMarket& fareMarket,
                     bool performFullFmVal)
{
  LOG4CXX_INFO(logger, "Entering Transfers1::validate()");

  const TransfersInfoWrapper* trInfoWrapper = dynamic_cast<const TransfersInfoWrapper*>(rule);

  if (UNLIKELY(!trInfoWrapper))
  {
    LOG4CXX_ERROR(logger, "Rules.Transfers1::validate(): trInfoWrapper = 0");
    LOG4CXX_INFO(logger, "Leaving Transfers1::validate()");
    return tse::FAIL;
  }

  const TransfersInfo1* trInfo = trInfoWrapper->trInfo();

  if (UNLIKELY(!trInfo))
  {
    LOG4CXX_ERROR(logger, "Rules.Transfers1::validate(): trInfo = 0");
    LOG4CXX_INFO(logger, "Leaving Transfers1::validate()");
    return tse::FAIL;
  }

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  bool diagEnabled = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic309 &&
               diagPtr->parseFareMarket(trx, fareMarket)))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(trx);
    diagPtr->enable(Diagnostic309);
    diagEnabled = true;
    printDiagHeader(trx, *diagPtr, *trInfoWrapper, *trInfo, &fareMarket);
  }

  Record3ReturnTypes ret = tse::SKIP;

  if (UNLIKELY(!checkPreconditions(*trInfo, ret, diagPtr)))
  {
    if (UNLIKELY(diagPtr))
    {
      diagPtr->flushMsg();
    }
    LOG4CXX_INFO(logger, "Leaving Transfers1::validate()");
    return ret;
  }

  TravelSegMarkupContainer tsmCon;

  std::string failReason;
  ProcessingResult res = Transfers1::SKIP;

  if (LIKELY(initializeTravelSegMarkupContainer(trx, tsmCon, fareMarket)))
  {
    res = processSurfaceSectorRestrictions(trx, *trInfo, fareMarket, diagEnabled ? &failReason : nullptr);

    if ((res == Transfers1::PASS) || (res == Transfers1::CONTINUE))
    {
      if ((atoi(trInfo->noTransfersMin().c_str()) > 0) && (fare.owrt() != ONE_WAY_MAY_BE_DOUBLED) &&
          (fare.owrt() != ONE_WAY_MAYNOT_BE_DOUBLED) && !performFullFmVal)
      {
        ret = tse::SOFTPASS;
        if (UNLIKELY(diagEnabled))
        {
          (*diagPtr) << "TRANSFERS : PASS - NOT ONEWAY FARE."
                     << " NEED REVALIDATION" << std::endl;

          diagPtr->flushMsg();
        }
        trInfoWrapper->ignoreNoMatch() = true;

        LOG4CXX_INFO(logger, "Leaving Transfers1::validate()");
        return ret;
      }

      // If any segment has a TSI then return PASS
      const std::vector<TransfersInfoSeg1*>& trInfoSegs = trInfo->segs();
      if (!trInfoSegs.empty())
      {
        std::vector<TransfersInfoSeg1*>::const_iterator iter = trInfoSegs.begin();
        std::vector<TransfersInfoSeg1*>::const_iterator iterEnd = trInfoSegs.end();

        for (; iter != iterEnd; ++iter)
        {
          if ((*iter)->tsi())
          {
            if (UNLIKELY(diagEnabled))
            {
              (*diagPtr) << "TRANSFERS : PASS - "
                         << " TSI CHECK - NEED REVALIDATION" << std::endl;

              diagPtr->flushMsg();
            }
            trInfoWrapper->ignoreNoMatch() = true;

            LOG4CXX_INFO(logger, "Leaving Transfers1::validate()");
            return tse::SOFTPASS;
          }
        }
      }

      res = processTransfers(
          trx, tsmCon, tsmCon, *trInfoWrapper, nullptr, nullptr, &fareMarket, &fare, failReason, nullptr);
    }
  }
  else
  {
    failReason = "INTERNAL PROCESSING ERROR";
  }

  if (res == Transfers1::FAIL)
  {
    ret = tse::FAIL;
  }
  else if (UNLIKELY(res == Transfers1::PASS))
  {
    ret = tse::PASS;
  }
  else if (LIKELY(res == Transfers1::CONTINUE))
  {
    ret = tse::PASS;
  }
  else if (res == Transfers1::SKIP)
  {
    ret = tse::SKIP;
  }
  else if (res == Transfers1::STOP)
  {
    ret = tse::FAIL;
  } // keep it?

  // Transfers can only SOFTPASS for Fare Validator phase.
  if (ret == tse::PASS)
  {
    ret = tse::SOFTPASS;
  }

  if (UNLIKELY(diagEnabled))
  {
    printDiagnostics(*diagPtr, *trInfo, tsmCon, fareMarket);

    (*diagPtr) << "TRANSFERS : ";

    if (ret == tse::PASS)
      (*diagPtr) << "PASS - NOT FINAL VALIDATION";
    else if (ret == tse::SOFTPASS)
      (*diagPtr) << "SOFTPASS - NOT FINAL VALIDATION";
    else if (ret == tse::SKIP)
      (*diagPtr) << "SKIP";
    else if (ret == tse::FAIL)
      (*diagPtr) << "FAIL - " << failReason;
    else if (ret == tse::STOP)
      (*diagPtr) << "STOP - " << failReason;

    (*diagPtr) << std::endl;

    diagPtr->flushMsg();
  }

  LOG4CXX_INFO(logger, "Leaving Transfers1::validate()");
  return ret;
}

bool
Transfers1::checkPreconditions(const TransfersInfo1& trInfo,
                               Record3ReturnTypes& ret,
                               DiagCollector* dc)

{
  if (UNLIKELY(trInfo.unavailTag() == Transfers1::UNAVAIL_TAG_NOT_AVAIL))
  {
    if (UNLIKELY(dc))
    {
      (*dc) << "TRANSFERS : FAIL - R3 NOT AVAILABLE FOR USE" << std::endl;
    }
    ret = tse::FAIL;
    return false;
  }

  if (UNLIKELY(trInfo.unavailTag() == Transfers1::UNAVAIL_TAG_TEXT_ONLY))
  {
    if (UNLIKELY(dc))
    {
      (*dc) << "TRANSFERS : PASS - TEXT PURPOSE ONLY" << std::endl;
    }
    ret = tse::PASS;
    return false;
  }

  if (UNLIKELY(isRtw()))
  {
    bool maxSpecified = isNumTransfersSpecified(trInfo.noTransfersMax());
    bool inOutSpecified = isNumTransfersSpecified(trInfo.noOfTransfersIn()) ||
                          isNumTransfersSpecified(trInfo.noOfTransfersOut());

    if (!maxSpecified && inOutSpecified)
    {
      if (UNLIKELY(dc))
      {
        (*dc) << "TRANSFERS : SKIP - MAX NOT SPECIFIED FOR RTW" << std::endl;
      }
      ret = tse::SKIP;
      return false;
    }
  }

  return true;
}

bool
Transfers1::initializeTravelSegMarkupContainer(PricingTrx& trx,
                                               TravelSegMarkupContainer& tsmCon,
                                               const FareMarket& fm)
{
  std::vector<TravelSeg*>::const_iterator iter = fm.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = fm.travelSeg().end();

  int16_t segmentNumber = 1;

  for (; iter != iterEnd; ++iter, ++segmentNumber)
  {
    TravelSeg* ts = *iter;
    AirSeg* as = dynamic_cast<AirSeg*>(ts);

    if (as)
    {
      TravelSegMarkup* tsm = nullptr;
      _dataHandle.get(tsm);

      if (UNLIKELY(!tsm))
      {
        LOG4CXX_ERROR(
            logger,
            "Rules.Transfers1::initializeTravelSegMarkupContainer(): DataHandle::get() returned 0");
        return false;
      }

      // lint --e{413}
      tsm->travelSeg() = ts;
      tsm->segmentNumber() = segmentNumber;
      tsm->geoTravelType() = fm.geoTravelType();
      tsm->carrier() = as->carrier();
      tsm->fareMarket() = &fm;
      tsm->flightNumber() = as->flightNumber();
      tsm->governingCarrier() = fm.governingCarrier();

      if (!tsmCon.empty())
      {
        tsmCon.back()->nextTravelSeg() = ts;
        tsmCon.back()->carrierOut() = as->carrier();
        tsmCon.back()->flightNumberOut() = as->flightNumber();
      }

      if (fm.direction() == FMDirection::INBOUND)
      {
        tsm->direction() = TravelSegMarkup::INBOUND;
      }
      else if (fm.direction() == FMDirection::OUTBOUND)
      {
        tsm->direction() = TravelSegMarkup::OUTBOUND;
      }
      else
      {
        tsm->direction() = TravelSegMarkup::UNKNOWN;
      }

      tsmCon.push_back(tsm);
    }
    else if (!tsmCon.empty() && ts != fm.travelSeg().back())
        tsmCon.back()->altOffPointGeo() = ts->destination();
          }
  return true;
}

bool
Transfers1::isSameVCTRSequence(const FareUsage* fu,
                               const FareUsage* ruleFu,
                               const TransfersInfoWrapper& trInfoWrapper)
{
  bool isSameVCTRSeq = true;

  if ((fu != ruleFu) &&
      ((!trInfoWrapper.isSameVCTR(*fu)) ||
       (fu->transfersMatchingVCTR().sequenceNumber() != trInfoWrapper.getVCTR().sequenceNumber())))
  {
    isSameVCTRSeq = false;
  }

  return isSameVCTRSeq;
}

bool
Transfers1::initializeTravelSegMarkupContainer(PricingTrx& trx,
                                               TravelSegMarkupContainer& tsmProcOrder,
                                               TravelSegMarkupContainer& tsmPUOrder,
                                               const PricingUnit& pu,
                                               const FareUsage& fareUsage,
                                               const TransfersInfoWrapper& trInfoWrapper,
                                               const Itin* itin)
{
  trInfoWrapper.transferFCscope() = trInfoWrapper.transferFCscopeInitial();
  trInfoWrapper.transferFailsPU() = trInfoWrapper.transferFailsPUInitial();
  trInfoWrapper.recurringFCscope() = false;

  int16_t segmentNumber = 1;
  bool isHalfRoundTrip = (pu.puType() != PricingUnit::Type::ONEWAY);

  const TransfersInfo1* trInfo = trInfoWrapper.trInfo();

  bool isCmdPricing = false;
  if (UNLIKELY(((fareUsage.paxTypeFare()->isFareByRule() || !fareUsage.cat25Fare()) &&
                 fareUsage.paxTypeFare()->isCmdPricing()) ||
                (fareUsage.cat25Fare() && fareUsage.cat25Fare()->isCmdPricing())))
  {
    isCmdPricing = true;
  }

  const bool fareComponentOnly = trInfo->noTransfersMax().empty() || trInfoWrapper.transferFCscope();

  TravelSegMarkupContainer secondaryContainer;

  for (FareUsage* fu : pu.fareUsage())
  {
    const PaxTypeFare* const ptf = fu->paxTypeFare();
    const FareMarket* const fm = LIKELY(ptf) ? ptf->fareMarket() : nullptr;
    TravelSegMarkupContainer tempContainer;

    const bool isMirrorImage = isHalfRoundTrip && isSameVCTRSequence(fu, &fareUsage, trInfoWrapper);
    const bool sameFU = (fu == &fareUsage);
    const bool swapFareUsageOrder = fu->isInbound() && sameFU && !isMirrorImage && !isCmdPricing;

    bool validateEntireRule = true;

    if ((fareComponentOnly && (fu != &fareUsage)) ||
        (!checkIOIndTransferInfoSegs(trInfoWrapper, fareUsage, sameFU)))
    {
      validateEntireRule = false;
      if (fallback::cat9RecurringSegFcScope(&trx))
        trInfoWrapper.transferFCscope() = true;
    }

    for (TravelSeg* ts : fu->travelSeg())
    {
      AirSeg* as = ts->toAirSeg();

      if (LIKELY(itin))
        segmentNumber = itin->segmentOrder(ts);
      else
        ++segmentNumber;

      if (as)
      {
        TravelSegMarkup* tsm = nullptr;
        _dataHandle.get(tsm);

        if (UNLIKELY(!tsm))
        {
          LOG4CXX_ERROR(logger,
                        "Rules.Transfers1::initializeTravelSegMarkupContainer(): "
                        "DataHandle::get() returned 0");
          return false;
        }

        // lint --e{413}
        tsm->travelSeg() = ts;
        tsm->segmentNumber() = segmentNumber;
        tsm->geoTravelType() = pu.geoTravelType();
        tsm->carrier() = as->carrier();
        tsm->flightNumber() = as->flightNumber();
        tsm->fareUsage() = fu;
        tsm->fareMarket() = fm;
        tsm->validateEntireRule() = validateEntireRule;

        if (LIKELY(fm))
          tsm->governingCarrier() = fm->governingCarrier();

        tsm->direction() = fu->isInbound() ? tsm->direction() = TravelSegMarkup::INBOUND
                                           : tsm->direction() = TravelSegMarkup::OUTBOUND;

        if (fu->isInbound() && !swapFareUsageOrder)
          tempContainer.push_front(tsm);
        else
          tempContainer.push_back(tsm);

        if (!tsmPUOrder.empty())
        {
          tsmPUOrder.back()->nextTravelSeg() = ts;
          tsmPUOrder.back()->carrierOut() = as->carrier();
          tsmPUOrder.back()->flightNumberOut() = as->flightNumber();
        }

        tsmPUOrder.push_back(tsm);
      }
      else if (!tsmPUOrder.empty() && ts != fu->travelSeg().back())
          tsmPUOrder.back()->altOffPointGeo() = ts->destination();
            }

    for (TravelSegMarkup* tsm : tempContainer)
    {
      TravelSegMarkupContainer& target = validateEntireRule ? tsmProcOrder : secondaryContainer;

      if (swapFareUsageOrder)
        target.push_front(tsm);
      else
        target.push_back(tsm);
    }
  }

  for (TravelSegMarkup* tsm : secondaryContainer)
    tsmProcOrder.push_back(tsm);

  return true;
}

void
Transfers1::initializeTransfersInfoSegMarkup(PricingTrx& trx,
                                             DataHandle& localDataHandle,
                                             std::vector<TransfersInfoSegMarkup*>& trInfoSegMarkup,
                                             const TransfersInfo1& trInfo,
                                             const std::vector<TransfersInfoSeg1*>& segs,
                                             const TravelSegMarkupContainer& tsmCon,
                                             bool& allSegsGeoApplOK,
                                             bool& allSegsGeoApplNot,
                                             bool& geoRestrictionsExist)
{
  allSegsGeoApplOK = true;
  allSegsGeoApplNot = true;
  geoRestrictionsExist = false;

  int16_t segNumTransfersSum = 0;

  std::vector<TransfersInfoSeg1*>::const_iterator iter = segs.begin();
  std::vector<TransfersInfoSeg1*>::const_iterator iterEnd = segs.end();

  for (; iter != iterEnd; ++iter)
  {
    TransfersInfoSeg1* trInfoSeg = *iter;

    if (UNLIKELY(isRtw() && ignoreRecurringSegForRtw(*trInfoSeg)))
      continue;

    TransfersInfoSegMarkup* trISM = nullptr;
    localDataHandle.get(trISM);

    if (LIKELY(trISM))
    {
      trISM->initialize(trInfoSeg);
      trInfoSegMarkup.push_back(trISM);
    }

    if (trInfoSeg->transferAppl() != Transfers1::SEG_GEO_APPL_NOT_PERMITTED)
    {
      allSegsGeoApplNot = false;
      segNumTransfersSum += trISM->numTransfers();
    }
    else
    {
      allSegsGeoApplOK = false;
    }

    if (trInfoSeg->loc1().locType() != LOCTYPE_NONE ||
        trInfoSeg->loc2().locType() != LOCTYPE_NONE || trInfoSeg->tsi() != 0)
    {
      geoRestrictionsExist = true;
    }

    trISM->tsmBtwnOutFirst() = tsmCon.end();
    trISM->tsmBtwnOutLast() = tsmCon.end();
    trISM->tsmBtwnInFirst() = tsmCon.end();
    trISM->tsmBtwnInLast() = tsmCon.end();

    if (trInfoSeg->betweenAppl() == Transfers1::SEG_BETWEEN_APPL_BETWEEN)
    {
      TravelSegMarkupCI outFirst1 = tsmCon.end();
      TravelSegMarkupCI outFirst2 = tsmCon.end();
      TravelSegMarkupCI outLast1 = tsmCon.end();
      TravelSegMarkupCI outLast2 = tsmCon.end();
      TravelSegMarkupCI inFirst1 = tsmCon.end();
      TravelSegMarkupCI inFirst2 = tsmCon.end();
      TravelSegMarkupCI inLast1 = tsmCon.end();
      TravelSegMarkupCI inLast2 = tsmCon.end();

      const LocKey loc1 = trInfoSeg->loc1();
      const LocKey loc2 = trInfoSeg->loc2();

      Zone zone;
      if (trInfoSeg->zoneTblItemNo())
      {
        std::ostringstream strZone;
        strZone << trInfoSeg->zoneTblItemNo();
        zone = strZone.str();
      }

      TravelSegMarkupCI tsmIter = tsmCon.begin();
      TravelSegMarkupCI tsmIterEnd = tsmCon.end();

      for (; tsmIter != tsmIterEnd; ++tsmIter)
      {
        const TravelSegMarkup* tsm = *tsmIter;
        const TravelSeg* ts = tsm->travelSeg();

        if (tsm->direction() == TravelSegMarkup::INBOUND)
        {
          if (checkIsInLoc(trx, *(ts->origin()), loc1, trInfo.vendor()))
          {
            inFirst1 = tsmIter;
          }

          if (inLast1 == tsmIterEnd)
          {
            if (loc2.locType() != LOCTYPE_NONE)
            {
              if (checkIsInLoc(trx, *(ts->destination()), loc2, trInfo.vendor()))
              {
                inLast1 = tsmIter;
              }
            }
            else if (trInfoSeg->zoneTblItemNo())
            {
              if (LocUtil::isInZone(*(ts->destination()), trInfo.vendor(), zone, USER_DEFINED))
              {
                inLast1 = tsmIter;
              }
            }
          }

          if (LIKELY(loc2.locType() != LOCTYPE_NONE))
          {
            if (checkIsInLoc(trx, *(ts->origin()), loc2, trInfo.vendor()))
            {
              inFirst2 = tsmIter;
            }
          }
          else if (trInfoSeg->zoneTblItemNo())
          {
            if (LocUtil::isInZone(*(ts->origin()), trInfo.vendor(), zone, USER_DEFINED))
            {
              inFirst2 = tsmIter;
            }
          }

          if (inLast2 == tsmIterEnd)
          {
            if (checkIsInLoc(trx, *(ts->destination()), loc1, trInfo.vendor()))
            {
              inLast2 = tsmIter;
            }
          }
        }
        else
        {
          if (checkIsInLoc(trx, *(ts->origin()), loc1, trInfo.vendor()))
          {
            outFirst1 = tsmIter;
          }

          if (outLast1 == tsmIterEnd)
          {
            if (loc2.locType() != LOCTYPE_NONE)
            {
              if (checkIsInLoc(trx, *(ts->destination()), loc2, trInfo.vendor()))
              {
                outLast1 = tsmIter;
              }
            }
            else if (trInfoSeg->zoneTblItemNo())
            {
              if (LocUtil::isInZone(*(ts->destination()), trInfo.vendor(), zone, USER_DEFINED))
              {
                outLast1 = tsmIter;
              }
            }
          }

          if (LIKELY(loc2.locType() != LOCTYPE_NONE))
          {
            if (checkIsInLoc(trx, *(ts->origin()), loc2, trInfo.vendor()))
            {
              outFirst2 = tsmIter;
            }
          }
          else if (trInfoSeg->zoneTblItemNo())
          {
            if (LocUtil::isInZone(*(ts->origin()), trInfo.vendor(), zone, USER_DEFINED))
            {
              outFirst2 = tsmIter;
            }
          }

          if (outLast2 == tsmIterEnd)
          {
            if (checkIsInLoc(trx, *(ts->destination()), loc1, trInfo.vendor()))
            {
              outLast2 = tsmIter;
            }
          }
        }
      }

      if ((outFirst1 == tsmCon.end()) || (outLast1 == tsmCon.end()) || (outFirst1 > outLast1))
      {
        if ((outFirst2 != tsmCon.end()) && (outLast2 != tsmCon.end()) && (outFirst2 <= outLast2))
        {
          trISM->tsmBtwnOutFirst() = outFirst2;
          trISM->tsmBtwnOutLast() = outLast2;
        }
      }
      else if ((outFirst1 != tsmCon.end()) && (outLast1 != tsmCon.end()))
      {
        trISM->tsmBtwnOutFirst() = outFirst1;
        trISM->tsmBtwnOutLast() = outLast1;
      }

      if ((inFirst1 == tsmCon.end()) || (inLast1 == tsmCon.end()) || (inFirst1 > inLast1))
      {
        if ((inFirst2 != tsmCon.end()) && (inLast2 != tsmCon.end()) && (inFirst2 <= inLast2))
        {
          trISM->tsmBtwnInFirst() = inFirst2;
          trISM->tsmBtwnInLast() = inLast2;
        }
      }
      else if ((inFirst1 != tsmCon.end()) && (inLast1 != tsmCon.end()))
      {
        trISM->tsmBtwnInFirst() = inFirst1;
        trISM->tsmBtwnInLast() = inLast1;
      }
    }
  }
}

Transfers1::ProcessingResult
Transfers1::processTransfers(PricingTrx& trx,
                             TravelSegMarkupContainer& tsmProcOrder,
                             TravelSegMarkupContainer& tsmPUOrder,
                             const TransfersInfoWrapper& trInfoWrapper,
                             const FarePath* fp,
                             const PricingUnit* pu,
                             const FareMarket* fm,
                             const PaxTypeFare* ptf,
                             std::string& failReason,
                             const FareUsage* fu)
{
  const TransfersInfo1* trInfo = trInfoWrapper.trInfo();

  ProcessingResult res = identifyTransfers(trx, tsmProcOrder, *trInfo, trInfoWrapper, fu);

  identifyGateways(trx, tsmProcOrder, *trInfo, pu, fm);

  if (UNLIKELY((res == Transfers1::FAIL) || (res == Transfers1::SKIP)))
  {
    return res;
  }

  res = processTransferTypeRestrictions(tsmProcOrder, trInfoWrapper, ptf);

  if (UNLIKELY((res == Transfers1::FAIL) || (res == Transfers1::SKIP)))
  {
    return res;
  }

  if (!fallback::cat9FixMaxTransfers(&trx))
  {
    res = checkMaxTransfers(trx, trInfoWrapper, tsmProcOrder, failReason);
    if (UNLIKELY((res == Transfers1::FAIL) || (res == Transfers1::SKIP)))
      return res;
  }

  if (!trInfo->segs().empty())
  {
    res = processTransferInfoSegs(
        trx, tsmProcOrder, tsmPUOrder, trInfoWrapper, fp, pu, fm, ptf, failReason, fu);

    if ((res == Transfers1::FAIL) || (res == Transfers1::SKIP))
    {
      return res;
    }
  }

  res = checkMinTransfers(trx, tsmProcOrder, *trInfo, ptf, failReason);

  if ((res == Transfers1::FAIL) || (res == Transfers1::SKIP))
  {
    return res;
  }

  res = checkMaxTransfersInOut(trx, tsmProcOrder, trInfoWrapper, ptf, failReason);

  if ((res == Transfers1::FAIL) || (res == Transfers1::SKIP))
  {
    return res;
  }

  return Transfers1::CONTINUE;
}

Transfers1::ProcessingResult
Transfers1::processFareBreakSurfaceRestrictions(PricingTrx& trx,
                                                const TransfersInfo1& trInfo,
                                                const FareMarket& fm,
                                                std::string* failReason)
{
  bool fareBreakPermitted = isFareBreakSurfacePermitted(trx, trInfo, fm);

  if (fareBreakPermitted && !trInfo.fareBreakSurfaceTblItemNo())
    return Transfers1::CONTINUE;

  const TravelSeg* tsFirst = fm.travelSeg().front();
  const TravelSeg* tsLast = fm.travelSeg().back();

  std::vector<const TravelSeg*> travelSegs;
  travelSegs.push_back(tsFirst);
  if (tsFirst != tsLast)
    travelSegs.push_back(tsLast);

  std::vector<SurfaceTransfersInfo*> fareBreakSTI;
  bool stiInitialized = false;

  for (const TravelSeg* ts : travelSegs)
  {
    if (!isSurfaceSector(fm, *ts))
      continue;

    if (trInfo.fareBreakSurfaceTblItemNo() && !stiInitialized)
    {
      fareBreakSTI =
          trx.dataHandle().getSurfaceTransfers(trInfo.vendor(), trInfo.fareBreakSurfaceTblItemNo());
      stiInitialized = true;
    }

    bool surfaceMatched = fareBreakSTI.empty();

    for (const SurfaceTransfersInfo* sti : fareBreakSTI)
    {
      if (matchSurfaceIntlDom(*ts, *sti) && matchSurfaceOrigDest(*ts, fm, *sti) &&
          matchSurfaceGeoFareBreak(trx, *ts, *sti, ts == tsFirst))
      {
        surfaceMatched = true;
        break;
      }
    }

    if (fareBreakPermitted && !surfaceMatched)
    {
      if (failReason)
        printSurfaceValidationResult("FARE BREAK SURFACE FAILED: ", ts, *failReason);
      return Transfers1::FAIL;
    }

    if (!fareBreakPermitted && surfaceMatched)
    {
      if (failReason)
        printSurfaceValidationResult("FARE BREAK SURFACE NOT PERMITTED: ", ts, *failReason);
      return Transfers1::FAIL;
    }
  }

  return Transfers1::CONTINUE;
}

Transfers1::ProcessingResult
Transfers1::processEmbeddedSurfaceRestrictions(PricingTrx& trx,
                                               const TransfersInfo1& trInfo,
                                               const FareMarket& fm,
                                               std::string* failReason)
{
  if (fm.travelSeg().size() <= 2)
    return Transfers1::CONTINUE;

  uint32_t numEmbedded = 0;
  const uint32_t maxNumEmbedded = getMaxNumEmbeddedSurfaces(trx, trInfo);
  const bool embeddedPermitted = maxNumEmbedded > 0;

  // In RTW there are special restrictions for surfaces between IATA area so we can't skip
  // the validation even if there are no explicit restrictions for embedded surfaces.
  if (!isRtw() && maxNumEmbedded > (fm.travelSeg().size() - 2) &&
      !trInfo.embeddedSurfaceTblItemNo())
    return CONTINUE;

  std::vector<SurfaceTransfersInfo*> embeddedSTI;
  bool stiInitialized = false;

  for (std::size_t tsId = 1, tsEnd = fm.travelSeg().size() - 1; tsId < tsEnd; ++tsId)
  {
    const TravelSeg& ts = *fm.travelSeg()[tsId];

    if (LIKELY(!isSurfaceSector(fm, ts)))
      continue;

    if (trInfo.embeddedSurfaceTblItemNo() && !stiInitialized)
    {
      embeddedSTI =
          trx.dataHandle().getSurfaceTransfers(trInfo.vendor(), trInfo.embeddedSurfaceTblItemNo());
      stiInitialized = true;
    }

    bool surfaceMatched = embeddedSTI.empty();
    bool requireStiGeoLocations = false;

    if (embeddedPermitted)
    {
      if (maxNumEmbedded != SURFACE_UNLIMITED && ++numEmbedded > maxNumEmbedded)
      {
        if (failReason)
          printSurfaceValidationResult("MAX EMBEDDED SURFACES EXCEEDED", nullptr, *failReason);
        return Transfers1::FAIL;
      }

      if (isRtw() && ts.origin()->area() != ts.destination()->area())
      {
        requireStiGeoLocations = true;
        surfaceMatched = false;
      }
    }

    for (const SurfaceTransfersInfo* sti : embeddedSTI)
    {
      if (requireStiGeoLocations && !hasGeoLocationsSpecified(*sti))
        continue;

      if (matchSurfaceIntlDom(ts, *sti) && matchSurfaceGeo(trx, ts, *sti))
      {
        surfaceMatched = true;
        break;
      }
    }

    if (embeddedPermitted && !surfaceMatched)
    {
      if (failReason)
        printSurfaceValidationResult("EMBEDDED SURFACE FAILED: ", &ts, *failReason);
      return Transfers1::FAIL;
    }

    if (!embeddedPermitted && surfaceMatched)
    {
      if (failReason)
        printSurfaceValidationResult("EMBEDDED SURFACE NOT PERMITTED: ", &ts, *failReason);
      return Transfers1::FAIL;
    }
  }

  return Transfers1::CONTINUE;
}

Transfers1::ProcessingResult
Transfers1::processSurfaceSectorRestrictions(PricingTrx& trx,
                                             const TransfersInfo1& trInfo,
                                             const FareMarket& fm,
                                             std::string* failReason)
{
  if (fm.travelSeg().empty() || _excludeSurfaceBytes)
    return Transfers1::CONTINUE;

  ProcessingResult ret = processFareBreakSurfaceRestrictions(trx, trInfo, fm, failReason);
  if (ret != Transfers1::CONTINUE)
    return ret;

  return processEmbeddedSurfaceRestrictions(trx, trInfo, fm, failReason);
}

Transfers1::ProcessingResult
Transfers1::processSurcharges(PricingTrx& trx,
                              TravelSegMarkupContainer& tsmCon,
                              const TransfersInfoWrapper& trInfoWrapper,
                              const TransfersInfo1& trInfo,
                              const FareUsage& fareUsage,
                              const FarePath& farePath)
{
  const PaxType* paxType = farePath.paxType();

  if (UNLIKELY(!paxType))
  {
    LOG4CXX_ERROR(logger, "Rules.Transfers1::processSurcharges(): FarePath.paxType() =0");
    return Transfers1::FAIL;
  }

  TruePaxType truePaxType(trx, farePath);
  PaxTypeCode paxTypeCode = truePaxType.mixedPaxType() ? paxType->paxType() : truePaxType.paxType();

  if (trInfoWrapper.transferFCscope())
  {
    trInfoWrapper.initCharge1Count() = 0;
    trInfoWrapper.initCharge2Count() = 0;
  }

  for (TravelSegMarkup* tsm : tsmCon)
  {
    TravelSeg* ts = tsm->travelSeg();
    FareUsage* fu = tsm->fareUsage();

    if ((trInfoWrapper.checkIsTransfer(ts)) && (trInfoWrapper.checkPassedValidation(ts)) &&
        (trInfoWrapper.getRuleItemMatch(ts) == trInfo.itemNo()))
    {
      // Only process surcharges for the current FareUsage
      //
      if (fu == &fareUsage)
      {
        if (tsm->tiSegMatch().empty())
        {
          if (UNLIKELY(!trInfoWrapper.addSurcharge(trx, ts, fu, paxTypeCode)))
          {
            return Transfers1::FAIL;
          }
        }
        else
        {
          bool segSpecific = tsm->isChargeSegmentSpecific();
          bool surchargeAdded = false;

          for (const TransferInfoSegMatch& tiSeg : tsm->tiSegMatch())
          {
            if (tiSeg.segCheckResult() == TransferInfoSegMatch::PASS)
            {
              bool segSpecific = tiSeg.geoMatchResult() == Transfers1::MATCH ||
                                 tiSeg.geoMatchResult() == Transfers1::DOES_NOT_APPLY;

              if (tiSeg.chargeType() == TransferInfoSegMatch::CHARGE1)
              {
                if (UNLIKELY(!trInfoWrapper.addSurcharge(trx, ts, fu, paxTypeCode, segSpecific)))
                {
                  return Transfers1::FAIL;
                }
                surchargeAdded = true;
              }
              else if (tiSeg.chargeType() == TransferInfoSegMatch::CHARGE2)
              {
                if (!trInfoWrapper.addSurcharge(trx, ts, fu, paxTypeCode, segSpecific, true))
                {
                  return Transfers1::FAIL;
                }
                surchargeAdded = true;
              }
            }
          }
          if (!surchargeAdded)
          {
            if (!trInfoWrapper.addSurcharge(trx, ts, fu, paxTypeCode, segSpecific))
            {
              return Transfers1::FAIL;
            }
          }
        }
      }
    }
  }
  return Transfers1::CONTINUE;
}

Transfers1::ProcessingResult
Transfers1::identifyTransfers(PricingTrx& trx,
                              TravelSegMarkupContainer& tsmCon,
                              const TransfersInfo1& trInfo,
                              const TransfersInfoWrapper& trInfoWrapper,
                              const FareUsage* const fareUsage)
{
  TravelSegMarkupI iter = tsmCon.begin();
  const TravelSegMarkupI iterEnd = tsmCon.end();

  for (; iter != iterEnd; ++iter)
  {
    TravelSegMarkup* tsm = (*iter);

    FareUsage* fu = tsm->fareUsage();
    if (fu != fareUsage &&
        (trInfoWrapper.transferFCscopeReal(trx) || !trInfoWrapper.hasPricingUnitScope(fu)))
    {
      // Ignore transfers that belong to fare components that are outside of the PU validation scope
      // (see also RuleSetPreprocessor::processTransfers() which identifies fareUsages for
      // hasPricingUnitScope())
      tsm->transferType() = Transfers1::TFR_NONE;
      tsm->stopType() = Transfers1::NONE;
      continue;
    }

    if (fu)
    {
      // Don't process the final travel seg of each fare component.
      std::vector<TravelSeg*>::const_reverse_iterator tsI = fu->travelSeg().rbegin();
      std::vector<TravelSeg*>::const_reverse_iterator tsEnd = fu->travelSeg().rend();

      // Loop backward through the travel segs in the fare usage.
      for (; tsI != tsEnd; ++tsI)
      {
        TravelSeg* ts = *tsI;
        AirSeg* as = dynamic_cast<AirSeg*>(ts);

        if (as)
        {
          // If the current travel seg is the final air segment
          //  in the fare usage, then don't process it.
          if (tsm->travelSeg() != ts)
          {
            setStopType(*tsm, trInfo);
            setTransferType(*tsm, trInfo);
          }
          break;
        }
      }
    }
    else
    {
      setStopType(*tsm, trInfo);
      setTransferType(*tsm, trInfo);
    }
  }
  return Transfers1::CONTINUE;
}

void
Transfers1::identifyGateways(PricingTrx& trx,
                             TravelSegMarkupContainer& tsmCon,
                             const TransfersInfo1& trInfo,
                             const PricingUnit* pu,
                             const FareMarket* fm)
{
  if (trInfo.segs().empty())
  {
    return;
  }

  bool gatewayRestrictionExists = false;

  std::vector<TransfersInfoSeg1*>::const_iterator iter = trInfo.segs().begin();
  std::vector<TransfersInfoSeg1*>::const_iterator iterEnd = trInfo.segs().end();

  for (; iter != iterEnd; ++iter)
  {
    if ((*iter)->gateway() == SEG_GATEWAY_ONLY)
    {
      gatewayRestrictionExists = true;
      break;
    }
  }

  if (!gatewayRestrictionExists)
  {
    return;
  }

  identifyGateways(trx, tsmCon, pu, fm);
}

// This methods should moved out after refactoring before R9
// This is copied from Stopovers code. This is not refactored in this SPR
// because there are private classes between Stopovers and Transfers1
// that need to be decoupled.
void
Transfers1::identifyGateways(PricingTrx& trx,
                             TravelSegMarkupContainer& tsmCon,
                             const PricingUnit* pu,
                             const FareMarket* fm)
{
  if (pu) // FarePathFactoryPhase
  {
    for (const auto fu : pu->fareUsage())
    {
      const PaxTypeFare* ptFare = fu->paxTypeFare();
      if (ptFare && ptFare->fareMarket())
        markGateways(trx, tsmCon, *(ptFare->fareMarket()));
    }
  }
  else if (fm) // FarePhase
  {
    markGateways(trx, tsmCon, *fm);
  }
}

void
Transfers1::markGateways(PricingTrx& trx, TravelSegMarkupContainer& tsmCon, const FareMarket& fm)
{
  TSIGateway tsiSetupGW;

  if (RtwUtil::isRtw(trx))
  {
    if (!tsiSetupGW.markGwRtw(TSIGateway::MARK_ALL_GATEWAY, fm.travelSeg()))
      return;
  }
  else
  {
    if (!tsiSetupGW.markGW(TSIGateway::MARK_ALL_GATEWAY, fm.travelSeg()))
      return;
  }

  for (TravelSegMarkup* tsm : tsmCon)
  {
      tsm->isGateway() |= tsiSetupGW.isArrivalOnGW(tsm->travelSeg());
  }
}

void
Transfers1::setStopType(TravelSegMarkup& tsm, const TransfersInfo1& trInfo)
{
  const TravelSeg* travelSeg = tsm.travelSeg();
  const TravelSeg* nextTravelSeg = tsm.nextTravelSeg();

  if (!nextTravelSeg)
  {
    tsm.stopType() = Transfers1::NONE;
    return;
  }

  StopType stopType = Transfers1::CONNECTION;

  if (UNLIKELY(travelSeg->isForcedStopOver()))
  {
    stopType = Transfers1::STOPOVER;
  }
  else if (UNLIKELY(travelSeg->isForcedConx()))
  {
    stopType = Transfers1::CONNECTION;
  }
  else if (UNLIKELY(travelSeg->segmentType() == tse::Open))
  {
    DateTime departDT = travelSeg->departureDT();
    DateTime nextDepartDT = nextTravelSeg->departureDT();

    if (departDT.isValid() && nextDepartDT.isValid())
    {
      if ((departDT.year() != nextDepartDT.year()) || (departDT.month() != nextDepartDT.month()) ||
          (departDT.day() != nextDepartDT.day()))
      {
        stopType = Transfers1::STOPOVER;
      }
    }
  }
  else
  {
    DateTime arriveDT = travelSeg->arrivalDT();
    DateTime departDT = nextTravelSeg->departureDT();

    int64_t stopTimeSeconds = DateTime::diffTime(departDT, arriveDT);

    if ((tsm.geoTravelType() == GeoTravelType::International) ||
        (tsm.geoTravelType() == GeoTravelType::ForeignDomestic))
    {
      if (stopTimeSeconds >= RuleConst::STOPOVER_SEC_INTL)
      {
        stopType = Transfers1::STOPOVER;
      }
    }
    else
    {
      if (stopTimeSeconds >= RuleConst::STOPOVER_SEC_DOMESTIC)
      {
        stopType = Transfers1::STOPOVER;
      }
    }

    if (stopType != Transfers1::STOPOVER)
    {
      if (UNLIKELY(tsm.fareMarket() &&
          std::find(tsm.fareMarket()->stopOverTravelSeg().begin(),
                    tsm.fareMarket()->stopOverTravelSeg().end(),
                    travelSeg) != tsm.fareMarket()->stopOverTravelSeg().end()))
      {
        stopType = Transfers1::STOPOVER;
      }
    }
  }

  tsm.stopType() = stopType;
}

void
Transfers1::setTransferType(TravelSegMarkup& tsm, const TransfersInfo1& trInfo)
{
  TransferType transferType = Transfers1::TFR_NONE;

  if (tsm.nextTravelSeg())
  {
    if ((tsm.carrier() == tsm.governingCarrier()) || (tsm.carrierOut() == tsm.governingCarrier()))
    {
      if (tsm.carrier() == tsm.carrierOut())
      {
        transferType = Transfers1::PRIMARY_PRIMARY;
      }
      else
      {
        transferType = Transfers1::PRIMARY_OTHER;
      }
    }
    else
    {
      if (tsm.carrier() == tsm.carrierOut())
      {
        transferType = Transfers1::SAME_SAME;
      }
      else
      {
        transferType = Transfers1::OTHER_OTHER;
      }
    }
  }
  tsm.transferType() = transferType;
}

Transfers1::ProcessingResult
Transfers1::processTransferTypeRestrictions(TravelSegMarkupContainer& tsmCon,
                                            const TransfersInfoWrapper& trInfoWrapper,
                                            const PaxTypeFare* ptf)
{
  const TransfersInfo1* trInfo = trInfoWrapper.trInfo();

  Indicator primePrimeInd = trInfo->primeCxrPrimeCxr();
  Indicator sameSameInd = trInfo->sameCxrSameCxr();
  Indicator primeOtherInd = trInfo->primeCxrOtherCxr();
  Indicator otherOtherInd = trInfo->otherCxrOtherCxr();

  TravelSegMarkupI iter = tsmCon.begin();
  TravelSegMarkupI iterEnd = tsmCon.end();

  for (; iter != iterEnd; ++iter)
  {
    TravelSegMarkup* tsm = (*iter);

    if (tsm->transferType() == Transfers1::TFR_NONE)
    {
      continue;
    }

    if (UNLIKELY(!tsm->validateEntireRule() &&
                 (!tsm->fareUsage() || tsm->fareUsage()->paxTypeFare() != ptf)))
    {
      tsm->transferTypeMatchResult() = Transfers1::DOES_NOT_APPLY;
      continue;
    }

    if (primePrimeInd == Transfers1::ALLOW_BLANK && sameSameInd == Transfers1::ALLOW_BLANK &&
        primeOtherInd == Transfers1::ALLOW_BLANK && otherOtherInd == Transfers1::ALLOW_BLANK)
    {
      tsm->transferTypeMatchResult() = Transfers1::MATCH;
    }
    else
    {
      if (tsm->transferType() == Transfers1::PRIMARY_PRIMARY)
      {
        if (primePrimeInd == Transfers1::ALLOW_PRIME_PRIME)
        {
          tsm->transferTypeMatchResult() = Transfers1::MATCH;
        }
        else
        {
          tsm->transferTypeMatchResult() = Transfers1::NOT_MATCH;
        }
      }
      else if (tsm->transferType() == Transfers1::PRIMARY_OTHER)
      {
        if (primeOtherInd == Transfers1::ALLOW_PRIME_OTHER)
        {
          tsm->transferTypeMatchResult() = Transfers1::MATCH;
        }
        else
        {
          tsm->transferTypeMatchResult() = Transfers1::NOT_MATCH;
        }
      }
      else if (tsm->transferType() == Transfers1::SAME_SAME)
      {
        if (sameSameInd == Transfers1::ALLOW_SAME_SAME)
        {
          tsm->transferTypeMatchResult() = Transfers1::MATCH;
        }
        else
        {
          tsm->transferTypeMatchResult() = Transfers1::NOT_MATCH;
        }
      }
      else if (LIKELY(tsm->transferType() == Transfers1::OTHER_OTHER))
      {
        if (otherOtherInd == Transfers1::ALLOW_OTHER_OTHER)
        {
          tsm->transferTypeMatchResult() = Transfers1::MATCH;
        }
        else
        {
          tsm->transferTypeMatchResult() = Transfers1::NOT_MATCH;
        }
      }
    }
  }

  return Transfers1::CONTINUE;
}

Transfers1::ProcessingResult
Transfers1::processTransferInfoSegs(PricingTrx& trx,
                                    TravelSegMarkupContainer& tsmProcOrder,
                                    TravelSegMarkupContainer& tsmPUOrder,
                                    const TransfersInfoWrapper& trInfoWrapper,
                                    const FarePath* fp,
                                    const PricingUnit* pu,
                                    const FareMarket* fm,
                                    const PaxTypeFare* ptf,
                                    std::string& failReason,
                                    const FareUsage* fu)
{
  const TransfersInfo1* trInfo = trInfoWrapper.trInfo();
  const std::vector<TransfersInfoSeg1*>& trInfoSegs = trInfo->segs();

  if (UNLIKELY(trInfoSegs.empty()))
  {
    return Transfers1::CONTINUE;
  }

  bool allSegsGeoApplNot = true;
  bool allSegsGeoApplOK = true;
  bool geoRestrictionsExist = false;

  DataHandle localDataHandle(trx.ticketingDate());
  std::vector<TransfersInfoSegMarkup*> trInfoSegMarkup;

  initializeTransfersInfoSegMarkup(trx,
                                   localDataHandle,
                                   trInfoSegMarkup,
                                   *trInfo,
                                   trInfo->segs(),
                                   tsmPUOrder,
                                   allSegsGeoApplOK,
                                   allSegsGeoApplNot,
                                   geoRestrictionsExist);

  if (UNLIKELY(isRtw() && trInfoSegMarkup.empty()))
    return Transfers1::CONTINUE;

  bool validateOutbound = true;
  bool validateInbound = true;
  bool validateEntireRule = true;

  bool pftPresent = false;

  bool isCmdPricing = false;
  const PaxTypeFare* cat25Fare = fu ? fu->cat25Fare() : nullptr;
  if (UNLIKELY(((ptf->isFareByRule() || !cat25Fare) && ptf->isCmdPricing()) ||
      (cat25Fare && cat25Fare->isCmdPricing())))
  {
    isCmdPricing = true;
  }

  for (TravelSegMarkup* tsm : tsmProcOrder)
  {
    if ((tsm->transferType() == Transfers1::TFR_NONE) ||
        (tsm->transferTypeMatchResult() == Transfers1::NOT_MATCH))
    {
      continue;
    }

    pftPresent = (tsm->fareUsage() && tsm->fareUsage()->paxTypeFare() &&
                  tsm->fareUsage()->paxTypeFare()->fareMarket());

    if (UNLIKELY(!pu && !tsm->validateEntireRule()))
    {
      tsm->trSegCheckResult() = TravelSegMarkup::DOES_NOT_APPLY;

      if (tsm->direction() == TravelSegMarkup::OUTBOUND)
        validateOutbound = false;
      else if (tsm->direction() == TravelSegMarkup::INBOUND)
        validateInbound = false;

      validateEntireRule = false;

      continue;
    }

    bool processChargeCalcOnly = false;
    bool isTentativeMatch = false;
    uint32_t ruleItemMatch = trInfoWrapper.getRuleItemMatch(tsm->travelSeg(), isTentativeMatch);

    if (ruleItemMatch)
    {
      if ((ruleItemMatch != trInfo->itemNo()) && (!isTentativeMatch))
      {
        processChargeCalcOnly = true;
      }
    }

    TravelSeg* ts = tsm->travelSeg();
    FareUsage* fu = tsm->fareUsage();

    RecurringSegContext recurringSegContext;

    for (TransfersInfoSegMarkup* trISM : trInfoSegMarkup)
    {
      const TransfersInfoSeg1* trInfoSeg = trISM->trInfoSeg();

      Indicator segAppl = trInfoSeg->transferAppl();
      Indicator chargeAppl = trInfoSeg->chargeAppl();

      int16_t numTransfers = trISM->numTransfers();
      bool numTransfersUnlimited = trISM->numTransfersUnlimited();

      TransferInfoSegMatch tism(trInfoSeg);
      tsm->tiSegMatch().push_back(tism);

      TransferInfoSegMatch& tiSegMatch = tsm->tiSegMatch().back();

      if (UNLIKELY(!recurringSegContext.processNextSegment(chargeAppl)))
      {
        tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
        tiSegMatch.segCheckResult() = TransferInfoSegMatch::DOES_NOT_APPLY;
        continue;
      }

      // This code is to prevent processing all Fare components when there are
      // recurring segment(s) are present and they have different scope (I/O value).
      // Also, if the fare is not belonging to this FC
      if (UNLIKELY(pftPresent && (!tsm->validateEntireRule() &&
                         (trInfoWrapper.recurringFCscope() &&
                          (tsm->fareUsage()->paxTypeFare() != ptf))) &&
          !isRecurringSegForPU(*trInfo, *trInfoSeg)))
      {
        tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
        tiSegMatch.segCheckResult() = TransferInfoSegMatch::DOES_NOT_APPLY;
        continue;
      }

      if (trISM->checkTransferType())
      {
        tiSegMatch.transferTypeMatchResult() = checkInfoSegTransferType(*tsm, *trInfoSeg);

        if (tiSegMatch.transferTypeMatchResult() == Transfers1::NOT_MATCH)
        {
          tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::DOES_NOT_APPLY;
          continue;
        }
      }

      if (trISM->checkInOut())
      {
        // NOTE: direction can only be UNKNOWN during fare component
        //        validation
        if (tsm->direction() == TravelSegMarkup::UNKNOWN)
        {
          tsm->transferCheckResult() = TravelSegMarkup::SOFTPASS;
          tsm->trSegCheckResult() = TravelSegMarkup::SOFTPASS;
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::PASS;
          break;
        }

        if ((trISM->inOutOut()) && (tsm->direction() == TravelSegMarkup::INBOUND))
        {
          tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::PASS;
          continue;
        }
        else if ((trISM->inOutIn()) && (tsm->direction() == TravelSegMarkup::OUTBOUND))
        {
          tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::PASS;
          continue;
        }
      }

      if (trInfoSeg->gateway() == SEG_GATEWAY_ONLY)
      {
        if (tsm->isGateway())
        {
          tiSegMatch.gatewayMatchResult() = Transfers1::MATCH;
        }
        else
        {
          tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          tiSegMatch.gatewayMatchResult() = Transfers1::NOT_MATCH;
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::DOES_NOT_APPLY;
          continue;
        }
      }

      if (trInfoSeg->restriction() != Transfers1::SEG_RESTRICTION_BLANK)
      {
        tiSegMatch.restrictionMatchResult() = checkInfoSegRestriction(*tsm, *trInfoSeg);

        if (tiSegMatch.restrictionMatchResult() == Transfers1::NOT_MATCH)
        {
          tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::DOES_NOT_APPLY;
          continue;
        }
      }

      MatchResult geoMatch = Transfers1::NOT_CHECKED;

      bool processNextInfoSeg = true;
      bool failed = false;
      std::string shortFailReason;

      const FareMarket* fareMarket = fm;
      const PaxTypeFare* paxTypeFare = ptf;
      const VendorCode& vendorOfFare = ptf->vendor();

      if (pftPresent && (tsm->validateEntireRule() ||
                         (trInfoWrapper.recurringFCscope() &&
                          (tsm->fareUsage()->paxTypeFare() == ptf))))
      {
        paxTypeFare = tsm->fareUsage()->paxTypeFare();
        fareMarket = paxTypeFare->fareMarket();
      }

      tiSegMatch.stopConnMatchResult() = checkInfoSegStopConn(*tsm, *trInfoSeg);
      if (tiSegMatch.stopConnMatchResult() == Transfers1::NOT_MATCH)
      {
        tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
        tiSegMatch.segCheckResult() = TransferInfoSegMatch::PASS;
        continue;
      }

      if (!processInfoSegCarrier(trx, *tsm, tiSegMatch, *trInfo, *trInfoSeg, vendorOfFare))
        continue;

      setRecurringSegCounters(trx, fm, vendorOfFare, *ts->destination(), *trISM);

      if (!checkInfoSegDirectionality(*tsm, *trISM, processNextInfoSeg, failed, shortFailReason, failReason))
      {
        if (UNLIKELY(failed))
        {
          tsm->transferCheckResult() = TravelSegMarkup::FAIL;
          tsm->trSegCheckResult() = TravelSegMarkup::FAIL;
          tsm->failReason() = shortFailReason;

          tiSegMatch.segCheckResult() = TransferInfoSegMatch::FAIL;
          tiSegMatch.inOutTransfersResult() = TransferInfoSegMatch::FAIL;

          return Transfers1::FAIL;
        }
        else if (LIKELY(processNextInfoSeg))
        {
          tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::DOES_NOT_APPLY;
          continue;
        }
        else
        {
          tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::PASS;
          break;
        }
      }
      else if (trISM->checkInOut())
      {
        tiSegMatch.inOutTransfersResult() = TransferInfoSegMatch::PASS;
      }

      if (trInfoSeg->betweenAppl() == Transfers1::SEG_BETWEEN_APPL_BETWEEN)
      {
        if (!checkInfoSegBetweenGeo(
                trx, *tsm, *trISM, *trInfo, tsmPUOrder, geoMatch, processNextInfoSeg, failed))
        {
          if (failed)
          {
            tsm->transferCheckResult() = TravelSegMarkup::FAIL;
            tsm->trSegCheckResult() = TravelSegMarkup::FAIL;
            tsm->failReason() = shortFailReason;

            tiSegMatch.segCheckResult() = TransferInfoSegMatch::FAIL;
            return Transfers1::FAIL;
          }
          else if (!processNextInfoSeg)
          {
            tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            tiSegMatch.segCheckResult() = TransferInfoSegMatch::PASS;
            tiSegMatch.geoMatchResult() = geoMatch;
            break;
          }
        }
      }
      else
      {
        if (!checkInfoSegGeo(trx,
                             *tsm,
                             *trISM,
                             *trInfo,
                             trInfoSegMarkup,
                             fp,
                             pu,
                             fareMarket,
                             paxTypeFare,
                             processChargeCalcOnly,
                             geoMatch,
                             processNextInfoSeg,
                             failed,
                             failReason,
                             vendorOfFare))
        {
          if (UNLIKELY(failed))
          {
            tsm->transferCheckResult() = TravelSegMarkup::FAIL;
            tsm->trSegCheckResult() = TravelSegMarkup::FAIL;
            tsm->failReason() = shortFailReason;

            tiSegMatch.segCheckResult() = TransferInfoSegMatch::FAIL;
            return Transfers1::FAIL;
          }
          else if (!processNextInfoSeg)
          {
            if (!fallback::fallbackapo37432Cat9BlankIORecurSeg(&trx))
            {
               //apo-37432: if geomatch is true, seg_geo_appl_not_permitted, and I/O is blank
               //and we have no more rec.segs to search then fail the transfer rule
               if ((pu)  && (trISM->trInfoSeg()->outInPortion() == Transfers1::SEG_INOUT_BLANK)  &&
                   (trISM->trInfoSeg()->transferAppl() == Transfers1::SEG_GEO_APPL_NOT_PERMITTED) &&
                   (geoMatch == Transfers1::MATCH) )
               {
                 tsm->transferCheckResult() = TravelSegMarkup::FAIL;
                 tsm->trSegCheckResult() = TravelSegMarkup::FAIL;
                 tiSegMatch.segCheckResult() = TransferInfoSegMatch::FAIL;
                 tiSegMatch.geoMatchResult() = geoMatch;
                 failReason = "SEE ABOVE FOR FAILED SEGMENTS" ;
                 return Transfers1::FAIL;
               }
               else
               {
                  tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
                  tiSegMatch.segCheckResult() = TransferInfoSegMatch::PASS;
                  tiSegMatch.geoMatchResult() = geoMatch;
                  break;
               }
            }
            else //fallback; remove at fallback removal
            {
               tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
               tiSegMatch.segCheckResult() = TransferInfoSegMatch::PASS;
               tiSegMatch.geoMatchResult() = geoMatch;
               break;
            }
          }
        }
      }

      tiSegMatch.geoMatchResult() = geoMatch;

      if (allSegsGeoApplOK)
      {
        if (geoMatch == Transfers1::NOT_MATCH)
        {
          tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::DOES_NOT_APPLY;
          continue;
        }
      }

      if ((segAppl == Transfers1::SEG_GEO_APPL_NOT_PERMITTED) &&
          ((geoMatch == Transfers1::MATCH) || (geoMatch == Transfers1::DOES_NOT_APPLY)))
      {
        if (chargeAppl == Transfers1::SEG_NO_CHARGE)
        {
          tsm->trSegCheckResult() = TravelSegMarkup::STOP;
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::FAIL;
          break;
        }
        else
        {
          tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::DOES_NOT_APPLY;
          recurringSegContext.setIgnoreChargeGroup(chargeAppl);
          continue;
        }
      }

      // If we reach this point, then the travel seg is a match
      //  on all data and is considered a transfer point.

      // Check Out/In Either
      if ((segAppl != Transfers1::SEG_GEO_APPL_NOT_PERMITTED) &&
          (trISM->checkInOut() && trISM->inOutEither()))
      {
        if ((tsm->direction() == TravelSegMarkup::OUTBOUND && trISM->matchCountIn()) ||
            (tsm->direction() == TravelSegMarkup::INBOUND && trISM->matchCountOut()))
        {
          tsm->transferCheckResult() = TravelSegMarkup::FAIL;
          tsm->trSegCheckResult() = TravelSegMarkup::FAIL;
          tsm->failReason() = "IN/OUT-ONLY";
          failReason = " TRANSFER ONLY PERMITTED ON \n  INBOUND OR OUTBOUND. NOT BOTH";
          tiSegMatch.segCheckResult() = TransferInfoSegMatch::FAIL;
          tiSegMatch.inOutTransfersResult() = TransferInfoSegMatch::FAIL;

          if (isCmdPricing)
          {
            trInfoWrapper.setMaxExceeded(ts, fu, TransfersInfoWrapper::TOTAL_IO_EXCEED);
            break;
          }
          else
          {
            return Transfers1::FAIL;
          }
        }
      }

      // Check Out/In Both
      if ((!numTransfersUnlimited) && (segAppl != Transfers1::SEG_GEO_APPL_NOT_PERMITTED))
      {
        if (UNLIKELY(trISM->matchCount() == numTransfers))
        {
          if (chargeAppl == Transfers1::SEG_NO_CHARGE)
          {
            tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            tiSegMatch.segCheckResult() = TransferInfoSegMatch::PASS;
            break;
          }
          else
          {
            if (segAppl == Transfers1::SEG_GEO_APPL_BLANK)
              recurringSegContext.setIgnoreChargeGroup(chargeAppl);
            tsm->trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            tiSegMatch.segCheckResult() = TransferInfoSegMatch::DOES_NOT_APPLY;
            continue;
          }
        }
      }

      if ((segAppl == Transfers1::SEG_GEO_APPL_NOT_PERMITTED) && (geoMatch != Transfers1::MATCH) &&
          (geoMatch != Transfers1::DOES_NOT_APPLY))
      {
        continue;
      }
      else if ((segAppl != Transfers1::SEG_GEO_APPL_NOT_PERMITTED) &&
               (geoMatch != Transfers1::MATCH) && (geoMatch != Transfers1::DOES_NOT_APPLY))
      {
        continue;
      }

      if (chargeAppl == Transfers1::SEG_USE_CHARGE_1)
      {
        tiSegMatch.chargeType() = TransferInfoSegMatch::CHARGE1;
      }
      else if (chargeAppl == Transfers1::SEG_USE_CHARGE_2)
      {
        tiSegMatch.chargeType() = TransferInfoSegMatch::CHARGE2;
      }
      else if (chargeAppl == Transfers1::SEG_NO_CHARGE)
      {
        tsm->useSegmentsToApplyCharges() = false;
      }

      // Now adding current match to the count before checking either in out
      if (!processChargeCalcOnly)
      {
        ++trISM->matchCount();
        if (tsm->direction() == TravelSegMarkup::INBOUND)
          ++trISM->matchCountIn();
        else if (tsm->direction() == TravelSegMarkup::OUTBOUND)
          ++trISM->matchCountOut();
      }

      tiSegMatch.segCheckResult() = TransferInfoSegMatch::PASS;
      tsm->trSegCheckResult() = TravelSegMarkup::PASS;
      tsm->isChargeSegmentSpecific() = true;
      break;
    }

    if (geoRestrictionsExist)
    {
      if (allSegsGeoApplNot)
      {
        // If all trInfoSegs have GeoAppl = NOT_PERMITTED and none match
        //  the transfer, then the transfer is considered a match.

        if (checkImplicitInfoSegMatch(*tsm, allSegsGeoApplNot))
        {
          tsm->trSegCheckResult() = TravelSegMarkup::PASS;
          tsm->isChargeSegmentSpecific() = true;
        }
      }
      else
      {
        if (tsm->trSegCheckResult() == TravelSegMarkup::NEED_REVALIDATION)
        {
          TiSegMatchListCI smIter = tsm->tiSegMatch().begin();
          TiSegMatchListCI smIterEnd = tsm->tiSegMatch().end();
          bool notMatch = true;
          for (; smIter != smIterEnd; ++smIter)
          {
            const TransferInfoSegMatch& tism = *smIter;

            if (tism.geoMatchResult() != Transfers1::NOT_MATCH &&
                tism.geoMatchResult() != Transfers1::NOT_CHECKED)
            {
              notMatch = false;
              break;
            }
          }
          if (notMatch && trInfoWrapper.recurringFCscope() &&
              tsm->fareUsage()->paxTypeFare() == ptf)
          {
            if (!trInfoWrapper.isRelationAndExists())
            {
              tsm->trSegCheckResult() = TravelSegMarkup::FAIL;
              tsm->transferCheckResult() = TravelSegMarkup::FAIL;
              std::ostringstream failString;
              failString << "GEO NOT MATCH";
              failReason = failString.str();
              if (!isCmdPricing)
                return Transfers1::FAIL;
            }
            tsm->trSegCheckResult() = TravelSegMarkup::NOT_ACCEPTED;
          }
        }
      }
    }
  }

  for (TransfersInfoSegMarkup* trISM : trInfoSegMarkup)
  {
    const TransfersInfoSeg1* trInfoSeg = trISM->trInfoSeg();

    if (trInfoSeg->transferAppl() == Transfers1::SEG_GEO_APPL_REQUIRED)
    {
      if (trISM->checkInOut() &&
          ((trISM->inOutOut() && !validateOutbound) || (trISM->inOutIn() && !validateInbound) ||
           (trISM->inOutEither() &&
            (trISM->matchCountIn() || trISM->matchCountOut() || !validateEntireRule))))
      {
        continue;
      }
      // Check if the scope is for FareComponent.

      if (fu)
      {
        if (trInfo->noTransfersMax().empty() &&
            ((fu->isInbound() && trISM->inOutOut()) || (!fu->isInbound() && trISM->inOutIn())))
        {
          continue;
        }
      }

      if (!trISM->matchCount())
      {
        std::ostringstream failString;
        failString << "TRANSFER REQUIRED BY RULE SEGMENT: " << trInfoSeg->orderNo();

        failReason = failString.str();

        return Transfers1::FAIL;
      }
    }
  }
  return Transfers1::CONTINUE;
}

bool
Transfers1::isLocTypeWithNoLocData(const TransfersInfoSeg1& tis) const
{
  return tis.loc1().locType() != LOCTYPE_NONE && tis.loc1().loc().empty() &&
         tis.loc2().loc().empty();
}

void
Transfers1::setRecurringSegCounters(const PricingTrx& trx,
                                    const FareMarket* fm,
                                    const VendorCode& vendorOfFare,
                                    const Loc& transferLoc,
                                    TransfersInfoSegMarkup& trISM)
{
  const TransfersInfoSeg1& tis = *trISM.trInfoSeg();

  if (isLocTypeWithNoLocData(tis) && tis.transferAppl() == Transfers1::SEG_GEO_APPL_BLANK)
  {
    const LocTypeCode locType = tis.loc1().locType();
    std::string locOfType = TransStopUtil::getLocOfType(trx, fm, vendorOfFare, transferLoc, locType);
    trISM.setLocCounts(locOfType);
    return;
  }

  trISM.setDefaultCounts();
}

Transfers1::MatchResult
Transfers1::checkInfoSegTransferType(TravelSegMarkup& tsm, const TransfersInfoSeg1& trInfoSeg)
{
  if (UNLIKELY(tsm.transferType() == TFR_NONE))
  {
    return Transfers1::DOES_NOT_APPLY;
  }

  if (tsm.transferType() == PRIMARY_PRIMARY)
  {
    if (trInfoSeg.primeOnline() == ALLOW_PRIME_PRIME)
    {
      return Transfers1::MATCH;
    }
  }
  if (tsm.transferType() == SAME_SAME)
  {
    if (trInfoSeg.sameOnline() == ALLOW_SAME_SAME)
    {
      return Transfers1::MATCH;
    }
  }
  if (tsm.transferType() == PRIMARY_OTHER)
  {
    if (trInfoSeg.primeInterline() == ALLOW_PRIME_OTHER)
    {
      return Transfers1::MATCH;
    }
  }
  if (UNLIKELY(tsm.transferType() == OTHER_OTHER))
  {
    if (trInfoSeg.otherInterline() == ALLOW_OTHER_OTHER)
    {
      return Transfers1::MATCH;
    }
  }
  return Transfers1::NOT_MATCH;
}

Transfers1::MatchResult
Transfers1::checkInfoSegStopConn(TravelSegMarkup& tsm, const TransfersInfoSeg1& trInfoSeg)
{
  Indicator stopConnInd = trInfoSeg.stopoverConnectInd();
  if (stopConnInd == ALLOW_CONN_OR_STOP)
  {
    return Transfers1::DOES_NOT_APPLY;
  }

  if (stopConnInd == ALLOW_STOPOVER)
  {
    if (tsm.stopType() == Transfers1::STOPOVER)
    {
      return Transfers1::MATCH;
    }
    else
    {
      return Transfers1::NOT_MATCH;
    }
  }
  else if (LIKELY(stopConnInd == ALLOW_CONNECTION))
  {
    if (tsm.stopType() == Transfers1::CONNECTION)
    {
      return Transfers1::MATCH;
    }
    else
    {
      return Transfers1::NOT_MATCH;
    }
  }
  return Transfers1::DOES_NOT_APPLY;
}

bool
Transfers1::isDomestic(const TravelSeg& ts) const
{
  // Note that US-CA is international travel based on ATPCO CAT9 data application.
  if (ts.origin()->nation() == ts.destination()->nation())
    return true;
  if (LocUtil::isRussia(*ts.origin()) && LocUtil::isRussia(*ts.destination()))
    return true;
  return false;
}

Transfers1::MatchResult
Transfers1::checkInfoSegRestriction(TravelSegMarkup& tsm, const TransfersInfoSeg1& trInfoSeg)
{
  if (trInfoSeg.restriction() == SEG_RESTRICTION_BLANK)
    return DOES_NOT_APPLY;

  if (!tsm.travelSeg() || !tsm.nextTravelSeg())
    return DOES_NOT_APPLY;

  const bool isCurrentDomestic = isDomestic(*tsm.travelSeg());
  const bool isNextDomestic = isDomestic(*tsm.nextTravelSeg());

  switch (trInfoSeg.restriction())
  {
  case SEG_RESTRICTION_DOMESTIC:
    return (isCurrentDomestic && isNextDomestic) ? MATCH : NOT_MATCH;
  case SEG_RESTRICTION_INTERNATIONAL:
    return (!isCurrentDomestic && !isNextDomestic) ? MATCH : NOT_MATCH;
  case SEG_RESTRICTION_DOM_INTL:
    return (isCurrentDomestic != isNextDomestic) ? MATCH : NOT_MATCH;
  default:
    return DOES_NOT_APPLY;
  }
}

bool
Transfers1::processInfoSegCarrier(PricingTrx& trx,
                                  TravelSegMarkup& tsm,
                                  TransferInfoSegMatch& tiSegMatch,
                                  const TransfersInfo1& trInfo,
                                  const TransfersInfoSeg1& trInfoSeg,
                                  const VendorCode& vendorOfFare)
{
  MatchResult inCarrierMatch = Transfers1::DOES_NOT_APPLY;
  MatchResult outCarrierMatch = Transfers1::DOES_NOT_APPLY;

  checkInfoSegCarrier(trx, tsm, trInfo, trInfoSeg, inCarrierMatch, outCarrierMatch, vendorOfFare);

  tiSegMatch.inCarrierMatchResult() = inCarrierMatch;
  tiSegMatch.outCarrierMatchResult() = outCarrierMatch;

  if ((inCarrierMatch == Transfers1::NOT_MATCH) || (outCarrierMatch == Transfers1::NOT_MATCH))
  {
    tsm.trSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
    tiSegMatch.segCheckResult() = TransferInfoSegMatch::PASS;
    return false;
  }
  return true;
}

Transfers1::MatchResult
Transfers1::checkInfoSegCarrier(PricingTrx& trx,
                                TravelSegMarkup& tsm,
                                const TransfersInfo1& trInfo,
                                const TransfersInfoSeg1& trInfoSeg,
                                MatchResult& inCarrierMatch,
                                MatchResult& outCarrierMatch,
                                const VendorCode& vendorOfFare)
{
  MatchResult carrierInMatch = Transfers1::DOES_NOT_APPLY;
  MatchResult carrierOutMatch = Transfers1::DOES_NOT_APPLY;

  Indicator carrierAppl = trInfoSeg.carrierAppl();
  if (carrierAppl == Transfers1::SEG_CARRIER_APPL_BLANK)
  {
    carrierInMatch = checkInfoSegCarrierIn(trx, tsm.carrier(), trInfo, trInfoSeg, vendorOfFare);
    carrierOutMatch =
        checkInfoSegCarrierOut(trx, tsm.carrierOut(), trInfo, trInfoSeg, vendorOfFare);
  }
  else if (LIKELY(carrierAppl == Transfers1::SEG_CARRIER_APPL_BETWEEN))
  {
    carrierInMatch = checkInfoSegCarrierIn(trx, tsm.carrier(), trInfo, trInfoSeg, vendorOfFare);
    carrierOutMatch =
        checkInfoSegCarrierOut(trx, tsm.carrierOut(), trInfo, trInfoSeg, vendorOfFare);

    if ((carrierInMatch == Transfers1::NOT_MATCH) || (carrierOutMatch == Transfers1::NOT_MATCH))
    {
      MatchResult carrierInMatch2 =
          checkInfoSegCarrierIn(trx, tsm.carrierOut(), trInfo, trInfoSeg, vendorOfFare);
      MatchResult carrierOutMatch2 =
          checkInfoSegCarrierOut(trx, tsm.carrier(), trInfo, trInfoSeg, vendorOfFare);

      if ((carrierInMatch2 == Transfers1::MATCH) && (carrierOutMatch2 == Transfers1::MATCH))
      {
        carrierInMatch = carrierInMatch2;
        carrierOutMatch = carrierOutMatch2;
      }
    }
  }

  inCarrierMatch = carrierInMatch;
  outCarrierMatch = carrierOutMatch;

  if ((inCarrierMatch == Transfers1::NOT_MATCH) || (outCarrierMatch == Transfers1::NOT_MATCH))
  {
    return Transfers1::NOT_MATCH;
  }
  else if ((inCarrierMatch == Transfers1::DOES_NOT_APPLY) &&
           (outCarrierMatch == Transfers1::DOES_NOT_APPLY))
  {
    return Transfers1::DOES_NOT_APPLY;
  }

  return Transfers1::MATCH;
}

Transfers1::MatchResult
Transfers1::checkInfoSegCarrierIn(PricingTrx& trx,
                                  const CarrierCode& carrier,
                                  const TransfersInfo1& trInfo,
                                  const TransfersInfoSeg1& trInfoSeg,
                                  const VendorCode& vendorOfFare)
{
  CarrierCode carrierIn = trInfoSeg.carrierIn();
  uint32_t carrierInTbl = trInfoSeg.inCarrierApplTblItemNo();

  if (carrierIn.empty() && !carrierInTbl)
  {
    return Transfers1::DOES_NOT_APPLY;
  }

  if (!carrierIn.empty() && (carrier == carrierIn))
  {
    return Transfers1::MATCH;
  }

  if (carrierInTbl)
  {
    return checkCarrierApplicationTable(trx, carrier, carrierInTbl, trInfo, vendorOfFare);
  }
  return Transfers1::NOT_MATCH;
}

Transfers1::MatchResult
Transfers1::checkInfoSegCarrierOut(PricingTrx& trx,
                                   const CarrierCode& carrier,
                                   const TransfersInfo1& trInfo,
                                   const TransfersInfoSeg1& trInfoSeg,
                                   const VendorCode& vendorOfFare)
{
  CarrierCode carrierOut = trInfoSeg.carrierOut();
  uint32_t carrierOutTbl = trInfoSeg.outCarrierApplTblItemNo();

  if (carrierOut.empty() && !carrierOutTbl)
  {
    return Transfers1::DOES_NOT_APPLY;
  }

  if (!carrierOut.empty() && (carrier == carrierOut))
  {
    return Transfers1::MATCH;
  }

  if (carrierOutTbl)
  {
    return checkCarrierApplicationTable(trx, carrier, carrierOutTbl, trInfo, vendorOfFare);
  }
  return Transfers1::NOT_MATCH;
}

Transfers1::MatchResult
Transfers1::checkCarrierApplicationTable(PricingTrx& trx,
                                         const CarrierCode& carrier,
                                         const uint32_t carrierAppTblNo,
                                         const TransfersInfo1& trInfo,
                                         const VendorCode& vendorOfFare)
{
  if (UNLIKELY(!carrierAppTblNo))
  {
    return Transfers1::DOES_NOT_APPLY;
  }

  std::vector<CarrierApplicationInfo*> carrierAppVec =
      trx.dataHandle().getCarrierApplication(vendorOfFare, carrierAppTblNo);

  if (LIKELY(!carrierAppVec.empty()))
  {
    std::vector<CarrierApplicationInfo*>::const_iterator iter = carrierAppVec.begin();
    std::vector<CarrierApplicationInfo*>::const_iterator iterEnd = carrierAppVec.end();

    for (; iter != iterEnd; ++iter)
    {
      if (((*iter)->carrier() == carrier) || ((*iter)->carrier() == tse::DOLLAR_CARRIER))
      {
        if (LIKELY((*iter)->applInd() == SEG_CARRIER_TBL_APPL_PERMITTED))
        {
          return Transfers1::MATCH;
        }
        else if ((*iter)->applInd() == SEG_CARRIER_TBL_APPL_NOT_PERMITTED)
        {
          return Transfers1::NOT_MATCH;
        }
      }
    }
  }
  return Transfers1::NOT_MATCH;
}

bool
Transfers1::checkInfoSegDirectionality(TravelSegMarkup& tsm,
                                       TransfersInfoSegMarkup& trISM,
                                       bool& processNextInfoSeg,
                                       bool& failed,
                                       std::string& shortFailReason,
                                       std::string& failReason)
{
  if (trISM.checkInOut())
  {
    Indicator chargeAppl = trISM.trInfoSeg()->chargeAppl();
    Indicator transferAppl = trISM.trInfoSeg()->transferAppl();

    if (trISM.inOutEither())
    {
      if (trISM.matchCountIn() && trISM.matchCountOut())
      {
        if (chargeAppl == Transfers1::SEG_NO_CHARGE)
        {
          shortFailReason = "IN/OUT-ONLY";

          std::ostringstream failString;
          failString << " TRANSFER ONLY PERMITTED ON " << std::endl
                     << "  INBOUND OR OUTBOUND. NOT BOTH";

          failReason = failString.str();
          failed = true;
          return false;
        }
        else
        {
          processNextInfoSeg = true;
          return false;
        }
      }
    }
    else if (trISM.inOutBoth())
    {
      if ((!trISM.numTransfersUnlimited()) && ((tsm.direction() == TravelSegMarkup::OUTBOUND &&
                                                trISM.matchCountOut() == trISM.numTransfers()) ||
                                               (tsm.direction() == TravelSegMarkup::INBOUND &&
                                                trISM.matchCountIn() == trISM.numTransfers())))
      {
        if (transferAppl == SEG_GEO_APPL_NOT_PERMITTED)
        {
          processNextInfoSeg = true;
          return true;
        }
        else if (UNLIKELY(chargeAppl == Transfers1::SEG_NO_CHARGE))
        {
          processNextInfoSeg = false;
          return false;
        }
        else
        {
          processNextInfoSeg = true;
          return false;
        }
      }
    }
    else if ((trISM.inOutOut()) && (!trISM.numTransfersUnlimited()) &&
             (trISM.matchCountOut() == trISM.numTransfers()))
    {
      if (transferAppl == SEG_GEO_APPL_NOT_PERMITTED)
      {
        processNextInfoSeg = true;
        return true;
      }
      if (chargeAppl == Transfers1::SEG_NO_CHARGE)
      {
        processNextInfoSeg = false;
        return false;
      }
      else
      {
        processNextInfoSeg = true;
        return false;
      }
    }
    else if ((trISM.inOutIn()) && (!trISM.numTransfersUnlimited()) &&
             (trISM.matchCountIn() == trISM.numTransfers()))
    {
      if (transferAppl == SEG_GEO_APPL_NOT_PERMITTED)
      {
        processNextInfoSeg = true;
        return true;
      }
      if (chargeAppl == Transfers1::SEG_NO_CHARGE)
      {
        processNextInfoSeg = false;
        return false;
      }
      else
      {
        processNextInfoSeg = true;
        return false;
      }
    }
  }
  processNextInfoSeg = true;
  return true;
}

bool
Transfers1::checkInfoSegBetweenGeo(PricingTrx& trx,
                                   TravelSegMarkup& tsm,
                                   TransfersInfoSegMarkup& trISM,
                                   const TransfersInfo1& trInfo,
                                   const TravelSegMarkupContainer& tsmCon,
                                   MatchResult& geoMatch,
                                   bool& processNextInfoSeg,
                                   bool& failed)
{
  geoMatch = Transfers1::NOT_MATCH;

  const TransfersInfoSeg1* trInfoSeg = trISM.trInfoSeg();

  const LocTypeCode& locType1 = trInfoSeg->loc1().locType();
  const LocTypeCode& locType2 = trInfoSeg->loc2().locType();

  if ((locType1 == LOCTYPE_NONE) && (locType2 == LOCTYPE_NONE) && (!trInfoSeg->zoneTblItemNo()))
  {
    geoMatch = Transfers1::DOES_NOT_APPLY;
  }
  else
  {
    TravelSegMarkupCI tsmIter = tsmCon.find(tsm.travelSeg());

    if (tsm.direction() == TravelSegMarkup::INBOUND)
    {
      if (trISM.tsmBtwnInFirst() != tsmCon.end() && trISM.tsmBtwnInLast() != tsmCon.end())
      {
        if (tsmIter >= trISM.tsmBtwnInFirst() && tsmIter < trISM.tsmBtwnInLast())
        {
          geoMatch = Transfers1::MATCH;
        }
      }
    }
    else
    {
      if (trISM.tsmBtwnOutFirst() != tsmCon.end() && trISM.tsmBtwnOutLast() != tsmCon.end())
      {
        if (tsmIter >= trISM.tsmBtwnOutFirst() && tsmIter < trISM.tsmBtwnOutLast())
        {
          geoMatch = Transfers1::MATCH;
        }
      }
    }
  }

  if (((geoMatch == Transfers1::MATCH) || (geoMatch == Transfers1::DOES_NOT_APPLY)) &&
      trInfoSeg->transferAppl() == Transfers1::SEG_GEO_APPL_NOT_PERMITTED)
  {
    if (trInfoSeg->chargeAppl() == Transfers1::SEG_NO_CHARGE)
    {
      processNextInfoSeg = false;
      failed = false;
      return false;
    }
    else
    {
      processNextInfoSeg = true;
      failed = false;
      return false;
    }
  }

  processNextInfoSeg = true;
  failed = false;
  return true;
}

bool
Transfers1::checkInfoSegGeo(PricingTrx& trx,
                            TravelSegMarkup& tsm,
                            TransfersInfoSegMarkup& trISM,
                            const TransfersInfo1& trInfo,
                            std::vector<TransfersInfoSegMarkup*>& trISMVector,
                            const FarePath* fp,
                            const PricingUnit* pu,
                            const FareMarket* fm,
                            const PaxTypeFare* ptf,
                            bool processChargeCalcOnly,
                            MatchResult& geoMatch,
                            bool& processNextInfoSeg,
                            bool& failed,
                            std::string& failReason,
                            const VendorCode& vendorOfFare)
{
  if (!checkInfoSegMainGeo(trx,
                           tsm,
                           trISM,
                           trInfo,
                           fp,
                           pu,
                           fm,
                           ptf,
                           geoMatch,
                           processNextInfoSeg,
                           failed,
                           failReason,
                           vendorOfFare))
  {
    return false;
  }

  bool geoNotPermitted =
      (trISM.trInfoSeg()->transferAppl() == Transfers1::SEG_GEO_APPL_NOT_PERMITTED);

  if ((tsm.altOffPointGeo()) && ((!geoNotPermitted && (geoMatch == Transfers1::MATCH)) ||
                                 (geoNotPermitted && (geoMatch != Transfers1::MATCH))))
  {
    MatchResult altGeoMatch = Transfers1::NOT_MATCH;
    const TransfersInfoSeg1* altGeoMatchedInfoSeg = nullptr;

    if (!checkInfoSegAltGeo(trx,
                            tsm,
                            trISM,
                            trInfo,
                            trISMVector,
                            fp,
                            pu,
                            fm,
                            ptf,
                            processChargeCalcOnly,
                            altGeoMatch,
                            processNextInfoSeg,
                            failed,
                            failReason,
                            altGeoMatchedInfoSeg,
                            vendorOfFare))
    {
      return false;
    }

    if (geoNotPermitted)
    {
      if ((altGeoMatchedInfoSeg != nullptr) &&
          (altGeoMatchedInfoSeg->transferAppl() != Transfers1::SEG_GEO_APPL_NOT_PERMITTED) &&
          (geoMatch == Transfers1::NOT_MATCH) &&
          (altGeoMatch == Transfers1::MATCH)) // Alt GEO matched permitted segment
      {
        geoMatch = Transfers1::NOT_MATCH;
      }
      else if ((geoMatch == Transfers1::MATCH) || (altGeoMatch == Transfers1::MATCH))
      {
        geoMatch = Transfers1::MATCH;
      }
    }
    else
    {
       if ((geoMatch == Transfers1::MATCH) && (altGeoMatch == Transfers1::MATCH))
       {
          geoMatch = Transfers1::MATCH;
       }
       else
       {
         if (altGeoMatchedInfoSeg == nullptr && altGeoMatch != Transfers1::MATCH)
         {
           //Based on ATPCO DataApp if alternate seg geo is present it must match otherwise fail see example 70 on Data Application examples.
           geoMatch = Transfers1::NOT_MATCH;
         }
         else
         {
           geoMatch = Transfers1::NOT_MATCH;
         }
       }
    }
  }
  return true;
}

bool
Transfers1::checkImplicitInfoSegMatch(TravelSegMarkup& tsm, bool allSegsGeoApplNot)
{
  TiSegMatchListCI smIter = tsm.tiSegMatch().begin();
  TiSegMatchListCI smIterEnd = tsm.tiSegMatch().end();

  for (; smIter != smIterEnd; ++smIter)
  {
    const TransferInfoSegMatch& tism = *smIter;

    bool inboundCxr = (tism.inCarrierMatchResult() == Transfers1::MATCH ||
                       tism.inCarrierMatchResult() == Transfers1::NOT_CHECKED ||
                       tism.inCarrierMatchResult() == Transfers1::DOES_NOT_APPLY);

    bool outboundCxr = (tism.outCarrierMatchResult() == Transfers1::MATCH ||
                        tism.outCarrierMatchResult() == Transfers1::NOT_CHECKED ||
                        tism.outCarrierMatchResult() == Transfers1::DOES_NOT_APPLY);

    if (LIKELY(allSegsGeoApplNot))
    {
      if ((tism.geoMatchResult() == Transfers1::MATCH ||
           tism.geoMatchResult() == Transfers1::DOES_NOT_APPLY) &&
          inboundCxr && outboundCxr)
      {
        return false;
      }
    }
    else
    {
      if (tism.trInfoSeg()->transferAppl() == Transfers1::SEG_GEO_APPL_NOT_PERMITTED)
      {
        if ((tism.geoMatchResult() == Transfers1::MATCH ||
             tism.geoMatchResult() == Transfers1::DOES_NOT_APPLY) &&
            inboundCxr && outboundCxr)
        {
          return false;
        }
      }
      else
      {
        if ((tism.geoMatchResult() == Transfers1::MATCH ||
             tism.geoMatchResult() == Transfers1::DOES_NOT_APPLY) &&
            (!inboundCxr || !outboundCxr))
        {
          return false;
        }
      }
    }
  }

  return true;
}

bool
Transfers1::checkInfoSegMainGeo(PricingTrx& trx,
                                TravelSegMarkup& tsm,
                                TransfersInfoSegMarkup& trISM,
                                const TransfersInfo1& trInfo,
                                const FarePath* fp,
                                const PricingUnit* pu,
                                const FareMarket* fm,
                                const PaxTypeFare* ptf,
                                MatchResult& geoMatch,
                                bool& processNextInfoSeg,
                                bool& failed,
                                std::string& failReason,
                                const VendorCode& vendorOfFare)
{
  geoMatch = Transfers1::NOT_MATCH;

  TSITravelSegMap tsiTravelSegMap;

  const TransfersInfoSeg1* trInfoSeg = trISM.trInfoSeg();

  if (trInfoSeg->tsi())
  {
    if (!processTSI(trx, tsiTravelSegMap, trInfoSeg, fp, pu, fm, ptf, failReason))
    {
      failed = true;
      return false;
    }
    else
    {
      if (!tsiTravelSegMap.empty())
      {
        TSITravelSegMap::const_iterator tsiIter = tsiTravelSegMap.find(tsm.travelSeg());
        TSITravelSegMap::const_iterator tsiIterNext = tsiTravelSegMap.find(tsm.nextTravelSeg());

        bool isTsiAllIntlSector = (trInfoSeg->tsi() == 18);
        if (isTsiAllIntlSector)
        {
          if ((tsiIter != tsiTravelSegMap.end()) && (tsiIterNext != tsiTravelSegMap.end()))
            geoMatch = Transfers1::MATCH;
        }
        else if (((tsiIter != tsiTravelSegMap.end()) && ((*tsiIter).second->destMatch())) ||
                 ((tsiIterNext != tsiTravelSegMap.end()) && ((*tsiIterNext).second->origMatch())))
        {
          geoMatch = Transfers1::MATCH;
        }
      }
      else if (trInfoSeg->transferAppl() == Transfers1::SEG_GEO_APPL_REQUIRED)
      {
        std::ostringstream failString;
        failString << " TRANSFER REQUIRED BUT NO SEGMENTS MATCH TSI: " << trInfoSeg->tsi();
        failReason = failString.str();
        failed = true;
        return false;
      }
    }
  }
  else
  {
    geoMatch = matchGeo(trx, tsm.travelSeg()->destination(), trInfo, *trInfoSeg, vendorOfFare);
  }

  if (((geoMatch == Transfers1::MATCH) || (geoMatch == Transfers1::DOES_NOT_APPLY)) &&
      trInfoSeg->transferAppl() == Transfers1::SEG_GEO_APPL_NOT_PERMITTED)
  {
    if (trInfoSeg->chargeAppl() == Transfers1::SEG_NO_CHARGE)
    {
      processNextInfoSeg = false;
      failed = false;
      return false;
    }
    else
    {
      processNextInfoSeg = true;
      failed = false;
      return false;
    }
  }

  processNextInfoSeg = true;
  failed = false;
  return true;
}

bool
Transfers1::checkInfoSegAltGeo(PricingTrx& trx,
                               TravelSegMarkup& tsm,
                               TransfersInfoSegMarkup& trISM,
                               const TransfersInfo1& trInfo,
                               std::vector<TransfersInfoSegMarkup*>& trISMVector,
                               const FarePath* fp,
                               const PricingUnit* pu,
                               const FareMarket* fm,
                               const PaxTypeFare* ptf,
                               bool processChargeCalcOnly,
                               MatchResult& geoMatch,
                               bool& processNextInfoSeg,
                               bool& failed,
                               std::string& failReason,
                               const TransfersInfoSeg1*& matchedInfoSeg,
                               const VendorCode& vendorOfFare)
{
  if (!tsm.altOffPointGeo())
  {
    processNextInfoSeg = true;
    failed = false;
    return false;
  }

  geoMatch = Transfers1::NOT_MATCH;

  TSITravelSegMap tsiTravelSegMap;

  const TransfersInfoSeg1* trInfoSeg = trISM.trInfoSeg();

  if (trInfoSeg->tsi())
  {
    if (!processTSI(trx, tsiTravelSegMap, trInfoSeg, fp, pu, fm, ptf, failReason))
    {
      failed = true;
      return false;
    }
    else
    {
      if (!tsiTravelSegMap.empty())
      {
        TSITravelSegMap::const_iterator tsiIterNext = tsiTravelSegMap.find(tsm.nextTravelSeg());

        if ((tsiIterNext != tsiTravelSegMap.end()) && ((*tsiIterNext).second->origMatch()))
        {
          geoMatch = Transfers1::MATCH;
        }
      }
    }
  }
  else
  {
    geoMatch = matchGeo(trx, tsm.altOffPointGeo(), trInfo, *trInfoSeg, vendorOfFare);
  }

  if (geoMatch == Transfers1::MATCH)
    matchedInfoSeg = trInfoSeg;

  if ((geoMatch == Transfers1::MATCH) || (geoMatch == Transfers1::DOES_NOT_APPLY))
  {
    processNextInfoSeg = true;
    failed = false;
    return true;
  }

  std::vector<TransfersInfoSegMarkup*>::iterator it = trISMVector.begin();
  std::vector<TransfersInfoSegMarkup*>::iterator itE = trISMVector.end();

  for (; it != itE; ++it)
  {
    TransfersInfoSegMarkup* trISMptr = *it;
    if (trISMptr == &trISM)
    {
      continue;
    }

    const TransfersInfoSeg1* trIS = trISMptr->trInfoSeg();

    setRecurringSegCounters(trx, fm, vendorOfFare, *tsm.altOffPointGeo(), *trISMptr);

    // *****************************************
    //@TODO: Need to check transfer type here...
    // *****************************************

    if (!trISMptr->numTransfersUnlimited())
    {
      if (trISMptr->matchCount() == trISMptr->numTransfers())
      {
        continue;
      }
      else if (trISMptr->checkInOut())
      {
        if (((trISMptr->inOutIn()) && (trISMptr->matchCountIn() == trISMptr->numTransfers())) ||
            ((trISMptr->inOutOut()) && (trISMptr->matchCountOut() == trISMptr->numTransfers())))
        {
          continue;
        }
      }
    }

    if (trISMptr->checkInOut())
    {
      if ((trISMptr->inOutOut()) && (tsm.direction() == TravelSegMarkup::INBOUND))
      {
        continue;
      }
      else if ((trISMptr->inOutIn()) && (tsm.direction() == TravelSegMarkup::OUTBOUND))
      {
        continue;
      }
    }

    if (checkInfoSegStopConn(tsm, *trIS) == Transfers1::NOT_MATCH)
    {
      continue;
    }

    MatchResult inCxrMatch;
    MatchResult outCxrMatch;

    if (checkInfoSegCarrier(trx, tsm, trInfo, *trIS, inCxrMatch, outCxrMatch, vendorOfFare) ==
            Transfers1::NOT_MATCH ||
        checkInfoSegCarrierIn(trx, tsm.carrier(), trInfo, *trIS, vendorOfFare) ==
            Transfers1::NOT_MATCH ||
        checkInfoSegCarrierOut(trx, tsm.carrierOut(), trInfo, *trIS, vendorOfFare) ==
            Transfers1::NOT_MATCH)
    {
      continue;
    }

    bool dummy1(true);
    bool dummy2(false);
    std::string dummy3;
    std::string dummy4;
    if (!checkInfoSegDirectionality(tsm, *trISMptr, dummy1, dummy2, dummy3, dummy4))
    {
      continue;
    }

    tsiTravelSegMap.clear();
    if (trIS->tsi())
    {
      if (!processTSI(trx, tsiTravelSegMap, trIS, fp, pu, fm, ptf, failReason))
      {
        failed = true;
        return false;
      }
      else
      {
        if (!tsiTravelSegMap.empty())
        {
          TSITravelSegMap::const_iterator tsiIterNext = tsiTravelSegMap.find(tsm.nextTravelSeg());

          if ((tsiIterNext != tsiTravelSegMap.end()) && ((*tsiIterNext).second->origMatch()))
          {
            geoMatch = Transfers1::MATCH;
          }
        }
      }
    }
    else
    {
      geoMatch = matchGeo(trx, tsm.altOffPointGeo(), trInfo, *trIS, vendorOfFare);
    }

    if (geoMatch == Transfers1::MATCH)
      matchedInfoSeg = trIS;

    if ((geoMatch == Transfers1::MATCH) || (geoMatch == Transfers1::DOES_NOT_APPLY))
    {
      processNextInfoSeg = true;
      failed = false;
      return true;
    }
  }

  return true;
}

bool
Transfers1::processTSI(PricingTrx& trx,
                       TSITravelSegMap& tsiTravelSegMap,
                       const TransfersInfoSeg1* trInfoSeg,
                       const FarePath* fp,
                       const PricingUnit* pu,
                       const FareMarket* fm,
                       const PaxTypeFare* ptf,
                       std::string& failReason)
{
  TSICode tsi = trInfoSeg->tsi();
  if (LIKELY(tsi))
  {
    RuleUtil::TravelSegWrapperVector applTravSeg;

    bool doProcessTSI = true;

    RuleConst::TSIScopeParamType scope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;

    if ((pu && ptf) && (ptf->owrt() != ONE_WAY_MAY_BE_DOUBLED) &&
        (ptf->owrt() != ONE_WAY_MAYNOT_BE_DOUBLED))
    {
      scope = RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY;
    }

    RuleConst::TSIScopeType tsiScope;

    if (!RuleUtil::getTSIScope(tsi, trx, tsiScope))
    {
      std::ostringstream errMsg;
      errMsg << "Rules.Transfers1: Could not get TSI scope for TSI: " << tsi;
      LOG4CXX_ERROR(logger, errMsg.str());
      std::ostringstream failString;
      failString << "TSI ERROR - SEG: " << trInfoSeg->orderNo();
      failReason = failString.str();
      return false;
    }

    if ((tsiScope == RuleConst::TSI_SCOPE_SUB_JOURNEY) &&
        (scope != RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY))
    {
      // We can't process a SubJourney scope TSI without a PU.
      // Category 9 can't override TSI scope, so we must skip
      //  TSI processing.
      if (!pu)
      {
        doProcessTSI = false;
      }
    }

    if (LIKELY(doProcessTSI))
    {
      if (!RuleUtil::scopeTSIGeo(tsi,
                                 trInfoSeg->loc1(),
                                 trInfoSeg->loc2(),
                                 scope,
                                 false,
                                 false,
                                 false,
                                 trx,
                                 fp,
                                 nullptr,
                                 pu,
                                 fm,
                                 TrxUtil::getTicketingDT(trx),
                                 applTravSeg,
                                 Diagnostic309))
      {
        LOG4CXX_ERROR(logger, "Rules.Transfers1: Error processing TSI.");
        std::ostringstream failString;
        failString << "TSI ERROR - SEG: " << trInfoSeg->orderNo();
        failReason = failString.str();
        return false;
      }

      if (!applTravSeg.empty())
      {
        RuleUtil::TravelSegWrapperVectorCI tsiIter = applTravSeg.begin();
        RuleUtil::TravelSegWrapperVectorCI tsiIterEnd = applTravSeg.end();

        for (; tsiIter != tsiIterEnd; ++tsiIter)
        {
          TSITravelSegMap::value_type value((*tsiIter)->travelSeg(), (*tsiIter));

          tsiTravelSegMap.insert(value);
        }
      }
    }
  }
  return true;
}

Transfers1::ProcessingResult
Transfers1::checkMinTransfers(PricingTrx& trx,
                              const TravelSegMarkupContainer& tsmCon,
                              const TransfersInfo1& trInfo,
                              const PaxTypeFare* ptf,
                              std::string& failReason)
{
  int16_t numTransfers = 0;
  int16_t numTransfersMin = 0;

  std::string noTransfersMin = trInfo.noTransfersMin();

  if (noTransfersMin.empty())
    return Transfers1::PASS;

  numTransfersMin = atoi(noTransfersMin.c_str());

  bool maxExists = !trInfo.noTransfersMax().empty();

  TravelSegMarkupCI iter = tsmCon.begin();
  TravelSegMarkupCI iterEnd = tsmCon.end();

  for (; iter != iterEnd; ++iter)
  {
    const TravelSegMarkup* tsm = (*iter);

    if (tsm->transferType() != Transfers1::TFR_NONE &&
        (!tsm->fareUsage() ||
         (maxExists || (!maxExists && tsm->fareUsage()->paxTypeFare() == ptf))))
    {
      ++numTransfers;
    }
  }

  if (numTransfers < numTransfersMin)
  {
    std::ostringstream failString;
    failString << "MIN " << numTransfersMin << " TRANSFERS REQUIRED";
    failReason = failString.str();
    return Transfers1::FAIL;
  }
  return Transfers1::PASS;
}

Transfers1::ProcessingResult
Transfers1::checkMaxTransfers(PricingTrx& trx,
                              const TransfersInfoWrapper& trInfoWrapper,
                              const TravelSegMarkupContainer& tsmCon,
                              std::string& failReason)
{
  // It may look like a duplication of checkMaxTransfersOutIn but the latter is doing just
  // everything and should be named "doPostprocessing" or sth like that.
  // Note that we don't check here whether the transfer is in scope according to FC/PU application
  // rules and that's correct as long as identifyTransfers() takes care of it. Please avoid making
  // redundant conditions...

  if (trInfoWrapper.noTransfersMax() >= RuleConst::MAX_NUMBER_XX)
    return Transfers1::PASS;

  size_t numTransfers = std::count_if(tsmCon.begin(),
                                      tsmCon.end(),
                                      [](const TravelSegMarkup* tsm)
                                      { return tsm->transferType() != Transfers1::TFR_NONE; });

  if (numTransfers > trInfoWrapper.noTransfersMax())
  {
    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic309))
      failReason = "MAX " + std::to_string(trInfoWrapper.noTransfersMax()) + " TRANSFERS PERMITTED";
    return Transfers1::FAIL;
  }

  return Transfers1::PASS;
}

Transfers1::ProcessingResult
Transfers1::checkMaxTransfersInOut(PricingTrx& trx,
                                   TravelSegMarkupContainer& tsmCon,
                                   const TransfersInfoWrapper& trInfoWrapper,
                                   const PaxTypeFare* ptf,
                                   std::string& failReason)
{
  const TransfersInfo1* trInfo = trInfoWrapper.trInfo();

  int16_t outTransfersCtr = 0;
  int16_t inTransfersCtr = 0;
  int16_t totalTransfersCtr = 0;

  bool checkTransferTypeMax = false;

  int16_t primePrimeCtr = 0;
  int16_t sameSameCtr = 0;
  int16_t primeOtherCtr = 0;
  int16_t otherOtherCtr = 0;

  int16_t primePrimeMax = 0;
  int16_t sameSameMax = 0;
  int16_t primeOtherMax = 0;
  int16_t otherOtherMax = 0;

  bool numTransfersMaxUnlimited = false;
  bool numTransfersOutUnlimited = false;
  bool numTransfersInUnlimited = false;

  bool primePrimeUnlimited = false;
  bool sameSameUnlimited = false;
  bool primeOtherUnlimited = false;
  bool otherOtherUnlimited = false;

  int16_t numTransfersMax = strToInt(trInfo->noTransfersMax(), numTransfersMaxUnlimited);
  int16_t numTransfersOut = 0;
  int16_t numTransfersIn = 0;

  if (UNLIKELY(isRtw()))
  {
    numTransfersOutUnlimited = true;
    numTransfersInUnlimited = true;
  }
  else
  {
    numTransfersOut = strToInt(trInfo->noOfTransfersOut(), numTransfersOutUnlimited);
    numTransfersIn = strToInt(trInfo->noOfTransfersIn(), numTransfersInUnlimited);
  }

  Indicator primePrimeInd = trInfo->primeCxrPrimeCxr();
  Indicator sameSameInd = trInfo->sameCxrSameCxr();
  Indicator primeOtherInd = trInfo->primeCxrOtherCxr();
  Indicator otherOtherInd = trInfo->otherCxrOtherCxr();

  if (primePrimeInd == Transfers1::ALLOW_BLANK && sameSameInd == Transfers1::ALLOW_BLANK &&
      primeOtherInd == Transfers1::ALLOW_BLANK && otherOtherInd == Transfers1::ALLOW_BLANK)
  {
    primePrimeUnlimited = true;
    sameSameUnlimited = true;
    primeOtherUnlimited = true;
    otherOtherUnlimited = true;
  }
  else
  {
    checkTransferTypeMax = true;

    if (primePrimeInd == Transfers1::ALLOW_PRIME_PRIME)
    {
      primePrimeMax = strToInt(trInfo->primePrimeMaxTransfers(), primePrimeUnlimited);
    }
    if (sameSameInd == Transfers1::ALLOW_SAME_SAME)
    {
      sameSameMax = strToInt(trInfo->sameSameMaxTransfers(), sameSameUnlimited);
    }
    if (primeOtherInd == Transfers1::ALLOW_PRIME_OTHER)
    {
      primeOtherMax = strToInt(trInfo->primeOtherMaxTransfers(), primeOtherUnlimited);
    }
    if (otherOtherInd == Transfers1::ALLOW_OTHER_OTHER)
    {
      otherOtherMax = strToInt(trInfo->otherOtherMaxTransfers(), otherOtherUnlimited);
    }
  }

  const bool fcVsPu = isFCvsPU(*trInfo);
  bool trfOnOutBound = false;
  bool trfOnInBound = false;

  PaxTypeFare* tempPTF = nullptr;
  TravelSegMarkupI iter = tsmCon.begin();
  TravelSegMarkupI iterEnd = tsmCon.end();

  for (; iter != iterEnd; ++iter)
  {
    TravelSegMarkup* tsm = (*iter);

    bool passedByLeastRestrictive = false;

    if (tsm->transferType() == Transfers1::TFR_NONE)
      continue;

    const bool pftPresent = (tsm->fareUsage() && tsm->fareUsage()->paxTypeFare() &&
                             tsm->fareUsage()->paxTypeFare()->fareMarket());
    const bool validateEntireRule =
        tsm->validateEntireRule() || (pftPresent && tsm->fareUsage()->paxTypeFare() == ptf);

    TravelSeg* ts = tsm->travelSeg();
    FareUsage* fu = tsm->fareUsage();

    if (!tempPTF && fu)
    {
      tempPTF = fu->paxTypeFare();
    }
    else
    {
      if (fu && tempPTF && tempPTF != fu->paxTypeFare())
      {
        if (validateEntireRule && (trInfoWrapper.transferFCscope() || fcVsPu))
        {
          tempPTF = fu->paxTypeFare();
          totalTransfersCtr = 0;
        }
      }
    }

    trInfoWrapper.setIsTransfer(ts, fu, validateEntireRule);

    if ((tsm->transferTypeMatchResult() == Transfers1::MATCH) || (!validateEntireRule))
    {
      bool isTentativeMatch = false;
      uint32_t ruleItemMatch = trInfoWrapper.getRuleItemMatch(ts, isTentativeMatch);

      bool passedValidation = trInfoWrapper.checkPassedValidation(ts);

      if (ruleItemMatch)
      {
        if (ruleItemMatch == trInfo->itemNo())
        {
          continue;
        }
        else
        {
          if (LIKELY((!isTentativeMatch) && (passedValidation || tsm->isTentativeMatch())))

          {
            continue;
          }
        }
      }
    }

    if ((tsm->transferTypeMatchResult() == Transfers1::MATCH) && (validateEntireRule))
    {
      if (tsm->transferCheckResult() == TravelSegMarkup::SOFTPASS)
      {
        trInfoWrapper.setRuleItemMatch(ts, fu, trInfo->itemNo(), tsm->isTentativeMatch());
        trInfoWrapper.setPassedValidation(ts, fu, false, trInfoWrapper.recurringFCscope());
        continue;
      }

      // NOTE: Direction can only be UNKNOWN during fare component
      //        validation.
      if (tsm->direction() == TravelSegMarkup::UNKNOWN)
      {
        if ((!numTransfersOutUnlimited) || (!numTransfersInUnlimited))
        {
          trInfoWrapper.setRuleItemMatch(ts, fu, trInfo->itemNo(), tsm->isTentativeMatch());
          trInfoWrapper.setPassedValidation(ts, fu, false, trInfoWrapper.recurringFCscope());
          continue;
        }
      }
    }

    if (UNLIKELY(tsm->transferCheckResult() == TravelSegMarkup::FAIL))
    {
      continue;
    }

    bool segCheck =
        ((tsm->trSegCheckResult() == TravelSegMarkup::PASS) || (tsm->tiSegMatch().empty()));

    bool trMatch = (tsm->transferTypeMatchResult() == Transfers1::MATCH);

    if ((segCheck && trMatch) || !validateEntireRule)
    {
      int16_t tempOutTransfersCtr = outTransfersCtr;
      int16_t tempInTransfersCtr = inTransfersCtr;
      int16_t tempTotalTransfersCtr = totalTransfersCtr;

      int16_t tempPrimePrimeCtr = primePrimeCtr;
      int16_t tempSameSameCtr = sameSameCtr;
      int16_t tempPrimeOtherCtr = primeOtherCtr;
      int16_t tempOtherOtherCtr = otherOtherCtr;

      if (fallback::cat9FixMaxTransfers(&trx) || (segCheck && trMatch))
      {
        ++tempTotalTransfersCtr;

        if (numTransfersMax == 0)
        {
          if (UNLIKELY(!validateEntireRule)) // Do not count this transfer
            --tempTotalTransfersCtr;
        }
      }

      // If the transfer rule allows no transfers and a transfer
      //  exists, then fail unless the Least Restrictive Provision
      //  applies.
      // If the Least Restrictive Provision applies and the least
      //  restrictive rule allows no transfers, then fail.
      //
      if ((!numTransfersMaxUnlimited) && (numTransfersMax == 0) && (tempTotalTransfersCtr > 0) &&
          (!trInfoWrapper.applyLeastRestrictiveProvision() ||
           (!trInfoWrapper.leastRestrictiveTransfersUnlimited() &&
            trInfoWrapper.leastRestrictiveTransfersPermitted() == 0)))
      {
        failReason = "NO TRANSFERS PERMITTED";
        return Transfers1::FAIL;
      }
      if (LIKELY(validateEntireRule))
      {
        if (tsm->direction() == TravelSegMarkup::OUTBOUND)
        {
          ++tempOutTransfersCtr;

          /// Do not reset trfOutBound and trfInBound because
          /// both are used only in PU scope when OR is coded X
          trfOnOutBound = true;
        }
        else if (tsm->direction() == TravelSegMarkup::INBOUND)
        {
          ++tempInTransfersCtr;
          trfOnInBound = true;
        }

        int16_t transferTypeCtr = 0;
        int16_t transferTypeMax = 0;
        bool transferTypeMaxUnlimited = false;

        if (checkTransferTypeMax)
        {
          if (tsm->transferType() == Transfers1::PRIMARY_PRIMARY)
          {
            transferTypeMax = primePrimeMax;
            transferTypeMaxUnlimited = primePrimeUnlimited;
            ++tempPrimePrimeCtr;
            transferTypeCtr = tempPrimePrimeCtr;
          }
          else if (tsm->transferType() == Transfers1::PRIMARY_OTHER)
          {
            transferTypeMax = primeOtherMax;
            transferTypeMaxUnlimited = primeOtherUnlimited;
            ++tempPrimeOtherCtr;
            transferTypeCtr = tempPrimeOtherCtr;
          }
          else if (tsm->transferType() == Transfers1::SAME_SAME)
          {
            transferTypeMax = sameSameMax;
            transferTypeMaxUnlimited = sameSameUnlimited;
            ++tempSameSameCtr;
            transferTypeCtr = tempSameSameCtr;
          }
          else if (tsm->transferType() == Transfers1::OTHER_OTHER)
          {
            transferTypeMax = otherOtherMax;
            transferTypeMaxUnlimited = otherOtherUnlimited;
            ++tempOtherOtherCtr;
            transferTypeCtr = tempOtherOtherCtr;
          }
        }

        if (trInfo->noOfTransfersAppl() == Transfers1::OUT_OR_RETURN_EXCLUSIVE)
        {
          bool ignoreORField = trInfoWrapper.transferFCscope() || isRtw();
          bool transfersInBothDirections = trfOnInBound && trfOnOutBound;

          if (!ignoreORField && transfersInBothDirections)
          {
            failReason = "TRANSFERS PERMITTED OUT OR IN. NOT BOTH";
            return Transfers1::FAIL;
          }
        }

        if ((!numTransfersMaxUnlimited) && (tempTotalTransfersCtr > numTransfersMax))
        {
          trInfoWrapper.setMaxExceeded(ts, fu, TransfersInfoWrapper::TOTAL_MAX_EXCEED);
          continue;
        }

        if ((!numTransfersOutUnlimited) && (tempOutTransfersCtr > numTransfersOut))
        {
          trInfoWrapper.setMaxExceeded(ts, fu, TransfersInfoWrapper::TOTAL_MAX_EXCEED);
          continue;
        }

        if ((!numTransfersInUnlimited) && (tempInTransfersCtr > numTransfersIn))
        {
          trInfoWrapper.setMaxExceeded(ts, fu, TransfersInfoWrapper::TOTAL_MAX_EXCEED);
          continue;
        }

        if ((checkTransferTypeMax) && (!transferTypeMaxUnlimited) &&
            (transferTypeCtr > transferTypeMax))
        {
          continue;
        }

        trInfoWrapper.setRuleItemMatch(ts, fu, trInfo->itemNo(), tsm->isTentativeMatch());
      }
      else
      {
        if (trInfoWrapper.applyLeastRestrictiveProvision())
        {
          if (trInfoWrapper.leastRestrictiveTransfersUnlimited())
          {
            passedByLeastRestrictive = true;
          }
          else
          {
            if (trInfoWrapper.leastRestrictiveTransfersPermitted() >= tempTotalTransfersCtr)
            {
              passedByLeastRestrictive = true;
            }
            else
            {
              continue;
            }
          }
        }
        else
        {
          if ((!numTransfersMaxUnlimited) && (tempTotalTransfersCtr > numTransfersMax))
          {
            continue;
          }
          // this code is needed mostly to support 309 diagnostic in part to display
          // for RuleItem number the "----"
          if (tsm->fareUsage() && ptf != tsm->fareUsage()->paxTypeFare() &&
              trInfoWrapper.transferFCscope())
          {
            trInfoWrapper.setRuleItemMatch(ts,
                                           tsm->fareUsage(),
                                           Transfers1::TRANSFER_RULE_ITEM_FOR_NOMATCH,
                                           tsm->isTentativeMatch());
          }
          else
            trInfoWrapper.setRuleItemMatch(ts, fu, trInfo->itemNo(), tsm->isTentativeMatch());
        }
      }

      tsm->transferCheckResult() = TravelSegMarkup::PASS;

      trInfoWrapper.setPassedValidation(
          ts, fu, passedByLeastRestrictive, trInfoWrapper.recurringFCscope());

      outTransfersCtr = tempOutTransfersCtr;
      inTransfersCtr = tempInTransfersCtr;
      totalTransfersCtr = tempTotalTransfersCtr;

      primePrimeCtr = tempPrimePrimeCtr;
      sameSameCtr = tempSameSameCtr;
      primeOtherCtr = tempPrimeOtherCtr;
      otherOtherCtr = tempOtherOtherCtr;
    }
  }
  return Transfers1::PASS;
}

Transfers1::MatchResult
Transfers1::matchGeo(PricingTrx& trx,
                     const Loc* loc,
                     const TransfersInfo1& trInfo,
                     const TransfersInfoSeg1& trInfoSeg,
                     const VendorCode& vendorOfFare) const
{
  const LocTypeCode& locType1 = trInfoSeg.loc1().locType();
  const LocTypeCode& locType2 = trInfoSeg.loc2().locType();

  if ((locType1 == LOCTYPE_NONE) && (locType2 == LOCTYPE_NONE) && (!trInfoSeg.zoneTblItemNo()))
  {
    return Transfers1::DOES_NOT_APPLY;
  }

  if (isLocTypeWithNoLocData(trInfoSeg))
  {
    return Transfers1::MATCH;
  }

  if (checkIsInLoc(trx, *loc, trInfoSeg.loc1(), vendorOfFare))
  {
    return Transfers1::MATCH;
  }

  if (locType2 != LOCTYPE_NONE)
  {
    if (checkIsInLoc(trx, *loc, trInfoSeg.loc2(), vendorOfFare))
    {
      return Transfers1::MATCH;
    }
  }
  else if (trInfoSeg.zoneTblItemNo())
  {
    std::ostringstream strZone;
    strZone << trInfoSeg.zoneTblItemNo();
    Zone zone = strZone.str();

    if (LocUtil::isInZone(*loc, trInfo.vendor(), zone, USER_DEFINED))
    {
      return Transfers1::MATCH;
    }
  }
  return Transfers1::NOT_MATCH;
}

bool
Transfers1::checkIsInLoc(PricingTrx& trx,
                         const Loc& locToCheck,
                         const LocKey& locToCheckWithin,
                         const VendorCode& vendor) const
{
  return locToCheckWithin.locType() != LOCTYPE_NONE && !locToCheckWithin.loc().empty() &&
         TransStopUtil::isInLoc(trx, vendor, locToCheck, locToCheckWithin);
}

bool
Transfers1::isFareBreakSurfacePermitted(PricingTrx& trx,
                                        const TransfersInfo1& trInfo,
                                        const FareMarket& fm) const
{
  if (trInfo.fareBreakSurfaceInd() == Transfers1::SURFACE_NOT_PERMITTED)
  {
    return false;
  }

  const CarrierPreference* cxrPref =
      trx.dataHandle().getCarrierPreference(fm.governingCarrier(), trx.getRequest()->ticketingDT());

  if (UNLIKELY(cxrPref &&
      cxrPref->noSurfaceAtFareBreak() == Transfers1::CXR_PREF_FARE_BRK_SURFACE_NOT_PERMITTED))
  {
    return false;
  }

  return true;
}

uint32_t
Transfers1::getMaxNumEmbeddedSurfaces(PricingTrx& trx, const TransfersInfo1& trInfo) const
{
  if (trInfo.embeddedSurfaceInd() == Transfers1::SURFACE_NOT_PERMITTED)
  {
    return 0;
  }
  else if (trInfo.embeddedSurfaceInd() >= '1' && trInfo.embeddedSurfaceInd() <= '9')
  {
    return static_cast<uint32_t>(trInfo.embeddedSurfaceInd() - '0');
  }

  return SURFACE_UNLIMITED;
}

bool
Transfers1::isSurfaceSector(const FareMarket& fm, const TravelSeg& ts) const
{
  if (ts.isAir())
    return false;

  LocCode origMTC = LocUtil::getMultiTransportCity(
      ts.origAirport(), fm.governingCarrier(), fm.geoTravelType(), fm.travelDate());
  LocCode destMTC = LocUtil::getMultiTransportCity(
      ts.destAirport(), fm.governingCarrier(), fm.geoTravelType(), fm.travelDate());
  if (!origMTC.empty() && origMTC == destMTC)
    return false;

  return true;
}

bool
Transfers1::matchSurfaceIntlDom(const TravelSeg& ts, const SurfaceTransfersInfo& sti) const
{
  switch (sti.restriction())
  {
  case SURFACE_TABLE_RESTR_DOMESTIC:
    return isDomestic(ts);
  case SURFACE_TABLE_RESTR_INTERNATIONAL:
    return !isDomestic(ts);
  default:
    return true;
  }
}

bool
Transfers1::matchSurfaceOrigDest(const TravelSeg& ts,
                                 const FareMarket& fm,
                                 const SurfaceTransfersInfo& sti) const
{
  if (fm.travelSeg().empty())
    return false;

  switch (sti.originDest())
  {
  case SURFACE_TABLE_OD_EITHER:
    return true;
  case SURFACE_TABLE_OD_ORIG:
    return &ts == fm.travelSeg().front();
  case SURFACE_TABLE_OD_DEST:
    return &ts == fm.travelSeg().back();
  default:
    return false;
  }
}

bool
Transfers1::matchSurfaceGeo(PricingTrx& trx, const TravelSeg& ts, const SurfaceTransfersInfo& sti)
    const
{
  const LocKey& locKey1 = sti.fareBrkEmbeddedLoc();
  const LocKey& locKey2 = sti.surfaceLoc();

  const LocTypeCode& locType1 = locKey1.locType();
  const LocTypeCode& locType2 = locKey2.locType();

  if (locType1 != LOCTYPE_NONE && locType2 != LOCTYPE_NONE)
  {
    return (checkIsInLoc(trx, *ts.origin(), locKey1, sti.vendor()) &&
            checkIsInLoc(trx, *ts.destination(), locKey2, sti.vendor())) ||
           (checkIsInLoc(trx, *ts.destination(), locKey1, sti.vendor()) &&
            checkIsInLoc(trx, *ts.origin(), locKey2, sti.vendor()));
  }

  if (locType1 != LOCTYPE_NONE)
    return checkIsInLoc(trx, *ts.origin(), locKey1, sti.vendor());

  if (locType2 != LOCTYPE_NONE)
    return checkIsInLoc(trx, *ts.destination(), locKey2, sti.vendor());

  return true;
}

bool
Transfers1::matchSurfaceGeoFareBreak(PricingTrx& trx,
                                     const TravelSeg& ts,
                                     const SurfaceTransfersInfo& sti,
                                     bool matchFirst) const
{
  const LocKey& locKey1 = sti.fareBrkEmbeddedLoc();
  const LocKey& locKey2 = sti.surfaceLoc();

  const LocTypeCode& locType1 = locKey1.locType();
  const LocTypeCode& locType2 = locKey2.locType();

  const Loc* fareBreakLoc = matchFirst ? ts.origin() : ts.destination();
  const Loc* nonFareBreakLoc = matchFirst ? ts.destination() : ts.origin();

  if ((locType1 != LOCTYPE_NONE) && (locType2 != LOCTYPE_NONE))
  {
    return checkIsInLoc(trx, *fareBreakLoc, locKey1, sti.vendor()) &&
           checkIsInLoc(trx, *nonFareBreakLoc, locKey2, sti.vendor());
  }

  if (locType1 != LOCTYPE_NONE)
    return checkIsInLoc(trx, *fareBreakLoc, locKey1, sti.vendor());
  if (locType2 != LOCTYPE_NONE)
    return checkIsInLoc(trx, *nonFareBreakLoc, locKey2, sti.vendor());

  return true;
}

bool
Transfers1::hasGeoLocationsSpecified(const SurfaceTransfersInfo& sti) const
{
  return sti.fareBrkEmbeddedLoc().locType() != LOCTYPE_NONE &&
         sti.surfaceLoc().locType() != LOCTYPE_NONE;
}

int16_t
Transfers1::strToInt(const std::string& str, bool& unlimited) const
{
  int16_t num = 0;
  unlimited = false;

  if (str.empty() || (str == Transfers1::NUM_TRANSFERS_UNLIMITED))
  {
    unlimited = true;
  }
  else
  {
    num = atoi(str.c_str());
  }
  return num;
}

void
Transfers1::printDiagnostics(DiagCollector& diag,
                             const TransfersInfo1& soInfo,
                             const TravelSegMarkupContainer& tsmCon,
                             const FareMarket& fm) const
{
  diag << "---FARE COMPONENT ITEM BREAKDOWN---" << std::endl;

  TravelSegMarkupCI tsmIterEnd = tsmCon.end();

  std::vector<TravelSeg*>::const_iterator iter = fm.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = fm.travelSeg().end();

  for (; iter != iterEnd; ++iter)
  {
    const TravelSeg* ts = *iter;

    if (dynamic_cast<const AirSeg*>(ts))
    {
      TravelSegMarkupCI tsmIter = tsmCon.find(ts);

      if (tsmIter != tsmIterEnd)
      {
        const TravelSegMarkup* tsm = (*tsmIter);

        printDiagTravelSeg(diag, ts, tsm->segmentNumber());
        printDiagTsmDetail(diag, soInfo, *tsm, ts);
      }
      else
      {
        printDiagTravelSeg(diag, ts, 0);
      }
    }
    else
    {
      printDiagTravelSeg(diag, ts, 0);
    }
    diag << std::endl;
  }
  diag << " " << std::endl;
}

void
Transfers1::printDiagnostics(DiagCollector& diag,
                             const TransfersInfo1& trInfo,
                             const TravelSegMarkupContainer& tsmCon,
                             const PricingUnit& pu) const
{
  diag << "---PRICING UNIT ITEM BREAKDOWN---" << std::endl;

  TravelSegMarkupCI tsmIterEnd = tsmCon.end();

  std::vector<FareUsage*>::const_iterator fuI = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();

  int16_t fcNum = 0;

  for (; fuI != fuEnd; ++fuI)
  {
    ++fcNum;

    const FareUsage* fu = *fuI;
    const PaxTypeFare* ptf = fu->paxTypeFare();

    diag << "FARECOMP-" << fcNum << " : " << std::setw(12) << std::left << ptf->fareClass()
         << std::right << "   ";

    if (fu->isInbound())
    {
      diag << ".IN.  ";
    }
    else
    {
      diag << ".OUT. ";
    }

    int16_t airSegCount = 0;

    std::vector<TravelSeg*>::const_iterator tsI = fu->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tsEnd = fu->travelSeg().end();

    for (; tsI != tsEnd; ++tsI)
    {
      if (dynamic_cast<const AirSeg*>(*tsI))
      {
        ++airSegCount;
      }
    }

    if (airSegCount > 2)
    {
      diag << std::setw(3) << airSegCount - 1 << " OFFPOINTS";
    }
    else if (airSegCount == 2)
    {
      diag << "  1 OFFPOINT ";
    }
    else
    {
      diag << " NO OFFPOINTS";
    }

    diag << " FROM " << ptf->fareMarket()->origin()->loc() << " TO "
         << ptf->fareMarket()->destination()->loc() << std::endl;

    for (tsI = fu->travelSeg().begin(); tsI != tsEnd; ++tsI)
    {
      const TravelSeg* ts = *tsI;

      if (dynamic_cast<const AirSeg*>(ts))
      {
        TravelSegMarkupCI tsmIter = tsmCon.find(ts);

        if (tsmIter != tsmIterEnd)
        {
          const TravelSegMarkup* tsm = (*tsmIter);

          printDiagTravelSeg(diag, ts, tsm->segmentNumber());
          printDiagTsmDetail(diag, trInfo, *tsm, ts);
        }
        else
        {
          printDiagTravelSeg(diag, ts, 0);
        }
      }
      else
      {
        printDiagTravelSeg(diag, ts, 0);
      }
      diag << std::endl;
    }
    diag << " " << std::endl;
  }
}

void
Transfers1::printDiagHeader(PricingTrx& trx,
                            DiagCollector& diag,
                            const TransfersInfoWrapper& trInfoWrapper,
                            const TransfersInfo1& trInfo,
                            const FareMarket* fm) const
{
  if (!fallback::cat9FixMaxTransfers(&trx) &&
      trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "SUBSET")
  {
    diag << " \n---TRANSFER SUBSET DATA" << std::endl;
    diag << "TRANSFERS..... MAX:" << std::setw(3) << std::right;

    if (trInfoWrapper.noTransfersMax() >= RuleConst::MAX_NUMBER_XX)
      diag << "XX\n";
    else
      diag << trInfoWrapper.noTransfersMax() << "\n";
  }

  diag << " " << std::endl << "---TRANSFER RULE DATA---" << std::endl << "TRANSFERS..... "
       << "MIN:" << std::setw(3) << std::right << trInfo.noTransfersMin() << "  "
       << "MAX:" << std::setw(3) << std::right << trInfo.noTransfersMax() << "  "
       << "OR: " << std::setw(3) << std::right << trInfo.noOfTransfersAppl() << "  "
       << "OUT:" << std::setw(3) << std::right << trInfo.noOfTransfersOut() << "  "
       << "IN: " << std::setw(3) << std::right << trInfo.noOfTransfersIn() << std::endl
       << "ONLINE........"
       << " PRI/PRI: " << trInfo.primeCxrPrimeCxr() << std::setw(7) << std::right
       << trInfo.primePrimeMaxTransfers() << "     SME/SME: " << trInfo.sameCxrSameCxr()
       << std::setw(7) << std::right << trInfo.sameSameMaxTransfers() << std::endl
       << "INTERLINE....."
       << " PRI/OTH: " << trInfo.primeCxrOtherCxr() << std::setw(7) << std::right
       << trInfo.primeOtherMaxTransfers() << "     OTH/OTH: " << trInfo.otherCxrOtherCxr()
       << std::setw(7) << std::right << trInfo.otherOtherMaxTransfers() << std::endl;

  uint32_t maxNumEmbedded = SURFACE_UNLIMITED;
  bool fareBreakPermitted = true;

  maxNumEmbedded = getMaxNumEmbeddedSurfaces(trx, trInfo);
  fareBreakPermitted = isFareBreakSurfacePermitted(trx, trInfo, *fm);

  diag << " " << std::endl << "---SURFACE RESTRICTIONS-- PERMIT TBL976 I/D O/D LOC1    LOC2    "
       << std::endl << "   FARE BREAK SURFACE.:     ";
  if (fareBreakPermitted)
  {
    diag << std::setw(4) << std::left << "Y";
  }
  else
  {
    diag << std::setw(4) << std::left << "N";
  }

  if (trInfo.fareBreakSurfaceTblItemNo())
  {
    diag << std::setw(7) << std::right << trInfo.fareBreakSurfaceTblItemNo();

    std::vector<SurfaceTransfersInfo*> fareBreakSTI =
        trx.dataHandle().getSurfaceTransfers(trInfo.vendor(), trInfo.fareBreakSurfaceTblItemNo());
    if (!fareBreakSTI.empty())
    {
      std::vector<SurfaceTransfersInfo*>::const_iterator stiIter = fareBreakSTI.begin();
      std::vector<SurfaceTransfersInfo*>::const_iterator stiIterEnd = fareBreakSTI.end();

      bool firstTime = true;

      for (; stiIter != stiIterEnd; ++stiIter)
      {
        const SurfaceTransfersInfo* sti = *stiIter;

        if (firstTime)
        {
          firstTime = false;
        }
        else
        {
          diag << std::setw(39) << std::left << " ";
        }

        diag << std::setw(2) << std::left << " " << std::setw(4) << sti->restriction()
             << std::setw(3) << sti->originDest() << std::setw(2)
             << sti->fareBrkEmbeddedLoc().locType() << std::setw(6)
             << sti->fareBrkEmbeddedLoc().loc() << std::setw(2) << sti->surfaceLoc().locType()
             << std::setw(6) << sti->surfaceLoc().loc() << std::endl;
      }
    }
    else
    {
      diag << std::endl;
    }
  }
  else
  {
    diag << std::endl;
  }

  diag << "   EMBEDDED SURFACE...:     ";
  if (maxNumEmbedded == SURFACE_UNLIMITED)
    diag << std::setw(4) << std::left << "Y";
  else if (maxNumEmbedded > 0)
    diag << std::setw(4) << maxNumEmbedded;
  else
    diag << std::setw(4) << std::left << "N";

  if (trInfo.embeddedSurfaceTblItemNo())
  {
    diag << std::setw(7) << std::right << trInfo.embeddedSurfaceTblItemNo();

    std::vector<SurfaceTransfersInfo*> embeddedSTI =
        trx.dataHandle().getSurfaceTransfers(trInfo.vendor(), trInfo.embeddedSurfaceTblItemNo());
    if (!embeddedSTI.empty())
    {
      std::vector<SurfaceTransfersInfo*>::const_iterator stiIter = embeddedSTI.begin();
      std::vector<SurfaceTransfersInfo*>::const_iterator stiIterEnd = embeddedSTI.end();

      bool firstTime = true;

      for (; stiIter != stiIterEnd; ++stiIter)
      {
        const SurfaceTransfersInfo* sti = *stiIter;

        if (firstTime)
        {
          firstTime = false;
        }
        else
        {
          diag << std::setw(39) << std::left << " ";
        }

        diag << std::setw(2) << std::left << " " << std::setw(4) << sti->restriction()
             << std::setw(3) << sti->originDest() << std::setw(2)
             << sti->fareBrkEmbeddedLoc().locType() << std::setw(6)
             << sti->fareBrkEmbeddedLoc().loc() << std::setw(2) << sti->surfaceLoc().locType()
             << std::setw(6) << sti->surfaceLoc().loc() << std::endl;
      }
    }
    else
    {
      diag << std::endl;
    }
  }
  else
  {
    diag << std::endl;
  }

  diag << " " << std::endl << "CHARGES PAX:" << std::setw(2) << std::left
       << trInfo.transfersChargeAppl() << "  " << std::setw(3) << std::left << trInfo.cur1()
       << "  ";

  if (trInfo.charge1Cur1Amt() != 0)
  {
    diag << std::setw(4) << std::right << trInfo.charge1Cur1Amt();
  }
  else
  {
    diag << std::setw(4) << std::left << " ";
  }

  if (!trInfo.maxNoTransfersCharge1().empty())
  {
    diag << " *" << std::setw(2) << std::left << trInfo.maxNoTransfersCharge1();
  }
  else
  {
    diag << std::setw(4) << std::left << " ";
  }
  diag << "  ";

  if (trInfo.charge2Cur1Amt() != 0)
  {
    diag << std::setw(4) << std::right << trInfo.charge2Cur1Amt();
  }
  else
  {
    diag << std::setw(4) << std::left << " ";
  }

  if (!trInfo.maxNoTransfersCharge2().empty())
  {
    diag << " *" << std::setw(2) << std::left << trInfo.maxNoTransfersCharge2();
  }
  else
  {
    diag << std::setw(4) << std::left << " ";
  }

  diag << std::endl << std::setw(14) << std::left << " "
       << "  " << std::setw(3) << std::left << trInfo.cur2() << "  ";

  if (trInfo.charge1Cur2Amt() != 0)
  {
    diag << std::setw(4) << std::right << trInfo.charge1Cur2Amt();
  }
  else
  {
    diag << std::setw(4) << std::left << " ";
  }

  if (!trInfo.maxNoTransfersCharge1().empty())
  {
    diag << " *" << std::setw(2) << std::left << trInfo.maxNoTransfersCharge1();
  }
  else
  {
    diag << std::setw(4) << std::left << " ";
  }
  diag << "  ";

  if (trInfo.charge2Cur2Amt() != 0)
  {
    diag << std::setw(4) << std::right << trInfo.charge2Cur2Amt();
  }
  else
  {
    diag << std::setw(4) << std::left << " ";
  }

  if (!trInfo.maxNoTransfersCharge2().empty())
  {
    diag << " *" << std::setw(2) << std::left << trInfo.maxNoTransfersCharge2();
  }
  else
  {
    diag << std::setw(4) << std::left << " ";
  }

  diag << std::endl;

  diag << " " << std::endl << "---TRANSFER RULE SEGMENT DATA---" << std::endl
       << "   ---TRANS--- -GEOGRAPHIC RESTRICTIONS- -CARRIER------ -OTHER--" << std::endl
       << "   NO TY SC AP LOC     ZONETBL TSI BW GW AP IN    OUT   RS IO CH" << std::endl;

  std::vector<TransfersInfoSeg1*>::const_iterator iter = trInfo.segs().begin();
  std::vector<TransfersInfoSeg1*>::const_iterator iterEnd = trInfo.segs().end();

  for (; iter != iterEnd; ++iter)
  {
    TransfersInfoSeg1* trInfoSeg = *iter;

    std::vector<std::string> transTypes;

    diag << std::setw(2) << std::right << trInfoSeg->orderNo() << ":" << std::setw(2) << std::right
         << trInfoSeg->noTransfersPermitted() << " ";

    if (trInfoSeg->primeOnline() == Transfers1::ALLOW_PRIME_PRIME)
    {
      transTypes.push_back("PP");
    }
    if (trInfoSeg->sameOnline() == Transfers1::ALLOW_SAME_SAME)
    {
      transTypes.push_back("SS");
    }
    if (trInfoSeg->primeInterline() == Transfers1::ALLOW_PRIME_OTHER)
    {
      transTypes.push_back("PO");
    }
    if (trInfoSeg->otherInterline() == Transfers1::ALLOW_OTHER_OTHER)
    {
      transTypes.push_back("OO");
    }

    std::vector<std::string>::iterator typeIter = transTypes.begin();
    std::vector<std::string>::iterator typeIterEnd = transTypes.end();

    if (typeIter != typeIterEnd)
    {
      diag << std::setw(2) << std::left << (*typeIter);
      ++typeIter;
    }
    else
    {
      diag << std::setw(2) << std::left << " ";
    }

    diag << " " << std::setw(2) << std::left << trInfoSeg->stopoverConnectInd() << " "
         << std::setw(2) << std::left << trInfoSeg->transferAppl() << " ";

    if (trInfoSeg->loc1().locType() == LOCTYPE_NONE)
    {
      diag << std::setw(7) << " ";
    }
    else
    {
      diag << std::setw(2) << std::left << trInfoSeg->loc1().locType() << std::setw(5) << std::left
           << trInfoSeg->loc1().loc();
    }

    diag << " ";
    if (trInfoSeg->zoneTblItemNo())
    {
      diag << std::setw(7) << std::setfill('0') << std::right << trInfoSeg->zoneTblItemNo();
      diag << std::setfill(' ');
    }
    else
    {
      diag << std::setw(7) << std::left << " ";
    }
    if (trInfoSeg->tsi())
    {
      diag << " " << std::setw(3) << std::right << trInfoSeg->tsi();
    }
    else
    {
      diag << " " << std::setw(3) << std::left << " ";
    }

    diag << " " << std::setw(3) << std::left << trInfoSeg->betweenAppl() << std::setw(3)
         << std::left << trInfoSeg->gateway() << std::setw(3) << std::left
         << trInfoSeg->carrierAppl();

    if (!trInfoSeg->carrierIn().empty())
    {
      diag << std::setw(6) << std::left << trInfoSeg->carrierIn();
    }
    else if (trInfoSeg->inCarrierApplTblItemNo())
    {
      diag << std::setw(6) << std::left << trInfoSeg->inCarrierApplTblItemNo();
    }
    else
    {
      diag << std::setw(6) << std::left << " ";
    }

    if (!trInfoSeg->carrierOut().empty())
    {
      diag << std::setw(6) << std::left << trInfoSeg->carrierOut();
    }
    else if (trInfoSeg->outCarrierApplTblItemNo())
    {
      diag << std::setw(6) << std::left << trInfoSeg->outCarrierApplTblItemNo();
    }
    else
    {
      diag << std::setw(6) << std::left << " ";
    }

    diag << std::setw(3) << std::left << trInfoSeg->restriction() << std::setw(3) << std::left
         << trInfoSeg->outInPortion() << std::setw(2) << std::right << trInfoSeg->chargeAppl()
         << std::endl;

    std::ostringstream diagRow2;

    diagRow2 << std::setw(6) << std::left << " ";

    if (typeIter != typeIterEnd)
    {
      diagRow2 << std::setw(3) << std::left << (*typeIter);
      ++typeIter;
    }
    else
    {
      diagRow2 << std::setw(3) << std::left << " ";
    }
    diagRow2 << std::setw(6) << std::left << " ";

    if (trInfoSeg->loc2().locType() == LOCTYPE_NONE)
    {
      diagRow2 << std::setw(7) << " ";
    }
    else
    {
      diagRow2 << std::setw(2) << std::left << trInfoSeg->loc2().locType() << std::setw(5)
               << std::left << trInfoSeg->loc2().loc();
    }
    diagRow2 << std::setw(22) << " ";
    if ((!trInfoSeg->carrierIn().empty()) && (trInfoSeg->inCarrierApplTblItemNo()))
    {
      diagRow2 << std::setw(6) << std::left << trInfoSeg->inCarrierApplTblItemNo();
    }
    else
    {
      diagRow2 << std::setw(6) << " ";
    }
    if ((!trInfoSeg->carrierOut().empty()) && (trInfoSeg->outCarrierApplTblItemNo()))
    {
      diagRow2 << std::setw(6) << std::left << trInfoSeg->outCarrierApplTblItemNo();
    }
    else
    {
      diagRow2 << std::setw(6) << " ";
    }

    std::string diagRow2StrTemp = diagRow2.str();
    std::string diagRow2Str;

    std::remove_copy_if(diagRow2StrTemp.begin(),
                        diagRow2StrTemp.end(),
                        std::back_inserter(diagRow2Str),
                        std::bind2nd(std::equal_to<char>(), ' '));

    if (!diagRow2Str.empty())
    {
      diag << diagRow2.str() << std::endl;
    }

    for (; typeIter != typeIterEnd; ++typeIter)
    {
      diag << std::setw(6) << std::left << " " << std::setw(3) << std::left << (*typeIter)
           << std::endl;
    }
  }

  diag << " " << std::endl;
}

void
Transfers1::printDiagTsmDetail(DiagCollector& diag,
                               const TransfersInfo1& trInfo,
                               const TravelSegMarkup& tsm,
                               const TravelSeg* ts) const
{
  if (tsm.stopType() == Transfers1::STOPOVER)
  {
    diag << "SO ";
  }
  else if (tsm.stopType() == Transfers1::CONNECTION)
  {
    diag << "CX ";
  }
  else
  {
    diag << "   ";
  }

  if (tsm.isGateway())
  {
    diag << "GW ";
  }
  else
  {
    diag << "   ";
  }

  switch (tsm.transferType())
  {
  case Transfers1::PRIMARY_PRIMARY:
    diag << "TY:P/P ";
    break;
  case Transfers1::SAME_SAME:
    diag << "TY:S/S ";
    break;
  case Transfers1::PRIMARY_OTHER:
    diag << "TY:P/O ";
    break;
  case Transfers1::OTHER_OTHER:
    diag << "TY:O/O ";
    break;
  case Transfers1::TFR_NONE:
    diag << "       ";
    break;
  default:
    diag << "       ";
    break;
  }

  switch (tsm.transferTypeMatchResult())
  {
  case Transfers1::NOT_CHECKED:
    diag << " --------- ";
    break;
  case Transfers1::MATCH:
    diag << " MATCH     ";
    break;
  case Transfers1::NOT_MATCH:
    diag << " NOT MATCH ";
    break;
  case Transfers1::DOES_NOT_APPLY:
    diag << " N/A       ";
    break;
  default:
    diag << " ERROR     ";
    break;
  }

  switch (tsm.transferCheckResult())
  {
  case TravelSegMarkup::NOT_CHECKED:
    diag << "---- ";
    break;
  case TravelSegMarkup::PASS:
    diag << "PASS ";
    break;
  case TravelSegMarkup::FAIL:
    diag << "FAIL " << tsm.failReason();
    break;
  default:
    diag << "ERR  ";
    break;
  }

  diag << std::endl;

  TiSegMatchList tsmList = tsm.tiSegMatch();

  if (!tsmList.empty())
  {
    TiSegMatchListCI tsmIter = tsmList.begin();
    TiSegMatchListCI tsmIterEnd = tsmList.end();

    for (; tsmIter != tsmIterEnd; ++tsmIter)
    {
      const TransferInfoSegMatch& tism = *tsmIter;

      const TransfersInfoSeg1* tiSeg = tism.trInfoSeg();
      if (!tiSeg)
      {
        diag << "**ERROR TR-INFO-SEG IS NULL**" << std::endl;
        continue;
      }

      diag << "   " << std::setw(2) << std::right << tiSeg->orderNo() << ":";

      diag << "TY:" << getMatchResultText(tism.transferTypeMatchResult())
           << "SC:" << getMatchResultText(tism.stopConnMatchResult())
           << "GW:" << getMatchResultText(tism.gatewayMatchResult())
           << "RS:" << getMatchResultText(tism.restrictionMatchResult())
           << "GEO:" << getMatchResultText(tism.geoMatchResult());

      diag << "CXR:";
      if (tism.inCarrierMatchResult() == Transfers1::NOT_MATCH ||
          tism.outCarrierMatchResult() == Transfers1::NOT_MATCH)
      {
        diag << "NOT ";
      }
      else if (tism.inCarrierMatchResult() == Transfers1::MATCH ||
               tism.outCarrierMatchResult() == Transfers1::MATCH)
      {
        diag << "MAT ";
      }
      else if (tism.inCarrierMatchResult() == Transfers1::DOES_NOT_APPLY ||
               tism.outCarrierMatchResult() == Transfers1::DOES_NOT_APPLY)
      {
        diag << "N/A ";
      }
      else
      {
        diag << "--- ";
      }

      diag << "IO:";
      if (tism.inOutTransfersResult() == TransferInfoSegMatch::FAIL)
      {
        diag << "FAIL ";
      }
      else if (tism.inOutTransfersResult() == TransferInfoSegMatch::PASS)
      {
        diag << "PASS ";
      }
      else if (tism.inOutTransfersResult() == TransferInfoSegMatch::DOES_NOT_APPLY)
      {
        diag << "N/A  ";
      }
      else
      {
        diag << "---- ";
      }

      diag << std::endl;
    }
    diag << "    - SEGMENT CHECK RESULT: ";
    switch (tsm.trSegCheckResult())
    {
    case TravelSegMarkup::PASS:
      diag << "PASS";
      break;
    case TravelSegMarkup::FAIL:
      diag << "FAIL";
      break;
    case TravelSegMarkup::STOP:
      diag << "STOP";
      break;
    case TravelSegMarkup::NOT_CHECKED:
      diag << "NOT CHECKED";
      break;
    case TravelSegMarkup::DOES_NOT_APPLY:
      diag << "N/A";
      break;
    case TravelSegMarkup::NEED_REVALIDATION:
      diag << "NEED REVALIDATION";
      break;
    case TravelSegMarkup::NOT_ACCEPTED:
      diag << "GEO NOT MATCH";
      break;
    default:
      break;
    }

    diag << std::endl << " " << std::endl;
  }
}

void
Transfers1::printDiagTravelSeg(DiagCollector& diag,
                               const TravelSeg* ts,
                               const int16_t segmentNumber) const
{
  if (ts)
  {
    const AirSeg* as = dynamic_cast<const AirSeg*>(ts);

    if (segmentNumber)
      diag << std::setw(3) << std::right << segmentNumber << " ";
    else
      diag << std::setw(4) << std::right << " ";

    if (as)
    {
      diag << std::setw(3) << std::left << as->carrier() << std::setw(4) << std::right
           << as->flightNumber() << std::left << " " << ts->departureDT().dateToString(DDMMM, "")
           << " " << ts->origin()->loc() << ts->destination()->loc() << " "
           << ts->departureDT().timeToString(HHMM, "") << " "
           << ts->arrivalDT().timeToString(HHMM, "") << " ";
    }
    else
    {
      diag << std::setw(14) << std::left << "ARNK";
      if (ts->origin() && ts->destination())
      {
        diag << ts->origin()->loc() << ts->destination()->loc();
      }
    }
  }
}

void
Transfers1::printSurfaceValidationResult(const std::string& message,
                                         const TravelSeg* ts,
                                         std::string& out) const
{
  std::ostringstream oss;
  oss << message;
  if (ts)
    oss << ts->origin()->loc() << "-" << ts->destination()->loc();
  out = oss.str();
}

std::string
Transfers1::getMatchResultText(const Transfers1::MatchResult& matchResult) const
{
  switch (matchResult)
  {
  case Transfers1::MATCH:
    return "MAT ";
    break;
  case Transfers1::NOT_MATCH:
    return "NOT ";
    break;
  case Transfers1::NOT_CHECKED:
    return "--- ";
    break;
  case Transfers1::DOES_NOT_APPLY:
    return "N/A ";
    break;
  default:
    return "--- ";
    break;
  }
  return "--- ";
}

bool
Transfers1::checkIOIndTransferInfoSegs(const TransfersInfoWrapper& trInfoWrapper,
                                       const FareUsage& fu,
                                       bool samePU)
{
  const TransfersInfo1* trInfo = trInfoWrapper.trInfo();

  const std::vector<TransfersInfoSeg1*>& trInfoSegs = trInfo->segs();

  if (!trInfoSegs.empty())
  {
    std::vector<TransfersInfoSeg1*>::const_iterator iter = trInfoSegs.begin();
    std::vector<TransfersInfoSeg1*>::const_iterator iterEnd = trInfoSegs.end();

    for (; iter != iterEnd; ++iter)
    {
      Indicator inOutInd = (*iter)->outInPortion();
      if (inOutInd != Transfers1::SEG_INOUT_BLANK && inOutInd != Transfers1::SEG_INOUT_EITHER)
      {
        trInfoWrapper.recurringFCscope() = true;
        return false;
      }
      else if (samePU && (fu.travelSeg().size() == 1))
      {
        return false;
      }
    }
  }
  return true;
}

bool
Transfers1::isFCvsPU(const TransfersInfo1& trInfo)
{
  if (!trInfo.noTransfersMax().empty() &&
      trInfo.noTransfersMax() != Transfers1::NUM_TRANSFERS_UNLIMITED && trInfo.segs().empty())
  {
    return true;
  }
  return false;
}

bool
Transfers1::isRecurringSegForPU(const TransfersInfo1& trInfo, const TransfersInfoSeg1& seg)
{
  if (!trInfo.noTransfersMax().empty() && (seg.outInPortion() == Transfers1::SEG_INOUT_BLANK ||
                                           seg.outInPortion() == Transfers1::SEG_INOUT_EITHER))
  {
    return true;
  }
  return false;
}
Record3ReturnTypes
Transfers1::applySystemDefaultAssumption(const Itin& itin, const FareMarket& fareMarket)
{
  if (!fareMarket.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))
    return tse::SKIP;

  if (UNLIKELY(!fareMarket.travelSeg().front()->isAir() || !fareMarket.travelSeg().back()->isAir()))
    return tse::FAIL;

  return tse::SKIP;
}

bool
Transfers1::ignoreRecurringSegForRtw(const TransfersInfoSeg1& seg) const
{
  return seg.outInPortion() != Transfers1::SEG_INOUT_BLANK;
}

bool
Transfers1::isNumTransfersSpecified(const std::string& str) const
{
  return !str.empty();
}
} // namespace tse
