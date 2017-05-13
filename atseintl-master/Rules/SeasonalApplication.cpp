//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "Rules/SeasonalApplication.h"

#include "Common/DateTime.h"
#include "Common/Global.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/NoPNRTravelSegmentTimeUpdater.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/SeasonalAppl.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{
Logger
SeasonalApplication::_logger("atseintl.Rules.SeasonalApplication");

Record3ReturnTypes
SeasonalApplication::validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& paxTypeFare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket)
{
  LOG4CXX_INFO(_logger, " Entered SeasonalApplication::validate()");

  const SeasonalAppl* seasonRule = dynamic_cast<const SeasonalAppl*>(rule);

  if (UNLIKELY(!seasonRule))
    return FAIL;

  DiagManager diag(trx, Diagnostic303);

  Record3ReturnTypes ret = checkUnavailableAndText(seasonRule);

  if (UNLIKELY(ret == FAIL))
  {
    LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - FAIL");

    if (diag.isActive())
    {
      diag << " SEASONS: FAILED "
           << "- CHECK UNAVAILABLE\n";
    }

    return FAIL;
  }
  else if (ret == SKIP)
  {
    LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - SKIP");

    if (diag.isActive())
    {
      diag << " SEASONS: SKIP "
           << "- TEXT ONLY\n";
    }

    return SKIP;
  }

  if (UNLIKELY(diag.isActive()))
    diagRestriction(*seasonRule, diag);

  if (UNLIKELY(seasonRule->tvlstartmonth() < 1 || seasonRule->tvlstartmonth() > 12 ||
      seasonRule->tvlStopmonth() < 1 || seasonRule->tvlStopmonth() > 12))
  {
    LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - FAIL");

    if (UNLIKELY(diag.isActive()))
    {
      diag << " SEASONS: FAIL - BAD DATA \n";
    }
    return FAIL;
  }

  //-------------------------------------------------------------
  // Added for SITA v1.1
  // HIGH_SEASON requirement for OpenSeg with no validate date
  //-------------------------------------------------------------

  if (paxTypeFare.vendor() == Vendor::SITA)
  {
    Record3ReturnTypes rcHS;
    rcHS = checkHighSeasonOpenSeg(seasonRule, paxTypeFare, fareMarket.travelSeg());
    if (rcHS == FAIL)
    {
      LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - FAIL");
      if (UNLIKELY(diag.isActive()))
      {
        diag << " SEASONS: FAIL"
             << " NOT HIGH SEASON WITH OPEN SEGMENT\n";
      }
      return FAIL;
    }
    else if (rcHS == PASS)
    {
      LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - PASS");
      if (UNLIKELY(diag.isActive()))
      {
        diag << " SEASONS: PASS"
             << " HIGH SEASON WITH OPEN SEGMENT\n";
      }
      return PASS;
    }
    // otherwise, continue to check other fields in rule
  }

  //---------------------------------------------------------------------------
  // Set scope type
  // System assumption:
  // - Validate Seasons dates against the departure date from the origin
  //   of the Pricing Unit (subjourney);
  // - Assumption Override Tag may be set indicate the fare component trip date
  //   must be use to validate against season dates
  //---------------------------------------------------------------------------

  if (diag.isActive())
  {
    diag << "ASSUMPTION OVERRIDE: " << seasonRule->assumptionOverride() << "\n";
    diag << "GEO TBL: " << seasonRule->geoTblItemNo() << "\n";
  }

  ret = checkAssumptionOverride(seasonRule, itin, &trx);

  if (ret == FAIL)
  {
    LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - FAIL");
    if (diag.isActive())
    {
      diag << " SEASONS: FAIL - NO DATES IN SEASONS \n";
    }
    return FAIL;
  }
  else if (ret == PASS)
  {
    if (seasonRule->geoTblItemNo() != 0 && trx.getTrxType() != PricingTrx::IS_TRX)
    {
      LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - SOFTPASS");
      if (diag.isActive())
      {
        diag << " SEASONS: SOFTPASS - NEED VALIDATE GEO\n";
      }
      return SOFTPASS;
    }

    LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - PASS");
    if (diag.isActive())
    {
      diag << " SEASONS: PASS - ALL DATES IN SEASONS \n";
    }
    return PASS;
  }
  else if (ret == SOFTPASS)
  {
    // Now by default we are validating in PU scope, but Journey TSI can
    // override PU scope, then we need to validate as we have Itin
    // otherwise softpass
    RuleConst::TSIScopeType scope = RuleConst::TSI_SCOPE_SUB_JOURNEY;

    if (seasonRule->geoTblItemNo() != 0)
    {
      RuleUtil::getTSIScopeFromGeoRuleItem(
          seasonRule->geoTblItemNo(), paxTypeFare.vendor(), trx, scope);
    }

    if (LIKELY(scope != RuleConst::TSI_SCOPE_JOURNEY))
    {
      LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - SOFTPASS");
      if (diag.isActive())
      {
        diag << " SEASONS: SOFT PASS"
             << " - NEED REVALIDATION\n";
      }
      if (trx.getTrxType() != PricingTrx::IS_TRX)
      {
        return SOFTPASS;
      }
      else
      {
        ShoppingTrx* shopTrx = static_cast<ShoppingTrx*>(&trx);
        if (!shopTrx->isRuleTuningISProcess() || !trx.isAltDates())
        {
          return SOFTPASS;
        }
        bool allOpenSegs = true;
        if (UNLIKELY(itin.travelSeg().size() == 1))
        {
          allOpenSegs = (itin.travelSeg().front()->segmentType() == Open);
        }
        else if (itin.travelSeg().size() == 2)
        {
          allOpenSegs = ((itin.travelSeg().front()->segmentType() == Open) &&
                         (itin.travelSeg().back()->segmentType() == Open));
        }
        else
        {
          allOpenSegs = false;
        }

        if (!shopTrx->isSimpleTrip() || allOpenSegs)
        {
          return SOFTPASS;
        }
        ItinUtil::setFurthestPoint(trx, &itin);
      }
    }
  }

  ret = checkGeoTblItemNo(seasonRule, paxTypeFare, fareMarket, itin, trx);
  if (ret == SKIP)
  {
    LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - SKIP");
    if (diag.isActive())
    {
      diag << " SEASONS: SKIP - NO GEO MATCH FROM TABLE 995 \n";
    }
    return SKIP;
  }
  else if (ret == FAIL)
  {
    LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - FAIL");
    if (diag.isActive())
    {
      diag << " SEASONS: FAILED -  \n";
    }
    return FAIL;
  }

  LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - PASS");

  if (diag.isActive())
  {
    diag << " SEASONS: PASSED "
         << " - COMPLETED ALL VALIDATION CODE\n";
  }

  return PASS;
}

