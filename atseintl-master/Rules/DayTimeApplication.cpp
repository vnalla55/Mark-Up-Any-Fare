#include "Rules/DayTimeApplication.h"

#include "Common/DateTime.h"
#include "Common/Logger.h"
#include "Common/TravelSegUtil.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/LocKey.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(apo45023ApplyCat2DefaultsInOOJPU)
Logger
DayTimeApplication::_logger("atseintl.Rules.DayTimeApplication");

void
DayTimeApplication::initialize(const DayTimeAppInfo* ruleItemInfo)
{
  _itemInfo = ruleItemInfo;
}

Record3ReturnTypes
DayTimeApplication::validate(PricingTrx& trx,
                             Itin& itin,
                             const PaxTypeFare& paxTypeFare,
                             const RuleItemInfo* rule,
                             const FareMarket& fareMarket)
{
  Record3ReturnTypes retval = checkUnavailableAndText();
  if (retval != PASS)
    return retval;

  DiagManager diag(trx, Diagnostic302);
  //----------------------------------------------------------------------
  // Validate the Override Date Table (994)
  //----------------------------------------------------------------------
  // Moved one level up

  //----------------------------------------------------------------------
  // Validate the Geographic Specification Table (995)
  //----------------------------------------------------------------------

  // Define a pointer for the Application Point
  const TravelSeg* pItinAP = nullptr;

  // We should support arrival TSI. According to latest ATPCO document,
  // Cat2 does not use TSI that check both departure and arrival,
  // so we do not check arrival time unless origCheck is set false by
  // geoTbl (TSI) call.
  bool origCheck = true;

  RuleUtil::TravelSegWrapperVector appTravelSegs;
  const Fare& fare = *paxTypeFare.fare();
  if (_itemInfo->geoTblItemNo() != 0)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "GEO SPECIFICATION:\n";
      RuleUtil::diagGeoTblItem(_itemInfo->geoTblItemNo(), fare.vendor(), trx, diag.collector());
      diag << "\n";
    }

    // System assumption
    RuleConst::TSIScopeParamType scope = (_itemInfo->dayTimeAppl() == subJourneyBased)
                                             ? RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY
                                             : RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;

    PricingUnit* pm = nullptr;
    FarePath* farePath = nullptr;
    retval = callGeoSpecTable(fare, &fareMarket, pm, farePath, trx, appTravelSegs, scope);

    if (retval != PASS)
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << " RETURN FROM GEO TABLE CALL:";
        if (retval == SKIP)
        {
          diag << " SKIP\n";
          diag << " DAYTIMEAPP - PASS\n";
        }
        else
          diag << " FAIL\n";
      }
      return retval;
    }

    if (LIKELY(appTravelSegs.begin() != appTravelSegs.end()))
    {
      pItinAP = (*(appTravelSegs.begin()))->travelSeg();
      if (UNLIKELY(diag.isActive()))
      {
        diag << " GEO RETURN FIRST TRAVEL SEG: " << pItinAP->departureDT().toIsoExtendedString()
             << " " << pItinAP->origAirport() << " " << pItinAP->destAirport() << "\n";
      }
      origCheck = (*(appTravelSegs.begin()))->origMatch();
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
        diag << " GEO RETURN TRAVEL SEG EMPTY\n";
    }
  }
  else // No Geo Spec Table
  {
    // Apply system assumption:
    // The data applies to the departure day and time of the origin flight
    // of the fare component. However, the Application Tag field may
    // override this assumption and state that the data applies to the
    // departure from the origin of the pricing unit (subjourney).
    if (_itemInfo->dayTimeAppl() == subJourneyBased)
    {
      // Pricing Unit
      if (UNLIKELY(diag.isActive()))
        diag << " SOFTPASS - APPL IS SJ BASED\n";

      return SOFTPASS;
    }

    // Set up the Application Point: origin of the F.C. (fareUsage)
    // Skip the Arunk segment
    pItinAP = TravelSegUtil::firstNoArunkSeg(fareMarket.travelSeg());
  }

  //----------------------------------------------------------------------
  // Validate Day and Time restrictions
  //----------------------------------------------------------------------

  // The use of an arrival TSI can override the departure application of
  // the category, causing the data to apply to the arrival at the point
  // specified by the TSI.
  // Therefore, the next check is needed to determine if the departure
  // assumption was overridden.
  DateTime travelDateTime;
  uint32_t travelDOW;
  bool openDate = true;
  bool openTime = true;
  NoPNRPricingTrx* noPnrTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);

  if (UNLIKELY(noPnrTrx && noPnrTrx->itin().front()->dateType() == Itin::NoDate))
  {
    // WQ: if there are no dates entered by user - revalidate at PU scope
    // (warning messages will have to be shown in such case)

    // If there is time of day restriction we must display a warning message.
    if (_itemInfo->startTime() != dayStart || _itemInfo->stopTime() != dayEnd)
      fare.warningMap().set(WarningMap::cat2_warning_2);

    // if there is day of week restriction, we must display warning
    if (_itemInfo->dow().size() > 0)
      fare.warningMap().set(WarningMap::cat2_warning_1);

    return SOFTPASS;
  }

  if (LIKELY(pItinAP))
  {
    if (origCheck)
    {
      travelDateTime = pItinAP->departureDT();
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
        diag << " TSI IS FOR ARRIVAL \n";
      travelDateTime = pItinAP->arrivalDT();
    }

    if (UNLIKELY(noPnrTrx))
    {
      openDate = false;
      noPnrTrx->updateOpenDateIfNeccesary(pItinAP, travelDateTime);
    }
    else
    {
      openDate = pItinAP->openSegAfterDatedSeg() || !travelDateTime.isValid();
    }

    openTime = pItinAP->segmentType() == Open;
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << " TVL DATE ";
    if (openDate)
      diag << "IS OPEN\n";
    else
      diag << travelDateTime.toIsoExtendedString() << "\n";
  }

  //----------------------------------------------------------------------
  // Validate Day of Week
  //----------------------------------------------------------------------
  bool skipTODcheck = openTime;

  if (LIKELY(!openDate))
  {
    travelDOW = travelDateTime.date().day_of_week().as_number();
    if (travelDOW == 0)
      travelDOW = 7; // In ATPCo Sunday is 7, not 0

    bool displayWqWarning = false;

    retval = validateDayOfWeek(travelDateTime, travelDOW, skipTODcheck, displayWqWarning);

    // apo-45023
    // does the rec 2 have a inOut set? is this an intl itin? is this not a first fc??
    if (!fallback::apo45023ApplyCat2DefaultsInOOJPU(&trx) )
    {
       bool intlItin =  (itin.geoTravelType() == GeoTravelType::International);
       bool isFirstFC = (paxTypeFare.fareMarket()->travelSeg().front() == itin.firstTravelSeg());
       if (intlItin && !isFirstFC && (retval != PASS) )
       {
         retval = SOFTPASS;
         return retval;
       }
    }
    if (retval != PASS)
    {
      if (UNLIKELY(diag.isActive()))
        diag << " FAIL - DAY OF WEEK\n";

      return retval;
    }

    if (UNLIKELY(displayWqWarning && noPnrTrx))
      fare.warningMap().set(WarningMap::cat2_warning_1);
  }

  if (!skipTODcheck || noPnrTrx)
  {
    bool displayWqWarning = false;

    retval = checkTOD(travelDateTime, displayWqWarning);

    if (UNLIKELY(noPnrTrx && displayWqWarning))
      fare.warningMap().set(WarningMap::cat2_warning_2);

    if (retval != PASS)
    {
      if (UNLIKELY(noPnrTrx))
      {
        retval = PASS;
      }
      else
      {
        if (UNLIKELY(diag.isActive()))
          diag << " FAIL - TIME OF DAY\n";

        return retval;
      }
    }
  }

  //------------------------------------------------------------------------
  //------------------------------------------------------------------------
  // Before we move forward and perform DOW Same and DOW Occurrence
  // validations, let's check the following:
  // - When we are in the single fuzzy or pricing phases, the real
  //   subjourney is not available yet
  // - To perform DOW Same and DOW Occurrence validations we need a specific
  //   travel date
  //------------------------------------------------------------------------
  //------------------------------------------------------------------------

  // There's no restriction to this fare at this point

  //----------------------------------------------------------------------
  // if there is no restriction on Day of Week - Same,
  // Day of Week - Occurrence, which can only be validated when PU is ready
  // we return PASS, otherwise we return SOFTPASS
  //----------------------------------------------------------------------
  if (UNLIKELY((_itemInfo->dowSame() == sameDay) || (_itemInfo->dowOccur() != 0)))
  {
    if (UNLIKELY(diag.isActive()))
      diag << " SOFTPASS - SAME/OCCUR\n";

    return SOFTPASS;
  }
  else
  {
    if (UNLIKELY(diag.isActive()))
      diag << " PASS\n";
  }

  return PASS;
}

