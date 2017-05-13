//-------------------------------------------------------------------
//
//  File:        SitaGatewayPair.cpp
//  Created:     Jun 11, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents data and methods of SITA
//               process to build single- and double- ended
//               constructed fares
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

#include "AddonConstruction/SitaGatewayPair.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/SitaConstructedFare.h"
#include "Common/TseConsts.h"
#include "DBAccess/DBEGlobalClass.h"
#include "DBAccess/SITAAddonFareInfo.h"
#include "DBAccess/SITAConstructedFareInfo.h"
#include "DBAccess/SITAFareInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Diagnostic/Diag255Collector.h"
#include "Util/BranchPrediction.h"

#include <algorithm>

namespace tse
{
FALLBACK_DECL(removeDynamicCastForAddonConstruction);

ConstructedFare*
SitaGatewayPair::getConstructedFare()
{
  SitaConstructedFare* cf;

  // lint -e{530}
  _cJob->dataHandle().get(cf);

  return cf;
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

FareMatchCode
SitaGatewayPair::matchAddonAndSpecified(AddonFareCortege& afc,
                                        SpecifiedFare& specFare,
                                        const bool oppositeSpecified,
                                        const bool isOriginAddon,
                                        AtpcoFareDateInterval& validInterval)
{
  // if add-on or specified fare isn't a SITA fare
  // expect exception here. which is good...

  const SITAAddonFareInfo& af = static_cast<const SITAAddonFareInfo&>(*afc.addonFare());

  const SITAFareInfo& sf = static_cast<const SITAFareInfo&>(*specFare._specFare);

  // first to check add-on and specified fares have common interval

  TSEDateInterval cfValid;
  if (UNLIKELY(_cJob->dataHandle().isHistorical()))
  {
    if (!cfValid.defineIntersectionH(afc.effInterval(), sf.effInterval()))
      return FM_DATE_INTERVAL_MISMATCH;
  }
  else
  {
    if (UNLIKELY(!cfValid.defineIntersection(afc.effInterval(), sf.effInterval())))
      return FM_DATE_INTERVAL_MISMATCH;
  }

  // trap for debuging. please do not remove =======================>
  //
  // if( sf.fareClass() == "YBB" &&
  //    af.gatewayMarket() == "PAR" &&
  //    af.interiorMarket() == "NTE"
  //  )
  //  cout << "!!!\n";
  //
  // >=================================================== end of trap

  // match Tariff Number

  FareMatchCode matchResult = matchTariff(af, sf);

  // match Rule Tariff
  if (LIKELY(matchResult == FM_GOOD_MATCH))
    matchResult = matchRuleTariff(af, sf);

  // match OWRT

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchOWRT(af, sf);

  // match Base Fare Routing (BRTG), Base MPM flag (BMPM)

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchRouting(af, sf);

  // match SITA Route Code (Global Indicator)

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchRouteCode(af, sf);

  // match Tariff Family

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchTariffFamily(af, sf);

  // match Fare Quality Code

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchFareQualityCode(af, sf);

  // match Fare Basis/ Global Class Flag

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchFareBasisOrGlobalClass(af, sf);

  // match Applicable rule

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchApplicableRule(af, sf);

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchAddonDirection(af, sf, oppositeSpecified, isOriginAddon);

  // create validity intervals if good match

  if (matchResult == FM_GOOD_MATCH)
  {
    validInterval.effInterval() = cfValid;
  }

  return matchResult;
}

#else

FareMatchCode
SitaGatewayPair::matchAddonAndSpecified(AddonFareCortege& afc,
                                        ConstructedFare& constrFare,
                                        const bool isOriginAddon,
                                        std::vector<DateIntervalBase*>& constrFareIntervals)
{
  // if add-on or specified fare isn't a SITA fare
  // expect exception here. which is good...

  const SITAAddonFareInfo& af = static_cast<const SITAAddonFareInfo&>(*afc.addonFare());

  const SITAFareInfo& sf = static_cast<const SITAFareInfo&>(*constrFare.specifiedFare());

  // first to check add-on and specified fares have common interval

  TSEDateInterval cfValid;
  if (UNLIKELY(_cJob->dataHandle().isHistorical()))
  {
    if (!cfValid.defineIntersectionH(afc.effInterval(), constrFare.prevEffInterval()))
      return FM_DATE_INTERVAL_MISMATCH;
  }
  else
  {
    if (UNLIKELY(!cfValid.defineIntersection(afc.effInterval(), constrFare.prevEffInterval())))
      return FM_DATE_INTERVAL_MISMATCH;
  }

  // trap for debuging. please do not remove =======================>
  //
  // if( sf.fareClass() == "YBB" &&
  //    af.gatewayMarket() == "PAR" &&
  //    af.interiorMarket() == "NTE"
  //  )
  //  cout << "!!!\n";
  //
  // >=================================================== end of trap

  // match Tariff Number

  FareMatchCode matchResult = matchTariff(af, sf);

  // match Rule Tariff
  if (LIKELY(matchResult == FM_GOOD_MATCH))
    matchResult = matchRuleTariff(af, sf);

  // match OWRT

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchOWRT(af, sf);

  // match Base Fare Routing (BRTG), Base MPM flag (BMPM)

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchRouting(af, sf);

  // match SITA Route Code (Global Indicator)

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchRouteCode(af, sf);

  // match Tariff Family

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchTariffFamily(af, sf);

  // match Fare Quality Code

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchFareQualityCode(af, sf);

  // match Fare Basis/ Global Class Flag

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchFareBasisOrGlobalClass(af, sf);

  // match Applicable rule

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchApplicableRule(af, sf);

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchAddonDirection(af, constrFare, isOriginAddon);

  // create validity intervals if good match

  if (matchResult == FM_GOOD_MATCH)
  {
    SitaFareDateInterval* sti;
    // lint -e{530}
    _cJob->dataHandle().get(sti);

    sti->effInterval() = cfValid;

    constrFareIntervals.push_back(sti);
  }

  return matchResult;
}

#endif

FareMatchCode
SitaGatewayPair::finalMatch()
{
  FareMatchCode matchResult = FM_GOOD_MATCH;
  Diag255Collector* diag255{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag255 = _cJob->diagnostic<Diag255Collector>();
  }
  else
  {
    diag255 = _cJob->diag255();
  }

  // loop via all ConstructedFares and do final check/update

  ConstructedFareList::iterator i = _constructedFares.begin();
  ConstructedFareList::iterator ie = _constructedFares.end();
  for (; i < ie; ++i)
  {
    SitaConstructedFare& cf = static_cast<SitaConstructedFare&>(**i);

    if (!cf.isValid())
      continue;

    const SITAAddonFareInfo* origAddon = nullptr;
    if (cf.origAddon() != nullptr)
      origAddon = static_cast<const SITAAddonFareInfo*>(cf.origAddon()->addonFare());

    const SITAAddonFareInfo* destAddon = nullptr;
    if (cf.destAddon() != nullptr)
      destAddon = static_cast<const SITAAddonFareInfo*>(cf.destAddon()->addonFare());

    if (cf.isDoubleEnded())
    {
      // for double-ended fares compare fields from origin and
      // destination add-ons. if fields are not equal fail the
      // fare to avoid ambiguity. otherwise update corresponding
      // fieds in constructed fare.

      // Thru Rule (SITA TRULE)

      if (UNLIKELY(origAddon->throughRule() != destAddon->throughRule()))
      {
        matchResult = FM_SITA_TRULE;
        cf.valid() = false;
      }
      else
        cf.throughRule() = origAddon->throughRule();

      // TMPM (SITA TMPM)

      if (UNLIKELY(origAddon->throughMPMInd() != destAddon->throughMPMInd()))
      {
        matchResult = FM_SITA_TMPM;
        cf.valid() = false;
      }
      else
        cf.throughMPMInd() = origAddon->throughMPMInd();

      // Thru Fare Routing (SITA TRTG)

      if (UNLIKELY(origAddon->thruFareRouting() != destAddon->thruFareRouting()))
      {
        matchResult = FM_SITA_TRTG;
        cf.valid() = false;
      }
      else
        cf.throughFareRouting() = origAddon->thruFareRouting();

      if (UNLIKELY(!cf.isValid() && diag255 != nullptr))
      {
        diag255->writeBadFare(*origAddon, *cf.specifiedFare(), *destAddon, matchResult);
      }
    }
    else
    {
      // for single-ended ambiguity isn't possible. just update
      // corresponding fieds in constructed fare.

      if (origAddon != nullptr)
      {
        cf.throughRule() = origAddon->throughRule();
        cf.throughMPMInd() = origAddon->throughMPMInd();
        cf.throughFareRouting() = origAddon->thruFareRouting();
      }
      else
      {
        cf.throughRule() = destAddon->throughRule();
        cf.throughMPMInd() = destAddon->throughMPMInd();
        cf.throughFareRouting() = destAddon->thruFareRouting();
      }
    }
  }

  return FM_GOOD_MATCH;
}

FareMatchCode
SitaGatewayPair::matchOWRT(const SITAAddonFareInfo& af, const SITAFareInfo& sf)
{
  // valid combinations of specified and arbitrary OWRT
  //
  // ----------------+-----------------------+---------------------
  //  Arbitrary Fare ! Arbitrary Fare Amount ! Can be matched with
  //      OWRT       !   (as it is in DB)    ! Specified Fare OWRT
  // ----------------+-----------------------+---------------------
  //        1        !       one-way         !    1, 3
  //        2        !      round trip       !    2
  //        3        !       one-way         !    3
  //        4        !       one-way         !    1
  //        5        !       one-way         !    1, 2, 3
  // ----------------+-----------------------+---------------------

  bool okToMatch;

  switch (af.owrt())
  {
  case ONE_WAY_MAY_BE_DOUBLED:
    okToMatch = (sf.owrt() == ONE_WAY_MAY_BE_DOUBLED) || (sf.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED);
    break;

  case ROUND_TRIP_MAYNOT_BE_HALVED:
    okToMatch = (sf.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED);
    break;

  case ONE_WAY_MAYNOT_BE_DOUBLED:
    okToMatch = (sf.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED);
    break;

  case SITA_OWRT_ADDON_4:
    okToMatch = (sf.owrt() == ONE_WAY_MAY_BE_DOUBLED);
    break;

  case SITA_OWRT_ADDON_5:
    okToMatch = (sf.owrt() == ONE_WAY_MAY_BE_DOUBLED) ||
                (sf.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) ||
                (sf.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED);
    break;

  default:
    okToMatch = false;
  }

  return (okToMatch ? FM_GOOD_MATCH : FM_OWRT);
}

FareMatchCode
SitaGatewayPair::matchRouting(const SITAAddonFareInfo& af, const SITAFareInfo& sf)
{
  // BMPM action codes

  static const char BMPM_ANY = 'Y'; // match any fares
  static const char CLFB_ROUTING_ONLY = 'N'; // match spec. fares with routings
  static const char CLFB_MPM_ONLY = 'O'; // match spec. fares with mpm

  FareMatchCode matchResult = FM_GOOD_MATCH;

  // BMPM flag defines method we match the fares

  switch (af.baseMPMInd())
  {
  case BMPM_ANY:

    // any type of fare (MPM or routing) is allowed

    break;

  case CLFB_ROUTING_ONLY:

    // only match with routing specified fare is considered

    if (sf.routingNumber() == MILEAGE_ROUTING)
    {
      // can't match the add-on with mileage specified

      matchResult = FM_SITA_BMPM_CANT_WITH_MILEAGE;
    }

    else if (af.baseFareRouting() != MILEAGE_ROUTING)
    {
      // Base Fare Routing is defined. this requires an exact match
      // of add-on Base Fare Routing and specified Routing Number

      if (af.baseFareRouting() != sf.routingNumber())
        matchResult = FM_SITA_BRTG;
    }

    break;

  case CLFB_MPM_ONLY:

    // only match with MPM specified is considered

    if (sf.routingNumber() != MILEAGE_ROUTING)
      matchResult = FM_SITA_BMPM_CANT_WITH_ROUTING;

    break;

  default:
    matchResult = FM_SITA_BMPM_UNKNOWN;
  }

  return matchResult;
}

FareMatchCode
SitaGatewayPair::matchRouteCode(const SITAAddonFareInfo& af, const SITAFareInfo& sf)
{
  // exact match. these fields are never empty in DB

  if (af.routeCode() == sf.routeCode())
    return FM_GOOD_MATCH;

  // no match

  return FM_SITA_ROUTE_CODE;
}

FareMatchCode
SitaGatewayPair::matchTariffFamily(const SITAAddonFareInfo& af, const SITAFareInfo& sf)
{
  // tariff family match requires:
  //    1) exact match
  // or
  //    2) add-on tariff family 'G' can be matched with D/Q/U/V
  // or
  //    3) add-on tariff family 'H' can be matched with R/W/B
  // or
  //    4) add-on tariff family 'J' can be matched with U/D/T/V/Q
  // or
  //    5) add-on tariff family 'K' can be matched with R/W/B

  static const char* arbitraryMatch[] = { "GDQUV", "HRWB", "JUDTVQ", "KRWB", nullptr };

  if (af.tariffFamily() == ' ') // datatype is Indicator
    return FM_GOOD_MATCH;

  // step 1. try exact match

  if (af.tariffFamily() == sf.tariffFamily())
    return FM_GOOD_MATCH;

  // step 2. generic family match

  for (const char** c = arbitraryMatch; *c != nullptr; ++c)
    if (**c == af.tariffFamily())
      if (UNLIKELY(strchr(*c + 1, sf.tariffFamily()) != nullptr))
        return FM_GOOD_MATCH;

  // no match

  return FM_SITA_TARIFF_FAMILY;
}

FareMatchCode
SitaGatewayPair::matchTariff(const SITAAddonFareInfo& af, const SITAFareInfo& sf)
{
  // 0 addon tariff matches to all specified tariff

  if (af.addonTariff() == 0)
    return FM_GOOD_MATCH;

  // otherwise should be exact match

  if (af.addonTariff() == sf.fareTariff())
    return FM_GOOD_MATCH;

  // no match

  return FM_FARE_TARIFF;
}

FareMatchCode
SitaGatewayPair::matchFareQualityCode(const SITAAddonFareInfo& af, const SITAFareInfo& sf)
{
  // Fare Quality processing codes

  static const Indicator FQ_INCLUDE = 'I';
  static const Indicator FQ_EXCLUDE = 'E';

  FareMatchCode matchResult = FM_GOOD_MATCH;

  // af.fareQualInd() can be to equal 'I' even if af.fareQualCodes()
  // is empty!!!!  This is unbelievable...

  if (UNLIKELY(sf.fareQualCode() != ' ' // fareQualCode is Indicator
      &&
      !af.fareQualCodes().empty()))
  {
    std::set<Indicator>::const_iterator i = af.fareQualCodes().find(sf.fareQualCode());

    switch (af.fareQualInd())
    {
    case FQ_INCLUDE:
      if (i == af.fareQualCodes().end())
        matchResult = FM_SITA_FARE_QUALITY_INCLUDE;

      break;

    case FQ_EXCLUDE:
      if (i != af.fareQualCodes().end())
        matchResult = FM_SITA_FARE_QUALITY_EXCLUDE;

      break;

    default:
      matchResult = FM_SITA_FARE_QUALITY_UNKNOWN;
    }
  }

  return matchResult;
}

FareMatchCode
SitaGatewayPair::matchRuleTariff(const SITAAddonFareInfo& af, const SITAFareInfo& sf)
{
  if (af.ruleTariff() == 0)
    return FM_GOOD_MATCH;

  if (af.ruleTariff() != sf.fareTariff())
    return FM_SITA_RTAR;

  return FM_GOOD_MATCH;
}

FareMatchCode
SitaGatewayPair::matchApplicableRule(const SITAAddonFareInfo& af, const SITAFareInfo& sf)
{
  // ARULE processing codes

  static const Indicator ARULE_INCLUDE = 'N';
  static const Indicator ARULE_EXCLUDE = 'Y';

  FareMatchCode matchResult = FM_GOOD_MATCH;

  if (LIKELY(!sf.ruleNumber().empty()))
  {
    bool ruleIsFound = (af.rules().find(sf.ruleNumber()) != af.rules().end());

    switch (af.ruleExcludeInd())
    {
    case ARULE_INCLUDE:
      if (!af.rules().empty() && !ruleIsFound)
        matchResult = FM_SITA_ARULE_INCLUDE;

      break;

    case ARULE_EXCLUDE:
      if (ruleIsFound)
        matchResult = FM_SITA_ARULE_EXCLUDE;

      break;

    default:
      matchResult = FM_SITA_ARULE_UNKNOWN;
    }
  }

  return matchResult;
}

FareMatchCode
SitaGatewayPair::matchFareBasisOrGlobalClass(const SITAAddonFareInfo& af, const SITAFareInfo& sf)
    const
{
  // CLFB action codes

  static const char CLFB_FBC = 'F'; // match Fare Basis (i.e. Fare class)
  static const char CLFB_GC = 'C'; // match Global Class Flags
  static const char CLFB_ANY = ' '; // match to any specified fare

  FareMatchCode matchResult = FM_GOOD_MATCH;

  switch (af.classFareBasisInd())
  {
  case CLFB_FBC:

    // match Fare Basis (i.e. Fare class in terms of ATPCO)
    //
    //    1) one (both) field(s) might be empty
    // or
    //    2) exact match

    if (!af.fareClass().empty() && !sf.fareClass().empty() && af.fareClass() != sf.fareClass())
      matchResult = FM_SITA_CLFB_FBC;

    break;

  case CLFB_GC:

    // match Global Class Flag

    matchResult = matchGlobalClass(af, sf);

    break;

  case CLFB_ANY:

    // no restrictions

    break;

  default:
    matchResult = FM_SITA_CLFB_UNKNOWN;
  }

  return matchResult;
}

FareMatchCode
SitaGatewayPair::matchGlobalClass(const SITAAddonFareInfo& af, const SITAFareInfo& sf) const
{
  // global class flag

  static const char GCF_SGC = 'Y'; // match to specified Global Class
  static const char GCF_DBE = 'N'; // match to specified DBE classes
  static const char GCF_ANY = 'A'; // match to any specified fare

  FareMatchCode matchResult = FM_GOOD_MATCH;

  switch (af.globalClassFlag())
  {
  case GCF_SGC:

    matchResult = matchGlobalDBEClasses(af, sf);

    break;

  case GCF_DBE:
  {
    std::set<DBEClass>::const_iterator i = af.dbeClasses().find(sf.dbeClass());

    if (i == af.dbeClasses().end())
      matchResult = FM_SITA_GLOBAL;
  }

  break;

  case GCF_ANY:

    // no restrictions

    break;

  default:

    matchResult = FM_SITA_GLOBAL_UNKNOWN;
  }

  return matchResult;
}

FareMatchCode
SitaGatewayPair::matchGlobalDBEClasses(const SITAAddonFareInfo& af, const SITAFareInfo& sf) const
{
  std::set<DBEClass>::const_iterator i = af.dbeClasses().begin();
  std::set<DBEClass>::const_iterator ie = af.dbeClasses().end();
  for (; i != ie; ++i)
  {
    const std::vector<DBEGlobalClass*>& gdc = _cJob->dataHandle().getDBEGlobalClass(*i);

    std::vector<DBEGlobalClass*>::const_iterator j = gdc.begin();
    std::vector<DBEGlobalClass*>::const_iterator je = gdc.end();
    for (; j != je; ++j)
    {
      std::vector<DBEClass>::const_iterator k = (*j)->dbeClasses().begin();
      std::vector<DBEClass>::const_iterator ke = (*j)->dbeClasses().end();

      for (; k != ke; ++k)
        if (*k == sf.dbeClass())
          return FM_GOOD_MATCH;
    }
  }

  return FM_SITA_GLOBAL_DBE;
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

FareMatchCode
SitaGatewayPair::matchAddonDirection(const SITAAddonFareInfo& af,
                                     const FareInfo& specFare,
                                     const bool oppositeSpecified,
                                     const bool isOriginAddon)
{
  const Indicator FROM_INTERIOR = 'O';
  const Indicator TO_INTERIOR = 'D';

  // trap for debuging. please do not remove =======================>
  //
  // if( addonFare->addonFare()->fareClass() == "C*****" &&
  //    addonFare->addonFare()->gatewayMarket() == "NYC" &&
  //    addonFare->addonFare()->interiorMarket() == "SBA"
  //  )
  //  OK = true;
  //
  // >=================================================== end of trap

  // directionality of spec. fire is defined in terms city1/city2.
  // needs to be redifined in terms gateway1/gateway2, so sfDir means:
  //   TO   - to gateway1
  //   FROM - from gateway1

  Directionality sfDir = specFare.directionality();
  if (oppositeSpecified && (sfDir != BOTH))
    sfDir = (sfDir == TO) ? FROM : TO;

  if (UNLIKELY(sfDir == BOTH))
    return FM_GOOD_MATCH;

  // SITA add-on directionality is opposite to ATPCO add-on directionality:
  //   O - from interior
  //   D - to interior
  //
  // combinations for origin add-on:
  // --------------------------+--------------------------+----------
  //     origin add-on dir.    ! spec.fire directionality ! validity
  // --------------------------+--------------------------+----------
  //  TO_INTERIOR (int<-gw1)   !     TO   (gw1<-gw2)      ! valid
  //  TO_INTERIOR (int<-gw1)   !     FROM (gw1->gw2)      ! not valid
  //  FROM_INTERIOR (int->gw1) !     TO   (gw1<-gw2)      ! not valid
  //  FROM_INTERIOR (int->gw1) !     FROM (gw1->gw2)      ! valid
  // --------------------------+--------------------------+----------
  //
  // combinations for destination add-on:
  // --------------------------+--------------------------+----------
  //  spec.fire directionality !  destination add-on dir. ! validity
  // --------------------------+--------------------------+----------
  //      TO   (gw1<-gw2)      ! TO_INTERIOR (gw2->int)   ! not valid
  //      FROM (gw1->gw2)      ! TO_INTERIOR (gw2->int)   ! valid
  //      TO   (gw1<-gw2)      ! FROM_INTERIOR (gw2<-int) ! valid
  //      FROM (gw1->gw2)      ! FROM_INTERIOR (gw2<-int) ! not valid
  // --------------------------+--------------------------+----------

  bool OK = true;

  if (af.directionality() == FROM_INTERIOR)
    OK = (isOriginAddon && sfDir == FROM) || (!isOriginAddon && sfDir == TO);

  else if (af.directionality() == TO_INTERIOR)
    OK = (isOriginAddon && sfDir == TO) || (!isOriginAddon && sfDir == FROM);

  if (!OK)
    return FM_ADDON_DIRECTION;

  return FM_GOOD_MATCH;
}

#else

FareMatchCode
SitaGatewayPair::matchAddonDirection(const SITAAddonFareInfo& af,
                                     const ConstructedFare& constrFare,
                                     const bool isOriginAddon)
{
  const Indicator FROM_INTERIOR = 'O';
  const Indicator TO_INTERIOR = 'D';

  // trap for debuging. please do not remove =======================>
  //
  // if( addonFare->addonFare()->fareClass() == "C*****" &&
  //    addonFare->addonFare()->gatewayMarket() == "NYC" &&
  //    addonFare->addonFare()->interiorMarket() == "SBA"
  //  )
  //  OK = true;
  //
  // >=================================================== end of trap

  // directionality of spec. fire is defined in terms city1/city2.
  // needs to be redifined in terms gateway1/gateway2, so sfDir means:
  //   TO   - to gateway1
  //   FROM - from gateway1

  Directionality sfDir = constrFare.defineDirectionality();

  if (UNLIKELY(sfDir == BOTH))
    return FM_GOOD_MATCH;

  // SITA add-on directionality is opposite to ATPCO add-on directionality:
  //   O - from interior
  //   D - to interior
  //
  // combinations for origin add-on:
  // --------------------------+--------------------------+----------
  //     origin add-on dir.    ! spec.fire directionality ! validity
  // --------------------------+--------------------------+----------
  //  TO_INTERIOR (int<-gw1)   !     TO   (gw1<-gw2)      ! valid
  //  TO_INTERIOR (int<-gw1)   !     FROM (gw1->gw2)      ! not valid
  //  FROM_INTERIOR (int->gw1) !     TO   (gw1<-gw2)      ! not valid
  //  FROM_INTERIOR (int->gw1) !     FROM (gw1->gw2)      ! valid
  // --------------------------+--------------------------+----------
  //
  // combinations for destination add-on:
  // --------------------------+--------------------------+----------
  //  spec.fire directionality !  destination add-on dir. ! validity
  // --------------------------+--------------------------+----------
  //      TO   (gw1<-gw2)      ! TO_INTERIOR (gw2->int)   ! not valid
  //      FROM (gw1->gw2)      ! TO_INTERIOR (gw2->int)   ! valid
  //      TO   (gw1<-gw2)      ! FROM_INTERIOR (gw2<-int) ! valid
  //      FROM (gw1->gw2)      ! FROM_INTERIOR (gw2<-int) ! not valid
  // --------------------------+--------------------------+----------

  bool OK = true;

  if (af.directionality() == FROM_INTERIOR)
    OK = (isOriginAddon && sfDir == FROM) || (!isOriginAddon && sfDir == TO);

  else if (af.directionality() == TO_INTERIOR)
    OK = (isOriginAddon && sfDir == TO) || (!isOriginAddon && sfDir == FROM);

  if (!OK)
    return FM_ADDON_DIRECTION;

  return FM_GOOD_MATCH;
}

#endif

}
