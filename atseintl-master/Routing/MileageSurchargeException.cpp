#include "Routing/MileageSurchargeException.h"

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RoutingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/MileageSurchCxr.h"
#include "DBAccess/MileageSurchExcept.h"
#include "DBAccess/Routing.h"
#include "Routing/RoutingConsts.h"
#include "Routing/TravelRoute.h"
#include "Rules/RuleUtil.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackMileageSurchargeExceptionValidation);

static Logger
logger("atseintl.Routing.MileageSurchargeException");

void
MileageSurchargeException::validate(const TravelRoute& tvlRoute,
                                    std::vector<PaxTypeFare*>& pFares,
                                    PricingTrx& trx,
                                    MileageInfo& mInfo) const
{
  LOG4CXX_INFO(logger, " Entered MileageSurcharageException::validate()");

  if (pFares.empty())
    return;

  std::vector<PaxTypeFare*>::iterator i(pFares.begin());
  for (; i != pFares.end(); ++i)
  {
    if (fallback::fixed::fallbackMileageSurchargeExceptionValidation())
    {
      if ((*i)->mileageSurchargePctg() == 0)
        continue;

      PaxTypeFare& fare = **i;

      std::vector<MileageSurchExcept*> validMarkets = getData(fare, trx);

      if (!validMarkets.empty())
      {
        std::vector<MileageSurchExcept*>::iterator j(validMarkets.begin()),
            jend(validMarkets.end());
        const TravelRoute* currentTvlRoute = &tvlRoute;

        if (TrxUtil::isFullMapRoutingActivated(trx))
        {
          const Routing* routing = RoutingUtil::getRoutingData(trx, **i, (*i)->routingNumber());
          if (routing && routing->unticketedPointInd() == TKTPTS_TKTONLY &&
              currentTvlRoute->travelRouteTktOnly())
          {
            currentTvlRoute = currentTvlRoute->travelRouteTktOnly();
          }
        }
        for (; j != jend; ++j)
        {
          if (!isExceptionApplicable(**j, fare, *currentTvlRoute, trx))
          {
            continue;
          }
          else
          {
            apply((*j)->mpmSurchExcept(), fare, mInfo);
            break;
          }
        }
      }
      if (fare.mileageSurchargePctg() > 25)
        fare.setRoutingValid(false);
    }
    else
    {
      validateSingleFare(tvlRoute, **i, trx, mInfo);
    }
  }

  LOG4CXX_INFO(logger, " Leaving MileageSurcharageException::validate()");
}

bool
MileageSurchargeException::validateSingleFare(const TravelRoute& tvlRoute,
                                              PaxTypeFare& pFare,
                                              PricingTrx& trx,
                                              const MileageInfo& mInfo) const
{
  if (!pFare.mileageSurchargePctg())
    return true;

  bool result = false;
  std::vector<MileageSurchExcept*> validMarkets = getData(pFare, trx);

  if (!validMarkets.empty())
  {
    const TravelRoute* currentTvlRoute = &tvlRoute;
    if (LIKELY(TrxUtil::isFullMapRoutingActivated(trx)))
    {
      const Routing* routing = RoutingUtil::getRoutingData(trx, pFare, pFare.routingNumber());
      bool isTravelRouteTktOnly = routing && (routing->unticketedPointInd() == TKTPTS_TKTONLY) &&
                                  currentTvlRoute->travelRouteTktOnly();
      if (isTravelRouteTktOnly)
      {
        currentTvlRoute = currentTvlRoute->travelRouteTktOnly();
      }
    }

    std::vector<MileageSurchExcept*>::iterator j(validMarkets.begin());
    for (; j != validMarkets.end(); ++j)
    {
      if (!isExceptionApplicable(**j, pFare, *currentTvlRoute, trx))
        continue;
      else
      {
        apply((*j)->mpmSurchExcept(), pFare, mInfo);
        result = true;
        break;
      }
    }
  }
  if (pFare.mileageSurchargePctg() > 25)
  {
    pFare.setRoutingValid(false);
    pFare.setRoutingInvalidByMPM();
  }

  return result;
}