Record3ReturnTypes
DayTimeApplication::validate(PricingTrx& trx,
                             const RuleItemInfo* rule,
                             const FarePath& farePath,
                             const PricingUnit& pricingUnit,
                             const FareUsage& fareUsage)
{
  return validate(trx, rule, &farePath, pricingUnit, fareUsage);
}

Record3ReturnTypes
DayTimeApplication::validate(PricingTrx& trx,
                             const RuleItemInfo* rule,
                             const FarePath* farePath,
                             const PricingUnit& pricingUnit,
                             const FareUsage& fareUsage) const
{
  Record3ReturnTypes retval = checkUnavailableAndText();
  if (UNLIKELY(retval != PASS))
    return retval;

  DiagManager diag(trx, Diagnostic302);
  //----------------------------------------------------------------------
  // Validate the Override Date Table (994)
  //----------------------------------------------------------------------
  // Moved one level up

  //----------------------------------------------------------------------
  // Validate the Geographic Specification Table (995)
  //----------------------------------------------------------------------

  // Define a pointer for the Application Point
  const TravelSeg* pItinAP = nullptr;

  // We should support arrival TSI. According to latest ATPCO document,
  // Cat2 does not use TSI that check both departure and arrival,
  // so we do not check arrival time unless origCheck is set false by
  // geoTbl (TSI) call.
  bool origCheck = true;

  RuleUtil::TravelSegWrapperVector appTravelSegs;
  std::vector<TravelSeg*> partOfTravel;

  const PaxTypeFare& paxTypeFare = *fareUsage.paxTypeFare();
  const Fare& fare = *paxTypeFare.fare();
  const FareMarket* fareMarket = paxTypeFare.fareMarket();

  if (_itemInfo->geoTblItemNo() != 0)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "GEO SPECIFICATION:\n";
      RuleUtil::diagGeoTblItem(_itemInfo->geoTblItemNo(), fare.vendor(), trx, diag.collector());
      diag << "\n";
    }

    // System assumption
    RuleConst::TSIScopeParamType scope = (_itemInfo->dayTimeAppl() == subJourneyBased)
                                             ? RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY
                                             : RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;

    retval = callGeoSpecTable(fare, fareMarket, &pricingUnit, farePath, trx, appTravelSegs, scope);
    if (UNLIKELY(retval != PASS))
    {

      if (UNLIKELY(diag.isActive()))
      {
        diag << " RETURN FROM GEO TABLE CALL:";
        if (retval == SKIP)
          diag << " SKIP\n";
        else
          diag << " FAIL\n";
      }

      return retval;
    }

    if (LIKELY(appTravelSegs.begin() != appTravelSegs.end()))
    {
      pItinAP = (*(appTravelSegs.begin()))->travelSeg();
      origCheck = (*(appTravelSegs.begin()))->origMatch();
    }
  }
  else // No Geo Spec Table
  {
    // Apply system assumption:
    // The data applies to the departure day and time of the origin flight
    // of the fare component. However, the Application Tag field may
    // override this assumption and state that the data applies to the
    // departure from the origin of the pricing unit (subjourney).

    // FC validation could be skipped because CategoryRuleItemInfo::inOutInd
    // is Inbound or Outbound while unknown about fare market is in/outbound
    if (_itemInfo->dayTimeAppl() != subJourneyBased)
    {
      // Skip the Arunk segment
      pItinAP = TravelSegUtil::firstNoArunkSeg(fareMarket->travelSeg());
    }
    else
    {
      // Set up the Application Point: origin of the P.U. (subjourney)
      // Skip the Arunk segment
      pItinAP = TravelSegUtil::firstNoArunkSeg(pricingUnit.travelSeg());
    }
  }

  //----------------------------------------------------------------------
  // Validate Day and Time restrictions
  //----------------------------------------------------------------------

  // The use of an arrival TSI can override the departure application of
  // the category, causing the data to apply to the arrival at the point
  // specified by the TSI.
  // Therefore, the next check is needed to determine if the departure
  // assumption was overridden.
  DateTime travelDateTime;
  uint32_t travelDOW;
  bool openDate = true;
  bool openTime = true;
  NoPNRPricingTrx* noPnrTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);

  if (LIKELY(pItinAP))
  {
    if (LIKELY(origCheck))
    {
      travelDateTime = pItinAP->departureDT();
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << " TSI IS FOR ARRIVAL \n";
      }
      travelDateTime = pItinAP->arrivalDT();
    }

    if (UNLIKELY(noPnrTrx))
    {
      openDate = false;

      if (noPnrTrx->itin().front()->dateType() == Itin::NoDate)
      {
        // find out, if validated segment is part of first fare component of the first pricing unit
        // for first segment - this is always true
        bool isFirstFareComponentOfFirstPU = (pItinAP->segmentOrder() == 1);
        if (!isFirstFareComponentOfFirstPU) // need some more checking
        {
          std::vector<TravelSeg*>::iterator tsIter =
              pricingUnit.fareUsage().front()->travelSeg().begin();
          std::vector<TravelSeg*>::iterator tsEnd =
              pricingUnit.fareUsage().front()->travelSeg().end();
          for (; tsIter != tsEnd; ++tsIter)
          {
            if ((*tsIter)->segmentOrder() == pItinAP->segmentOrder())
            {
              isFirstFareComponentOfFirstPU = true;
              break;
            }
          }
        }
        if (!isFirstFareComponentOfFirstPU)
        {
          // autopass the segment
          if (UNLIKELY(diag.isActive()))
          {
            diag << " NOPNR TRANSACTION - AUTOPASS\n";
          }
          return PASS;
        }
        // use today's date for validation
        travelDateTime = trx.itin().front()->bookingDate();
      }
      else
      {
        noPnrTrx->updateOpenDateIfNeccesary(pItinAP, travelDateTime);
      }
    }
    else
    {
      openDate = pItinAP->openSegAfterDatedSeg() || !travelDateTime.isValid();
    }

    openTime = pItinAP->segmentType() == Open;
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << " TVL DATE " << travelDateTime.toIsoExtendedString() << "\n";
  }

  //----------------------------------------------------------------------
  // Validate Day of Week
  //----------------------------------------------------------------------
  bool skipTODcheck = openTime;

  if (LIKELY(!openDate))
  {
    travelDOW = travelDateTime.date().day_of_week().as_number();
    if (travelDOW == 0)
      travelDOW = 7; // In ATPCo Sunday is 7, not 0

    bool displayWqWarning = false;

    retval = validateDayOfWeek(travelDateTime, travelDOW, skipTODcheck, displayWqWarning);

    //apo-45023: check if system defaults apply bcoz of intl. ooj pu and rec2 io byte settings
    if (!fallback::apo45023ApplyCat2DefaultsInOOJPU(&trx) )
    {
       if (applyCat2SystemDefaults() )
         retval = PASS;
    }
    if (retval != PASS)
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << " FAIL - DAY OF WEEK\n";
      }
      return retval;
    }

    if (UNLIKELY(displayWqWarning && noPnrTrx))
    {
      paxTypeFare.warningMap().set(WarningMap::cat2_warning_1);
    }
  }

  if (!skipTODcheck || noPnrTrx)
  {
    bool displayWqWarning = false;

    retval = checkTOD(travelDateTime, displayWqWarning);

    //apo-45023: if intl ooj pu and rec. 2 io byte is set, then apply system.assumptions to pass
    if (!fallback::apo45023ApplyCat2DefaultsInOOJPU(&trx) )
    {
       if (applyCat2SystemDefaults() ) 
         retval = PASS;
    }

    if (retval != PASS)
    {
      if (noPnrTrx)
      {
        retval = PASS;
      }
      else
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << " FAIL - TIME OF DAY\n";
        }
        return retval;
      }
    }

    if (LIKELY(displayWqWarning))
    {
      paxTypeFare.warningMap().set(WarningMap::cat2_warning_2);
    }
  }

  //----------------------------------------------------------------------
  // Validate Day of Week - Same
  //----------------------------------------------------------------------
  if (UNLIKELY(_itemInfo->dowSame() == sameDay))
  {
    retval = checkDOWSame(pricingUnit, farePath, fare, trx);

    //apo-45023: if intl ooj pu and rec. 2 io byte is set, then apply system.assumptions to pass
    if (!fallback::apo45023ApplyCat2DefaultsInOOJPU(&trx) )
    {
       if (applyCat2SystemDefaults() )
         retval = PASS;
    }

    if (retval != PASS)
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << " FAIL - SAME DAY OF WEEK\n";
      }
      return retval;
    }
  }

  //----------------------------------------------------------------------
  // Validate Day of Week - Occurrence
  //----------------------------------------------------------------------
  retval = checkOccurrence(pricingUnit, farePath, fare, trx);

  //apo-45023: if intl ooj pu and rec. 2 io byte is set, then apply system.assumptions to pass
  if (!fallback::apo45023ApplyCat2DefaultsInOOJPU(&trx) )
  {
     if (applyCat2SystemDefaults() )
       retval = PASS;
  }
  if (UNLIKELY(retval != PASS))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " FAIL - DAY OF WEEK OCCURRENCE\n";
    }
    return retval;
  }

  // There's no restriction to this fare at this point
  return PASS;
} // DayTimeApplication::process