Record3ReturnTypes
SeasonalApplication::validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage)
{
  return validate(trx, rule, &farePath, farePath.itin(), pricingUnit, fareUsage);
}

Record3ReturnTypes
SeasonalApplication::validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage)
{
  return validate(trx, rule, nullptr, pricingUnit, fareUsage);
}

Record3ReturnTypes
SeasonalApplication::validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath* farePath,
                              const Itin* itin,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage)
{
  LOG4CXX_INFO(_logger, " Entered SeasonalApplication::validate()");

  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);
  const SeasonalAppl* seasonRule = dynamic_cast<const SeasonalAppl*>(rule);

  if (UNLIKELY(!seasonRule))
    return FAIL;

  DiagManager diag(trx, Diagnostic303);

  Record3ReturnTypes ret = checkUnavailableAndText(seasonRule);
  if (UNLIKELY(ret == FAIL))
  {
    LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - FAIL");

    if (diag.isActive())
    {
      diag << " SEASONS: FAILED "
           << "- CHECK UNAVAILABLE\n";
    }

    return FAIL;
  }
  else if (ret == SKIP)
  {
    LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - SKIP");

    if (diag.isActive())
    {
      diag << " SEASONS: SKIP "
           << "- TEXT ONLY\n";
    }

    return SKIP;
  }

  if (UNLIKELY(diag.isActive()))
    diagRestriction(*seasonRule, diag);

  if (UNLIKELY(seasonRule->tvlstartmonth() < 1 || seasonRule->tvlstartmonth() > 12 ||
      seasonRule->tvlStopmonth() < 1 || seasonRule->tvlStopmonth() > 12))
  {
    LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - FAIL");

    if (diag.isActive())
    {
      diag << " SEASONS: FAIL - BAD DATA \n";
    }
    return FAIL;
  }

  // do not use code below because of "SOFTPASS" when direction=3/4 in FareComponent scope
  //
  //    //* @TODO Need to codde for PU - SUB_JOURNEY_SCOPE return PASS for now
  //    if ( seasonRule->assumptionOverride() == 'X' )
  //    {
  //      LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - PASS");
  //
  //          if ( diag.diagnosticType() == Diagnostic303)
  //          {
  //              diag << " SEASONS: PASS "
  //                   << "- SEASON RULE ASSUMPTION OVERRIDE IS X\n";
  //          }
  //
  //      return PASS;
  //    }

  // lint -e{530}
  const PaxTypeFare& paxTypeFare = *fareUsage.paxTypeFare();

  const FareMarket& fareMarket = *paxTypeFare.fareMarket();

  //-------------------------------------------------------------
  // Added for SITA v1.1
  // HIGH_SEASON requirement for OpenSeg with no validate date
  //-------------------------------------------------------------
  if (paxTypeFare.vendor() == Vendor::SITA)
  {
    //----------------------------------------------------------------------
    // Check for Open segments in HIGH_SEASON
    //----------------------------------------------------------------------
    LOG4CXX_INFO(_logger, "  Entered SeasonalApplication::checkHighSeasonOpenSeg()");

    bool openSegExist = false;

    std::vector<TravelSeg*>::const_iterator travelSegI = pricingUnit.travelSeg().begin();
    const std::vector<TravelSeg*>::const_iterator travelSegEndI = pricingUnit.travelSeg().end();

    for (; travelSegI != travelSegEndI; ++travelSegI)
    {
      if ((*travelSegI)->segmentType() == Open && (*travelSegI)->isOpenWithoutDate())
      {
        openSegExist = true;
        break;
      }
    }

    if (openSegExist)
    {
      if (seasonRule->seasonDateAppl() == HIGH_SEASON)
      {
        // apply high_season SITA logic for the open segments:
        // -pass High_Season faras without checking other fields in rule.
        // -fail non High_season fares.
        if (paxTypeFare.fareClassAppInfo()->_seasonType == HIGH_SEASON)
        {
          if (UNLIKELY(diag.isActive()))
          {
            diag << " SEASONS: PASS"
                 << " HIGH SEASON WITH OPEN SEGMENT\n";
          }
          return PASS;
        }
        else
        {
          if (UNLIKELY(diag.isActive()))
          {
            diag << " SEASONS: FAIL"
                 << " NOT HIGH SEASON WITH OPEN SEGMENT\n";
          }
          return FAIL;
        }
      }
      // otherwise, continue to check other fields in rule.
    }
  }

  //---------------------------------------------------------------------------
  // Set scope type
  // System assumption:
  // - Validate Seasons dates against the departure date from the origin
  //   of the Pricing Unit (subjourney);
  // - Assumption Override Tag may be set indicate the fare component trip date
  //   must be use to validate against season dates
  //---------------------------------------------------------------------------

  RuleConst::TSIScopeParamType scopeParam =
      RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY; // Cat 3 is a default PU based rule

  if (seasonRule->assumptionOverride() == 'X')
  {
    scopeParam = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
  }

  //--------------------------------------------------------------------------

  // validate table 995 if directed to by seasonal record

  RuleUtil::TravelSegWrapperVector applTravelSegment;
  bool origCheck = true;
  bool destCheck = false;
  bool fltStopCheck = false;
  TSICode tsiReturn;
  LocKey locKey1Return;
  LocKey locKey2Return;

  if (seasonRule->geoTblItemNo())
  {
    DateTime tktDate = TrxUtil::getTicketingDT(trx);
    if (!RuleUtil::validateGeoRuleItem(seasonRule->geoTblItemNo(),
                                       paxTypeFare.vendor(),
                                       scopeParam,
                                       false,
                                       false,
                                       false,
                                       trx,
                                       farePath,
                                       itin,
                                       &pricingUnit,
                                       paxTypeFare.fareMarket(), // fare market
                                       tktDate,
                                       applTravelSegment, // this will contain the results
                                       origCheck,
                                       destCheck,
                                       fltStopCheck,
                                       tsiReturn,
                                       locKey1Return,
                                       locKey2Return,
                                       Diagnostic303))
    {
      LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - SKIP");

      if (diag.isActive())
      {
        diag << " SEASONS: SKIPPED "
             << "- NO GEO MATCH FROM TABLE 995\n";
      }

      return SKIP;
    }
  }
  else
  {
    if (seasonRule->assumptionOverride() != 'X') // PU scope
    {
      // take the first airseg from the pricing unit and put it into the
      // applTravelSegment vector because there was no 995 for populate it
      if (LIKELY(!pricingUnit.travelSeg().empty()))
      {
        std::vector<TravelSeg*>::const_iterator travelSegmentI;
        travelSegmentI = pricingUnit.travelSeg().begin();
        for (; travelSegmentI != pricingUnit.travelSeg().end(); ++travelSegmentI)
        {
          if (LIKELY((*travelSegmentI)->isAir()))
          {
            RuleUtil::TravelSegWrapper* tsw = nullptr;
            trx.dataHandle().get(tsw);
            // lint --e{413}
            tsw->travelSeg() = *travelSegmentI;
            tsw->origMatch() = origCheck;
            tsw->destMatch() = destCheck;
            applTravelSegment.push_back(tsw);
            break;
          }
        }
      }
      else
      {
        LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - FAIL");

        if (diag.isActive())
        {
          diag << " SEASONS: FAILED "
               << "- PRICING UNIT TRAVEL SEG EMPTY\n";
        }

        return FAIL;
      }
    }
    else // FareComponent scope
    {
      // take the first airseg from the fare market and put it into the
      // applTravelSegment vector because there was no 995 for populate it

      if (LIKELY(!fareMarket.travelSeg().empty()))
      {

        std::vector<TravelSeg*>::const_iterator travelSegmentI;
        travelSegmentI = fareMarket.travelSeg().begin();
        for (; travelSegmentI != fareMarket.travelSeg().end(); ++travelSegmentI)
        {
          if (LIKELY((*travelSegmentI)->isAir()))
          {
            RuleUtil::TravelSegWrapper* tsw = nullptr;
            trx.dataHandle().get(tsw);
            // lint --e{413}
            tsw->travelSeg() = *travelSegmentI;
            tsw->origMatch() = origCheck;
            tsw->destMatch() = destCheck;
            applTravelSegment.push_back(tsw);
            break;
          }
        }
      }
      else
      {
        LOG4CXX_INFO(
            _logger,
            " Leaving SeasonalApplication::checkGeoTblItemNo() travel segment empty - FAIL");
        return FAIL;
      }
    }
  }

  RuleUtil::TravelSegWrapperVectorCI applTravelSegmentI;
  applTravelSegmentI = applTravelSegment.begin();

  if (diag.isActive())
  {
    if (!applTravelSegment.empty())
    {
      const TravelSeg* ts = (*applTravelSegmentI)->travelSeg();
      if ((*applTravelSegmentI)->origMatch())
      {
        diag << "DEPART DATE: " << ts->departureDT().dateToIsoExtendedString() << "\n";
      }
      if ((*applTravelSegmentI)->destMatch())
      {
        diag << "ARRIVE DATE: " << ts->arrivalDT().dateToIsoExtendedString() << "\n";
      }
    }
  }

  // loop through the travel segs and validate the season on the first none
  // open segment

  for (; applTravelSegmentI != applTravelSegment.end(); ++applTravelSegmentI)
  {
    const TravelSeg* ts = (*applTravelSegmentI)->travelSeg();

    if (UNLIKELY(ts->openSegAfterDatedSeg() && noPNRTrx == nullptr))
    {
      continue;
    }

    if (LIKELY((*applTravelSegmentI)->origMatch()))
    {

      // fail if date does not fall within range

      // for WQ trx create updated date for validation
      DateTime checkedDT = ts->departureDT();
      if (UNLIKELY(noPNRTrx != nullptr))
        noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

      if (!checkedDT.isBetween(seasonRule->tvlstartyear(),
                               seasonRule->tvlstartmonth(),
                               seasonRule->tvlstartDay(),
                               seasonRule->tvlStopyear(),
                               seasonRule->tvlStopmonth(),
                               seasonRule->tvlStopDay()))
      {
        LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - FAIL");

        if (diag.isActive())
        {
          diag << " SEASONS: FAILED "
               << "- ORIGIN DATE CHECK\n";
        }

        return FAIL;
      }
    }

    if (UNLIKELY((*applTravelSegmentI)->destMatch()))
    {
      // fail if date does not fall within range

      // for WQ trx create updated date for validation
      DateTime checkedDT = ts->arrivalDT();
      if (noPNRTrx != nullptr)
        noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

      if (!checkedDT.isBetween(seasonRule->tvlstartyear(),
                               seasonRule->tvlstartmonth(),
                               seasonRule->tvlstartDay(),
                               seasonRule->tvlStopyear(),
                               seasonRule->tvlStopmonth(),
                               seasonRule->tvlStopDay()))
      {
        LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - FAIL");

        if (diag.isActive())
        {
          diag << " SEASONS: FAILED "
               << "-  DEST DATE CHECK\n";
        }

        return FAIL;
      }
    }

    break; // only validate the first point
  }

  LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::validate() - PASS");

  if (diag.isActive())
  {
    diag << " SEASONS: PASSED "
         << " - COMPLETED ALL VALIDATION CODE\n";
  }

  return PASS;
}

