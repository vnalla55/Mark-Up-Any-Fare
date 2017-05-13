//-------------------------------------------------------------------
//
//  File:        Stopovers.cpp
//  Created:     July 6, 2004
//  Authors:     Andrew Ahmad
//
//  Description:
//
//
//  Copyright Sabre 2004
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

#include "Rules/Stopovers.h"

#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/RtwUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/SamePoint.h"
#include "DBAccess/StopoversInfo.h"
#include "DBAccess/StopoversInfoSeg.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/PricingUnitLogic.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/StopoversInfoWrapper.h"
#include "Rules/StopoversTravelSegWrapper.h"
#include "Rules/TransfersStopoversUtil.h"
#include "Rules/TSIGateway.h"
#include "Util/BranchPrediction.h"

#include <algorithm>

namespace tse
{
FIXEDFALLBACK_DECL(APO29538_StopoverMinStay);
FALLBACK_DECL(fallbackAPO44549SkipMaxTimecheckAtPULevel);
FALLBACK_DECL(fallbackAPO45157GatewayReqCheck);
FALLBACK_DECL(fallbackapo44172Cat8EmbeddedGtwyChk);

Logger
Stopovers::_logger("atseintl.Rules.Stopovers");
const std::string Stopovers::NUM_STOPS_UNLIMITED = "XX";

// the below method is a fix for APO-33047 bug. When a stopover is a connection
// for a travel segment, the travel segment in stopoverstravelsegWrapper is
// cleared of stopover surcharges.
void
Stopovers::clearSurchargesForCXSegments(const StopoversInfoWrapper* soInfoWrapper,
                                        const StopoversInfo* stopInfo,
                                        TravelSegMarkupContainer& tsmCon,
                                        const FareUsage* fu) const
{
  StopoversTravelSegWrapperMapCI stswIterEnd = soInfoWrapper->stopoversTravelSegWrappers().end();
  //iterate thru all the entries the tsmCon container identifying CX segments

  for (TravelSegMarkup* tsm : tsmCon)
  {
    // if fu is null, then it is at fc level, proceed with clearing connection segments.
    // if at pu level then clear for passed fu tvl segments only
    if (fu && fu != tsm->fareUsage() )
      continue;

    if (tsm->stopType() == Stopovers::CONNECTION)
    {
      TravelSeg*  ts = tsm->travelSeg();
      LOG4CXX_DEBUG(_logger,
                 "CONNECTION:"<< ts->origin()->loc()<< ts->destination()->loc() << " "<<
                 "Segment number:"<< tsm->segmentNumber() <<
                 "Item number:"<< stopInfo->itemNo() );

      StopoversTravelSegWrapperMapCI stswIter =
               soInfoWrapper->stopoversTravelSegWrappers().find(ts);
      if (UNLIKELY(stswIter != stswIterEnd))
      {
        const StopoversTravelSegWrapper& stsw = (*stswIter).second;
        if (stsw.isStopover() )
        {
          const_cast<StopoversTravelSegWrapper&> (stsw).passedValidation() = true;
          const_cast<StopoversTravelSegWrapper&> (stsw).isStopover() = false;
          const_cast<StopoversTravelSegWrapper&> (stsw).ruleItemMatch() = stopInfo->itemNo();
          const_cast<StopoversTravelSegWrapper&> (stsw).clearSurcharges();
        }
      }
    } // stoptype == connection
  }  //iten end
}

void
Stopovers::StopoversInfoSegMarkup::initialize(const StopoversInfoSeg* soInfoSeg)
{
  if (LIKELY(soInfoSeg))
  {
    _soInfoSeg = soInfoSeg;

    Indicator inOutInd = _soInfoSeg->stopoverInOutInd();
    if (inOutInd != Stopovers::SEG_INOUT_BLANK)
    {
      _checkInOut = true;

      if (inOutInd == Stopovers::SEG_INOUT_OUT)
      {
        _inOutOut = true;
      }
      else if (inOutInd == Stopovers::SEG_INOUT_IN)
      {
        _inOutIn = true;
      }
      else if (LIKELY(inOutInd == Stopovers::SEG_INOUT_EITHER))
      {
        _inOutEither = true;
      }
    }

    std::string noStopovers = soInfoSeg->noStops();

    if (noStopovers.empty() || (noStopovers == Stopovers::NUM_STOPS_UNLIMITED))
    {
      _numStopoversUnlimited = true;
    }
    else
    {
      _numStopovers = atoi(noStopovers.c_str());
    }
  }
}

bool
Stopovers::isGeoSpecified(const StopoversInfoSeg* soInfoSeg)
{
  if (LIKELY(soInfoSeg != nullptr))
    return ((soInfoSeg->loc1().locType() != LOCTYPE_NONE) ||
            (soInfoSeg->loc2().locType() != LOCTYPE_NONE));

  return false;
}

Record3ReturnTypes
Stopovers::validate(PricingTrx& trx,
                    const RuleItemInfo* rule,
                    const FarePath& farePath,
                    const PricingUnit& pricingUnit,
                    const FareUsage& fareUsage)
{
  DiagManager diag(trx, Diagnostic308);


  bool diagEnabled = diag.isActive() &&
                     diag.collector().parseFareMarket(trx, *fareUsage.paxTypeFare()->fareMarket());
  DiagCollector* diagPtr = nullptr;

  if (UNLIKELY(diagEnabled))
    diagPtr = &diag.collector();

  const StopoversInfoWrapper* soInfoWrapper = nullptr;
  const StopoversInfo* soInfo = nullptr;
  if (UNLIKELY(!locateStopoversInfo(rule, soInfoWrapper, soInfo) || !validateStopoversInfo(*soInfo)))
  {
    if (UNLIKELY(diagEnabled))
    {
      if (soInfo)
        printDiagHeader(*diagPtr, *soInfoWrapper, nullptr);
      *diagPtr << "INVALID STOPOVER INFO\n";
    }
    return tse::FAIL;
  }

  if (UNLIKELY(diagEnabled))
    printDiagHeader(*diagPtr, *soInfoWrapper, &pricingUnit);

  Record3ReturnTypes ret = tse::SKIP;

  if (UNLIKELY(!checkPreconditions(*soInfo, ret, diagPtr)))
    return ret;

  if (UNLIKELY(!checkTripType(trx, *soInfo, pricingUnit, farePath)))
  {
    ret = tse::SKIP;
    if (UNLIKELY(diagEnabled))
      (*diagPtr) << "STOPOVERS : SKIP - TRIP TYPE" << std::endl;

    return ret;
  }

  TravelSegMarkupContainer tsmProcOrder; // Processing order (current FU first)
  TravelSegMarkupContainer tsmPUOrder;

  std::string failReason;
  ProcessingResult res = Stopovers::SKIP;

  if (LIKELY(initializeTravelSegMarkupContainer(
          trx, tsmProcOrder, tsmPUOrder, pricingUnit, fareUsage, *soInfoWrapper, farePath.itin())))
  {
    res = processStopovers(trx,
                           tsmProcOrder,
                           tsmPUOrder,
                           *soInfoWrapper,
                           &farePath,
                           &pricingUnit,
                           nullptr,
                           failReason,
                           fareUsage.paxTypeFare()->vendor(),
                           &fareUsage);
  }
  else
  {
    failReason = "INTERNAL PROCESSING ERROR";
  }

  if (res == Stopovers::FAIL)
    ret = tse::FAIL;
  else if (UNLIKELY(res == Stopovers::PASS))
    ret = tse::PASS;
  else if (LIKELY(res == Stopovers::CONTINUE))
    ret = tse::PASS;
  else if (res == Stopovers::SKIP)
    ret = tse::SKIP;
  else if (res == Stopovers::STOP)
    ret = tse::FAIL;

  if (ret == tse::PASS)
  {
    clearSurchargesForCXSegments(soInfoWrapper,
                                 soInfo,
                                 tsmPUOrder,&fareUsage);

    processSurcharges(trx, tsmPUOrder, *soInfoWrapper, *soInfo, fareUsage, farePath);

    if (!fallback::fixed::APO29538_StopoverMinStay())
    {
      saveStopoverMinTime(tsmProcOrder, *soInfo);
    }
  }

  if (UNLIKELY(diagEnabled))
  {
    printDiagnostics(*diagPtr, *soInfo, tsmPUOrder, pricingUnit);

    (*diagPtr) << "STOPOVERS : ";

    if (ret == tse::PASS)
      (*diagPtr) << "PASS - NOT FINAL VALIDATION";
    else if (ret == tse::SKIP)
      (*diagPtr) << "SKIP";
    else if (ret == tse::FAIL)
      (*diagPtr) << "FAIL - " << failReason;
    else if (ret == tse::STOP)
      (*diagPtr) << "STOP - " << failReason;

    (*diagPtr) << std::endl;
  }

  return ret;
}

Record3ReturnTypes
Stopovers::validate(PricingTrx& trx,
                    Itin& itin,
                    const PaxTypeFare& fare,
                    const RuleItemInfo* rule,
                    const FareMarket& fareMarket)
{
  return validate(trx, itin, fare, rule, fareMarket, false);
}

Record3ReturnTypes
Stopovers::validate(PricingTrx& trx,
                    Itin& itin,
                    const PaxTypeFare& fare,
                    const RuleItemInfo* rule,
                    const FareMarket& fareMarket,
                    bool performFullFmVal)
{
  DiagManager diag(trx, Diagnostic308);

  bool diagEnabled = diag.isActive() && diag.collector().parseFareMarket(trx, fareMarket);
  DiagCollector* diagPtr = nullptr;

  if (UNLIKELY(diagEnabled))
    diagPtr = &diag.collector();

  const StopoversInfoWrapper* soInfoWrapper = nullptr;
  const StopoversInfo* soInfo = nullptr;
  if (UNLIKELY(!locateStopoversInfo(rule, soInfoWrapper, soInfo) || !validateStopoversInfo(*soInfo)))
  {
    if (UNLIKELY(diagEnabled))
    {
      if (soInfo)
        printDiagHeader(*diagPtr, *soInfoWrapper, nullptr);
      *diagPtr << "INVALID STOPOVER INFO\n";
    }
    return tse::FAIL;
  }

  if (UNLIKELY(diagEnabled))
    printDiagHeader(*diagPtr, *soInfoWrapper, nullptr);

  Record3ReturnTypes ret = tse::SKIP;

  if (!checkPreconditions(*soInfo, ret, diagPtr))
    return ret;

  TravelSegMarkupContainer tsmCon;
  std::string failReason;
  ProcessingResult res = Stopovers::SKIP;

  if (LIKELY(initializeTravelSegMarkupContainer(trx, tsmCon, fareMarket)))
    res = processStopovers(
        trx, tsmCon, tsmCon, *soInfoWrapper, nullptr, nullptr, &fareMarket, failReason, fare.vendor());
  else
    failReason = "INTERNAL PROCESSING ERROR";
  //apo-33047:if stopover is a connection, clear surchages in that ts
  if (res)
     clearSurchargesForCXSegments(soInfoWrapper, soInfo, tsmCon, nullptr);
  // NOTE: We must still process the stopover rule above in order
  //        to calculate the maximum number of stopovers permitted.
  //
  int16_t numStopsMax = 0;
  bool numStopsUnlimited = false;

  if (soInfo->noStopsMax().empty() || (soInfo->noStopsMax() == Stopovers::NUM_STOPS_UNLIMITED))
    numStopsUnlimited = true;
  else
    numStopsMax = atoi(soInfo->noStopsMax().c_str());

  if ((res == Stopovers::PASS) || (res == Stopovers::CONTINUE))
  {
    if (numStopsUnlimited)
      soInfoWrapper->setMaxStopoversPermittedUnlimited();
    else
      soInfoWrapper->maxStopoversPermitted() += numStopsMax;
  }

  if ((fare.owrt() != ONE_WAY_MAY_BE_DOUBLED) && (fare.owrt() != ONE_WAY_MAYNOT_BE_DOUBLED) &&
      !performFullFmVal)
  {
    // If the processing result was FAIL and the rule permits no stops
    //  then we allow the fare to fail even if it is a roundtrip fare.
    //
    if (!((res == Stopovers::FAIL) && (!numStopsUnlimited) && (numStopsMax == 0)))
    {
      //apo-44549: if max time was exceeded at stopover, fail the rule for all fares at fc level.
      if (!fallback::fallbackAPO44549SkipMaxTimecheckAtPULevel(&trx))
      {
         // res == stopsovers fail for max stop time exceeded, retain res value as fail
         if ( (!(res == Stopovers::FAIL)) || (!(failReason == "MAX STOP TIME EXCEEDED")) )
         {
            ret = tse::SOFTPASS;
            if (UNLIKELY(diagEnabled))
               (*diagPtr) << "STOPOVERS : SOFTPASS - NOT ONEWAY FARE."
                   << " NEED REVALIDATION" << std::endl;
            soInfoWrapper->ignoreNoMatch() = true;
            return ret;
         }
      }
      else // fallback ; remove at fallback removal time
      {
        ret = tse::SOFTPASS;
        if (UNLIKELY(diagEnabled))
          (*diagPtr) << "STOPOVERS : SOFTPASS - NOT ONEWAY FARE."
                   << " NEED REVALIDATION" << std::endl;
        soInfoWrapper->ignoreNoMatch() = true;
        return ret;
      }
    }
  }

  if (res == Stopovers::FAIL)
    ret = tse::FAIL;
  else if (res == Stopovers::PASS)
    ret = tse::PASS;
  else if (LIKELY(res == Stopovers::CONTINUE))
    ret = tse::PASS;
  else if (res == Stopovers::SKIP)
    ret = tse::SKIP;
  else if (res == Stopovers::STOP)
    ret = tse::FAIL;

  // Stopovers can only SOFTPASS for Fare Validator phase.
  if (ret == tse::PASS)
    ret = tse::SOFTPASS;

  if (UNLIKELY(diagEnabled))
  {
    printDiagnostics(*diagPtr, *soInfo, tsmCon, fareMarket);

    (*diagPtr) << "STOPOVERS : ";

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
  }

  return ret;
}

Record3ReturnTypes
Stopovers::applySystemDefaultAssumption(PricingTrx& trx,
                                        const Itin& itin,
                                        const PaxTypeFare& fare,
                                        const FareMarket& fareMarket)
{
  Stopovers so;
  TravelSegMarkupContainer tsmCon;

  if (LIKELY(so.initializeTravelSegMarkupContainer(trx,tsmCon, fareMarket)))
  {
    // Create a StopoversInfo (cat 8 record 3) and initialize it with all
    //  empty values.
    // This will result in the default 4/24 for domestic/international.
    // We have to initialize every member because the default constructor
    //  does not do that for us.
    StopoversInfo soInfo;

    soInfo.vendor() = "ATP";
    soInfo.itemNo() = 1;
    soInfo.unavailTag() = ' ';
    soInfo.geoTblItemNoBtw() = 0;
    soInfo.geoTblItemNoAnd() = 0;
    soInfo.geoTblItemNoGateway() = 0;
    soInfo.gtwyInd() = ' ';
    soInfo.samePointsTblItemNo() = 0;
    soInfo.samePntStops() = 0;
    soInfo.samePntTransit() = 0;
    soInfo.samePntConnections() = 0;
    soInfo.noStopsMin() = "";
    soInfo.noStopsMax() = "00";
    soInfo.noStopsOutbound() = "";
    soInfo.noStopsInbound() = "";
    soInfo.minStayTime() = 0;
    soInfo.maxStayTime() = 0;
    soInfo.minStayTimeUnit() = ' ';
    soInfo.maxStayTimeUnit() = ' ';
    soInfo.outOrReturnInd() = ' ';
    soInfo.sameCarrierInd() = ' ';
    soInfo.ojStopoverInd() = ' ';
    soInfo.ct2StopoverInd() = ' ';
    soInfo.ct2PlusStopoverInd() = ' ';
    soInfo.charge1FirstAmt() = 0;
    soInfo.charge1AddAmt() = 0;
    soInfo.charge1NoDec() = 0;
    soInfo.charge1Appl() = ' ';
    soInfo.charge1Total() = ' ';
    soInfo.charge1First() = "";
    soInfo.charge1AddNo() = "";
    soInfo.charge1Cur() = "";
    soInfo.charge2FirstAmt() = 0;
    soInfo.charge2AddAmt() = 0;
    soInfo.charge2NoDec() = 0;
    soInfo.charge2Appl() = ' ';
    soInfo.charge2Total() = ' ';
    soInfo.charge2First() = "";
    soInfo.charge2AddNo() = "";
    soInfo.charge2Cur() = "";

    so.identifyStopovers(tsmCon, soInfo, trx);

    TravelSegMarkupI iter = tsmCon.begin();
    TravelSegMarkupI iterEnd = tsmCon.end();

    for (; iter != iterEnd; ++iter)
    {
      TravelSegMarkup* tsm = (*iter);

      if (tsm->stopType() == Stopovers::STOPOVER)
      {
        return tse::FAIL;
      }
    }
  }
  return tse::SKIP;
}

bool
Stopovers::validateStopoversInfo(const StopoversInfo& soInfo)
{
  Indicator minStayTimeUnit = soInfo.minStayTimeUnit();
  Indicator maxStayTimeUnit = soInfo.maxStayTimeUnit();

  if (UNLIKELY(minStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_BLANK &&
      minStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_MINUTES &&
      minStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_HOURS &&
      minStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_DAYS &&
      minStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_MONTHS))
  {
    LOG4CXX_ERROR(_logger,
                  "Rules.Stopovers::validateStopoversInfo(): Unexpected value of minStayTimeUnit");
    return false;
  }

  if (UNLIKELY(maxStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_BLANK &&
      maxStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_MINUTES &&
      maxStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_HOURS &&
      maxStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_DAYS &&
      maxStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_MONTHS))
  {
    LOG4CXX_ERROR(_logger,
                  "Rules.Stopovers::validateStopoversInfo(): Unexpected value of maxStayTimeUnit");
    return false;
  }

  if (UNLIKELY(!validateStopoversInfo_Mandate2010(soInfo)))
    return false;

  chkApplScope(soInfo);

  return true;
}

bool
Stopovers::initializeTravelSegMarkupContainer(PricingTrx& trx,
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
    //apo28460..do not consider non embeded arunk segs in cat8 processing
    if ( ( (!as) &&
           ( (ts == fm.travelSeg().back()) ||
             (ts == fm.travelSeg().front()) ) ) )
     continue;

    TravelSegMarkup* tsm = nullptr;
    _dataHandle.get(tsm);

    if (UNLIKELY(!tsm))
    {
      LOG4CXX_ERROR(_logger,
                    "Rules.Stopovers::initializeTravelSegMarkupContainer(): "
                        << "DataHandle::get() returned 0");
      return false;
    }

    // lint --e{413}
    if (as)
      tsm->carrier() = as->carrier();

    tsm->travelSeg() = ts;
    tsm->segmentNumber() = segmentNumber;
    tsm->geoTravelType() = fm.geoTravelType();
    tsm->fareMarket() = &fm;

    if (fm.direction() == FMDirection::INBOUND)
      tsm->direction() = TravelSegMarkup::INBOUND;
    else if (fm.direction() == FMDirection::OUTBOUND)
      tsm->direction() = TravelSegMarkup::OUTBOUND;
    else
      tsm->direction() = TravelSegMarkup::UNKNOWN;

    if (!tsmCon.empty() && as)
    {
      setNextTravelSeg(tsmCon, ts);
      tsmCon.back()->carrierOut() = as->carrier();
    }

    if (!as)
    {
      tsm->isSurfaceSector() = true;
      if (!tsmCon.empty())
      {
        if (ts == fm.travelSeg().back())
        {
          TravelSegMarkupRI it = tsmCon.rbegin();
          ++it;
          if (it != tsmCon.rend())
          {
            (*it)->altOffPointGeo() = ts->origin();
          }
        }
        else
        {
          tsmCon.back()->altOffPointGeo() = ts->destination();
        }
        tsmCon.back()->arunkTravelSeg() = ts;
      }
    }

    tsmCon.push_back(tsm);
  }
  return true;
}