Record3ReturnTypes
DayTimeApplication::callGeoSpecTable(const Fare& fare,
                                     const FareMarket* fareMarket,
                                     const PricingUnit* pricingUnit,
                                     const FarePath* farePath,
                                     PricingTrx& trx,
                                     RuleUtil::TravelSegWrapperVector& appSegVec,
                                     RuleConst::TSIScopeParamType scope) const
{
  // The Table995::validate function will determine the point(s) of
  // application on the F.C., P.U. or Journey (depending upon the TSI).
  // It will then mark those Itinerary items that match the Geo
  // specification and/or TSI and set the corresponding indicators.

  const RuleConst::TSIScopeType origScope = (scope == RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT)
                                                ? RuleConst::TSI_SCOPE_FARE_COMPONENT
                                                : RuleConst::TSI_SCOPE_SUB_JOURNEY;

  const uint32_t itemNo = (uint32_t)_itemInfo->geoTblItemNo();
  const VendorCode& vendorCode = fare.vendor();

  if ((origScope == RuleConst::TSI_SCOPE_SUB_JOURNEY) && (pricingUnit == nullptr))
  {
    return SOFTPASS;
  }

  bool fltStopCheck;
  TSICode tsi;
  LocKey locKey1;
  LocKey locKey2;
  /// Do not allow any scope override
  bool ret = RuleUtil::validateGeoRuleItem(itemNo,
                                           vendorCode,
                                           scope,
                                           false,
                                           false,
                                           false,
                                           trx,
                                           farePath,
                                           nullptr,
                                           pricingUnit,
                                           fareMarket,
                                           trx.getRequest()->ticketingDT(),
                                           appSegVec,
                                           true,
                                           true,
                                           fltStopCheck,
                                           tsi,
                                           locKey1,
                                           locKey2,
                                           Diagnostic302,
                                           RuleUtil::LOGIC_AND);

  if (!ret)
    return SKIP;

  return PASS;
}