Record3ReturnTypes
SeasonalApplication::checkUnavailableAndText(const SeasonalAppl* seasonRule) const
{
  //----------------------------------------------------------------------
  // Check the Unavailable Data Tag
  //----------------------------------------------------------------------
  /// @todo Move some levels up?
  LOG4CXX_INFO(_logger, "  Entered SeasonalApplication::checkUnavailableAndText()");

  // Incomplete data?
  if (UNLIKELY(seasonRule->unavailtag() == dataUnavailable))
  {
    LOG4CXX_INFO(_logger, "  Leaving SeasonalApplication::checkUnavailableAndText() - FAIL ");
    return FAIL; // Yes, fail this fare
  }

  if (seasonRule->unavailtag() == textOnly)
  // Text data only?
  {
    LOG4CXX_INFO(_logger, "  Leaving SeasonalApplication::checkUnavailableAndText() - SKIP ");
    return SKIP; // Yes, skip this category
  }

  LOG4CXX_INFO(_logger, "  Leaving SeasonalApplication::checkUnavailableAndText() - PASS ");
  return PASS;
}

Record3ReturnTypes
SeasonalApplication::checkHighSeasonOpenSeg(const SeasonalAppl* seasonRule,
                                            const PaxTypeFare& paxTypeFare,
                                            const std::vector<TravelSeg*>& travelSeg) const
{
  //----------------------------------------------------------------------
  // Check for Open segments in HIGH_SEASON
  //----------------------------------------------------------------------
  LOG4CXX_INFO(_logger, "  Entered SeasonalApplication::checkHighSeasonOpenSeg()");

  bool openSegExist = false;

  std::vector<TravelSeg*>::const_iterator travelSegI = travelSeg.begin(),
                                          travelSegEndI = travelSeg.end();

  for (; travelSegI != travelSegEndI; ++travelSegI)
  {
    if ((*travelSegI)->segmentType() == Open && (*travelSegI)->isOpenWithoutDate())
    {
      openSegExist = true;
      break;
    }
  }

  if (openSegExist)
  {
    if (seasonRule->seasonDateAppl() == HIGH_SEASON)
    {
      // apply high_season SITA logic for the open segments:
      // -pass High_Season faras without checking other fields in rule.
      // -fail non High_season fares.
      if (paxTypeFare.fareClassAppInfo()->_seasonType == HIGH_SEASON)
      {
        LOG4CXX_INFO(_logger, "  Leaving SeasonalApplication::checkHighSeasonOpenSeg() - PASS ");
        return PASS;
      }
      else
      {
        LOG4CXX_INFO(_logger, "  Leaving SeasonalApplication::checkHighSeasonOpenSeg() - FAIL ");
        return FAIL;
      }
    }
  } // otherwise, continue to check other fields in rule.

  LOG4CXX_INFO(_logger, "  Leaving SeasonalApplication::checkHighSeasonOpenSeg() - SKIP ");
  return SKIP;
}