//---------------------------------------------------------------------------
// MileageSurchargeException::isExceptionApplicable()
//---------------------------------------------------------------------------

bool
MileageSurchargeException::isExceptionApplicable(const MileageSurchExcept& sur,
                                                 const PaxTypeFare& fare,
                                                 const TravelRoute& tvlRoute,
                                                 PricingTrx& trx) const
{
  LOG4CXX_INFO(logger, " Entered MileageSurcharageException::isExceptionApplicable()");

  if (!validateFareRestriction(sur, fare, trx))
    return false;

  if (!validateGeoRestriction(sur, tvlRoute))
    return false;

  if (!validateCxrRestriction(sur, tvlRoute))
    return false;

  if (sur.mustViaLoc().loc().empty() == false && sur.noStopoverInd() == NO_STOP_OVER_ALLOWED)
  {
    if (!validateStopOverRestriction(sur.mustViaLoc(), tvlRoute))
      return false;
  }

  return true;
}

//---------------------------------------------------------------------------
// MileageSurchargeException::
//---------------------------------------------------------------------------

struct GetSpecifiedCarrier : public std::unary_function<MileageSurchCxr, CarrierCode>

{
  CarrierCode operator()(const MileageSurchCxr* cxr) const { return cxr->carrier(); }

  // lint --e{1509}
};

struct GetUsedCarrier : public std::unary_function<TravelRoute::CityCarrier, CarrierCode>

{
  CarrierCode operator()(const TravelRoute::CityCarrier& cxr) const { return cxr.carrier(); }

  // lint --e{1509}
};

bool
MileageSurchargeException::validateCxrRestriction(const MileageSurchExcept& sur,
                                                  const TravelRoute& tvlRoute) const
{
  LOG4CXX_INFO(logger, " Entered MileageSurcharageException::validateCxrRestriction()");

  if (sur.cxrs().empty())
  {
    return true;
  }

  std::set<CarrierCode> resCxr; // carrier with restrition
  std::set<CarrierCode> usedCxr; // cxr present in the itinerary
  std::set<CarrierCode> result; // resulting set after set_intersection

  transform(
      sur.cxrs().begin(), sur.cxrs().end(), inserter(resCxr, resCxr.end()), GetSpecifiedCarrier());
  transform(tvlRoute.travelRoute().begin(),
            tvlRoute.travelRoute().end(),
            inserter(usedCxr, usedCxr.end()),
            GetUsedCarrier());

  if (sur.mustViaCxrExcept() == CARRIERS_NOT_ALLOWED)
  {

    set_intersection(resCxr.begin(),
                     resCxr.end(),
                     usedCxr.begin(),
                     usedCxr.end(),
                     inserter(result, result.begin()));
    LOG4CXX_INFO(logger, " Leaving MileageSurcharageException::validateCxrRestriction()");

    return result.empty();
  }
  result.clear();
  set_difference(usedCxr.begin(),
                 usedCxr.end(),
                 resCxr.begin(),
                 resCxr.end(),
                 inserter(result, result.begin()));

  LOG4CXX_INFO(logger, " Leaving MileageSurcharageException::validateCxrRestriction()");
  return result.empty();
}