Record3ReturnTypes
DayTimeApplication::checkUnavailableAndText() const
{
  //----------------------------------------------------------------------
  // Check the Unavailable Data Tag
  //----------------------------------------------------------------------
  /// @todo Move some levels up?

  if (UNLIKELY(_itemInfo == nullptr))
  {
    LOG4CXX_ERROR(_logger, "DayTimeApplication - _itemInfo is null");
    return FAIL;
  }

  // Incomplete data?
  if (UNLIKELY(_itemInfo->unavailtag() == dataUnavailable))
  {
    return FAIL; // Yes, fail this fare
  }

  if (_itemInfo->unavailtag() == textOnly)
  // Text data only?
  {
    return SKIP; // Yes, skip this category
  }

  return PASS;
}

Record3ReturnTypes
DayTimeApplication::validateDayOfWeek(const DateTime& travelDate,
                                      uint32_t travelDow,
                                      bool& skipTODCheck,
                                      bool& displayWqWarning) const
{
  displayWqWarning = false;

  if (_itemInfo->dow().empty())
    return PASS;

  if (UNLIKELY(!travelDate.isValid()))
    return PASS;

  Record3ReturnTypes result = FAIL;
  // bool matchDOW = false;
  uint32_t dowSize = _itemInfo->dow().size();

  if (LIKELY(dowSize > 0))
    displayWqWarning = true;

  boost::posix_time::time_duration travelTime = travelDate.time_of_day();
  boost::posix_time::time_duration stopTime = boost::posix_time::minutes(_itemInfo->stopTime());
  boost::posix_time::time_duration startTime = boost::posix_time::minutes(_itemInfo->startTime());
  const bool isStartAtMidNight = (_itemInfo->startTime() <= 1);
  const bool isStopAtMidNight =
      (_itemInfo->stopTime() >= (int)(HOURS_PER_DAY * MINUTES_PER_HOUR - 1));
  const bool isWholeTOD = isStartAtMidNight && isStopAtMidNight;

  for (uint32_t i = 0; i < dowSize; ++i)
  {
    // Convert record 3 cat 2 dow
    uint32_t ruleDOW = _itemInfo->dow()[i] - '0';

    if (UNLIKELY(ruleDOW < 1 || ruleDOW > 7)) // End of valid days of week
      break;

    if (ruleDOW == travelDow)
    {
      // matchDOW = true;
      if (_itemInfo->todAppl() == range)
      // Range application
      {
        if (i == 0) // first in the range of days
        {
          if (skipTODCheck)
          {
            result = (isStartAtMidNight) ? PASS : SOFTPASS;
          }
          else if (travelTime >= startTime)
          {
            // set pass for this day
            result = PASS;
          }
        } // end if first day in range
        else if (i == (dowSize - 1))
        { // Last in the range
          // Travel times are later than stop time
          if (skipTODCheck)
          {
            result = (isStopAtMidNight) ? PASS : SOFTPASS;
          }
          else if (travelTime <= stopTime)
          {
            result = PASS;
          }
        } // end if last in range of days
        else
        {
          result = PASS;
        }
      } // end if range application
      else // not range application
      {
        if (skipTODCheck)
        {
          result = (isWholeTOD) ? PASS : SOFTPASS;
        }
        else
        {
          int32_t minutesSinceMidnight = travelTime.hours() * 60 + travelTime.minutes();

          // Daily application
          // Travel times are outside the start/stop time range
          if (((_itemInfo->startTime() < _itemInfo->stopTime()) &&
               (minutesSinceMidnight < _itemInfo->startTime() ||
                minutesSinceMidnight > _itemInfo->stopTime())) ||
              ((_itemInfo->startTime() > _itemInfo->stopTime()) &&
               (minutesSinceMidnight < _itemInfo->startTime() &&
                minutesSinceMidnight > _itemInfo->stopTime())))
          {
            result = FAIL;
          }
          else
          {
            result = PASS;
          }
        }
      }
      skipTODCheck = true; // already done checking TOD

      break; // break out of while loop

    } // end if ruleDOW matches

  } // For loop

  const bool isPositive = (_itemInfo->dayTimeNeg() == nonNegative);

  if (!isPositive)
  {
    skipTODCheck = true;
  }

  if (result == SOFTPASS)
  {
    // caused by skipTODCheck (open time)
    return PASS;
  }
  else if ((result == PASS) == isPositive)
  {
    // result PASS   isPostive
    // result FAIL   !isPositive
    return PASS;
  }
  else
  {
    return FAIL;
  }
}