namespace
{
inline bool
isBetween(const DateTime& date, const SeasonalAppl& rule)
{
  return date.isBetween(rule.tvlstartyear(),
                        rule.tvlstartmonth(),
                        rule.tvlstartDay(),
                        rule.tvlStopyear(),
                        rule.tvlStopmonth(),
                        rule.tvlStopDay());
}
}

Record3ReturnTypes
SeasonalApplication::checkAssumptionOverride(const SeasonalAppl* seasonRule,
                                             Itin& itin,
                                             PricingTrx* ptrx) const
{
  LOG4CXX_INFO(_logger, "  Entered SeasonalApplication::checkAssumptionOverride()");

  bool isWQTrx = (dynamic_cast<NoPNRPricingTrx*>(ptrx) != nullptr);
  bool hasNoDateSeg = false;
  // validate table 994 if directed to by seasonal record
  if ((seasonRule->assumptionOverride() != 'X'))
  {
    std::vector<TravelSeg*>::const_iterator travelSegmentI;
    std::vector<TravelSeg*>::const_iterator travelSegmentIEnd;
    travelSegmentI = itin.travelSeg().begin();
    travelSegmentIEnd = itin.travelSeg().end();
    uint16_t segPass = 0;

    for (; travelSegmentI != travelSegmentIEnd; ++travelSegmentI)
    {
      const TravelSeg* ts = *travelSegmentI;
      if (UNLIKELY(isWQTrx && ts->segmentType() == Open && ts->hasEmptyDate()))
      {
        hasNoDateSeg = true;
      }
      if ((ts->segmentType() == Open && ItinUtil::isOpenSegAfterDatedSeg(itin, ts)) ||
          ts->departureDT().isBetween(seasonRule->tvlstartyear(),
                                      seasonRule->tvlstartmonth(),
                                      seasonRule->tvlstartDay(),
                                      seasonRule->tvlStopyear(),
                                      seasonRule->tvlStopmonth(),
                                      seasonRule->tvlStopDay()))
      {
        segPass++;
      }
    }
    if (UNLIKELY(isWQTrx && hasNoDateSeg))
    {
      LOG4CXX_INFO(_logger, " SeasonalApplication::checkAssumptionOverride() - SOFTPASS");
      return SOFTPASS;
    }

    if (segPass == 0)
    {
      LOG4CXX_INFO(_logger, " SeasonalApplication::checkAssumptionOverride() - FAIL");
      return FAIL;
    }
    else if (segPass == itin.travelSeg().size())
    {
      LOG4CXX_INFO(_logger, " SeasonalApplication::checkAssumptionOverride() - PASS");
      return PASS;
    }
    else
    {
      LOG4CXX_INFO(_logger, " SeasonalApplication::checkAssumptionOverride() - SOFTPASS");
      return SOFTPASS;
    }

  } // end of if statement

  LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::checkAssumptionOverride() - SKIP");
  return SKIP;
}