//---------------------------------------------------------------------------
// MileageSurchargeException::validateFareRestriction()
//---------------------------------------------------------------------------
bool
MileageSurchargeException::validateFareRestriction(const MileageSurchExcept& sur,
                                                   const PaxTypeFare& fare,
                                                   PricingTrx& trx) const
{
  LOG4CXX_INFO(logger, " Entered MileageSurcharageException::validateFareRestriction()");

  if (!isValidFareType(fare.fareClass(), sur.fareClass()))
  {
    return false;
  }

  if ((sur.psgTypeInfant() == MileageSurchargeException::ANY_INFANT) &&
      PaxTypeUtil::isInfant(trx, fare.fcasPaxType(), fare.vendor()))
  {
    return true;
  }

  if ((sur.psgTypeChild() == MileageSurchargeException::ANY_CHILD) &&
      PaxTypeUtil::isChild(trx, fare.fcasPaxType(), fare.vendor()))
  {
    return true;
  }

  if (LIKELY(isValidPaxType(sur.paxTypes(), fare.fcasPaxType())))
  {
    return true;
  }

  LOG4CXX_INFO(logger,
               " Leaving MileageSurcharageException::validateFareRestriction() : INVALID FARE");
  return false;
}

//---------------------------------------------------------------------------
// MileageSurchargeException::validateFareType()
//---------------------------------------------------------------------------
bool
MileageSurchargeException::isValidFareType(const FareClassCode& actualFareType,
                                           const FareClassCode& expectedFareType) const
{
  if (expectedFareType.empty())
  {
    return true;
  }
  return (actualFareType == expectedFareType);
}

//---------------------------------------------------------------------------
// MileageSurchargeException::
//---------------------------------------------------------------------------
bool
MileageSurchargeException::isValidPaxType(const std::vector<PaxTypeCode>& paxTypes,
                                          const PaxTypeCode& paxType) const
{
  if (LIKELY(paxTypes.empty()))
  {
    return true;
  }

  return find(paxTypes.begin(), paxTypes.end(), paxType.empty() ? ADULT : paxType) !=
         paxTypes.end();
}

//---------------------------------------------------------------------------
// MileageSurchargeException::validateStopOverRestriction()
//---------------------------------------------------------------------------

struct StopOverSegment : public std::unary_function<TravelRoute::CityCarrier, bool>
{
public:
  StopOverSegment() {};
  bool operator()(const TravelRoute::CityCarrier& seg) const { return seg.stopover() == false; }

  // lint --e{1509}
};

namespace
{
struct IsStopOver : public std::unary_function<TravelRoute::CityCarrier, bool>
{
public:
  IsStopOver() {};
  IsStopOver(const LocKey k) : _key(k) {};
  bool operator()(const TravelRoute::CityCarrier& seg) const
  {
    bool result =
        LocUtil::isInLoc(seg.offCity().loc(), _key.locType(), _key.loc(), Vendor::SABRE, MANUAL);
    return result;
  }

private:
  const LocKey _key;

  // lint --e{1509}
};
}

bool
MileageSurchargeException::validateStopOverRestriction(const LocKey& noStopOver,
                                                       const TravelRoute& tvlRoute) const
{
  LOG4CXX_INFO(logger, " Entered MileageSurcharageException::validateStopOverRestriction()");

  if (noStopOver.loc().empty())
  {
    return true;
  }

  std::vector<TravelRoute::CityCarrier> cityCarrier;

  remove_copy_if(tvlRoute.travelRoute().begin(),
                 tvlRoute.travelRoute().end(),
                 back_inserter(cityCarrier),
                 StopOverSegment());

  if (!cityCarrier.empty())
  {
    bool found =
        find_if(cityCarrier.begin(), cityCarrier.end(), IsStopOver(noStopOver)) == cityCarrier.end()
            ? true
            : false;
    return found;
  }

  return true;
}

//---------------------------------------------------------------------------
// MileageSurchargeException::validateGeoRestriction()
//---------------------------------------------------------------------------

struct GetViaLocs : public std::unary_function<TravelRoute::CityCarrier, LocCode>

{
  LocCode operator()(const TravelRoute::CityCarrier& cxr) const { return cxr.offCity().loc(); }

  // lint --e{1509}
};

namespace
{
struct IsInLoc : public std::unary_function<std::vector<LocCode>, bool>
{
public:
  IsInLoc() {};
  IsInLoc(const LocKey k) : _key(k) {};
  bool operator()(const LocCode& loc) const
  {
    bool result = LocUtil::isInLoc(loc, _key.locType(), _key.loc(), Vendor::SABRE, MANUAL);
    return result;
  }

private:
  LocKey _key;