bool
Stopovers::initializeTravelSegMarkupContainer(PricingTrx& trx,
                                              TravelSegMarkupContainer& tsmProcOrder,
                                              TravelSegMarkupContainer& tsmPUOrder,
                                              const PricingUnit& pu,
                                              const FareUsage& fareUsage,
                                              const StopoversInfoWrapper& soInfoWrapper,
                                              const Itin* itin)
{
  std::vector<FareUsage*>::const_iterator fuI = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();

  if ( _applScope == FC_SCOPE )
  {
    fuI = std::find(pu.fareUsage().begin(), pu.fareUsage().end(), &fareUsage);
    if (LIKELY(fuI != pu.fareUsage().end()))
      fuEnd = fuI + 1;
  }

  bool isMirrorImage = true;
  bool isHalfRoundTrip = (pu.puType() != PricingUnit::Type::ONEWAY);

  for (; fuI != fuEnd; ++fuI)
  {
    FareUsage* fu = *fuI;

    if (!isHalfRoundTrip)
    {
      isMirrorImage = false;
    }
    else if (UNLIKELY( (fu != &fareUsage) && !isPuScope() ))
    {
      isMirrorImage = false;
    }

    bool swapFareUsageOrder = false;
    if (fu->isInbound() && (fu == &fareUsage) && (!isMirrorImage))
    {
      swapFareUsageOrder = true;
    }

    bool notValidateEntireRule =
        soInfoWrapper.applyLeastRestrictiveProvision() && (fu != &fareUsage); //
    if (UNLIKELY(!initializeTravelSegMarkupContainerSingleFU(trx,
                                                    tsmProcOrder,
                                                    tsmPUOrder,
                                                    pu,
                                                    fu,
                                                    soInfoWrapper,
                                                    itin,
                                                    notValidateEntireRule,
                                                    swapFareUsageOrder)))
    {
      return false;
    }
  }
  return true;
}

bool
Stopovers::initializeTravelSegMarkupContainerSingleFU(PricingTrx& trx,
                                                      TravelSegMarkupContainer& tsmProcOrder,
                                                      TravelSegMarkupContainer& tsmPUOrder,
                                                      const PricingUnit& pu,
                                                      FareUsage* fu,
                                                      const StopoversInfoWrapper& soInfoWrapper,
                                                      const Itin* itin,
                                                      bool notValidateEntireRule,
                                                      bool swapFareUsageOrder)
{
  int16_t segmentNumber = 1;
  TravelSegMarkupContainer tempContainer;

  for (std::vector<TravelSeg*>::const_iterator tsI = fu->travelSeg().begin();
       tsI != fu->travelSeg().end();
       ++tsI)
  {
    TravelSeg* ts = *tsI;
    AirSeg* as = dynamic_cast<AirSeg*>(ts);

    //apo28460..do not consider non embedded arunk segs in cat8 processing
    if ( ( (!as)  &&
         ( (ts == fu->travelSeg().back()) ||
            (ts == fu->travelSeg().front()) ) ) )
     continue;

    if (LIKELY(itin))
      segmentNumber = itin->segmentOrder(ts);
    else
      ++segmentNumber;

    TravelSegMarkup* tsm = nullptr;
    _dataHandle.get(tsm);

    if (UNLIKELY(!tsm))
    {
      LOG4CXX_ERROR(_logger,
                    "Rules.Stopovers::initializeTravelSegMarkupContainerSingleFU(): "
                        << "DataHandle::get() returned 0");
      return false;
    }

    // lint --e{413}
    if (as)
      tsm->carrier() = as->carrier();

    tsm->travelSeg() = ts;
    tsm->segmentNumber() = segmentNumber;
    tsm->geoTravelType() = pu.geoTravelType();
    tsm->fareUsage() = fu;

    if (LIKELY(fu->paxTypeFare()))
      tsm->fareMarket() = fu->paxTypeFare()->fareMarket();

    if (notValidateEntireRule)
      tsm->validateEntireRule() = false;

    tsm->direction() = (fu->isInbound() ? TravelSegMarkup::INBOUND : TravelSegMarkup::OUTBOUND);

    if (fu->isInbound() && !swapFareUsageOrder)
    {
      if (!tempContainer.empty() && !as)
        tempContainer.front()->arunkTravelSeg() = ts;
      tempContainer.push_front(tsm);
    }
    else
    {
      if (!tempContainer.empty() && !as)
        tempContainer.back()->arunkTravelSeg() = ts;
      tempContainer.push_back(tsm);
    }

    if (!tsmPUOrder.empty() && as)
    {
      setNextTravelSeg(tsmPUOrder, ts);
      tsmPUOrder.back()->carrierOut() = as->carrier();
    }

    if (!as)
    {
      tsm->isSurfaceSector() = true;
      if (!tsmPUOrder.empty())
      {
        if (ts == fu->travelSeg().back())
        {
          TravelSegMarkupRI it = tsmPUOrder.rbegin();
          ++it;
          if (it != tsmPUOrder.rend() && !(*it)->altOffPointGeo())
          {
            (*it)->altOffPointGeo() = ts->origin();
          }
        }
        else if (!tsmPUOrder.back()->altOffPointGeo())
        {
          tsmPUOrder.back()->altOffPointGeo() = ts->destination();
        }
        tsmPUOrder.back()->arunkTravelSeg() = ts;
      }
    }
    tsmPUOrder.push_back(tsm);
  }

  for (const auto& tsMarkup : tempContainer)
  {
    if (fu->isInbound() && swapFareUsageOrder)
      tsmProcOrder.push_front(tsMarkup);
    else
      tsmProcOrder.push_back(tsMarkup);
  }

  return true;
}

bool
Stopovers::ignoreRecurringSegForRtw(const StopoversInfoSeg& seg) const
{
  return seg.stopoverInOutInd() != Stopovers::SEG_INOUT_BLANK;
}

void
Stopovers::initializeStopoversInfoSegMarkup(DataHandle& localDataHandle,
                                            std::vector<StopoversInfoSegMarkup*>& soInfoSegMarkup,
                                            const std::vector<StopoversInfoSeg*>& segs,
                                            bool& allSegsGeoApplOK,
                                            bool& allSegsGeoApplNot,
                                            bool& geoRestrictionsExist)
{
  allSegsGeoApplOK = true;
  allSegsGeoApplNot = true;
  geoRestrictionsExist = false;

  std::vector<StopoversInfoSeg*>::const_iterator iter = segs.begin();
  std::vector<StopoversInfoSeg*>::const_iterator iterEnd = segs.end();

  for (; iter != iterEnd; ++iter)
  {
    StopoversInfoSeg* soInfoSeg = *iter;

    if (UNLIKELY( (_applScope == FC_SCOPE) && (soInfoSeg->stopoverInOutInd() == SEG_INOUT_EITHER) ))
    {
      continue; // skip
    }

    if (UNLIKELY(isRtw() && ignoreRecurringSegForRtw(*soInfoSeg)))
      continue;

    StopoversInfoSegMarkup* soISM = nullptr;
    localDataHandle.get(soISM);

    if (LIKELY(soISM))
    {
      soISM->initialize(soInfoSeg);
      soInfoSegMarkup.push_back(soISM);
    }

    if (soInfoSeg->stopoverGeoAppl() != Stopovers::SEG_GEO_APPL_NOT_PERMITTED)
    {
      allSegsGeoApplNot = false;
    }
    else
    {
      allSegsGeoApplOK = false;
    }

    if (soInfoSeg->loc1().locType() != LOCTYPE_NONE || soInfoSeg->loc2().locType() != LOCTYPE_NONE)
    {
      geoRestrictionsExist = true;
    }
  }
}

Stopovers::ProcessingResult
Stopovers::processStopovers(PricingTrx& trx,
                            TravelSegMarkupContainer& tsmProcOrder,
                            TravelSegMarkupContainer& tsmPUOrder,
                            const StopoversInfoWrapper& soInfoWrapper,
                            const FarePath* fp,
                            const PricingUnit* pu,
                            const FareMarket* fm,
                            std::string& failReason,
                            const VendorCode& vendorOfFare,
                            const FareUsage* fUsage)
{
  const StopoversInfo* soInfo = soInfoWrapper.soInfo();

  identifyStopovers(tsmProcOrder, *soInfo, trx);

  identifyGateways(trx, tsmPUOrder, *soInfo, pu, fm);

  identifyGatewayGeoTableMatches(trx, tsmPUOrder, *soInfo, fp, pu, fm, vendorOfFare);

  ProcessingResult res;

  res = processSamePointRestrictions(trx, fm, tsmProcOrder, *soInfo, failReason);

  if (UNLIKELY((res == Stopovers::FAIL) || (res == Stopovers::SKIP)))
  {
    return res;
  }

  if (!soInfo->segs().empty())
  {
    res = processStopoverInfoSegs(trx, fm, vendorOfFare, tsmProcOrder, soInfoWrapper, failReason);

    if ((res == Stopovers::FAIL) || (res == Stopovers::SKIP))
    {
      return res;
    }
  }
  else
  {
    res = processGatewayRestrictions(trx, tsmProcOrder, *soInfo, failReason);

    if (UNLIKELY((res == Stopovers::FAIL) || (res == Stopovers::SKIP)))
    {
      return res;
    }
  }

  //APO-44549: max stop is to be validated at fc level only.
  if (!fallback::fallbackAPO44549SkipMaxTimecheckAtPULevel(&trx)) 
  {
     if (fm && !pu)
     {
        res = checkMaxStopTime(trx, tsmProcOrder, *soInfo, failReason);
        if ((res == Stopovers::FAIL) || (res == Stopovers::SKIP))
           return res;
     }
     // ssdsp-1938 at pu level, validate max time for the passed in fusage only
     if (pu && fUsage)
     {
        res = checkMaxStopTime(trx, tsmProcOrder, *soInfo, failReason, fUsage);
        if ((res == Stopovers::FAIL) || (res == Stopovers::SKIP))
           return res;
     }
  }
  else //fallback; remove at fallback removal time
  {
     res = checkMaxStopTime(trx, tsmProcOrder, *soInfo, failReason);
     if ((res == Stopovers::FAIL) || (res == Stopovers::SKIP))
        return res;
  }
  res = checkMinStops(trx, tsmProcOrder, *soInfo, failReason);

  if (UNLIKELY((res == Stopovers::FAIL) || (res == Stopovers::SKIP)))
  {
    return res;
  }

  res = checkMaxStopsInOut(trx, fp, tsmProcOrder, soInfoWrapper, failReason);

  if (res == Stopovers::FAIL)
  {
    return res;
  }
  else if (res == Stopovers::SKIP)
  {
    // SKIP returned from checkMaxStopsInOut is only due to UNKNOWN direction.
    bool maySoftPass = ((pu == nullptr) && (fm != nullptr) && // Fare Market Scope
                        (fm->direction() == FMDirection::UNKNOWN));

    return (maySoftPass ? Stopovers::PASS : Stopovers::SKIP);
  }

  return Stopovers::CONTINUE;
}

Stopovers::ProcessingResult
Stopovers::processSurcharges(PricingTrx& trx,
                             TravelSegMarkupContainer& tsmCon,
                             const StopoversInfoWrapper& soInfoWrapper,
                             const StopoversInfo& soInfo,
                             const FareUsage& fareUsage,
                             const FarePath& farePath)
{
  TravelSegMarkupI iter = tsmCon.begin();
  TravelSegMarkupI iterEnd = tsmCon.end();

  bool processOtherFU = (_applScope == PU_SCOPE);

  for (; iter != iterEnd; ++iter)
  {
    TravelSegMarkup* tsm = (*iter);
    if (tsm->isSurfaceSector())
      continue; // no surcharges for surface sectors

    TravelSeg* ts = tsm->travelSeg();
    FareUsage* fu = tsm->fareUsage();

    /*
     * @todo It is not efficient to call map thrice when this
     * can be done once
     */
    if ((soInfoWrapper.checkIsStopover(ts)) && (soInfoWrapper.checkPassedValidation(ts)) &&
        (soInfoWrapper.getRuleItemMatch(ts) == soInfo.itemNo()))
    {
      // Only process surcharges for the current FareUsage
      //

      if (LIKELY(processOtherFU || fu == &fareUsage))
      {
        if ((tsm->siSegMatch().empty()) || (!tsm->useSegmentsToApplyCharges()))
        {
          if (UNLIKELY(!soInfoWrapper.addSurcharge(
                  trx, ts, fu, farePath, nullptr, false, false, true, false, fu == &fareUsage)))
          {
            return Stopovers::FAIL;
          }
        }
        else
        {
          bool segSpecific = tsm->isChargeSegmentSpecific();

          SiSegMatchListCI siSegIter = tsm->siSegMatch().begin();
          SiSegMatchListCI siSegIterEnd = tsm->siSegMatch().end();

          for (; siSegIter != siSegIterEnd; ++siSegIter)
          {
            const StopoverInfoSegMatch& siSeg = *siSegIter;

            if (siSeg.segCheckResult() == StopoverInfoSegMatch::PASS)
            {
              if (siSeg.chargeType() == StopoverInfoSegMatch::CHARGE1)
              {
                const bool forceFirstAmt = soInfo.charge1First().empty() &&
                                           isGeoWithinSameLoc(siSeg.soInfoSeg());
                if (!soInfoWrapper.addSurcharge(trx,
                                                ts,
                                                fu,
                                                farePath,
                                                siSeg.soInfoSeg(),
                                                segSpecific,
                                                false,
                                                false,
                                                forceFirstAmt,
                                                fu == &fareUsage))
                {
                  return Stopovers::FAIL;
                }
              }
              else if (siSeg.chargeType() == StopoverInfoSegMatch::CHARGE2)
              {
                if (!soInfoWrapper.addSurcharge(trx,
                                                ts,
                                                fu,
                                                farePath,
                                                siSeg.soInfoSeg(),
                                                segSpecific,
                                                true,
                                                false,
                                                false,
                                                fu == &fareUsage))
                {
                  return Stopovers::FAIL;
                }
              }
            }
          }
        }
      }
    }
  }
  return Stopovers::CONTINUE;
}