Record3ReturnTypes
SeasonalApplication::checkOverrideForDateRange(const SeasonalAppl* seasonRule,
                                               const FareMarket& fareMarket) const
{
  LOG4CXX_INFO(_logger, "  Entered SeasonalApplication::checkAssumptionOverride()");

  // validate table 995 if directed to by seasonal record
  if ((seasonRule->assumptionOverride() != 'X'))
  {
    std::vector<TravelSeg*>::const_iterator travelSegmentI;
    std::vector<TravelSeg*>::const_iterator travelSegmentIEnd;
    travelSegmentI = fareMarket.travelSeg().begin();
    travelSegmentIEnd = fareMarket.travelSeg().end();
    uint16_t segPass = 0;

    for (; travelSegmentI != travelSegmentIEnd; ++travelSegmentI)
    {
      const TravelSeg* ts = *travelSegmentI;
      if (ts->earliestDepartureDT().isRangeInBetween(seasonRule->tvlstartyear(),
                                                     seasonRule->tvlstartmonth(),
                                                     seasonRule->tvlstartDay(),
                                                     seasonRule->tvlStopyear(),
                                                     seasonRule->tvlStopmonth(),
                                                     seasonRule->tvlStopDay(),
                                                     ts->latestDepartureDT()))

      {
        segPass++;
      }
    }

    if (segPass == 0)
    {
      LOG4CXX_INFO(_logger, " SeasonalApplication::checkAssumptionOverride() - FAIL");
      return FAIL;
    }
    else if (segPass == fareMarket.travelSeg().size())
    {
      LOG4CXX_INFO(_logger, " SeasonalApplication::checkAssumptionOverride() - PASS");
      return PASS;
    }
    else
    {
      LOG4CXX_INFO(_logger, " SeasonalApplication::checkAssumptionOverride() - SOFTPASS");
      return SOFTPASS;
    }

  } // end of if statement

  LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::checkAssumptionOverride() - SKIP");
  return SKIP;
}