Record3ReturnTypes
DayTimeApplication::checkDOWSame(const PricingUnit& pricingUnit,
                                 const FarePath* farePath,
                                 const Fare& fare,
                                 PricingTrx& trx) const
{
  // Only check if there's more than one Fare Component
  // in the subjourney (P.U.)
  if (pricingUnit.fareUsage().size() == 1)
  {
    return PASS;
  }

  uint32_t validDOW = DateTime::invalidDOW();
  // the DOW of the first Fare Component that has valid travel DOW,
  // initialized as invalidDOW

  const TravelSeg* pItinAP = nullptr;

  //--------------------------------------------------------------------
  // Loop thru all remaining Fare Component in this subjourney (P.U.)
  //--------------------------------------------------------------------
  std::vector<FareUsage*>::const_iterator fuIT, fuEND;
  fuIT = pricingUnit.fareUsage().begin();
  fuEND = pricingUnit.fareUsage().end();
  RuleUtil::TravelSegWrapperVector appTravelSegs;

  for (; fuIT != fuEND; fuIT++)
  {
    bool origCheck = true;

    // Validate the Geographic Specification Table (995)
    if (_itemInfo->geoTblItemNo() != 0)
    {
      // System assumption
      RuleConst::TSIScopeParamType scope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
      // lint -e{530}
      FareMarket* fareMarket = (*fuIT)->paxTypeFare()->fareMarket();

      Record3ReturnTypes retval =
          callGeoSpecTable(fare, fareMarket, &pricingUnit, farePath, trx, appTravelSegs, scope);
      if (retval == SKIP)
      {
        continue;
      }
      else if (retval != PASS)
      {
        return retval;
      }
      if (appTravelSegs.begin() != appTravelSegs.end())
      {
        pItinAP = (*(appTravelSegs.begin()))->travelSeg();
        origCheck = (*(appTravelSegs.begin()))->origMatch();
      }
    }
    else // No Geo Spec Table
    {
      // Apply system assumption:
      // The data applies to the departure day and time of the origin
      // flight of the fare component.

      // Set up the Application Point: origin of the F.C. (fareUsage)
      pItinAP = TravelSegUtil::firstNoArunkSeg((*fuIT)->travelSeg());
    }

    if (!pItinAP)
    {
      continue;
    }

    // The use of an arrival TSI can override the departure
    // application of the category, causing the data to apply to
    // the arrival at the point specified by the TSI.
    // Therefore, the next check is needed to determine if
    // the departure assumption was overridden.
    // NOTE: At this point we know for sure that earliest and
    //       latest dates are the same
    uint32_t travelDOW = 0;

    const DateTime& tvlDate = (origCheck) ? pItinAP->departureDT() : pItinAP->arrivalDT();

    if (pItinAP->openSegAfterDatedSeg() || !tvlDate.isValid())
    {
      // OpenSeg with OpenDate or EmptyDate
      continue;
    }

    travelDOW = tvlDate.date().day_of_week();

    // Fail this fare if the DOW of the Travel date of this
    // Fare Component is not the same as the one of the Fare
    // Component already validated
    if (validDOW == DateTime::invalidDOW())
    {
      validDOW = travelDOW;
    }
    else
    {
      if (travelDOW != validDOW)
        return FAIL;
    }
  } // loop all fare component

  return PASS;
}