void
Stopovers::saveStopoverMinTime(TravelSegMarkupContainer& tsmCon, const StopoversInfo& soInfo)
{
  for (TravelSegMarkup* tsm : tsmCon)
  {
    FareUsage* fareUsage = tsm->fareUsage();

    if (LIKELY(fareUsage)) // && (soInfo.minStayTimeUnit() != RuleConst::STOPOVER_TIME_UNIT_BLANK))
    {
      fareUsage->stopoverMinTime().set(soInfo.minStayTime(), soInfo.minStayTimeUnit());
    }
  }
}

void
Stopovers::identifyStopovers(TravelSegMarkupContainer& tsmCon, const StopoversInfo& soInfo,
                             PricingTrx& trx)
{
  TravelSegMarkupI iter = tsmCon.begin();
  TravelSegMarkupI iterEnd = tsmCon.end();

  for (; iter != iterEnd; ++iter)
  {
    TravelSegMarkup* tsm = (*iter);
    FareUsage* fu = tsm->fareUsage();

    if (fu)
    {
      // Don't process the final air segment of each fare component.
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
            setStopType(*tsm, soInfo, trx);
          }
          break;
        }
      }
    }
    else
    {
      setStopType(*tsm, soInfo,trx);
    }
  }
}

void
Stopovers::identifyGateways(PricingTrx& trx,
                            TravelSegMarkupContainer& tsmCon,
                            const StopoversInfo& soInfo,
                            const PricingUnit* pu,
                            const FareMarket* fm)
{
  if (soInfo.gtwyInd() == Stopovers::GTWY_BLANK)
  {
    return;
  }

  if (pu)
  {
    std::vector<FareUsage*>::const_iterator fuI = pu->fareUsage().begin();
    std::vector<FareUsage*>::const_iterator fuIEnd = pu->fareUsage().end();

    for (; fuI != fuIEnd; fuI++)
    {
      markGateways(trx, tsmCon, *((*fuI)->paxTypeFare()->fareMarket()));
    }
  }
  else
  {
    markGateways(trx, tsmCon, *fm);
  }
}

void
Stopovers::identifyGatewayGeoTableMatches(PricingTrx& trx,
                                          TravelSegMarkupContainer& tsmCon,
                                          const StopoversInfo& soInfo,
                                          const FarePath* fp,
                                          const PricingUnit* pu,
                                          const FareMarket* fm,
                                          const VendorCode& vendorOfFare)
{
  if (!soInfo.geoTblItemNoGateway())
  {
    return;
  }

  typedef std::map<const TravelSeg*, const RuleUtil::TravelSegWrapper*> GeoTravelSegMap;

  GeoTravelSegMap gtwyTravSegMap;

  RuleUtil::TravelSegWrapperVector travelSegVec;

  bool origCheck = true;
  bool destCheck = true;
  bool fltStopCheck = false;
  TSICode tsiCode;
  LocKey locKey1;
  LocKey locKey2;

  RuleConst::TSIScopeParamType defaultScope = RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY;

  if (fm && (!pu))
  {
    defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
  }

  if (!RuleUtil::validateGeoRuleItem(soInfo.geoTblItemNoGateway(),
                                     vendorOfFare,
                                     defaultScope,
                                     false,
                                     false,
                                     false,
                                     trx,
                                     fp,
                                     nullptr,
                                     pu,
                                     fm,
                                     trx.getRequest()->ticketingDT(),
                                     travelSegVec,
                                     origCheck,
                                     destCheck,
                                     fltStopCheck,
                                     tsiCode,
                                     locKey1,
                                     locKey2,
                                     Diagnostic308))
  {
    return;
  }

  RuleUtil::TravelSegWrapperVectorCI tsIter = travelSegVec.begin();
  RuleUtil::TravelSegWrapperVectorCI tsIterEnd = travelSegVec.end();

  for (; tsIter != tsIterEnd; ++tsIter)
  {
    GeoTravelSegMap::value_type value((*tsIter)->travelSeg(), *tsIter);

    gtwyTravSegMap.insert(value);
  }

  TravelSegMarkupI iter = tsmCon.begin();
  TravelSegMarkupI iterEnd = tsmCon.end();

  for (; iter != iterEnd; ++iter)
  {
    TravelSegMarkup* tsm = (*iter);
    bool destMatch = false;
    bool nextOrigMatch = false;

    GeoTravelSegMap::const_iterator geoIter = gtwyTravSegMap.find(tsm->travelSeg());

    if (geoIter != gtwyTravSegMap.end())
    {
      destMatch = (*geoIter).second->destMatch();
    }

    if (tsm->nextTravelSeg())
    {
      GeoTravelSegMap::const_iterator geoIterNext = gtwyTravSegMap.find(tsm->nextTravelSeg());

      if (geoIterNext != gtwyTravSegMap.end())
      {
        nextOrigMatch = (*geoIterNext).second->origMatch();
      }
    }

    if (destMatch || nextOrigMatch)
    {
      tsm->gatewayGeoTableMatchResult() = Stopovers::MATCH;
    }
    else
    {
      tsm->gatewayGeoTableMatchResult() = Stopovers::NOT_MATCH;
    }
  }
}

void
Stopovers::setStopType(TravelSegMarkup& tsm, const StopoversInfo& soInfo,
                       PricingTrx& trx )
{
  const TravelSeg* travelSeg = tsm.travelSeg();
  const TravelSeg* nextTravelSeg = tsm.nextTravelSeg();

  if (!nextTravelSeg)
  {
    tsm.stopType() = Stopovers::NONE;
    return;
  }

  StopType stopType = Stopovers::CONNECTION;

  if (fallback::fixed::APO29538_StopoverMinStay())
  {
    int16_t minStayTime = soInfo.minStayTime();
    Indicator minStayTimeUnit = soInfo.minStayTimeUnit();

    if (UNLIKELY(travelSeg->isForcedStopOver()))
    {
      stopType = Stopovers::STOPOVER;
    }
    else if (UNLIKELY(travelSeg->isForcedConx()))
    {
      stopType = Stopovers::CONNECTION;
    }
    else if ((travelSeg->segmentType() == tse::Open) || (nextTravelSeg->segmentType() == tse::Open))
    {
      DateTime arrivalDT = travelSeg->arrivalDT();
      DateTime nextDepartDT = nextTravelSeg->departureDT();

      if (travelSeg->isOpenWithoutDate() && nextTravelSeg->isOpenWithoutDate())
      {
        stopType = Stopovers::STOPOVER;
      }
      else if (travelSeg->isOpenWithoutDate() && !nextTravelSeg->isOpenWithoutDate())
      {
        stopType = Stopovers::STOPOVER;
      }
      else if (!travelSeg->isOpenWithoutDate() && nextTravelSeg->isOpenWithoutDate())
      {
        stopType = Stopovers::STOPOVER;
      }
      else if (arrivalDT.isValid() && nextDepartDT.isValid())
      {
        if ((arrivalDT.year() != nextDepartDT.year()) ||
            (arrivalDT.month() != nextDepartDT.month()) || (arrivalDT.day() != nextDepartDT.day()))
        {
          stopType = Stopovers::STOPOVER;
        }
      }
    }
    else
    {
      DateTime arriveDT = travelSeg->arrivalDT();
      DateTime departDT = nextTravelSeg->departureDT();

      int64_t stopTimeSeconds = DateTime::diffTime(departDT, arriveDT);

      if ((minStayTimeUnit == RuleConst::STOPOVER_TIME_UNIT_BLANK) ||
          ((minStayTime == 0) && (minStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_DAYS)))
      {
        if ((tsm.geoTravelType() == GeoTravelType::International) ||
            (tsm.geoTravelType() == GeoTravelType::ForeignDomestic))
        {
          if (stopTimeSeconds > RuleConst::STOPOVER_SEC_INTL)
          {
            stopType = Stopovers::STOPOVER;
          }
        }
        else
        {
          if (stopTimeSeconds > RuleConst::STOPOVER_SEC_DOMESTIC)
          {
            stopType = Stopovers::STOPOVER;
          }
        }
      }
      else if (minStayTimeUnit == RuleConst::STOPOVER_TIME_UNIT_MINUTES)
      {
        if (stopTimeSeconds >= (minStayTime * SECONDS_PER_MINUTE))
        {
          stopType = Stopovers::STOPOVER;
        }
      }
      else if (minStayTimeUnit == RuleConst::STOPOVER_TIME_UNIT_HOURS)
      {
        if (stopTimeSeconds > (minStayTime * SECONDS_PER_HOUR))
        {
          stopType = Stopovers::STOPOVER;
        }
      }
      else if (LIKELY(minStayTimeUnit == RuleConst::STOPOVER_TIME_UNIT_DAYS))
      {
        if (!isStayTimeWithinDays(arriveDT, departDT, minStayTime))
          stopType = Stopovers::STOPOVER;
      }
      else if (minStayTimeUnit == RuleConst::STOPOVER_TIME_UNIT_MONTHS)
      {
        if (departDT >= arriveDT.addMonths(minStayTime))
        {
          stopType = Stopovers::STOPOVER;
        }
      }

      else if (stopType != Stopovers::STOPOVER)
      {
        if (tsm.fareMarket() && std::find(tsm.fareMarket()->stopOverTravelSeg().begin(),
                                          tsm.fareMarket()->stopOverTravelSeg().end(),
                                          travelSeg) != tsm.fareMarket()->stopOverTravelSeg().end())
        {
          stopType = Stopovers::STOPOVER;
        }
      }
    }
  }
  else // fallback
  {
    TimeAndUnit minStopTime;

    if ( soInfo.minStayTimeUnit() != RuleConst::STOPOVER_TIME_UNIT_BLANK )
       minStopTime.set(soInfo.minStayTime(), soInfo.minStayTimeUnit());
    else
    {
     // ToDo refactor later, setStopOver and isStopOver. setStopOver should have minStopTime, maxStopTime(in minutes) passed in, so we do not go through time unit convertion again and again
     if ((tsm.geoTravelType() == GeoTravelType::International) || (tsm.geoTravelType() == GeoTravelType::ForeignDomestic))
     {
        if ( (soInfo.maxStayTimeUnit() == RuleConst::STOPOVER_TIME_UNIT_HOURS &&
              soInfo.maxStayTime() < 25) ||
             (soInfo.maxStayTimeUnit() == RuleConst::STOPOVER_TIME_UNIT_MINUTES &&
              soInfo.maxStayTime() < 1441) )
              minStopTime.set(soInfo.maxStayTime(), soInfo.maxStayTimeUnit());
      }
    }
    if (UNLIKELY(travelSeg->isForcedStopOver()))
    {
      stopType = Stopovers::STOPOVER;
    }
    else if (UNLIKELY(travelSeg->isForcedConx()))
    {
      stopType = Stopovers::CONNECTION;
    }
    else if (ItinUtil::isStopover(
                 *travelSeg, *nextTravelSeg, tsm.geoTravelType(), minStopTime, tsm.fareMarket()))
    {
      stopType = Stopovers::STOPOVER;
    }
  } // fallback

  tsm.stopType() = stopType;
}

// Remove with APO29538_StopoverMinStay
bool
Stopovers::isStayTimeWithinDays(const DateTime& arriveDT,
                                const DateTime& departDT,
                                const int16_t& stayDays)
{
  if (stayDays == 0) // Special: Same day
  {
    return (departDT.year() == arriveDT.year() && departDT.month() == arriveDT.month() &&
            departDT.day() == arriveDT.day());
  }
  else
  {
    DateTime arrive(arriveDT.year(), arriveDT.month(), arriveDT.day(), 23, 59, 59);

    DateTime depart(departDT.year(), departDT.month(), departDT.day());

    return (depart < arrive.addDays(stayDays));
  }
}

Stopovers::ProcessingResult
Stopovers::checkMaxStopTime(PricingTrx& trx,
                            TravelSegMarkupContainer& tsmCon,
                            const StopoversInfo& soInfo,
                            std::string& failReason,
                            const FareUsage*  fUsage)
{
  TravelSegMarkupI iter = tsmCon.begin();
  TravelSegMarkupI iterEnd = tsmCon.end();
  bool noSegmentData = soInfo.segs().empty();

  for (; iter != iterEnd; ++iter)
  {
    TravelSegMarkup* tsm = (*iter);
    if (tsm->isSurfaceSector())
      continue;

    //APO-44549: if fUsage is present, then we are at pu level validation.  
    //At pu level, check max time for fUsage tvl segs only.
    if (!fallback::fallbackAPO44549SkipMaxTimecheckAtPULevel(&trx) )
    {
       if (fUsage)
       {
          if (tsm->fareUsage() &&  (fUsage != tsm->fareUsage()) )
             continue;
       }
    }
    if ((tsm->stopType() == Stopovers::STOPOVER) &&
        (noSegmentData || (tsm->soSegCheckResult() ==
                           TravelSegMarkup::PASS))) // Only check so far matched travel segment
    {
      const TravelSeg* travelSeg = tsm->travelSeg();
      const TravelSeg* nextTravelSeg = tsm->nextTravelSeg();

      int16_t maxStayTime = soInfo.maxStayTime();
      Indicator maxStayTimeUnit = soInfo.maxStayTimeUnit();

      if ((maxStayTimeUnit == RuleConst::STOPOVER_TIME_UNIT_BLANK) ||
          ((maxStayTime == 0) && (maxStayTimeUnit != RuleConst::STOPOVER_TIME_UNIT_DAYS)))
      {
        return Stopovers::CONTINUE;
      }

      if (nextTravelSeg)
      {
        DateTime arriveDT = travelSeg->arrivalDT();
        DateTime departDT = nextTravelSeg->departureDT();

        int64_t stopTimeSeconds = DateTime::diffTime(departDT, arriveDT);

        if (maxStayTimeUnit == RuleConst::STOPOVER_TIME_UNIT_MINUTES)
        {
          if (stopTimeSeconds > (maxStayTime * SECONDS_PER_MINUTE))
          {
            failReason = "MAX STOP TIME EXCEEDED";
            tsm->stopoverCheckResult() = TravelSegMarkup::FAIL;
            tsm->failReason() = "MAX STOP TIME";
            return Stopovers::FAIL;
          }
        }
        else if (maxStayTimeUnit == RuleConst::STOPOVER_TIME_UNIT_HOURS)
        {
          if (stopTimeSeconds > (maxStayTime * SECONDS_PER_HOUR))
          {
            failReason = "MAX STOP TIME EXCEEDED";
            tsm->stopoverCheckResult() = TravelSegMarkup::FAIL;
            tsm->failReason() = "MAX STOP TIME";
            return Stopovers::FAIL;
          }
        }
        else if (maxStayTimeUnit == RuleConst::STOPOVER_TIME_UNIT_DAYS)
        {
          if (!ItinUtil::isStayTimeWithinDays(arriveDT, departDT, maxStayTime))
          {
            failReason = "MAX STOP TIME EXCEEDED";
            tsm->stopoverCheckResult() = TravelSegMarkup::FAIL;
            tsm->failReason() = "MAX STOP TIME";
            return Stopovers::FAIL;
          }
        }
        else if (maxStayTimeUnit == RuleConst::STOPOVER_TIME_UNIT_MONTHS)
        {
          if (departDT > arriveDT.addMonths(maxStayTime))
          {
            failReason = "MAX STOP TIME EXCEEDED";
            tsm->stopoverCheckResult() = TravelSegMarkup::FAIL;
            tsm->failReason() = "MAX STOP TIME";
            return Stopovers::FAIL;
          }
        }
      }
    }
  }
  return Stopovers::CONTINUE;
}

Stopovers::ProcessingResult
Stopovers::processSamePointRestrictions(PricingTrx& trx,
                                        const FareMarket* fm,
                                        const TravelSegMarkupContainer& tsmCon,
                                        const StopoversInfo& soInfo,
                                        std::string& failReason)
{
  typedef std::pair<int32_t, int32_t> NumStopoversConnections;

  const int32_t maxSamePointStops = soInfo.samePntStops();
  const int32_t maxSamePointConns = soInfo.samePntConnections();
  const int32_t maxSamePointTrans = soInfo.samePntTransit();

  if (maxSamePointStops == 0 && maxSamePointConns == 0 && maxSamePointTrans == 0)
    return Stopovers::CONTINUE;

  std::ostringstream failStr;
  std::map<LocCode, NumStopoversConnections> samePointMap;

  for (TravelSegMarkup* tsm : tsmCon)
  {
    if (!tsm->validateEntireRule() || tsm->stopType() == Stopovers::NONE)
      continue;

    LocCode city = TransStopUtil::getCity(trx, fm, *tsm->travelSeg()->destination());

    NumStopoversConnections& numStopsConns = samePointMap[city];

    if (tsm->stopType() == Stopovers::STOPOVER)
      ++numStopsConns.first;
    else if (tsm->stopType() == Stopovers::CONNECTION)
      ++numStopsConns.second;

    const int32_t numStops = numStopsConns.first;
    const int32_t numConns = numStopsConns.second;
    const int32_t numTrans = numStops + numConns;

    if (maxSamePointStops && numStops > maxSamePointStops)
      failStr << "TOO MANY STOPOVERS IN " << city << " : " << numStops;
    else if (maxSamePointConns && numConns > maxSamePointConns)
      failStr << "TOO MANY CONNECTIONS IN " << city << " : " << numConns;
    else if (maxSamePointTrans && numTrans > maxSamePointTrans)
      failStr << "TOO MANY TRANSFERS IN " << city << " : " << numTrans;

    if (!failStr.str().empty())
    {
      failReason = failStr.str();
      tsm->failReason() = failReason;
      tsm->stopoverCheckResult() = TravelSegMarkup::FAIL;
      return Stopovers::FAIL;
    }
  }

  return Stopovers::CONTINUE;
}