Record3ReturnTypes
SeasonalApplication::checkGeoTblItemNo(const SeasonalAppl* seasonRule,
                                       const PaxTypeFare& paxTypeFare,
                                       const FareMarket& fareMarket,
                                       Itin& itin,
                                       PricingTrx& trx) const
{
  LOG4CXX_INFO(_logger, " Entered SeasonalApplication::checkGeoTblItemNo()");

  RuleUtil::TravelSegWrapperVector applTravelSegment;
  bool origCheck = true;
  bool destCheck = false;
  bool shopIsSimpleTrip = false;

  if (trx.isAltDates() && trx.getTrxType() == PricingTrx::IS_TRX)
  {
    ShoppingTrx* shopTrx = static_cast<ShoppingTrx*>(&trx);
    if (LIKELY((shopTrx != nullptr) && (shopTrx->isSimpleTrip()) && (shopTrx->isRuleTuningISProcess())))
    {
      shopIsSimpleTrip = true;
    }
  }

  // validate table 995 if directed to by seasonal record
  if (seasonRule->geoTblItemNo())
  {
    if (!shopIsSimpleTrip)
    {
      RuleConst::TSIScopeParamType scopeParam =
          RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY; // Cat 3 is a default PU based rule

      if (LIKELY(seasonRule->assumptionOverride() == 'X'))
      {
        scopeParam = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
      }

      bool origCheck = true;
      bool destCheck = false;
      bool fltStopCheck = false;
      TSICode tsiReturn;
      LocKey locKey1Return;
      LocKey locKey2Return;

      DateTime tktDate = TrxUtil::getTicketingDT(trx);
      if (!RuleUtil::validateGeoRuleItem(seasonRule->geoTblItemNo(),
                                         paxTypeFare.vendor(),
                                         scopeParam,
                                         false,
                                         false,
                                         false,
                                         trx,
                                         nullptr, // farePath,
                                         &itin,
                                         nullptr, // PU
                                         &fareMarket, // fare market
                                         tktDate,
                                         applTravelSegment, // this will contain the results
                                         origCheck,
                                         destCheck,
                                         fltStopCheck,
                                         tsiReturn,
                                         locKey1Return,
                                         locKey2Return,
                                         Diagnostic303))
      {
        LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::checkGeoTblItemNo() - SKIP");
        return SKIP;
      }
    } // if !shopSimpleTrip
    else if (LIKELY(!itin.travelSeg().empty()))
    {
      std::vector<TravelSeg*>::const_iterator travelSegmentI;
      travelSegmentI = itin.travelSeg().begin();
      TravelSeg* savedTvlSeg = nullptr;
      for (; travelSegmentI != itin.travelSeg().end(); ++travelSegmentI)
      {
        if (((*travelSegmentI)->geoTravelType() == GeoTravelType::International) ||
            ((*travelSegmentI)->segmentType() == Open))
        {
          RuleUtil::TravelSegWrapper* tsw = nullptr;
          trx.dataHandle().get(tsw);
          tsw->travelSeg() = *travelSegmentI;
          tsw->origMatch() = origCheck;
          tsw->destMatch() = destCheck;
          applTravelSegment.push_back(tsw);
          break;
        }
        else if (!savedTvlSeg)
        {
          savedTvlSeg = *travelSegmentI;
        }
      }
      if (UNLIKELY(applTravelSegment.empty() && savedTvlSeg))
      {
        RuleUtil::TravelSegWrapper* tsw = nullptr;
        trx.dataHandle().get(tsw);
        tsw->travelSeg() = savedTvlSeg;
        tsw->origMatch() = origCheck;
        tsw->destMatch() = destCheck;
        applTravelSegment.push_back(tsw);
      }
    }
  } // if geoTblItemNo
  else
  {
    // take the first airseg from the fare market and put it into the
    // applTravelSegment vector because there was no 995 for populate it

    if (LIKELY(!fareMarket.travelSeg().empty()))
    {
      std::vector<TravelSeg*>::const_iterator travelSegmentI;
      travelSegmentI = fareMarket.travelSeg().begin();
      for (; travelSegmentI != fareMarket.travelSeg().end(); ++travelSegmentI)
      {
        if (LIKELY((*travelSegmentI)->isAir()))
        {
          RuleUtil::TravelSegWrapper* tsw = nullptr;
          trx.dataHandle().get(tsw);
          // lint --e{413}
          tsw->travelSeg() = *travelSegmentI;
          tsw->origMatch() = origCheck;
          tsw->destMatch() = destCheck;
          applTravelSegment.push_back(tsw);
          break;
        }
      }
    }
    else
    {
      LOG4CXX_INFO(_logger,
                   " Leaving SeasonalApplication::checkGeoTblItemNo() travel segment empty - FAIL");
      return FAIL;
    }
  }

  RuleUtil::TravelSegWrapperVectorCI applTravelSegmentI;
  applTravelSegmentI = applTravelSegment.begin();

  // loop through the travel segs and validate the season
  for (; applTravelSegmentI != applTravelSegment.end(); ++applTravelSegmentI)
  {
    const TravelSeg* ts = (*applTravelSegmentI)->travelSeg();
    if (UNLIKELY(ts->segmentType() == Open && ItinUtil::isOpenSegAfterDatedSeg(itin, ts)))
    {
      continue;
    }

    if (LIKELY((*applTravelSegmentI)->origMatch()))
    {
      // fail if date does not fall within range
      if (!ts->departureDT().isBetween(seasonRule->tvlstartyear(),
                                       seasonRule->tvlstartmonth(),
                                       seasonRule->tvlstartDay(),
                                       seasonRule->tvlStopyear(),
                                       seasonRule->tvlStopmonth(),
                                       seasonRule->tvlStopDay()))
      {
        LOG4CXX_INFO(
            _logger,
            " Leaving SeasonalApplication::checkGeoTblItemNo() departdate not in range - FAIL");
        return FAIL;
      }
    }

    if (UNLIKELY((*applTravelSegmentI)->destMatch()))
    {
      // fail if date does not fall within range
      if (!ts->arrivalDT().isBetween(seasonRule->tvlstartyear(),
                                     seasonRule->tvlstartmonth(),
                                     seasonRule->tvlstartDay(),
                                     seasonRule->tvlStopyear(),
                                     seasonRule->tvlStopmonth(),
                                     seasonRule->tvlStopDay()))
      {
        LOG4CXX_INFO(
            _logger,
            " Leaving SeasonalApplication::checkGeoTblItemNo() arrivaldate not in range - FAIL");
        return FAIL;
      }
    }

    break; // only validate the first segment
  }
  LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::checkGeoTblItemNo - PASS");
  return PASS;
}