  // lint --e{1509}
};
}

bool
MileageSurchargeException::validateGeoRestriction(const MileageSurchExcept& sur,
                                                  const TravelRoute& tvlRoute) const
{
  LOG4CXX_INFO(logger, " Entered MileageSurcharageException::apply()");

  if (sur.mustViaLoc().loc().empty() && sur.mustNotViaLoc().loc().empty())
  {
    return true;
  }

  std::vector<LocCode> viaLocs;
  bool isMustbeVia(true), isMustNotbeVia(true);
  transform(tvlRoute.travelRoute().begin(),
            (tvlRoute.travelRoute().end() - 1),
            back_inserter(viaLocs),
            GetViaLocs());

  if (sur.mustViaLoc().loc().empty() == false)
  {
    std::vector<LocCode> resultVec;

    remove_copy_if(
        viaLocs.begin(), viaLocs.end(), back_inserter(resultVec), IsInLoc(sur.mustViaLoc()));
    if (sur.mustOnlyViaInd() == MUST_ONLY_VIA || sur.mustViaAllInd() == MUST_VIA_ALL)
    {
      isMustbeVia = resultVec.empty();
    }
    else
    {
      isMustbeVia = resultVec.size() < viaLocs.size() ? true : false;
    }
  }

  if (sur.mustNotViaLoc().loc().empty() == false)
  {
    std::vector<LocCode> resultVec;
    remove_copy_if(
        viaLocs.begin(), viaLocs.end(), back_inserter(resultVec), IsInLoc(sur.mustNotViaLoc()));
    isMustNotbeVia = (resultVec.size() == viaLocs.size());
  }

  return isMustbeVia && isMustNotbeVia;
}

//---------------------------------------------------------------------------
// MileageSurchargeException::apply()
//---------------------------------------------------------------------------
void
MileageSurchargeException::apply(Indicator& mpmSurchExcept,
                                 PaxTypeFare& fare,
                                 const MileageInfo& mInfo) const
{
  LOG4CXX_INFO(logger, " Entered MileageSurcharageException::apply()");

  if (mpmSurchExcept == SURCHARGE_DOESNT_APPLY)
  {
    update(fare, mInfo);
  }
  else if (mpmSurchExcept == TRAVEL_WITHIN_MPM)
  {
    fare.setRoutingValid(false);
    fare.surchargeExceptionApplies() = true;
    // fare.mileageSurchargePctgBeforeException() = fare.mileageSurchargePctg();
    fare.mileageSurchargePctg() = 30;
  }
  else
  {
    int16_t applicableSurcharge = (mpmSurchExcept - '0') * 5;
    if (fare.mileageSurchargePctg() <= applicableSurcharge)
    {
      update(fare, mInfo);
    }
  }
}

void
MileageSurchargeException::update(PaxTypeFare& fare, const MileageInfo& mInfo) const
{
  fare.surchargeExceptionApplies() = true;
  // fare.mileageSurchargePctgBeforeException() = fare.mileageSurchargePctg();
  fare.mileageSurchargePctg() = 0;
  fare.mileageSurchargePctg() = 0;
  fare.mileageSurchargeAmt() = 0;
}

struct ValidMarket : public std::unary_function<MileageSurchExcept, bool>
{
public:
  ValidMarket(const Loc* lc1,
              const Loc* lc2,
              const GlobalDirection gd,
              bool compCat17ItemOnly = false)
    : _loc1(lc1), _loc2(lc2), _globalDir(gd), _compCat17ItemOnly(compCat17ItemOnly) {};