FMDirection
Stopovers::fareUsageDirToValidate(const StopoversInfo& soInfo)
{
  if (UNLIKELY(isRtw()))
    return FMDirection::UNKNOWN;

  const std::string& noStopsMax = soInfo.noStopsMax();
  const std::string& noStopsOutbound = soInfo.noStopsOutbound();
  const std::string& noStopsInbound = soInfo.noStopsInbound();

  if (UNLIKELY(!noStopsOutbound.empty() && noStopsInbound.empty())) // OUT specified, IN not specified
  {
    if (noStopsOutbound == noStopsMax) // OUT same as MAX
      return FMDirection::OUTBOUND; // apply only outbound
  }
  else if (UNLIKELY(noStopsOutbound.empty() && !noStopsInbound.empty())) // OUT not specified, IN specified
  {
    if (noStopsInbound == noStopsMax) // IN same as MAX
      return FMDirection::INBOUND; // apply only inbound
  }

  return FMDirection::UNKNOWN; // apply both outbound and inbound
}

bool
Stopovers::isFareUsageDirPass(const FMDirection& fuDirToValidate,
                              const TravelSegMarkup& tsm)
{
  if (UNLIKELY(((fuDirToValidate == FMDirection::OUTBOUND) &&
       (tsm.direction() == TravelSegMarkup::INBOUND)) ||
      ((fuDirToValidate == FMDirection::INBOUND) && (tsm.direction() == TravelSegMarkup::OUTBOUND))))
  {
    return false;
  }

  return true;
}

Stopovers::ProcessingResult
Stopovers::processStopoverInfoSegs(PricingTrx& trx,
                                   const FareMarket* fm,
                                   const VendorCode& vendorOfFare,
                                   TravelSegMarkupContainer& tsmCon,
                                   const StopoversInfoWrapper& soInfoWrapper,
                                   std::string& failReason)
{
  const StopoversInfo* soInfo = soInfoWrapper.soInfo();
  const std::vector<StopoversInfoSeg*>& soInfoSegs = soInfo->segs();

  if (UNLIKELY(soInfoSegs.empty()))
  {
    return Stopovers::CONTINUE;
  }


  int16_t numStopsMax = 0;
  bool numStopsMaxUnlimited = isNumStopsMaxUnlimited(*soInfo, numStopsMax);

  FMDirection fuDirToValidate = fareUsageDirToValidate(*soInfo);

  int16_t charge1FirstNo = 0;
  int16_t charge1AddNo = 0;
  bool charge1FirstUnlimited = isNumStopsUnlimited(soInfo->charge1First(), charge1FirstNo);
  bool charge1AddUnlimited = isNumStopsUnlimited(soInfo->charge1AddNo(), charge1AddNo);

  bool allSegsGeoApplNot = true;
  bool allSegsGeoApplOK = true;
  bool geoRestrictionsExist = false;

  DataHandle localDataHandle(trx.ticketingDate());
  std::vector<StopoversInfoSegMarkup*> soInfoSegMarkup;

  initializeStopoversInfoSegMarkup(localDataHandle,
                                   soInfoSegMarkup,
                                   soInfo->segs(),
                                   allSegsGeoApplOK,
                                   allSegsGeoApplNot,
                                   geoRestrictionsExist);

  const StopoversInfoSeg* lastSoInfoSeg = soInfoSegs.back();

  if (UNLIKELY(isRtw()))
  {
    if (soInfoSegMarkup.empty())
      return Stopovers::CONTINUE;
    lastSoInfoSeg = soInfoSegMarkup.back()->soInfoSeg();
  }

  for (TravelSegMarkup* tsm : tsmCon)
  {
    if (tsm->stopType() != Stopovers::STOPOVER && !tsm->isSurfaceSector())
      continue;

    if (UNLIKELY(!isFareUsageDirPass(fuDirToValidate, *tsm)))
    {
      // Main portion of MAX OUT IN does not match Fare Usage direction,
      // skip recurring segment data validation for segments within the fare usage
      tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
      tsm->stopoverCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
      continue;
    }

    // APO-45157:if gateway is required and the stopover is not a gateway, continue 
    // to the next segment
    if ( (!fallback::fallbackAPO45157GatewayReqCheck(&trx)) &&
         (soInfo->gtwyInd() == Stopovers::GTWY_REQUIRED)  && !tsm->isGateway())
    {
       tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
       tsm->stopoverCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
       continue;
    }
    bool processChargeCalcOnly = false;
    bool isTentativeMatch = false;
    uint32_t ruleItemMatch = soInfoWrapper.getRuleItemMatch(tsm->travelSeg(), isTentativeMatch);

    if (ruleItemMatch)
    {
      if ((ruleItemMatch != soInfo->itemNo()) && (!isTentativeMatch))
      {
        processChargeCalcOnly = true;
      }
    }

    std::vector<StopoversInfoSegMarkup*>::iterator iter = soInfoSegMarkup.begin();
    std::vector<StopoversInfoSegMarkup*>::iterator iterEnd = soInfoSegMarkup.end();
    RecurringSegContext recurringSegContext;

    int16_t segNumStopoversSum = 0;
    bool segNumStopoversSumUnlimited = false;
    bool failedOnWithinSameLocType = false;

    bool doNotAllowImplicitPass = false;
    for (; iter != iterEnd; ++iter)
    {
      StopoversInfoSegMarkup* soISM = *iter;
      const StopoversInfoSeg* soInfoSeg = soISM->soInfoSeg();

      Indicator geoAppl = soInfoSeg->stopoverGeoAppl();
      Indicator chargeInd = soInfoSeg->chargeInd();

      int16_t numStopovers = soISM->numStopovers();
      bool numStopoversUnlimited = soISM->numStopoversUnlimited();

      segNumStopoversSum += numStopovers;
      if (numStopoversUnlimited)
        segNumStopoversSumUnlimited = true;

      StopoverInfoSegMatch sism(soInfoSeg);
      tsm->siSegMatch().push_back(sism);

      StopoverInfoSegMatch& siSegMatch = tsm->siSegMatch().back();

      if (!recurringSegContext.processNextSegment(chargeInd))
      {
        tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
        siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;
        continue;
      }

      // In order for the stopover to be considered a match with this
      //  seg record, we must match on the inbound/outbound direction,
      //  geo loc, carrierIn, carrierOut and sameCarrier.
      // If all of these things match, then we have a match on the
      //  stopover.
      // Then we must check the stopoverGeoAppl indicator to determine
      //  if this match is permitted, not permitted or required.
      if (soISM->checkInOut() && !tsm->isSurfaceSector())
      {
        if (tsm->direction() == TravelSegMarkup::UNKNOWN)
        {
          tsm->stopoverCheckResult() = TravelSegMarkup::SOFTPASS;
          tsm->soSegCheckResult() = TravelSegMarkup::SOFTPASS;
          siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
          break;
        }
        if (soInfoWrapper.fareUsage() && geoAppl == Stopovers::SEG_GEO_APPL_NOT_PERMITTED)
        {
          if (((soISM->inOutOut() && soInfoWrapper.fareUsage()->isInbound())) ||
              ((soISM->inOutIn() && soInfoWrapper.fareUsage()->isOutbound())))
          {
            tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;
            continue;
          }
        }

        bool inOutMissMatch =
            ((soISM->inOutOut() && (tsm->direction() == TravelSegMarkup::INBOUND)) ||
             (soISM->inOutIn() && (tsm->direction() == TravelSegMarkup::OUTBOUND)));

        if (inOutMissMatch)
        {
          if (geoAppl == Stopovers::SEG_GEO_APPL_REQUIRED)
          {
            tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;
          }
          else
          {
            tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
          }
          continue;
        }

        if (geoAppl == Stopovers::SEG_GEO_APPL_BLANK)
        {
          doNotAllowImplicitPass = true;
        }
      }

      MatchResult geoMatch = Stopovers::NOT_MATCH;

      bool processNextInfoSeg = true;
      std::string shortFailReason;
      const StopoversInfoSeg* arunkGeoMatchedInfoSeg = nullptr;

      failedOnWithinSameLocType = false;

      if (!checkInfoSegGeo(trx,
                           fm,
                           vendorOfFare,
                           *tsm,
                           *soISM,
                           soInfoSegMarkup,
                           geoMatch,
                           processNextInfoSeg,
                           arunkGeoMatchedInfoSeg))
      {
        // apo31441: if i/o byte is "blank or E" in rec.segment, then this rs isapplicable at pu level.
        // if so is not allowed by this rec.seg then return fail 
        if ( (!isRtw()) && (_applScope == PU_SCOPE) &&
             ( (soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_BLANK) ||
               (soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_EITHER) ) )
        {
           // if tsm is a gateway then skip this check as gateway logic will take care of this 
           if ( (geoMatch == Stopovers::MATCH) && (!tsm->isGateway() ) &&
                (soInfoSeg->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_NOT_PERMITTED) &&
                (chargeInd == Stopovers::SEG_NO_CHARGE) && 
                (!soInfoWrapper.applyLeastRestrictiveProvision())  )
           {
              tsm->stopoverCheckResult() = TravelSegMarkup::FAIL;
              tsm->soSegCheckResult() = TravelSegMarkup::FAIL;
              siSegMatch.segCheckResult() = StopoverInfoSegMatch::FAIL;
              siSegMatch.geoMatchResult() = geoMatch;
              processNextInfoSeg = false;
              return Stopovers::FAIL;  //donot process any more recSegs
           }
        } //end apo31441


        failedOnWithinSameLocType = geoMatch == Stopovers::NOT_MATCH && _applScope == PU_SCOPE &&
                                    isGeoWithinSameLoc(soISM->soInfoSeg());

        if (!processNextInfoSeg)
        {
          tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
          siSegMatch.geoMatchResult() = geoMatch;
          break;
        }
        else if (processNextInfoSeg && (geoAppl == Stopovers::SEG_GEO_APPL_REQUIRED ||
                                        geoAppl == Stopovers::SEG_GEO_APPL_BLANK))
        {
          tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;
          siSegMatch.geoMatchResult() = geoMatch;
          continue;
        }
        else if (failedOnWithinSameLocType && processNextInfoSeg)
        {
          tsm->soSegCheckResult() = TravelSegMarkup::FAIL;
          siSegMatch.segCheckResult() = StopoverInfoSegMatch::FAIL;
          siSegMatch.geoMatchResult() = geoMatch;
          continue;
        }
      }

      if (tsm->isSurfaceSector())
      {
        if (geoMatch == Stopovers::MATCH)
        {
          LOG4CXX_DEBUG(_logger,
                        "ARUNK MATCH: " << tsm->travelSeg()->origin()->loc()
                                        << tsm->travelSeg()->destination()->loc() << " "
                                        << soInfo->itemNo());

          if (geoAppl != Stopovers::SEG_GEO_APPL_NOT_PERMITTED)
          {
            soInfoWrapper.setRuleItemMatch(
                tsm->travelSeg(), tsm->fareUsage(), soInfo->itemNo(), tsm->isTentativeMatch());
          }
          else
          {
            soInfoWrapper.setRuleItemMatch(tsm->travelSeg(),
                                           tsm->fareUsage(),
                                           soInfo->itemNo(),
                                           tsm->isTentativeMatch(),
                                           true);
          }
        }
        break;
      }

      bool arunkPassNextInfoSeg = false;
      if (tsm->altOffPointGeo() && geoMatch == Stopovers::MATCH &&
          soInfoSeg != arunkGeoMatchedInfoSeg)
      {
        if (arunkGeoMatchedInfoSeg != nullptr)
        {
          if ((soInfoSeg->chargeInd() == Stopovers::SEG_USE_CHARGE_2) &&
              (arunkGeoMatchedInfoSeg->chargeInd() == Stopovers::SEG_USE_CHARGE_2))
          {
            chargeInd = Stopovers::SEG_USE_CHARGE_2;
          }
          else
          {
            chargeInd = Stopovers::SEG_USE_CHARGE_1;
          }
        }
        else if (geoAppl != Stopovers::SEG_GEO_APPL_NOT_PERMITTED)
        {
          geoMatch = Stopovers::NOT_MATCH;
        }
      }
      if (UNLIKELY(tsm->altOffPointGeo() && soISM->matchCountCharge() &&
          (soISM->matchCountOutCharge() || soISM->matchCountInCharge())))
      {
        arunkPassNextInfoSeg = (arunkGeoMatchedInfoSeg != nullptr && soInfoSeg != arunkGeoMatchedInfoSeg);
      }
      arunkGeoMatchedInfoSeg = nullptr;

      siSegMatch.geoMatchResult() = geoMatch;

      MatchResult gtwyGeoMatch = tsm->gatewayGeoTableMatchResult();

      if (allSegsGeoApplOK)
      {
        if ((soInfo->gtwyInd() == Stopovers::GTWY_BLANK) ||
            (soInfo->gtwyInd() == Stopovers::GTWY_PERMITTED))
        {
          if (UNLIKELY((geoMatch == Stopovers::NOT_MATCH) &&
              ((soInfo->geoTblItemNoGateway() == 0) || (gtwyGeoMatch == Stopovers::NOT_MATCH))))
          {
            tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
            continue;
          }
        }
      }

      if (UNLIKELY(!processInfoSegCarrier(*tsm, siSegMatch, *soInfoSeg)))
        continue;

      if (UNLIKELY(soInfo->gtwyInd() == Stopovers::GTWY_NOT_PERMITTED))
      {
        // Stopovers are not permitted at the specified gateway(s).
        // The disallowed locations are specified by the gateway geo
        //  table.
        // If the geo table field is blank, then stopovers are not
        //  permitted at any gateway.
        if (tsm->isGateway())
        {
          if (soInfo->geoTblItemNoGateway() == 0)
          {
            tsm->stopoverCheckResult() = TravelSegMarkup::FAIL;
            tsm->soSegCheckResult() = TravelSegMarkup::FAIL;
            siSegMatch.segCheckResult() = StopoverInfoSegMatch::FAIL;

            failReason = "GATEWAY STOPOVER NOT PERMITTED";
            return Stopovers::FAIL;
          }
          else if (gtwyGeoMatch == Stopovers::MATCH)
          {
            tsm->stopoverCheckResult() = TravelSegMarkup::FAIL;
            tsm->soSegCheckResult() = TravelSegMarkup::FAIL;
            siSegMatch.segCheckResult() = StopoverInfoSegMatch::FAIL;

            failReason = "STOPOVER NOT PERMITTED AT SPECIFIC GATEWAY";
            return Stopovers::FAIL;
          }
          else
          {
            // The stopover is at a gateway, but does not match the
            //  geo table restriction. This is a no-match.
            tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
            break;
          }
        }
        else
        {
          // The stopover is not at a gateway so process the geo
          // location data in the recurring segment record as if
          // there is no gateway restriction.
          if ((geoAppl == Stopovers::SEG_GEO_APPL_NOT_PERMITTED) && (geoMatch == Stopovers::MATCH))
          {
            if (chargeInd == Stopovers::SEG_NO_CHARGE)
            {
              tsm->soSegCheckResult() = TravelSegMarkup::STOP;
              siSegMatch.segCheckResult() = StopoverInfoSegMatch::FAIL;
              break;
            }
            else
            {
              tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
              siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;
              recurringSegContext.setIgnoreChargeGroup(chargeInd);
              continue;
            }
          }
          else if ((geoAppl == Stopovers::SEG_GEO_APPL_BLANK) && (geoMatch == Stopovers::NOT_MATCH))
          {
            tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
            continue;
          }
        }
      }
      else if (soInfo->gtwyInd() == Stopovers::GTWY_REQUIRED)
      {
        // Stopovers must be at the specified gateway(s). The permitted
        //  gateway locations are specified by the gateway geo table and
        //  the geo data in the recurring segment records.
        bool savePrevSegMatch = false;
        if (!checkGTWYGeoMatchResult(
                *tsm, siSegMatch, gtwyGeoMatch, geoMatch, geoAppl, chargeInd, savePrevSegMatch))
        {
          if (tsm->soSegCheckResult() == TravelSegMarkup::STOP)
          {
            break;
          }
          else
          {
            if (savePrevSegMatch)
              recurringSegContext.setIgnoreChargeGroup(chargeInd);
            continue;
          }
        }
      }
      else
      {
        if ((geoAppl == Stopovers::SEG_GEO_APPL_NOT_PERMITTED) && (geoMatch == Stopovers::MATCH))
        {
          if (chargeInd == Stopovers::SEG_NO_CHARGE)
          {
            tsm->soSegCheckResult() = TravelSegMarkup::STOP;
            siSegMatch.segCheckResult() = StopoverInfoSegMatch::FAIL;
            break;
          }
          else
          {
            tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;
            recurringSegContext.setIgnoreChargeGroup(chargeInd);
            continue;
          }
        }
        else if (UNLIKELY((soInfoSeg != lastSoInfoSeg) &&
                 (geoAppl != Stopovers::SEG_GEO_APPL_NOT_PERMITTED) &&
                 (geoMatch == Stopovers::NOT_MATCH)))
        {
          tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;
          continue;
        }
      }

      // Check Dir OUT IN EITHER
      bool failed = false;
      if (UNLIKELY(!checkInfoSegDirectionality(
              *tsm, *soISM, processNextInfoSeg, failed, shortFailReason, failReason)))
      {
        if (failed)
        {
          tsm->stopoverCheckResult() = TravelSegMarkup::FAIL;
          tsm->soSegCheckResult() = TravelSegMarkup::FAIL;
          tsm->failReason() = shortFailReason;

          siSegMatch.segCheckResult() = StopoverInfoSegMatch::FAIL;

          return Stopovers::FAIL;
        }
        else if (processNextInfoSeg)
        {
          tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;
          continue;
        }
        else
        {
          tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
          siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
          break;
        }
      }

      // If we reach this point, then the travel seg is a match
      //  on all data and is considered a stopover point.
      if (LIKELY(!arunkPassNextInfoSeg))
      {
        if (!numStopoversUnlimited)
        {
          if (soISM->matchCount() == numStopovers)
          {
            if (chargeInd == Stopovers::SEG_NO_CHARGE)
            {
              tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
              siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
              break;
            }
            else
            {
              if (geoAppl == Stopovers::SEG_GEO_APPL_BLANK)
                recurringSegContext.setIgnoreChargeGroup(chargeInd);
              tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
              siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;
              continue;
            }
          }
        }
        else
        {
          if (UNLIKELY(!checkFirstAddNo(chargeInd,
                               soISM->matchCount(),
                               charge1FirstUnlimited,
                               charge1AddUnlimited,
                               charge1FirstNo,
                               charge1AddNo)))
          {
            tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;

            continue;
          }
        }
      }

      if (!arunkPassNextInfoSeg && !processChargeCalcOnly)
      {
        soISM->increaseMatchCount(tsm->direction() == TravelSegMarkup::INBOUND,
                                  tsm->direction() == TravelSegMarkup::OUTBOUND);
      }

      if (UNLIKELY((soInfoSeg == lastSoInfoSeg) && (geoAppl == Stopovers::SEG_GEO_APPL_BLANK) &&
          (geoMatch != Stopovers::MATCH)))
      {
        if (allSegsGeoApplOK)
        {
          if (chargeInd == Stopovers::SEG_USE_CHARGE_1)
          {
            if (numStopsMaxUnlimited)
            {
              siSegMatch.chargeType() = StopoverInfoSegMatch::CHARGE2;
            }
            else
            {
              tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
              break;
            }
          }
        }
        else if ((!allSegsGeoApplOK) && (!allSegsGeoApplNot))
        {
          if (chargeInd == Stopovers::SEG_USE_CHARGE_1)
          {
            if (numStopsMaxUnlimited)
            {
              siSegMatch.chargeType() = StopoverInfoSegMatch::CHARGE2;
            }
            else
            {
              tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
              break;
            }
          }
        }
        else
        {
          if (chargeInd == Stopovers::SEG_USE_CHARGE_1)
          {
            siSegMatch.chargeType() = StopoverInfoSegMatch::CHARGE1;
          }
          else if (chargeInd == Stopovers::SEG_USE_CHARGE_2)
          {
            siSegMatch.chargeType() = StopoverInfoSegMatch::CHARGE2;
          }
          else if (chargeInd == Stopovers::SEG_NO_CHARGE)
          {
            tsm->useSegmentsToApplyCharges() = false;
          }
        }
      }
      else if ((soInfoSeg != lastSoInfoSeg) && (geoAppl == Stopovers::SEG_GEO_APPL_NOT_PERMITTED) &&
               (geoMatch != Stopovers::MATCH) &&
               hasMoreSoInfoSegWithSameCharge(iter, iterEnd, chargeInd))
      {
        continue;
      }
      else
      {
        if (chargeInd == Stopovers::SEG_USE_CHARGE_1)
        {
          siSegMatch.chargeType() = StopoverInfoSegMatch::CHARGE1;
        }
        else if (chargeInd == Stopovers::SEG_USE_CHARGE_2)
        {
          siSegMatch.chargeType() = StopoverInfoSegMatch::CHARGE2;
        }
        else if (chargeInd == Stopovers::SEG_NO_CHARGE)
        {
          tsm->useSegmentsToApplyCharges() = false;
        }
      }


      siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
      tsm->soSegCheckResult() = TravelSegMarkup::PASS;
      tsm->isChargeSegmentSpecific() = isGeoSpecified(soInfoSeg);
      tsm->segInfoPass() = soInfoSeg;

      break;
    }

    if (geoRestrictionsExist)
    {
      if (allSegsGeoApplNot)
      {
        // If all soInfoSegs have GeoAppl = NOT_PERMITTED and none match
        //  the stopover, then the stopover is considered a match.

        if (checkImplicitInfoSegMatch(*tsm, allSegsGeoApplNot))
        {
          tsm->soSegCheckResult() = TravelSegMarkup::PASS;

          // New code need to be added to check all segments for the charge
          // specified or not. If no charge specified in all segs, then turn off
          // the useSegmentsToApplyCharges indicator in TravelSegMarkup
          SiSegMatchListCI siSegIter = tsm->siSegMatch().begin();
          SiSegMatchListCI siSegIterEnd = tsm->siSegMatch().end();

          bool ind = false;
          for (; siSegIter != siSegIterEnd; ++siSegIter)
          {
            const StopoverInfoSegMatch& siSeg = *siSegIter;

            if (siSeg.segCheckResult() == StopoverInfoSegMatch::PASS)
            {
              if (siSeg.chargeType() != StopoverInfoSegMatch::NOCHARGE)
              {
                ind = true;
                break;
              }
            }
          }
          if (ind)
            tsm->isChargeSegmentSpecific() = true;
          else
            tsm->useSegmentsToApplyCharges() = false;
        }
      }
      else
      {
        if ((tsm->soSegCheckResult() != TravelSegMarkup::PASS) && !failedOnWithinSameLocType &&
            ((numStopsMaxUnlimited && !segNumStopoversSumUnlimited) ||
             ((!numStopsMaxUnlimited && !segNumStopoversSumUnlimited &&
               segNumStopoversSum < numStopsMax))))
        {
          if (!doNotAllowImplicitPass && checkImplicitInfoSegMatch(*tsm, allSegsGeoApplNot))
          {
            if (!processChargeCalcOnly)
            {
              if (tsm->noMoreStopover() || !atLeastOneRecurringSegPass(*tsm))
                tsm->soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
              else
              {
                tsm->soSegCheckResult() = TravelSegMarkup::PASS;
                tsm->isTentativeMatch() = true;
              }
            }
          }
        }
      }
    }
  }

  if (UNLIKELY(!checkRequiredSegmentsSatisfied(soInfoWrapper, soInfoSegMarkup, failReason)))
    return FAIL;

  return Stopovers::CONTINUE;
}