Record3ReturnTypes
DayTimeApplication::checkOccurrence(const PricingUnit& pricingUnit,
                                    const FarePath* farePath,
                                    const Fare& fare,
                                    PricingTrx& trx) const
{
  // DOW Occurrence field is set and there's more than one Fare Component
  // in the subjourney (P.U.)
  if (UNLIKELY(_itemInfo->dowOccur() != 0 && pricingUnit.fareUsage().size() != 1))
  {
    if (_itemInfo->dow().empty())
      // Need range of days in the DOW field: invalid data
      return FAIL;

    DateTime travelDatePrev; // Travel date of previous Fare Component
    int32_t travelDOWPrev = -1;

    //--------------------------------------------------------------------
    // Loop thru all Fare Component of this subjourney (P.U.)
    //--------------------------------------------------------------------
    std::vector<FareUsage*>::const_iterator fuIT, fuEND, fuBEG;
    fuIT = pricingUnit.fareUsage().begin();
    fuEND = pricingUnit.fareUsage().end();
    fuBEG = fuIT;
    RuleUtil::TravelSegWrapperVector appTravelSegs;
    // Record3ReturnTypes retval;
    const TravelSeg* pItinAP = nullptr;

    while (fuIT != fuEND)
    {
      // Validate the Geographic Specification Table (995)
      if (_itemInfo->geoTblItemNo() != 0)
      {
        RuleConst::TSIScopeParamType scope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;

        FareMarket* fareMarket = nullptr;
        callGeoSpecTable(fare, fareMarket, &pricingUnit, farePath, trx, appTravelSegs, scope);

        if (appTravelSegs.begin() != appTravelSegs.end())
        {
          pItinAP = (*(appTravelSegs.begin()))->travelSeg();
        }
      }
      else // No Geo Spec Table
      {
        // Apply system assumption:
        // The data applies to the departure day and time of the origin
        // flight of the fare component.

        // Set up the Application Point: origin of the F.C. (fareUsage)
        pItinAP = TravelSegUtil::firstNoArunkSeg((*fuIT)->travelSeg());
      }

      if (!pItinAP)
      {
        ++fuIT; // Point to next Fare Component
        continue;
      }

      if (pItinAP->openSegAfterDatedSeg())
      {
        // all segments from now on are open, pass occurrence check
        break;
      }
      const DateTime& travelDate = pItinAP->departureDT();
      // Travel date of Fare Component
      int32_t travelDOW = pItinAP->departureDT().date().day_of_week();

      // Check if this Fare Component meets the DOW restriction
      bool matchDOW = false;
      int32_t dowSize = _itemInfo->dow().size();
      // Set all days to fail in the date vector because soon we
      // will set some to pass
      // retval = FAIL;
      for (int i = 0; i < dowSize; ++i)
      {

        // Convert record 3 cat 2 dow
        int32_t ruleDOW = _itemInfo->dow()[i] - '0';

        if (ruleDOW < 1 || ruleDOW > 7) // End of valid days of week
          break;

        if (ruleDOW == travelDOW)
        {
          matchDOW = true;
          break; // break out of while loop
        }
      } // For loop

      if (!matchDOW) // No match
        return FAIL;

      // Validate the occurrence condition
      // Make sure this F.C. is not the 1st one within this subjourney
      if (travelDOWPrev == -1)
      {
        // first one
        travelDOWPrev = travelDOW;
        travelDatePrev = travelDate;
        ++fuIT;
        continue;
      }

      // Fail this fare if the Travel date of this Fare Component is not
      // on the specified occurrence from the preceding Fare Component
      int occurDOW = _itemInfo->dowOccur();

      if (!occurDOW) // invalid data
        return FAIL;

      const int32_t DOW = travelDOW;
      const int32_t DOWPrev = travelDOWPrev;

      if (_itemInfo->todAppl() == range) // Range application
      {
        // The DOW Range is considered a set, and the occurrence of
        // a specific DOW should be found in another set.

        int32_t firstDOW = _itemInfo->dow()[0] - '0';

        // Modify the travel date of the preceding fare component with
        // the acceptable travel date for the current fare component

        if ((DOW < firstDOW) && (DOWPrev >= firstDOW))
          travelDatePrev += boost::gregorian::days((occurDOW * DAYS_PER_WEEK) +
                                                   ((DAYS_PER_WEEK - DOWPrev) + DOW));
        else if ((DOW >= firstDOW) && (DOWPrev < firstDOW))
          travelDatePrev += boost::gregorian::days((occurDOW * DAYS_PER_WEEK) -
                                                   ((DAYS_PER_WEEK - DOW) + DOWPrev));
        else
          travelDatePrev += boost::gregorian::days((occurDOW * DAYS_PER_WEEK) + (DOW - DOWPrev));
      }
      else // Daily application
      {
        if (DOW > DOWPrev)
          occurDOW--;

        // Modify the travel date of the preceding fare component
        // with the acceptable travel date for the current
        // fare component
        travelDatePrev += boost::gregorian::days((occurDOW * DAYS_PER_WEEK) + (DOW - DOWPrev));
      }

      if (travelDatePrev.date() != travelDate.date())
        return FAIL;

      travelDatePrev = travelDate;
      travelDOWPrev = travelDOW;

      ++fuIT; // Point to next Fare Component
    } // while
  }
  return PASS;
}