  bool operator()(const MileageSurchExcept* sur) const
  {
    if (_compCat17ItemOnly && (sur->textTblItemNo() == 0)) // Only consider non-zero item
    {
      return false;
    }

    if (LIKELY(sur->globalDir() == _globalDir || sur->globalDir() == GlobalDirection::ZZ))
    {
      if (sur->directionality() == WITHIN && sur->loc2().loc().empty())
      {
        bool rc = LocUtil::isInLoc(
            *_loc1, sur->loc1().locType(), sur->loc1().loc(), Vendor::SABRE, MANUAL);
        return !rc;
      }
      else if (LIKELY(sur->directionality() == BETWEEN || sur->directionality() == BLANK))
      {
        bool rc = ((LocUtil::isInLoc(
                        *_loc1, sur->loc1().locType(), sur->loc1().loc(), Vendor::SABRE, MANUAL) &&
                    LocUtil::isInLoc(
                        *_loc2, sur->loc2().locType(), sur->loc2().loc(), Vendor::SABRE, MANUAL)) ||
                   (LocUtil::isInLoc(
                        *_loc2, sur->loc1().locType(), sur->loc1().loc(), Vendor::SABRE, MANUAL) &&
                    LocUtil::isInLoc(
                        *_loc1, sur->loc2().locType(), sur->loc2().loc(), Vendor::SABRE, MANUAL)));
        return !rc;
      }
      else if (sur->directionality() == FROM)
      {
        bool rc = (LocUtil::isInLoc(
                       *_loc1, sur->loc1().locType(), sur->loc1().loc(), Vendor::SABRE, MANUAL) &&
                   LocUtil::isInLoc(
                       *_loc2, sur->loc2().locType(), sur->loc2().loc(), Vendor::SABRE, MANUAL));
        return !rc;
      }

      return true;
    }
    return true;
  }

private:
  const Loc* _loc1;
  const Loc* _loc2;
  const GlobalDirection _globalDir;
  bool _compCat17ItemOnly;

  // lint --e{1509}
};

std::vector<MileageSurchExcept*>
MileageSurchargeException::getData(const PaxTypeFare& fare, PricingTrx& trx) const
{
  const PaxTypeFare* fareForCat17 = nullptr;
  const DateTime& tvlDate = fare.fareMarket()->travelDate();
  std::vector<MileageSurchExcept*> validMarkets;

  uint32_t itemNo = RuleUtil::getCat17Table996ItemNo(trx, fare, fareForCat17);
  if (UNLIKELY(fareForCat17 == nullptr))
    return validMarkets;

  // Cat17 from base fare
  if (fareForCat17 != &fare)
  {
    CarrierCode carrier = fareForCat17->carrier();
    if (carrier == INDUSTRY_CARRIER)
    {
      carrier = "";
    }

    const std::vector<MileageSurchExcept*>& milList =
        trx.dataHandle().getMileageSurchExcept(fareForCat17->vendor(),
                                               int(itemNo),
                                               carrier,
                                               fareForCat17->fareTariff(),
                                               fareForCat17->ruleNumber(),
                                               tvlDate);

    if (!milList.empty())
    {
      remove_copy_if(milList.begin(),
                     milList.end(),
                     back_inserter(validMarkets),
                     ValidMarket(fare.fareMarket()->origin(),
                                 fare.fareMarket()->destination(),
                                 fare.globalDirection(),
                                 true)); // Only check non-zero item
    }

    // Reset itemNo
    itemNo = 0;
  }

  // Try original fare
  CarrierCode carrier = fare.carrier();
  if (carrier == INDUSTRY_CARRIER)
  {
    carrier = "";
  }

  const std::vector<MileageSurchExcept*>& milList = trx.dataHandle().getMileageSurchExcept(
      fare.vendor(), int(itemNo), carrier, fare.fareTariff(), fare.ruleNumber(), tvlDate);

  if (!milList.empty())
  {
    remove_copy_if(milList.begin(),
                   milList.end(),
                   back_inserter(validMarkets),
                   ValidMarket(fare.fareMarket()->origin(),
                               fare.fareMarket()->destination(),
                               fare.globalDirection()));
  }

  return validMarkets;
}
}