bool
Stopovers::checkRequiredSegmentsSatisfied(const StopoversInfoWrapper& soWrapper,
                                          const std::vector<StopoversInfoSegMarkup*>& segs,
                                          std::string& failReason) const
{
  for (StopoversInfoSegMarkup* soISM : segs)
  {
    const StopoversInfoSeg& soInfoSeg = *soISM->soInfoSeg();

    if (LIKELY(soInfoSeg.stopoverGeoAppl() != SEG_GEO_APPL_REQUIRED))
      continue;
    if (soISM->numStopoversUnlimited())
      continue;

    const bool fcLevel = !soWrapper.fareUsage();
    const bool puScopeRecSeg =
        !soWrapper.soInfo()->noStopsMax().empty() && (!soISM->checkInOut() || soISM->inOutEither());

    if (fcLevel && puScopeRecSeg)
      continue;

    if (soISM->matchCount() != soISM->numStopovers())
    {
      std::ostringstream oss;
      oss << "STOPOVER REQUIRED BY RULE SEGMENT: " << soInfoSeg.orderNo();
      failReason = oss.str();
      return false;
    }
  }

  return true;
}

bool
Stopovers::hasMoreSoInfoSegWithSameCharge(
    const std::vector<StopoversInfoSegMarkup*>::iterator& curSeg,
    const std::vector<StopoversInfoSegMarkup*>::iterator& endSeg,
    const Indicator& curSegChargeInd)
{
  if (curSegChargeInd == Stopovers::SEG_NO_CHARGE)
    return true;

  std::vector<StopoversInfoSegMarkup*>::iterator iter = curSeg + 1;
  for (; iter != endSeg; ++iter)
  {
    StopoversInfoSegMarkup* soISM = *iter;
    const StopoversInfoSeg* soInfoSeg = soISM->soInfoSeg();
    if (soInfoSeg->chargeInd() == curSegChargeInd)
      return true;
  }

  return false;
}

bool
Stopovers::processInfoSegCarrier(TravelSegMarkup& tsm,
                                 StopoverInfoSegMatch& siSegMatch,
                                 const StopoversInfoSeg& soInfoSeg)
{
  siSegMatch.sameCarrierMatchResult() = checkInfoSegSameCarrier(tsm, soInfoSeg);
  if (UNLIKELY(siSegMatch.sameCarrierMatchResult() == Stopovers::NOT_MATCH))
  {
    tsm.soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
    siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
    return false;
  }

  siSegMatch.inCarrierMatchResult() = checkInfoSegCarrierIn(tsm, soInfoSeg);
  if (UNLIKELY(siSegMatch.inCarrierMatchResult() == Stopovers::NOT_MATCH))
  {
    tsm.soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
    siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
    return false;
  }

  siSegMatch.outCarrierMatchResult() = checkInfoSegCarrierOut(tsm, soInfoSeg);
  if (UNLIKELY(siSegMatch.outCarrierMatchResult() == Stopovers::NOT_MATCH))
  {
    tsm.soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
    siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
    return false;
  }

  return true;
}

Stopovers::MatchResult
Stopovers::checkInfoSegSameCarrier(TravelSegMarkup& tsm, const StopoversInfoSeg& soInfoSeg)
{
  Indicator carrierInd = soInfoSeg.carrierInd();
  if (LIKELY(carrierInd == Stopovers::SEG_SAME_CARRIER_BLANK))
  {
    return Stopovers::DOES_NOT_APPLY;
  }

  MatchResult carrierMatch = Stopovers::DOES_NOT_APPLY;

  if (tsm.carrier() == tsm.carrierOut())
  {
    carrierMatch = Stopovers::MATCH;
  }
  else
  {
    carrierMatch = Stopovers::NOT_MATCH;
  }

  if ((carrierInd == Stopovers::SEG_SAME_CARRIER_SAME) && (carrierMatch == Stopovers::MATCH))
  {
    return Stopovers::MATCH;
  }
  if ((carrierInd == Stopovers::SEG_SAME_CARRIER_NOT_SAME) &&
      (carrierMatch == Stopovers::NOT_MATCH))
  {
    return Stopovers::MATCH;
  }
  return Stopovers::NOT_MATCH;
}

Stopovers::MatchResult
Stopovers::checkInfoSegCarrierIn(TravelSegMarkup& tsm, const StopoversInfoSeg& soInfoSeg)
{
  CarrierCode carrierIn = soInfoSeg.carrierIn();
  if (LIKELY(carrierIn.empty()))
  {
    return Stopovers::DOES_NOT_APPLY;
  }

  if (tsm.carrier() == carrierIn)
  {
    return Stopovers::MATCH;
  }

  return Stopovers::NOT_MATCH;
}

Stopovers::MatchResult
Stopovers::checkInfoSegCarrierOut(TravelSegMarkup& tsm, const StopoversInfoSeg& soInfoSeg)
{
  CarrierCode carrierOut = soInfoSeg.carrierOut();
  if (LIKELY(carrierOut.empty()))
  {
    return Stopovers::DOES_NOT_APPLY;
  }

  if (tsm.carrierOut() == carrierOut)
  {
    return Stopovers::MATCH;
  }

  return Stopovers::NOT_MATCH;
}