Record3ReturnTypes
DayTimeApplication::checkTOD(const DateTime& travelDateTime, bool& displayWqWarning) const
{
  Record3ReturnTypes retval;
  int32_t minutesSinceMidnight =
      travelDateTime.time_of_day().hours() * 60 + travelDateTime.time_of_day().minutes();

  // If there is time of day restriction we must display a warning message.
  if (_itemInfo->startTime() != dayStart || _itemInfo->stopTime() != dayEnd)
  {
    displayWqWarning = true;
  }

  // Daily application
  if (LIKELY(_itemInfo->todAppl() != range))
  {
    // Travel times are outside the start/stop time range
    if (((_itemInfo->startTime() < _itemInfo->stopTime()) &&
         (minutesSinceMidnight < _itemInfo->startTime() ||
          minutesSinceMidnight > _itemInfo->stopTime())) ||
        ((_itemInfo->startTime() > _itemInfo->stopTime()) &&
         (minutesSinceMidnight < _itemInfo->startTime() &&
          minutesSinceMidnight > _itemInfo->stopTime())))
    {
      if (LIKELY(_itemInfo->dayTimeNeg() != negative)) // Positive application
      {
        retval = FAIL;

        return retval;
      }
      else // Negative application
      {
        // set date vector days to pass
        retval = PASS;

        return retval;
      }
    }

    // Travel times are inside the start/stop time range
    if (_itemInfo->dayTimeNeg() == negative)
    {
      retval = FAIL;

      return retval;
    }
  } // end if NOT range

  return PASS;
}

} // tse