Record3ReturnTypes
SeasonalApplication::checkGeoForDateRange(const SeasonalAppl* seasonRule,
                                          const PaxTypeFare& paxTypeFare,
                                          const FareMarket& fareMarket,
                                          Itin& itin,
                                          PricingTrx& trx) const
{
  LOG4CXX_INFO(_logger, " Entered SeasonalApplication::checkGeoTblItemNo()");

  RuleUtil::TravelSegWrapperVector applTravelSegment;
  bool origCheck = true;
  bool destCheck = false;

  // validate table 995 if directed to by seasonal record
  if (seasonRule->geoTblItemNo())
  {

    RuleConst::TSIScopeParamType scopeParam = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;

    bool origCheck = true;
    bool destCheck = false;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey locKey1Return;
    LocKey locKey2Return;

    DateTime tktDate = TrxUtil::getTicketingDT(trx);
    if (!RuleUtil::validateGeoRuleItem(seasonRule->geoTblItemNo(),
                                       paxTypeFare.vendor(),
                                       scopeParam,
                                       false,
                                       false,
                                       false,
                                       trx,
                                       nullptr, // farePath,
                                       &itin,
                                       nullptr, // PU
                                       &fareMarket, // fare market
                                       tktDate,
                                       applTravelSegment, // this will contain the results
                                       origCheck,
                                       destCheck,
                                       fltStopCheck,
                                       tsiReturn,
                                       locKey1Return,
                                       locKey2Return,
                                       Diagnostic303))
    {
      LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::checkGeoTblItemNo() - SKIP");
      return SKIP;
    }
  } // if geoTblItemNo
  else
  {
    // take the first airseg from the fare market and put it into the
    // applTravelSegment vector because there was no 995 for populate it

    if (!fareMarket.travelSeg().empty())
    {

      std::vector<TravelSeg*>::const_iterator travelSegmentI;
      travelSegmentI = fareMarket.travelSeg().begin();
      for (; travelSegmentI != fareMarket.travelSeg().end(); ++travelSegmentI)
      {
        if (dynamic_cast<AirSeg*>(*travelSegmentI))
        {
          RuleUtil::TravelSegWrapper* tsw = nullptr;
          trx.dataHandle().get(tsw);
          // lint --e{413}
          tsw->travelSeg() = *travelSegmentI;
          tsw->origMatch() = origCheck;
          tsw->destMatch() = destCheck;
          applTravelSegment.push_back(tsw);
          break;
        }
      }
    }
    else
    {
      LOG4CXX_INFO(_logger,
                   " Leaving SeasonalApplication::checkGeoTblItemNo() travel segment empty - FAIL");
      return FAIL;
    }
  }

  RuleUtil::TravelSegWrapperVectorCI applTravelSegmentI;
  applTravelSegmentI = applTravelSegment.begin();

  // loop through the travel segs and validate the season
  for (; applTravelSegmentI != applTravelSegment.end(); ++applTravelSegmentI)
  {

    const TravelSeg* ts = (*applTravelSegmentI)->travelSeg();
    if (ts->segmentType() == Open && ItinUtil::isOpenSegAfterDatedSeg(itin, ts))
    {
      continue;
    }

    if ((*applTravelSegmentI)->origMatch())
    {
      // fail if date does not fall within range
      if (!ts->earliestDepartureDT().isRangeInBetween(seasonRule->tvlstartyear(),
                                                      seasonRule->tvlstartmonth(),
                                                      seasonRule->tvlstartDay(),
                                                      seasonRule->tvlStopyear(),
                                                      seasonRule->tvlStopmonth(),
                                                      seasonRule->tvlStopDay(),
                                                      ts->latestDepartureDT()))

      {
        LOG4CXX_INFO(
            _logger,
            " Leaving SeasonalApplication::checkGeoTblItemNo() departdate not in range - FAIL");
        return FAIL;
      }
    }

    if ((*applTravelSegmentI)->destMatch())
    {
      // fail if date does not fall within range
      if (!ts->earliestArrivalDT().isRangeInBetween(seasonRule->tvlstartyear(),
                                                    seasonRule->tvlstartmonth(),
                                                    seasonRule->tvlstartDay(),
                                                    seasonRule->tvlStopyear(),
                                                    seasonRule->tvlStopmonth(),
                                                    seasonRule->tvlStopDay(),
                                                    ts->latestArrivalDT()))

      {
        LOG4CXX_INFO(
            _logger,
            " Leaving SeasonalApplication::checkGeoTblItemNo() arrivaldate not in range - FAIL");
        return FAIL;
      }
    }
  }
  LOG4CXX_INFO(_logger, " Leaving SeasonalApplication::checkGeoTblItemNo - PASS");
  return PASS;
}

void
SeasonalApplication::diagRestriction(const SeasonalAppl& seasonRule, DiagManager& diag) const
{
  diag << "SEASON START DATE: ";
  if (seasonRule.tvlstartyear() != 0)
    diag << seasonRule.tvlstartyear();
  else
    diag << "  ";
  diag << "-" << std::setw(2) << std::setfill('0') << seasonRule.tvlstartmonth() << "-"
       << std::setw(2) << std::setfill('0') << seasonRule.tvlstartDay() << "\n";

  diag << "SEASON STOP  DATE: ";
  if (seasonRule.tvlStopyear() != 0)
    diag << seasonRule.tvlStopyear();
  else
    diag << "  ";
  diag << "-" << std::setw(2) << std::setfill('0') << seasonRule.tvlStopmonth() << "-"
       << std::setw(2) << std::setfill('0') << seasonRule.tvlStopDay() << "\n";
}

} // tse