bool
Stopovers::checkInfoSegDirectionality(TravelSegMarkup& tsm,
                                      StopoversInfoSegMarkup& soISM,
                                      bool& processNextInfoSeg,
                                      bool& failed,
                                      std::string& shortFailReason,
                                      std::string& failReason)
{
  if (soISM.checkInOut())
  {
    Indicator chargeInd = soISM.soInfoSeg()->chargeInd();

    // At this point, current match is not added into count yet
    if (soISM.inOutEither() &&
        (((tsm.direction() == TravelSegMarkup::INBOUND) && soISM.matchCountOut()) ||
         ((tsm.direction() == TravelSegMarkup::OUTBOUND) && soISM.matchCountIn())))
    {
      if (chargeInd == Stopovers::SEG_NO_CHARGE)
      {
        shortFailReason = "IN/OUT-ONLY";

        std::ostringstream failString;
        failString << " STOPOVER ONLY PERMITTED ON " << std::endl
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
    else if ((soISM.inOutOut()) && (!soISM.numStopoversUnlimited()) &&
             (soISM.matchCountOut() == soISM.numStopovers()))
    {
      if (chargeInd == Stopovers::SEG_NO_CHARGE)
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
    else if ((soISM.inOutIn()) && (!soISM.numStopoversUnlimited()) &&
             (soISM.matchCountIn() == soISM.numStopovers()))
    {
      if (chargeInd == Stopovers::SEG_NO_CHARGE)
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
Stopovers::checkInfoSegGeo(PricingTrx& trx,
                           const FareMarket* fm,
                           const VendorCode& vendorOfFare,
                           TravelSegMarkup& tsm,
                           StopoversInfoSegMarkup& soISM,
                           std::vector<StopoversInfoSegMarkup*>& soISMVector,
                           MatchResult& geoMatch,
                           bool& processNextInfoSeg,
                           const StopoversInfoSeg*& surfGeoMatchedInfoSeg)
{
  if (!checkInfoSegMainGeo(trx,
                           fm,
                           vendorOfFare,
                           tsm,
                           soISM,
                           geoMatch,
                           processNextInfoSeg))
  {
    return tsm.isSurfaceSector();
  }

  // We return because we are treating an arunk as TravelSeg. We alrady checked
  // its geo location.
  if (tsm.isSurfaceSector())
    return true;

  bool geoNotPermitted =
      (soISM.soInfoSeg()->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_NOT_PERMITTED);

  if ((tsm.altOffPointGeo()) && ((!geoNotPermitted && (geoMatch == Stopovers::MATCH)) ||
                                 (geoNotPermitted && (geoMatch != Stopovers::MATCH))))
  {
    MatchResult altGeoMatch = Stopovers::NOT_MATCH;
    const StopoversInfoSeg* altGeoMatchedInfoSeg = nullptr;

    if (!checkInfoSegAltGeo(trx,
                            fm,
                            vendorOfFare,
                            tsm,
                            soISM,
                            soISMVector,
                            altGeoMatch,
                            processNextInfoSeg,
                            altGeoMatchedInfoSeg))
    {
      return false;
    }

    surfGeoMatchedInfoSeg = altGeoMatchedInfoSeg;
    if (geoNotPermitted)
    {
      if (surfGeoMatchedInfoSeg == nullptr ||
          surfGeoMatchedInfoSeg->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_NOT_PERMITTED ||
          geoMatch != Stopovers::NOT_MATCH ||
          altGeoMatch != Stopovers::MATCH) // Alt GEO matched permitted segment
      {
        if (geoMatch == Stopovers::MATCH && altGeoMatch == Stopovers::NOT_MATCH)
        {
          // SPR139022: when we need to vaildate between record3(s)
          // here we assume altGeoMatch occured and verify it at final stage.
          processNextInfoSeg = true;
          if (!surfGeoMatchedInfoSeg)
            surfGeoMatchedInfoSeg = soISM.soInfoSeg();

          // we need remember this force pass to validate in future
          tsm.forceArnkPass() = true;
        }
        else if (altGeoMatch == Stopovers::MATCH)
        {
          geoMatch = Stopovers::MATCH;
        }
      }
    }
    else
    {
      if (geoMatch == Stopovers::MATCH && altGeoMatch == Stopovers::NOT_MATCH)
      {
        // SPR139022: when we need to vaildate between record3
        // Here we assume altGeoMatch occured and verify it at final stage.
        processNextInfoSeg = true;

        if (!surfGeoMatchedInfoSeg)
          surfGeoMatchedInfoSeg = soISM.soInfoSeg();

        // we need to remember this force pass to validate in future
        tsm.forceArnkPass() = true;
      }
      else if (geoMatch != Stopovers::MATCH || altGeoMatch != Stopovers::MATCH)
      {
        geoMatch = Stopovers::NOT_MATCH;
      }
    }
  }
  return true;
}

bool
Stopovers::checkImplicitInfoSegMatch(TravelSegMarkup& tsm, bool allSegsGeoApplNot)
{
  SiSegMatchListCI smIter = tsm.siSegMatch().begin();
  SiSegMatchListCI smIterEnd = tsm.siSegMatch().end();

  for (; smIter != smIterEnd; ++smIter)
  {
    const StopoverInfoSegMatch& sism = *smIter;

    bool sameCxr = (sism.sameCarrierMatchResult() == Stopovers::MATCH ||
                    sism.sameCarrierMatchResult() == Stopovers::NOT_CHECKED ||
                    sism.sameCarrierMatchResult() == Stopovers::DOES_NOT_APPLY);

    bool inboundCxr = (sism.inCarrierMatchResult() == Stopovers::MATCH ||
                       sism.inCarrierMatchResult() == Stopovers::NOT_CHECKED ||
                       sism.inCarrierMatchResult() == Stopovers::DOES_NOT_APPLY);

    bool outboundCxr = (sism.outCarrierMatchResult() == Stopovers::MATCH ||
                        sism.outCarrierMatchResult() == Stopovers::NOT_CHECKED ||
                        sism.outCarrierMatchResult() == Stopovers::DOES_NOT_APPLY);

    if (allSegsGeoApplNot)
    {
      if (sism.geoMatchResult() == Stopovers::MATCH && sameCxr && inboundCxr && outboundCxr)
      {
        return false;
      }
    }
    else
    {
      if (_applScope == PU_SCOPE)
        return false;

      if (sism.soInfoSeg()->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_NOT_PERMITTED)
      {
        if (sism.geoMatchResult() == Stopovers::MATCH && sameCxr && inboundCxr && outboundCxr)
        {
          return false;
        }
      }
      else
      {
        if ((sism.geoMatchResult() == Stopovers::MATCH) &&
            (!sameCxr || !inboundCxr || !outboundCxr))
        {
          return false;
        }
        else if ((sism.soInfoSeg()->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_REQUIRED) &&
                 ((sism.segCheckResult() == StopoverInfoSegMatch::FAIL) ||
                  (sism.segCheckResult() == StopoverInfoSegMatch::DOES_NOT_APPLY)))
        {
          return false;
        }
      }
    }
  }
  return true;
}

bool
Stopovers::checkInfoSegMainGeo(PricingTrx& trx,
                               const FareMarket* fm,
                               const VendorCode& vendorOfFare,
                               TravelSegMarkup& tsm,
                               StopoversInfoSegMarkup& soISM,
                               MatchResult& geoMatch,
                               bool& processNextInfoSeg)
{
  const StopoversInfoSeg* soInfoSeg = soISM.soInfoSeg();

  geoMatch = matchGeo(trx, fm, vendorOfFare, tsm.travelSeg()->destination(), soISM);

  if ((geoMatch == Stopovers::MATCH) &&
      (soInfoSeg->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_NOT_PERMITTED))
  {
    if (soInfoSeg->chargeInd() == Stopovers::SEG_NO_CHARGE)
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
  else if (geoMatch == Stopovers::NOT_MATCH &&
           (soInfoSeg->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_REQUIRED ||
            soInfoSeg->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_BLANK))
  {
    processNextInfoSeg = true;
    return false;
  }

  processNextInfoSeg = true;
  if (UNLIKELY(geoMatch == Stopovers::NOT_MATCH && _applScope == PU_SCOPE && isGeoWithinSameLoc(soInfoSeg)))
  {
    return false;
  }
  return true;
}

bool
Stopovers::checkInfoSegAltGeo(PricingTrx& trx,
                              const FareMarket* fm,
                              const VendorCode& vendorOfFare,
                              TravelSegMarkup& tsm,
                              StopoversInfoSegMarkup& soISM,
                              std::vector<StopoversInfoSegMarkup*>& soISMVector,
                              MatchResult& geoMatch,
                              bool& processNextInfoSeg,
                              const StopoversInfoSeg*& matchedInfoSeg)
{
  if (!tsm.altOffPointGeo())
  {
    processNextInfoSeg = true;
    return false;
  }

  geoMatch = Stopovers::NOT_MATCH;

  const StopoversInfoSeg* soInfoSeg = soISM.soInfoSeg();

  geoMatch = matchGeo(trx, fm, vendorOfFare, tsm.altOffPointGeo(), soISM);

  if (geoMatch == Stopovers::MATCH)
  {
    matchedInfoSeg = soInfoSeg;
    processNextInfoSeg = true;
    return true;
  }

  if (soInfoSeg->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_NOT_PERMITTED)
  {
    processNextInfoSeg = true;
    return true;
  }

  std::vector<StopoversInfoSegMarkup*>::iterator it = soISMVector.begin();
  std::vector<StopoversInfoSegMarkup*>::iterator itE = soISMVector.end();

  for (; it != itE; ++it)
  {
    StopoversInfoSegMarkup* soISMptr = *it;
    if (soISMptr == &soISM)
    {
      continue;
    }

    const StopoversInfoSeg* soIS = soISMptr->soInfoSeg();

    if (!soISMptr->numStopoversUnlimited())
    {
      if (soISMptr->matchCount() == soISMptr->numStopovers())
      {
        continue;
      }
      else if (soISMptr->checkInOut())
      {
        if (((soISMptr->inOutIn()) && (soISMptr->matchCountIn() == soISMptr->numStopovers())) ||
            ((soISMptr->inOutOut()) && (soISMptr->matchCountOut() == soISMptr->numStopovers())))
        {
          continue;
        }
      }
    }

    if (soISMptr->checkInOut())
    {
      if ((soISMptr->inOutOut()) && (tsm.direction() == TravelSegMarkup::INBOUND))
      {
        continue;
      }
      else if ((soISMptr->inOutIn()) && (tsm.direction() == TravelSegMarkup::OUTBOUND))
      {
        continue;
      }
    }

    if (checkInfoSegSameCarrier(tsm, *soIS) == Stopovers::NOT_MATCH ||
        checkInfoSegCarrierIn(tsm, *soIS) == Stopovers::NOT_MATCH ||
        checkInfoSegCarrierOut(tsm, *soIS) == Stopovers::NOT_MATCH)
    {
      continue;
    }

    bool dummy1(true);
    bool dummy2(false);
    std::string dummy3;
    std::string dummy4;
    if (!checkInfoSegDirectionality(tsm, *soISMptr, dummy1, dummy2, dummy3, dummy4))
    {
      continue;
    }

    geoMatch = matchGeo(trx, fm, vendorOfFare, tsm.altOffPointGeo(), *soISMptr);

    if (geoMatch == Stopovers::MATCH)
      matchedInfoSeg = soIS;

    if (geoMatch == Stopovers::MATCH)
    {
      processNextInfoSeg = true;
      return true;
    }
  }

  return true;
}

Stopovers::MatchResult
Stopovers::matchGeo(PricingTrx& trx,
                    const FareMarket* fm,
                    const VendorCode& vendorOfFare,
                    const Loc* loc,
                    StopoversInfoSegMarkup& soISM)
{
  const StopoversInfoSeg& soInfoSeg = *(soISM.soInfoSeg());
  const LocTypeCode& locType1 = soInfoSeg.loc1().locType();

  if (locType1 == LOCTYPE_NONE)
  {
    return Stopovers::MATCH; // If segment loc is blank, it's always a match
  }

  const LocCode& locCode1 = soInfoSeg.loc1().loc();

  if (LIKELY(!(locCode1.empty() || locCode1 == " ")))
  {
    if (TransStopUtil::isInLoc(trx, vendorOfFare, *loc, soInfoSeg.loc1()))
      return MATCH;
  }
  else if ( checkStopoversInSameLoc(trx, fm, vendorOfFare, locType1, *loc, soISM) )
  {
    return Stopovers::MATCH;
  }

  const LocTypeCode& locType2 = soInfoSeg.loc2().locType();

  if (locType2 != LOCTYPE_NONE)
  {
    const LocCode& locCode2 = soInfoSeg.loc2().loc();

    if (!(locCode2.empty() || locCode2 == " "))
    {
      if (TransStopUtil::isInLoc(trx, vendorOfFare, *loc, soInfoSeg.loc2()))
        return MATCH;
    }
    else if ( checkStopoversInSameLoc(trx, fm, vendorOfFare, locType2, *loc, soISM) )
    {
      return Stopovers::MATCH;
    }
  }

  return Stopovers::NOT_MATCH;
}

Stopovers::ProcessingResult
Stopovers::checkMinStops(PricingTrx& trx,
                         const TravelSegMarkupContainer& tsmCon,
                         const StopoversInfo& soInfo,
                         std::string& failReason)
{
  int16_t numStops = 0;
  int16_t numStopsMin = 0;

  std::string noStopsMin = soInfo.noStopsMin();

  if (LIKELY(noStopsMin.empty()))
    return Stopovers::PASS;

  numStopsMin = atoi(noStopsMin.c_str());

  TravelSegMarkupCI iter = tsmCon.begin();
  TravelSegMarkupCI iterEnd = tsmCon.end();

  for (; iter != iterEnd; ++iter)
  {
    const TravelSegMarkup* tsm = (*iter);
    if (tsm->isSurfaceSector())
      continue;

    if (tsm->stopType() == Stopovers::STOPOVER)
      ++numStops;
  }

  if (numStops < numStopsMin)
  {
    std::ostringstream failString;
    failString << "MIN " << numStopsMin << " STOPOVERS REQUIRED";
    failReason = failString.str();
    return Stopovers::FAIL;
  }
  return Stopovers::PASS;
}

Stopovers::ProcessingResult
Stopovers::checkMaxStopsInOut(PricingTrx& trx,
                              const FarePath* fp,
                              TravelSegMarkupContainer& tsmCon,
                              const StopoversInfoWrapper& soInfoWrapper,
                              std::string& failReason)
{
  const StopoversInfo* soInfo = soInfoWrapper.soInfo();
  int16_t numStopsMax = 0;
  int16_t numStopsOutbound = 0;
  int16_t numStopsInbound = 0;

  bool numStopsMaxUnlimited = isNumStopsMaxUnlimited(*soInfo, numStopsMax);
  bool numStopsOutUnlimited = false;
  bool numStopsInUnlimited  = false;

  if (UNLIKELY(isRtw()))
  {
    numStopsOutUnlimited = true;
    numStopsInUnlimited = true;
  }
  else
  {
    numStopsOutUnlimited = isNumStopsUnlimited(soInfo->noStopsOutbound(), numStopsOutbound);
    numStopsInUnlimited  = isNumStopsUnlimited(soInfo->noStopsInbound(), numStopsInbound);
  }

  int16_t outStopsCtr = 0;
  int16_t inStopsCtr = 0;
  int16_t totalStopsCtr = 0;

  TravelSegMarkupI iter = tsmCon.begin();
  TravelSegMarkupI iterEnd = tsmCon.end();
  for (; iter != iterEnd; ++iter)
  {
    TravelSegMarkup* tsm = (*iter);
    if (tsm->stopType() != Stopovers::STOPOVER || tsm->isSurfaceSector())
      continue;

    TravelSeg* ts = tsm->travelSeg();
    FareUsage* fu = tsm->fareUsage();
    soInfoWrapper.setIsStopover(ts, fu);

    bool passedByLeastRestrictive = false;
    bool isTentativeMatch = false;

    // soInfoWrapper.getRuleItemMatch modified isTentativeMatch
    uint32_t ruleItemMatch = soInfoWrapper.getRuleItemMatch(ts, isTentativeMatch);
    if (ruleItemMatch)
    {
      if (ruleItemMatch == soInfo->itemNo())
      {
        continue;
      }
      else
      {
        // This implies that we matched a Record3 that passes both segment and arunk
        if (isTentativeMatch || (!tsm->forceArnkPass() && !tsm->isTentativeMatch()))
        {
          if (!soInfoWrapper.checkArunkForcedPass(ts))
          {
            continue; // if it is not forced-pass or seg and arnk have same R3
          }
        }
        else
        {
          bool passedValidation = soInfoWrapper.checkPassedValidation(ts);
          if ((!isTentativeMatch) && (passedValidation || tsm->isTentativeMatch()))
          {
            continue;
          }
        }
      }
    }

    if (tsm->direction() == TravelSegMarkup::UNKNOWN)
    {
      if ((!numStopsOutUnlimited) || (!numStopsInUnlimited))
      {
        tsm->stopoverCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
        return Stopovers::SKIP;
      }
    }

    if (UNLIKELY(tsm->stopoverCheckResult() == TravelSegMarkup::FAIL))
      continue;

    if (tsm->stopoverCheckResult() == TravelSegMarkup::NEED_REVALIDATION)
      continue;

    bool segCheck = (tsm->soSegCheckResult() == TravelSegMarkup::PASS || tsm->siSegMatch().empty());

    if ((!segCheck) && (tsm->soSegCheckResult() == TravelSegMarkup::NEED_REVALIDATION))
    {
      if (!soInfoWrapper.checkMatched(tsm->travelSeg()))
      {
        soInfoWrapper.setMatched(
            tsm->travelSeg(), tsm->fareUsage(), StopoversTravelSegWrapper::NOT_MATCHED);
      }
    }

    if (!tsm->validateEntireRule() || segCheck)
    {
      int16_t tempOutStopsCtr = outStopsCtr;
      int16_t tempInStopsCtr = inStopsCtr;
      int16_t tempTotalStopsCtr = totalStopsCtr;
      ++tempTotalStopsCtr;
      if (isStopoverNotPermitted(
              soInfoWrapper, numStopsMaxUnlimited, numStopsMax, tempTotalStopsCtr))
      {
        failReason = "NO STOPOVERS PERMITTED";
        return Stopovers::FAIL;
      }

      if (tsm->validateEntireRule())
      {
        if (UNLIKELY(ruleItemMatch && isTentativeMatch &&
            tsm->stopoverCheckResult() == TravelSegMarkup::PASS))
        {
          if (!fp || (fp && checkPaxType(trx, *fp, *fu, soInfoWrapper)))
            soInfoWrapper.setRuleItemMatch(ts, fu, soInfo->itemNo(), tsm->isTentativeMatch());
        }

        if (tsm->direction() == TravelSegMarkup::OUTBOUND)
          ++tempOutStopsCtr;
        else if (tsm->direction() == TravelSegMarkup::INBOUND)
          ++tempInStopsCtr;

        bool isOutOrInExceeded = ((tempOutStopsCtr > 0 && tempInStopsCtr > 0) ||
                                  tempOutStopsCtr > numStopsMax || tempInStopsCtr > numStopsMax);

        const bool isOutOrInExclusive =
            soInfo->outOrReturnInd() == Stopovers::OUT_OR_RETURN_EXCLUSIVE && !isRtw();

        if (checkMaxExceed(
                ts,
                fu,
                soInfoWrapper,
                isOutOrInExclusive,
                tsm->noMoreStopover(), // TO_MANY_SO
                isOutOrInExceeded, // TOTAL_MAX_EXECEED
                (!numStopsMaxUnlimited && tempTotalStopsCtr > numStopsMax), // TOTAL_MAX_EXECEED
                (!numStopsOutUnlimited && tempOutStopsCtr > numStopsOutbound), // OUT_MAX_EXECEED
                (!numStopsInUnlimited && tempInStopsCtr > numStopsInbound))) // IN_MAX_EXECEED
          continue;

        if (LIKELY(!fp || (fp && checkPaxType(trx, *fp, *fu, soInfoWrapper))))
          soInfoWrapper.setRuleItemMatch(ts, fu, soInfo->itemNo(), tsm->isTentativeMatch());
      }
      else
      {
        if (soInfoWrapper.applyLeastRestrictiveProvision())
        {
          if (soInfoWrapper.leastRestrictiveStopoversUnlimited())
          {
            passedByLeastRestrictive = true;
          }
          else
          {
            if (soInfoWrapper.leastRestrictiveStopoversPermitted() >= tempTotalStopsCtr)
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
          if (!numStopsMaxUnlimited && tempTotalStopsCtr > numStopsMax)
            continue;

          if (!fp || (fp && checkPaxType(trx, *fp, *fu, soInfoWrapper)))
            soInfoWrapper.setRuleItemMatch(ts, fu, soInfo->itemNo(), tsm->isTentativeMatch());
        }
      }

      tsm->stopoverCheckResult() = TravelSegMarkup::PASS;
      setPassedValidation(fu, soInfoWrapper, tsm, ts, passedByLeastRestrictive);

      soInfoWrapper.setMatched(
          tsm->travelSeg(), tsm->fareUsage(), StopoversTravelSegWrapper::MATCHED);

      outStopsCtr = tempOutStopsCtr;
      inStopsCtr = tempInStopsCtr;
      totalStopsCtr = tempTotalStopsCtr;
    }
  }
  return Stopovers::PASS;
}

// This method should only be called if there are no recurring segment
//  records (StopoversInfoSeg) attached to the Cat 8 record 3 (StopoversInfo).
//
Stopovers::ProcessingResult
Stopovers::processGatewayRestrictions(PricingTrx& trx,
                                      TravelSegMarkupContainer& tsmCon,
                                      const StopoversInfo& soInfo,
                                      std::string& failReason)
{
  if (UNLIKELY(!soInfo.segs().empty()))
  {
    return Stopovers::CONTINUE;
  }

  Indicator gtwyInd = soInfo.gtwyInd();

  if ((gtwyInd == Stopovers::GTWY_BLANK) && (soInfo.geoTblItemNoGateway() == 0))
  {
    return Stopovers::CONTINUE;
  }

  TravelSegMarkupI iter = tsmCon.begin();
  TravelSegMarkupI iterEnd = tsmCon.end();

  for (; iter != iterEnd; ++iter)
  {
    TravelSegMarkup* tsm = (*iter);
    if (!tsm->validateEntireRule())
    {
      continue;
    }

    if (tsm->stopType() == Stopovers::STOPOVER)
    {
      switch (gtwyInd)
      {
         case Stopovers::GTWY_BLANK:
           // no gateway application, so continue. 
            break;
         case Stopovers::GTWY_PERMITTED:
           // If the gateway indicator is PERMITTED (Y), then the stopover
           // is permitted at a gateway.
           // If there is a GeoTable (995) specified, the stopover must
           // occur at a location matched by the GeoTable.

           if ( tsm->isGateway() )
           {
              if (soInfo.geoTblItemNoGateway() != 0)
              {
                 if (tsm->gatewayGeoTableMatchResult() == Stopovers::MATCH)
                 {
                    tsm->isTentativeMatch() = false;
                    tsm->stopoverCheckResult() = TravelSegMarkup::PASS;
                 }
                 else
                   tsm->stopoverCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
              }
              else
              {
                 tsm->isTentativeMatch() = false;
                 tsm->stopoverCheckResult() = TravelSegMarkup::PASS;
               }
            }
            else
               tsm->stopoverCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
            break;
         case Stopovers::GTWY_REQUIRED:
           // Any stopovers must occur at the specified gateway(s). If
           //  the is no GeoTable specified, then the stopover can occur
           //  at any gateway. If a GeoTable is specified, any
           //  stopovers must occur at the specific gateway(s) matched
           //  by the GeoTable.

           if ( tsm->isGateway() )
           {
              if (soInfo.geoTblItemNoGateway() != 0)
              {
                 if (tsm->gatewayGeoTableMatchResult() == Stopovers::MATCH)
                 {
                    tsm->isTentativeMatch() = false;
                    tsm->stopoverCheckResult() = TravelSegMarkup::PASS;
                 }
                 else
                   tsm->stopoverCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
              }
           }
           else
             tsm->stopoverCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
           break;
         case Stopovers::GTWY_NOT_PERMITTED:
         // Stopovers are not permitted at the specified gateway(s).
         //  If there is no GeoTable specified, then the stopover must
         //  not occur at any gateway. If a GeoTable is specified,
         //  any stopovers must not occur at the gateway(s) matched
         //  by the GeoTable.

            if ( tsm->isGateway() )
            {
               if ( (soInfo.geoTblItemNoGateway() == 0) ||
                    (tsm->gatewayGeoTableMatchResult() == Stopovers::MATCH))
               {
                  tsm->stopoverCheckResult() = TravelSegMarkup::FAIL;
                  tsm->failReason() = "GATEWAY STOPOVER NOT PERMITTED";
                  failReason = "GATEWAY RESTRICTION";
                  return Stopovers::FAIL;
               }
            }
           else
              tsm->stopoverCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
           break;
         }  // switch
    } //stoptype == STOPOVER 
 }
  return Stopovers::CONTINUE;
}

bool
Stopovers::checkTripType(PricingTrx& trx,
                         const StopoversInfo& soInfo,
                         const PricingUnit& pu,
                         const FarePath& fp)
{
  bool tripTypeSpecified = false;

  std::vector<FareUsage*>::size_type fuCount = pu.fareUsage().size();

  if (UNLIKELY(soInfo.ojStopoverInd() == Stopovers::OPEN_JAW_REQUIRED))
  {
    tripTypeSpecified = true;

    if (pu.puType() == PricingUnit::Type::OPENJAW)
    {
      return true;
    }
  }

  bool pendingRTtoCTConversion = false;
  if (pu.puType() == PricingUnit::Type::ROUNDTRIP)
  {
    if (UNLIKELY(soInfo.ct2StopoverInd() == Stopovers::CIRCLE_TRIP_2_REQUIRED &&
        soInfo.ct2PlusStopoverInd() == Stopovers::CIRCLE_TRIP_2_PLUS_REQUIRED))
    {
      return true;
    }
    else if (LIKELY(soInfo.ct2StopoverInd() != Stopovers::CIRCLE_TRIP_2_REQUIRED &&
             soInfo.ct2PlusStopoverInd() != Stopovers::CIRCLE_TRIP_2_PLUS_REQUIRED))
    {
      return !tripTypeSpecified;
    }

    PricingUnit& puAllowChg = const_cast<PricingUnit&>(pu);
    pendingRTtoCTConversion = PricingUnitLogic::shouldConvertRTtoCT(trx, puAllowChg, fp);
  }

  if (UNLIKELY(soInfo.ct2StopoverInd() == Stopovers::CIRCLE_TRIP_2_REQUIRED))
  {
    tripTypeSpecified = true;

    if (pu.puType() == PricingUnit::Type::ROUNDTRIP && !pendingRTtoCTConversion)
    {
      return true;
    }
  }

  if (UNLIKELY(soInfo.ct2PlusStopoverInd() == Stopovers::CIRCLE_TRIP_2_PLUS_REQUIRED))
  {
    tripTypeSpecified = true;

    if (fuCount > 2)
    {
      if (pu.puType() == PricingUnit::Type::CIRCLETRIP)
      {
        return true;
      }
    }
    else if (pendingRTtoCTConversion)
    {
      return true;
    }
  }

  return !tripTypeSpecified;
}

void
Stopovers::printDiagnostics(DiagCollector& diag,
                            const StopoversInfo& soInfo,
                            const TravelSegMarkupContainer& tsmCon,
                            const FareMarket& fm)
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
Stopovers::printDiagnostics(DiagCollector& diag,
                            const StopoversInfo& soInfo,
                            const TravelSegMarkupContainer& tsmCon,
                            const PricingUnit& pu)
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
}

void
Stopovers::printDiagHeader(DiagCollector& diag,
                           const StopoversInfoWrapper& soInfoWrapper,
                           const PricingUnit* pu)
{
  const StopoversInfo* soInfoPtr = soInfoWrapper.soInfo();
  const StopoversInfo& soInfo = *soInfoPtr;

  diag << " " << std::endl << "---STOPOVER RULE DATA---" << std::endl
       << "STOP TIME.... MIN:   " << std::setw(3) << soInfo.minStayTime()
       << soInfo.minStayTimeUnit() << "  "
       << "MAX:   " << std::setw(3) << soInfo.maxStayTime() << soInfo.maxStayTimeUnit() << "  "
       << std::endl << "SAME POINT... STOPS: " << std::setw(3) << soInfo.samePntStops() << "   "
       << "CONNS: " << std::setw(3) << soInfo.samePntConnections() << "   "
       << "TRNST: " << std::setw(3) << soInfo.samePntTransit() << std::endl
       << "STOPOVERS.... MIN:   " << std::setw(3) << soInfo.noStopsMin() << "   "
       << "MAX:   " << std::setw(3) << soInfo.noStopsMax() << "   ";
  if (soInfo.outOrReturnInd() == Stopovers::OUT_OR_RETURN_EXCLUSIVE)
  {
    diag << "EITHER:" << std::setw(3) << soInfo.outOrReturnInd() << std::endl;
  }
  else
  {
    diag << "OUT:   " << std::setw(3) << soInfo.noStopsOutbound() << "   "
         << "IN:    " << std::setw(3) << soInfo.noStopsInbound() << std::endl;
  }
  diag << "LEAST REST... APPLY: ";

  if (pu)
  {
    if (soInfoWrapper.applyLeastRestrictiveProvision())
    {
      diag << "  Y   MAX:   ";

      if (soInfoWrapper.leastRestrictiveStopoversUnlimited())
      {
        diag << " XX";
      }
      else
      {
        diag << std::setw(3) << std::right << soInfoWrapper.leastRestrictiveStopoversPermitted();
      }
    }
    else
    {
      diag << "  N";
    }
  }
  else
  {
    diag << "  N/A";
  }

  diag << std::endl << "GATEWY REST... APPL: ";

  if (soInfo.gtwyInd() == Stopovers::GTWY_BLANK)
  {
    diag << std::setw(12) << std::left << "N/A";
  }
  else if (soInfo.gtwyInd() == Stopovers::GTWY_PERMITTED)
  {
    diag << std::setw(12) << std::left << "PERMITTED";
  }
  else if (soInfo.gtwyInd() == Stopovers::GTWY_NOT_PERMITTED)
  {
    diag << std::setw(12) << std::left << "NOT PERM";
  }
  else if (soInfo.gtwyInd() == Stopovers::GTWY_REQUIRED)
  {
    diag << std::setw(12) << std::left << "REQUIRED";
  }

  diag << "GEO TBL: " << soInfo.geoTblItemNoGateway();

  diag << std::endl;

  diag << "CHARGES PAX:" << std::setw(1) << soInfo.charge1Appl() << " CHG1:" << std::setw(4)
       << std::right << soInfo.charge1FirstAmt() << std::setw(3) << soInfo.charge1Cur();

  if (!soInfo.charge1First().empty())
  {
    diag << " *" << std::setw(2) << soInfo.charge1First();
  }
  else
  {
    diag << "    ";
  }

  diag << " ADD:" << std::setw(4) << soInfo.charge1AddAmt() << std::setw(3) << soInfo.charge1Cur();

  if (!soInfo.charge1AddNo().empty())
  {
    diag << " *" << std::setw(2) << soInfo.charge1AddNo();
  }
  else
  {
    diag << "    ";
  }

  diag << std::endl;

  printDiagStopoverInfoCharge2(diag, soInfo);

  if (soInfo.ojStopoverInd() == Stopovers::OPEN_JAW_REQUIRED)
  {
    diag << "TRIP TYPE: OJ       " << std::endl;
  }
  if (soInfo.ct2StopoverInd() == Stopovers::CIRCLE_TRIP_2_REQUIRED)
  {
    diag << "TRIP TYPE: CT2      " << std::endl;
  }
  if (soInfo.ct2PlusStopoverInd() == Stopovers::CIRCLE_TRIP_2_PLUS_REQUIRED)
  {
    diag << "TRIP TYPE: CT2 PLUS " << std::endl;
  }

  diag << " " << std::endl << "---STOPOVER RULE SEGMENT DATA---" << std::endl
       << "     --STOPS-- ------GEO-------  --CARRIER--  -OTHER--" << std::endl
       << "     NUM       LOC1  LOC2  APPL  IN  OUT SME  DIR  CHG" << std::endl;

  std::vector<StopoversInfoSeg*>::const_iterator iter = soInfo.segs().begin();
  std::vector<StopoversInfoSeg*>::const_iterator iterEnd = soInfo.segs().end();

  for (; iter != iterEnd; ++iter)
  {
    StopoversInfoSeg* soInfoSeg = *iter;

    diag << " " << std::setw(2) << std::right << soInfoSeg->orderNo() << ": " << std::setw(3)
         << std::right << soInfoSeg->noStops() << std::setw(7) << std::left << " ";

    if ((soInfoSeg->loc1().locType() == LOCTYPE_NONE) &&
        (soInfoSeg->loc2().locType() == LOCTYPE_NONE))
    {
      diag << std::setw(18) << " ";
    }
    else
    {
      if (soInfoSeg->loc1().locType() != LOCTYPE_NONE &&
          (soInfoSeg->loc1().loc().empty() || soInfoSeg->loc1().loc()[0] == ' '))
      {
        diag << std::setw(6) << std::left << soInfoSeg->loc1().locType();
      }
      else
      {
        diag << std::setw(6) << std::left << soInfoSeg->loc1().loc();
      }

      if (soInfoSeg->loc2().locType() != LOCTYPE_NONE &&
          (soInfoSeg->loc2().loc().empty() || soInfoSeg->loc2().loc()[0] == ' '))
      {
        diag << std::setw(6) << std::left << soInfoSeg->loc2().locType();
      }
      else
      {
        diag << std::setw(6) << std::left << soInfoSeg->loc2().loc();
      }

      if (soInfoSeg->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_REQUIRED)
      {
        diag << std::setw(6) << "REQ";
      }
      else if (soInfoSeg->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_NOT_PERMITTED)
      {
        diag << std::setw(6) << "NOT";
      }
      else if (soInfoSeg->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_BLANK)
      {
        diag << std::setw(6) << "OK";
      }
    }

    diag << std::setw(4) << std::left << soInfoSeg->carrierIn() << std::setw(4) << std::left
         << soInfoSeg->carrierOut() << std::setw(5) << std::left << soInfoSeg->carrierInd();

    if (soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_OUT)
    {
      diag << std::setw(5) << std::left << "OUT";
    }
    else if (soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_IN)
    {
      diag << std::setw(5) << std::left << "IN";
    }
    else if (soInfoSeg->stopoverInOutInd() == Stopovers::SEG_INOUT_EITHER)
    {
      diag << std::setw(5) << std::left << "I/O";
    }
    else
    {
      diag << std::setw(5) << std::left << soInfoSeg->stopoverInOutInd();
    }

    if (soInfoSeg->chargeInd() == Stopovers::SEG_USE_CHARGE_1)
    {
      diag << "1ST";
    }
    else if (soInfoSeg->chargeInd() == Stopovers::SEG_USE_CHARGE_2)
    {
      diag << "ADD";
    }
    else
    {
      diag << "   ";
    }
    diag << std::endl;
  }

  diag << " " << std::endl;
}

void
Stopovers::printDiagTsmDetail(DiagCollector& diag,
                              const StopoversInfo& soInfo,
                              const TravelSegMarkup& tsm,
                              const TravelSeg* ts)
{
  if (tsm.stopType() == Stopovers::STOPOVER)
  {
    diag << " SO ";
  }
  else if (tsm.stopType() == Stopovers::CONNECTION)
  {
    diag << " CX ";
  }
  else
  {
    diag << "    ";
  }

  if (tsm.isGateway())
  {
    diag << " GW ";
  }
  else
  {
    diag << "    ";
  }

  switch (tsm.stopoverCheckResult())
  {
  case TravelSegMarkup::NOT_CHECKED:
    diag << " N/C  ";
    break;
  case TravelSegMarkup::PASS:
    diag << " PASS ";
    break;
  case TravelSegMarkup::FAIL:
    diag << " FAIL " << tsm.failReason();
    break;
  case TravelSegMarkup::NEED_REVALIDATION:
    diag << " ---- ";
    break;
  case TravelSegMarkup::STOP:
    diag << " STOP ";
    break;
  default:
    diag << " ERR  ";
    break;
  }

  diag << std::endl;

  SiSegMatchList ssmList = tsm.siSegMatch();

  if (!ssmList.empty())
  {
    SiSegMatchListCI ssmIter = ssmList.begin();
    SiSegMatchListCI ssmIterEnd = ssmList.end();

    for (; ssmIter != ssmIterEnd; ++ssmIter)
    {
      const StopoverInfoSegMatch& sism = *ssmIter;

      const StopoversInfoSeg* siSeg = sism.soInfoSeg();
      if (!siSeg)
      {
        diag << "**ERROR SO-INFO-SEG IS NULL**" << std::endl;
        continue;
      }

      diag << "    -" << std::setw(2) << std::right << siSeg->orderNo() << ": " << std::setw(5)
           << std::left << siSeg->loc1().loc() << " " << std::setw(5) << std::left
           << siSeg->loc2().loc();

      diag << "  GEO:";

      if (sism.geoMatchResult() == Stopovers::MATCH)
      {
        diag << "MATCH ";
      }
      else if (sism.geoMatchResult() == Stopovers::NOT_MATCH)
      {
        diag << "NOT   ";
      }
      else if (sism.geoMatchResult() == Stopovers::NOT_CHECKED)
      {
        diag << "N/C   ";
      }
      else if (sism.geoMatchResult() == Stopovers::DOES_NOT_APPLY)
      {
        diag << "N/A   ";
      }

      diag << "IN:";

      if (sism.inCarrierMatchResult() == Stopovers::MATCH)
      {
        diag << "MAT ";
      }
      else if (sism.inCarrierMatchResult() == Stopovers::NOT_MATCH)
      {
        diag << "NOT ";
      }
      else if (sism.inCarrierMatchResult() == Stopovers::NOT_CHECKED)
      {
        diag << "N/C ";
      }
      else if (sism.inCarrierMatchResult() == Stopovers::DOES_NOT_APPLY)
      {
        diag << "N/A ";
      }

      diag << "OUT:";

      if (sism.outCarrierMatchResult() == Stopovers::MATCH)
      {
        diag << "MAT ";
      }
      else if (sism.outCarrierMatchResult() == Stopovers::NOT_MATCH)
      {
        diag << "NOT ";
      }
      else if (sism.outCarrierMatchResult() == Stopovers::NOT_CHECKED)
      {
        diag << "N/C ";
      }
      else if (sism.outCarrierMatchResult() == Stopovers::DOES_NOT_APPLY)
      {
        diag << "N/A ";
      }

      diag << std::endl;
    }
    diag << "    - SEGMENT CHECK RESULT: ";
    switch (tsm.soSegCheckResult())
    {
    case TravelSegMarkup::PASS:
      diag << "PASS";
      break;
    case TravelSegMarkup::FAIL:
      diag << "FAIL";
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
    case TravelSegMarkup::STOP:
      diag << "STOP";
      break;
    default:
      break;
    }

    diag << std::endl << " " << std::endl;
  }
}

void
Stopovers::printDiagTravelSeg(DiagCollector& diag, const TravelSeg* ts, const int16_t segmentNumber)
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
      diag << std::setw(3) << std::left << as->carrier();
      diag << std::setw(4) << std::right << as->flightNumber() << std::left;
      diag << " " << ts->departureDT().dateToString(DDMMM, "");
      diag << " " << ts->origin()->loc();
      diag << ts->destination()->loc();
      diag << " " << ts->departureDT().timeToString(HHMM, "");
      diag << " " << ts->arrivalDT().timeToString(HHMM, "");
    }
    else
    {
      diag << std::setw(31) << std::left << "ARNK";
    }
  }
}

bool
Stopovers::checkPaxType(PricingTrx& trx,
                        const FarePath& fp,
                        FareUsage& fu,
                        const StopoversInfoWrapper& soInfoWrapper)
{
  const StopoversInfo* soInfo = soInfoWrapper.soInfo();

  if ( soInfo->charge1Appl() == RuleConst::CHARGE_PAX_ANY )
  {
    return true;
  }

  return soInfoWrapper.chargeForPaxType(
             trx, soInfo->charge1Appl(), *(fu.paxTypeFare()), *(fp.paxType()), true );
}

bool
Stopovers::atLeastOneRecurringSegPass(TravelSegMarkup& tsm)
{
  if (tsm.siSegMatch().empty())
    return true;

  if (tsm.segInfoPass())
    return true;

  tsm.noMoreStopover() = true;
  return false;
}

void
Stopovers::printDiagStopoverInfoCharge2(DiagCollector& diag, const StopoversInfo& soInfo)
{
  diag << "             ";

  diag << " CHG2:" << std::setw(4) << soInfo.charge2FirstAmt() << std::setw(3)
       << soInfo.charge2Cur();

  diag << " ADD:" << std::setw(4) << soInfo.charge2AddAmt() << std::setw(3) << soInfo.charge2Cur();

  diag << std::endl;
}

void
Stopovers::chkApplScope(const StopoversInfo& soInfo)
{
  _applScope = (soInfo.noStopsMax().empty() &&
                (!soInfo.noStopsOutbound().empty() || !soInfo.noStopsInbound().empty()))
                   ? FC_SCOPE
                   : PU_SCOPE;
}

bool
Stopovers::validateStopoversInfo_Mandate2010(const StopoversInfo& soInfo)
{
  if (soInfo.noStopsMax().empty() && !soInfo.segs().empty())
  {
    bool allSegsInOutEither = true;

    std::vector<StopoversInfoSeg*>::const_iterator soInfoSegI = soInfo.segs().begin();
    std::vector<StopoversInfoSeg*>::const_iterator soInfoSegIEnd = soInfo.segs().end();
    for (; soInfoSegI != soInfoSegIEnd; ++soInfoSegI)
    {
      if (LIKELY((*soInfoSegI)->stopoverInOutInd() != SEG_INOUT_EITHER))
      {
        allSegsInOutEither = false;
        break;
      }
    }
    if (UNLIKELY(allSegsInOutEither))
      return false;
  }

  return true;
}

bool
Stopovers::checkPreconditions(const StopoversInfo& soInfo,
                              Record3ReturnTypes& ret,
                              DiagCollector* diagPtr)
{
  if (UNLIKELY(soInfo.unavailTag() == Stopovers::UNAVAIL_TAG_NOT_AVAIL))
  {
    if (UNLIKELY(diagPtr))
      *diagPtr << "STOPOVERS : FAIL - R3 NOT AVAILABLE FOR USE\n";
    ret = tse::FAIL;
    return false;
  }

  if (soInfo.unavailTag() == Stopovers::UNAVAIL_TAG_TEXT_ONLY)
  {
    if (UNLIKELY(diagPtr))
      *diagPtr << "STOPOVERS : SKIP - TEXT PURPOSE ONLY\n";
    ret = tse::SKIP;
    return false;
  }

  if (UNLIKELY(isRtw()))
  {
    if (soInfo.noStopsMax().empty() &&
        (!soInfo.noStopsOutbound().empty() || !soInfo.noStopsInbound().empty()))
    {
      if (UNLIKELY(diagPtr))
        *diagPtr << "STOPOVERS : SKIP - MAX NOT SPECIFIED FOR RTW\n";
      ret = tse::SKIP;
      return false;
    }
  }

  return true;
}

bool
Stopovers::locateStopoversInfo(const RuleItemInfo* rule,
                               const StopoversInfoWrapper*& soInfoWrapper,
                               const StopoversInfo*& soInfo)
{
  soInfoWrapper = dynamic_cast<const StopoversInfoWrapper*>(rule);

  if (UNLIKELY(!soInfoWrapper))
  {
    LOG4CXX_ERROR(_logger, "Rules.Stopovers::validate(): soInfoWrapper = 0");
    return false;
  }

  soInfo = soInfoWrapper->soInfo();

  if (UNLIKELY(!soInfo))
  {
    LOG4CXX_ERROR(_logger, "Rules.Stopovers::validate(): soInfo = 0");
    return false;
  }
  return true;
}

bool
Stopovers::checkStopoversInSameLoc(const PricingTrx& trx,
                                   const FareMarket* fm,
                                   const VendorCode& vendor,
                                   const LocTypeCode& locType,
                                   const Loc& loc,
                                   StopoversInfoSegMarkup& soISM)
{
  const std::string locCode = TransStopUtil::getLocOfType(trx, fm, vendor, loc, locType);
  StopoversInfoSegMarkup::MatchedCounts& counts = soISM.stoSameLocCnts()[locCode];
  soISM.setCurrentCounts(&counts);

  if (!soISM.numStopoversUnlimited() &&
      (soISM.soInfoSeg()->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_BLANK))
  {
    return (soISM.matchCount() < soISM.numStopovers());
  }

  return true;
}

bool
Stopovers::checkFirstAddNo(Indicator chargeInd,
                           int16_t matchedCount,
                           bool chargeFirstUnlimited,
                           bool chargeAddUnlimited,
                           int16_t chargeFirstNo,
                           int16_t chargeAddNo)
{
  if (chargeInd == Stopovers::SEG_USE_CHARGE_1)
  {
    if ((!chargeFirstUnlimited) && (matchedCount == chargeFirstNo))
    {
      if ((!chargeAddUnlimited) && (matchedCount == (chargeFirstNo + chargeAddNo)))
      {
        return false;
      }
    }
  }
  else if (chargeInd == Stopovers::SEG_USE_CHARGE_2)
  {
    if ((!chargeAddUnlimited) && (matchedCount == chargeAddNo))
    {
      return false;
    }
  }
  return true;
}

void
Stopovers::StopoversInfoSegMarkup::increaseMatchCount(bool countInbound,
                                                      bool countOutbound)
{
  ++matchCount();
  ++matchCountCharge();

  if (countInbound)
  {
    ++matchCountIn();
    ++matchCountInCharge();
  }
  else if (countOutbound)
  {
    ++matchCountOut();
    ++matchCountOutCharge();
  }
}

bool
Stopovers::checkGTWYGeoMatchResult(TravelSegMarkup& tsm,
                                   StopoverInfoSegMatch& siSegMatch,
                                   MatchResult gtwyGeoMatch,
                                   MatchResult geoMatch,
                                   Indicator geoAppl,
                                   Indicator chargeInd,
                                   bool& savePrevSegMatch)
{
  savePrevSegMatch = false;
  if (!tsm.isGateway())
  {
    // The stopover is not at a gateway. This is a no-match.
    tsm.soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
    siSegMatch.segCheckResult() = StopoverInfoSegMatch::PASS;
    return false;
  }

  if ((gtwyGeoMatch != Stopovers::MATCH) && (geoAppl != Stopovers::SEG_GEO_APPL_NOT_PERMITTED) &&
      (geoMatch != Stopovers::MATCH))
  {
    // The location does not match the geo table or
    //  the recurring segment record geo data.
    // This is a no-match.
    tsm.soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
    siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;
    return false;
  }

  if ((geoAppl == Stopovers::SEG_GEO_APPL_NOT_PERMITTED) && (geoMatch == Stopovers::MATCH))
  {
    // The location matches the recurring segment record geo
    //  data, but this location is not permitted.
    // This is a no-match.
    if (chargeInd == Stopovers::SEG_NO_CHARGE)
    {
      tsm.soSegCheckResult() = TravelSegMarkup::STOP;
      siSegMatch.segCheckResult() = StopoverInfoSegMatch::FAIL;
      return false;
    }
    else
    {
      tsm.soSegCheckResult() = TravelSegMarkup::NEED_REVALIDATION;
      siSegMatch.segCheckResult() = StopoverInfoSegMatch::DOES_NOT_APPLY;
      savePrevSegMatch = true;
      return false;
    }
  }

  return true;
}

bool
Stopovers::isGeoWithinSameLoc(const StopoversInfoSeg* soInfoSeg)
{
  if (UNLIKELY(!soInfoSeg))
    return false;

  return (!soInfoSeg->noStops().empty() && soInfoSeg->loc1().locType() != LOCTYPE_NONE &&
          soInfoSeg->loc1().loc().empty() &&
          soInfoSeg->stopoverGeoAppl() == Stopovers::SEG_GEO_APPL_BLANK);
}

bool
Stopovers::isNumStopsMaxUnlimited(const StopoversInfo& soInfo, int16_t& intValue) const
{
  const std::string& noStopsMax = soInfo.noStopsMax();
  const std::string& noStopsOutbound = soInfo.noStopsOutbound();
  const std::string& noStopsInbound = soInfo.noStopsInbound();
  if ((noStopsMax == Stopovers::NUM_STOPS_UNLIMITED) ||
      (noStopsMax.empty() && noStopsOutbound.empty() && noStopsInbound.empty()) ||
      (noStopsMax.empty() && noStopsInbound == Stopovers::NUM_STOPS_UNLIMITED) ||
      (noStopsMax.empty() && noStopsOutbound == Stopovers::NUM_STOPS_UNLIMITED))
  {
    return true;
  }
  intValue = getNumStopMax(soInfo);
  return false;
}

bool
Stopovers::isNumStopsUnlimited(const std::string& str, int16_t &intValue) const
{
  if (str.empty() || str == Stopovers::NUM_STOPS_UNLIMITED)
  {
    intValue = 0;
    return true;
  }

  intValue = static_cast<int16_t>(atoi(str.c_str()));
  return false;
}

int16_t
Stopovers::getNumStopMax(const StopoversInfo& soInfo) const
{
  int numStops = 0;

  if (!soInfo.noStopsMax().empty())
    numStops = atoi(soInfo.noStopsMax().c_str());
  else
    numStops = atoi(soInfo.noStopsOutbound().c_str()) + atoi(soInfo.noStopsInbound().c_str());

  return static_cast<int16_t>(numStops);
}

void
Stopovers::markGateways(PricingTrx& trx, TravelSegMarkupContainer& tsmCon, const FareMarket& fm)
{
  TSIGateway tsiSetupGW;

  if (UNLIKELY(RtwUtil::isRtw(trx)))
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

    if (UNLIKELY(tsm->arunkTravelSeg()))
    {
      //apo-44172: with embedded arunk segments, mark gateways
      //using arnk seg only.
      if (!fallback::fallbackapo44172Cat8EmbeddedGtwyChk(&trx))
      {
        if (tsm->nextTravelSeg() )
        // this is an embedded arunk segment.
           tsm->isGateway() |= tsiSetupGW.isArrivalOnGW(tsm->arunkTravelSeg());
      }
      else  //fallback; remove at fallback removal time.
        tsm->isGateway() |= tsiSetupGW.isArrivalOnGW(tsm->nextTravelSeg());
    }
  }
}

void
Stopovers::setNextTravelSeg(TravelSegMarkupContainer& cont, TravelSeg* ts)
{
  TravelSegMarkupContainer::reverse_iterator it = cont.rbegin();
  while (it != cont.rend() && (*it)->isSurfaceSector())
    ++it;

  if (LIKELY(it != cont.rend()))
    (*it)->nextTravelSeg() = ts;
}

// true->continue
bool
Stopovers::checkMaxExceed(TravelSeg* ts,
                          FareUsage* fu,
                          const StopoversInfoWrapper& soInfoWrapper,
                          bool isOutOrReturnExclusive,
                          bool isExceedNumStopovers,
                          bool isOutOrInExceeded,
                          bool isTotalMaxExceeded,
                          bool outMaxExceeded,
                          bool isInMaxExceeded) const
{
  if (UNLIKELY(isExceedNumStopovers))
  {
    soInfoWrapper.setMaxExceeded(ts, fu, StopoversInfoWrapper::TO_MANY_SO);
    return true;
  }

  if (isOutOrReturnExclusive)
  {
    if (isOutOrInExceeded)
    {
      soInfoWrapper.setMaxExceeded(ts, fu, StopoversInfoWrapper::TOTAL_MAX_EXECEED);
      return true;
    }
  }
  else
  {
    if (isTotalMaxExceeded)
    {
      soInfoWrapper.setMaxExceeded(ts, fu, StopoversInfoWrapper::TOTAL_MAX_EXECEED);
      return true;
    }
  }

  if (outMaxExceeded)
  {
    soInfoWrapper.setMaxExceeded(ts, fu, StopoversInfoWrapper::OUT_MAX_EXECEED);
    return true;
  }

  if (UNLIKELY(isInMaxExceeded))
  {
    soInfoWrapper.setMaxExceeded(ts, fu, StopoversInfoWrapper::IN_MAX_EXECEED);
    return true;
  }
  return false;
}

void
Stopovers::setPassedValidation(FareUsage* fu,
                               const StopoversInfoWrapper& soInfoWrapper,
                               TravelSegMarkup* tsm,
                               TravelSeg* ts,
                               bool passedByLeastRestrictive) const
{
  if (tsm->forceArnkPass() && tsm->arunkTravelSeg())
  {
    LOG4CXX_DEBUG(_logger,
                  "FORCED-PASS ts arunk: " << ts->origin()->loc() << ts->destination()->loc() << " "
                                           << tsm->arunkTravelSeg()->origin()->loc()
                                           << tsm->arunkTravelSeg()->destination()->loc());

    soInfoWrapper.setPassedValidation(ts, fu, passedByLeastRestrictive, tsm->arunkTravelSeg());
  }
  else
  {
    soInfoWrapper.setPassedValidation(ts, fu, passedByLeastRestrictive);
  }
}

/*
* If the stopover rule allows no stopovers and a stopover exists,
* then fail unless the Least Restrictive Provision applies.
* If the Least Restrictive Provision applies and the least restrictive
* rule allows no stopovers, then fail.
*/
bool
Stopovers::isStopoverNotPermitted(const StopoversInfoWrapper& soInfoWrapper,
                                  bool numStopsMaxUnlimited,
                                  int16_t numStopsMax,
                                  int16_t tempTotalStopsCtr) const
{
  if (!numStopsMaxUnlimited && numStopsMax == 0 && tempTotalStopsCtr > 0 &&
      (!soInfoWrapper.applyLeastRestrictiveProvision() ||
       (!soInfoWrapper.leastRestrictiveStopoversUnlimited() &&
        soInfoWrapper.leastRestrictiveStopoversPermitted() == 0)))
  {
    return true;
  }
  return false;
}

} // namespace tse
