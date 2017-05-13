//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "Rules/CommissionsRuleValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/TruePaxType.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CommissionRuleInfo.h"
#include "Diagnostic/Diag867Collector.h"
#include "Rules/RuleUtil.h"
#include "Pricing/FarePathUtils.h"
#include "Util/BranchPrediction.h"

namespace tse
{

FALLBACK_DECL(fallbackAMCFareBasisFix);
FALLBACK_DECL(fallbackAMCPhase2);

bool
CommissionsRuleValidator::isCommissionRuleMatched(const CommissionRuleInfo& cr,
                                                  const CarrierCode& valCxr)
{
  CommissionValidationStatus rc = SKIP_CR;
  if(cr.commissionTypeId() != 12)
  {
    getFareBasis();
    getClassOfService();
    getPaxTypes();

    rc = validateCommissionRule(cr, valCxr);
  }

  if (_diag867)
    _diag867->printCommissionRuleProcess(cr, rc);

  return (rc == PASS_CR);
}

CommissionValidationStatus
CommissionsRuleValidator::validateCommissionRule(const CommissionRuleInfo& cr,
                                                 const CarrierCode& valCxr) const
{
  CommissionValidationStatus rc = PASS_CR;
  if (!matchFareBasis(cr, rc))
  {
    return rc;
  }
  if (!matchFareBasisFragment(cr, rc))
  {
    return rc;
  }
  if (!matchClassOfService(cr, rc))
  {
    return rc;
  }
  if (!matchCabin(cr, rc))
  {
    return rc;
  }
  if (!matchOperatingCarrier(cr, rc))
  {
    return rc;
  }
  if (!matchMarketingCarrier(cr, rc))
  {
    return rc;
  }
  if (!matchTicketingCarrier(cr, rc, valCxr))
  {
    return rc;
  }
  if (!matchMarketingGovCarrier(cr, rc))
  {
    return rc;
  }
  if (!matchOperGovCarrier(cr, rc))
  {
    return rc;
  }
  if (!matchPassengerType(cr))
  {
    return FAIL_CR_PSGR_TYPE;
  }
  if (!matchRoundTrip(cr))
  {
    return FAIL_CR_ROUNDTRIP;
  }
  if (!matchConnectionAirports(cr, rc))
  {
    return rc;
  }
  if (!matchInterlineConnection(cr))
  {
    return FAIL_CR_INTERLINE_CONNECTION;
  }
  if (!matchRequiredNonStop(cr))
  {
    return FAIL_CR_REQ_NON_STOP;
  }
  // Due to the requirement is not correct, remove this logic for AMC Phase2
  //  if (!fallback::fallbackAMCPhase2(&_trx))
  //  {
  //    if (!matchTktDesignator(cr, rc))
  //    {
  //      return rc;
  //    }
  //  }

  return rc;
}

bool
CommissionsRuleValidator::matchFareBasis(const CommissionRuleInfo& cr,
                                         CommissionValidationStatus& rc) const
{
  if (!cr.fareBasisCodeExcl().empty() &&
      std::find(cr.fareBasisCodeExcl().begin(),
                   cr.fareBasisCodeExcl().end(), _fareBasis.at(0)) !=
                   cr.fareBasisCodeExcl().end())
  {
    rc = FAIL_CR_FARE_BASIS_EXCL;
    return false;
  }
  if (!cr.fareBasisCodeIncl().empty() &&
      std::find(cr.fareBasisCodeIncl().begin(),
                   cr.fareBasisCodeIncl().end(), _fareBasis.at(0)) ==
                   cr.fareBasisCodeIncl().end())
  {
    rc = FAIL_CR_FARE_BASIS_INCL;
    return false;
  }
  return true;
}

bool
CommissionsRuleValidator::matchClassOfService(const CommissionRuleInfo& cr,
                                              CommissionValidationStatus& rc) const
{
  if (!cr.classOfServiceExcl().empty() &&
      std::find(cr.classOfServiceExcl().begin(),
                   cr.classOfServiceExcl().end(), _rbd) !=
                   cr.classOfServiceExcl().end())
  {
    rc = FAIL_CR_CLASS_OF_SERVICE_EXCL;
    return false;
  }
  if (!cr.classOfServiceIncl().empty() &&
      std::find(cr.classOfServiceIncl().begin(),
                   cr.classOfServiceIncl().end(), _rbd) ==
                   cr.classOfServiceIncl().end())
  {
    rc = FAIL_CR_CLASS_OF_SERVICE_INCL;
    return false;
  }
  return true;
}

bool
CommissionsRuleValidator::matchFareBasisFragment(const CommissionRuleInfo& cr,
                                                 CommissionValidationStatus& rc) const
{
  if (!cr.fbcFragmentExcl().empty() && matchFareBasisFragmentExclIncl(cr.fbcFragmentExcl()))
  {
    rc = FAIL_CR_FBC_FRAGMENT_EXCL;
    return false;
  }
  if (!cr.fbcFragmentIncl().empty() && !matchFareBasisFragmentExclIncl(cr.fbcFragmentIncl()))
  {
    rc = FAIL_CR_FBC_FRAGMENT_INCL;
    return false;
  }
  return true;
}

bool
CommissionsRuleValidator::matchFareBasisFragmentExclIncl(const std::vector<std::string>& fragments) const
{
  if (!fallback::fallbackAMCPhase2(&_trx))
  {
    return matchString(fragments,_fareBasis);
  }
  else
  {
    for (const std::string fbcFragment : fragments)
    {
      if(fbcFragment.size() == 1 && fbcFragment.at(0) == '%')
        return true;
      if(matchFareBasisFragment(fbcFragment))
        return true;
    }
    return false;
  }
}

bool
CommissionsRuleValidator::matchCabin(const CommissionRuleInfo& cr,
                                     CommissionValidationStatus& rc) const
{
  if (!cr.excludedCabinType().empty() &&
      std::find(cr.excludedCabinType().begin(),
                   cr.excludedCabinType().end(), _fu.paxTypeFare()->cabin().getClassAlphaNumAnswer()) !=
                   cr.excludedCabinType().end())
  {
    rc = FAIL_CR_EXCL_CABIN;
    return false;
  }
  if (!cr.requiredCabinType().empty() &&
      std::find(cr.requiredCabinType().begin(),
                   cr.requiredCabinType().end(), _fu.paxTypeFare()->cabin().getClassAlphaNumAnswer()) ==
                   cr.requiredCabinType().end())
  {
    rc = FAIL_CR_REQ_CABIN;
    return false;
  }
  return true;
}

bool
CommissionsRuleValidator::matchPassengerType(const CommissionRuleInfo& cr) const
{
  if (cr.requiredPaxType().empty())
    return true;
  // currently the true pax type is coded according to Stan; he will check with Jaymini
  // if the requested should be used along with true, then FareCalcConfig needed as in
  // PricingResponseFormatter  (fcConfig.truePsgrTypeInd() == YES) for true pax type
  return std::find(cr.requiredPaxType().begin(),
                   cr.requiredPaxType().end(), _truePaxType) !=
                   cr.requiredPaxType().end();
}

bool
CommissionsRuleValidator::matchOperatingCarrier(const CommissionRuleInfo& cr,
                                                CommissionValidationStatus& rc) const
{
  if (!cr.operatingCarrierExcl().empty() &&
       matchCarrierExclIncl(cr.operatingCarrierExcl(), false, true))
  {
    rc = FAIL_CR_OPER_CARRIER_EXCL;
    return false;
  }
  if (!cr.operatingCarrierIncl().empty() &&
      !matchCarrierExclIncl(cr.operatingCarrierIncl(), false))
  {
    rc = FAIL_CR_OPER_CARRIER_INCL;
    return false;
  }
  return true;
}

bool
CommissionsRuleValidator::matchMarketingCarrier(const CommissionRuleInfo& cr,
                                                CommissionValidationStatus& rc) const
{
  if (!cr.marketingCarrierExcl().empty() &&
       matchCarrierExclIncl(cr.marketingCarrierExcl(), true, true))
  {
    rc = FAIL_CR_MARKET_CARRIER_EXCL;
    return false;
  }
  if (!cr.marketingCarrierIncl().empty() &&
      !matchCarrierExclIncl(cr.marketingCarrierIncl(), true))
  {
    rc = FAIL_CR_MARKET_CARRIER_INCL;
    return false;
  }
  return true;
}

bool
CommissionsRuleValidator::matchCarrierExclIncl(const std::vector<CarrierCode>& cxrs,
                                               bool marketing, bool exclude) const
{
  std::set <CarrierCode> travelCxrs;
  for (const TravelSeg* tvl : _fu.travelSeg())
  {
    const AirSeg* airSeg(nullptr);
    if (tvl->isAir())
    {
      airSeg = static_cast<const AirSeg*>(tvl);
    }
    if(!airSeg)
      continue;
    if(marketing)
      travelCxrs.insert(airSeg->marketingCarrierCode());
    else
      travelCxrs.insert(airSeg->operatingCarrierCode());
  }
  for(const CarrierCode cxr : travelCxrs)
  {
    if(exclude && std::find(cxrs.begin(), cxrs.end(), cxr) != cxrs.end())
      return true;

    if(!exclude && std::find(cxrs.begin(), cxrs.end(), cxr) == cxrs.end())
      return false;
  }
  return !exclude;
}

bool
CommissionsRuleValidator::matchTicketingCarrier(const CommissionRuleInfo& cr,
                                                CommissionValidationStatus& rc,
                                                const CarrierCode& valCxr) const
{
  if (!cr.ticketingCarrierExcl().empty() &&
      std::find(cr.ticketingCarrierExcl().begin(),
                cr.ticketingCarrierExcl().end(), valCxr) != cr.ticketingCarrierExcl().end())
  {
    rc = FAIL_CR_TICKET_CARRIER_EXCL;
    return false;
  }
  if (!cr.ticketingCarrierIncl().empty() &&
      std::find(cr.ticketingCarrierIncl().begin(),
                cr.ticketingCarrierIncl().end(), valCxr) == cr.ticketingCarrierIncl().end())
  {
    rc = FAIL_CR_TICKET_CARRIER_INCL;
    return false;
  }
  return true;
}

bool
CommissionsRuleValidator::matchMarketingGovCarrier(const CommissionRuleInfo& cr,
                                                   CommissionValidationStatus& rc) const
{
  if (!cr.excludedMktGovCxr().empty() && matchGoverningCarrier(cr.excludedMktGovCxr(), true))
  {
    rc = FAIL_CR_EXCL_MKT_GOV_CXR;
    return false;
  }
  if (!cr.requiredMktGovCxr().empty() && !matchGoverningCarrier(cr.requiredMktGovCxr(), true))
  {
    rc = FAIL_CR_REQ_MKT_GOV_CXR;
    return false;
  }
  return true;
}

bool
CommissionsRuleValidator::matchGoverningCarrier(const std::vector<CarrierCode>& govCxrs, bool marketing) const
{
  const TravelSeg* pTvlSeg = _fu.paxTypeFare()->fareMarket()->primarySector();
  if(pTvlSeg == 0)
    pTvlSeg = *(_fu.travelSeg().begin());
  const AirSeg* airSeg(nullptr);
  if (pTvlSeg->isAir())
  {
    airSeg = static_cast<const AirSeg*>(pTvlSeg);
  }
  if(!airSeg)
    return false;

  for(const CarrierCode cxr : govCxrs)
  {
    if( (marketing && cxr == airSeg->marketingCarrierCode()) ||
        (!marketing && cxr == airSeg->operatingCarrierCode())  )
      return true;
  }
  return false;
}

bool
CommissionsRuleValidator::matchOperGovCarrier(const CommissionRuleInfo& cr,
                                              CommissionValidationStatus& rc) const
{
  if (!cr.excludedOperGovCxr().empty() && matchGoverningCarrier(cr.excludedOperGovCxr()))
  {
    rc = FAIL_CR_EXCL_OPER_GOV_CXR;
    return false;
  }
  if (!cr.requiredOperGovCxr().empty() && !matchGoverningCarrier(cr.requiredOperGovCxr()))
  {
    rc = FAIL_CR_REQ_OPER_GOV_CXR;;
    return false;
  }
  return true;
}

void
CommissionsRuleValidator::getPaxTypes()
{
  TruePaxType tpt(_trx, _fp);
  _truePaxType = tpt.paxType();
}

bool
CommissionsRuleValidator::matchTktDesignator(const CommissionRuleInfo& cr,
                                             CommissionValidationStatus& rc) const
{
  if (!cr.excludedTktDesig().empty() &&
      !_tktDesignator.empty() &&
      (cr.excludedTktDesig()[0] == "*ANY*" ||
       cr.excludedTktDesig()[0] == "%" ||
       matchTktDesignatorFragmentExclIncl(cr.excludedTktDesig())))
  {
    rc = FAIL_CR_EXCL_TKT_DESIGNATOR;
    return false;
  }

  if (!cr.requiredTktDesig().empty()&&
      (_tktDesignator.empty() ||
      ((cr.requiredTktDesig()[0] != "*ANY*" && cr.requiredTktDesig()[0] != "%")&&
       !matchTktDesignatorFragmentExclIncl(cr.requiredTktDesig()))))
  {
    rc = FAIL_CR_REQ_TKT_DESIGNATOR;
    return false;
  }
  return true;
}

bool
CommissionsRuleValidator::matchString(const std::vector<TktDesignator>& tdCol, const std::string& td)const
{
  for (const TktDesignator& s:tdCol)
  {
    if (matchString(s.data(),td))
      return true;
  }
  return false;
}

bool
CommissionsRuleValidator::matchString(const std::vector<std::string>& fbCol, const std::string& fb)const
{
  for (const std::string& s:fbCol)
  {
    if (matchString(s,fb))
      return true;
  }
  return false;
}

// s is from db; x is from trx
bool
CommissionsRuleValidator::matchString(const std::string& s, const std::string& x)const
{
  if (s.size() == 1 && s.at(0) == '%')
    return true;
  return matchFragment(s,x);
}

bool
CommissionsRuleValidator::matchTktDesignatorFragmentExclIncl(const std::vector<TktDesignator>& excReqTktDesig)const
{
  return matchString(excReqTktDesig,_tktDesignator.data());
}

//fragment can be Fare basis or excluded/included ticket designator fragments
bool
CommissionsRuleValidator::matchFragment(const std::string& fragment, const std::string& target) const
{
  bool validateFragment = true;
  std::size_t nextWild = 0;
  std::size_t found = 0;
  while (validateFragment)
  {
    if(!matchSubFragment(fragment, nextWild, found, validateFragment,target))
      return false;
    // there is no next '%' exist in the fragment
    if (nextWild == std::string::npos)
      return true;
  }
  return true;
}

bool
CommissionsRuleValidator::matchInterlineConnection(const CommissionRuleInfo& cr) const
{
  if (cr.interlineConnectionRequired() == ' ' ||
      cr.interlineConnectionRequired() == 'N')
    return true;
  if (_fu.travelSeg().size() < 2 )
    return false;

  // current logic is looking for the different marketing carriers on the
  // travel segments to identify the interline
  CarrierCode cxr;
  for(const TravelSeg* tvl : _fu.travelSeg())
  {
    const AirSeg* airSeg(nullptr);
    if (tvl->isAir())
    {
      airSeg = static_cast<const AirSeg*>(tvl);
    }
    if(!airSeg)
      continue;
    if(cxr.empty())
    {
      cxr = airSeg->marketingCarrierCode();
      continue;
    }
    if(airSeg->marketingCarrierCode() != cxr)
      return true;
  }
  return false;
}

bool
CommissionsRuleValidator::matchConnectionAirports(const CommissionRuleInfo& cr,
                                                  CommissionValidationStatus& rc) const
{
  if (!cr.excludedCnxAirPCodes().empty() &&
      _fu.travelSeg().size() > 1 &&
      (cr.excludedCnxAirPCodes()[0] == "*ANY*" ||
       matchConnections(cr.excludedCnxAirPCodes(), true)))
  {
    rc = FAIL_CR_EXCL_CONN_AIRPORT;
    return false;
  }

  if (!cr.requiredCnxAirPCodes().empty() &&
      (_fu.travelSeg().size() < 2 ||
       (cr.requiredCnxAirPCodes()[0] != "*ANY*" &&
        !matchConnections(cr.requiredCnxAirPCodes()))))
  {
    rc = FAIL_CR_REQ_CONN_AIRPORT;
    return false;;
  }
  return true;
}

bool
CommissionsRuleValidator::matchConnections(const std::vector<LocCode>& connections,
                                           bool exclude) const
{
  std::set<LocCode> airportCodes;
  for (const TravelSeg* tvl : _fu.travelSeg())
  {
    if(!tvl->isAir())
      continue;

    if(tvl == _fu.travelSeg().front())
      airportCodes.insert(tvl->destAirport());
    if(tvl == _fu.travelSeg().back())
      airportCodes.insert(tvl->origAirport());
    if(tvl != _fu.travelSeg().front() &&
       tvl != _fu.travelSeg().back())
    {
      airportCodes.insert(tvl->destAirport());
      airportCodes.insert(tvl->origAirport());
    }
  }

  for (const LocCode ap : airportCodes)
  {
    if ( exclude && std::find(connections.begin(), connections.end(), ap) != connections.end())
      return true;
    if (!exclude && std::find(connections.begin(), connections.end(), ap) == connections.end())
      return false;
  }
  return !exclude;
}

bool
CommissionsRuleValidator::matchRequiredNonStop(const CommissionRuleInfo& cr) const
{
  // current logic is based on the various combinations of the cities and airports
  if (cr.requiredNonStop().empty())
    return true;

  for (const TravelSeg* tvl : _fu.travelSeg())
  {
    if(!tvl->isAir())
      continue;

    std::vector<CommissionRuleInfo::OriginDestination> origDest;
    origDest.emplace_back(tvl->origAirport(),tvl->destAirport());
    origDest.emplace_back(tvl->boardMultiCity(),tvl->offMultiCity());
    origDest.emplace_back(tvl->origin()->loc(),tvl->destination()->loc());
    origDest.emplace_back(tvl->origAirport(),tvl->offMultiCity());
    origDest.emplace_back(tvl->origAirport(),tvl->destination()->loc());
    origDest.emplace_back(tvl->boardMultiCity(),tvl->destAirport());
    origDest.emplace_back(tvl->boardMultiCity(),tvl->destination()->loc());
    origDest.emplace_back(tvl->origin()->loc(),tvl->destAirport());
    origDest.emplace_back(tvl->origin()->loc(),tvl->offMultiCity());

    for(const CommissionRuleInfo::OriginDestination od : origDest)
    {
      if(std::find(cr.requiredNonStop().begin(),
                   cr.requiredNonStop().end(), od) != cr.requiredNonStop().end())
        return true;
    }
  }
  return false;
}

bool
CommissionsRuleValidator::matchRoundTrip(const CommissionRuleInfo& cr) const
{
  // current logic is based on the journey RT determination,
  // origin multi city equal destination multi city
  // Milorad: much airports not cities
  if (cr.roundTrip() == ' ')
    return true;
/*
  if(( cr.roundTrip() == 'X' &&
       _fp.itin()->tripCharacteristics().isSet(Itin::RoundTrip)) ||
     (cr.roundTrip() == 'R' &&
      !_fp.itin()->tripCharacteristics().isSet(Itin::RoundTrip)) )
    return false;
*/
  if(!_fp.itin() || _fp.itin()->travelSeg().empty() ||
     !_fp.itin()->travelSeg().front() || !_fp.itin()->travelSeg().back())
    return false;

  if(( cr.roundTrip() == 'R' &&
     (_fp.itin()->travelSeg().front()->origAirport() == _fp.itin()->travelSeg().back()->destAirport())) ||
     ( cr.roundTrip() == 'X' &&
     (_fp.itin()->travelSeg().front()->origAirport() != _fp.itin()->travelSeg().back()->destAirport())))
    return true;

  return false;
}

void
CommissionsRuleValidator::getFareBasis()
{
  // get resulting farebasis and ticket designator
  // source - PricingResponseFormatter
  _fareBasis = _fu.paxTypeFare()->createFareBasis(_trx, false);
  std::string::size_type posBasis = _fareBasis.find("/");
  if (posBasis != std::string::npos)
  {
    _tktDesignator = _fareBasis.substr(posBasis + 1, _fareBasis.size() - 1);
    if (!fallback::fallbackAMCFareBasisFix(&_trx))
      _fareBasis.erase(posBasis);
    else
      _fareBasis.substr(0, posBasis);
  }
}

void
CommissionsRuleValidator::getClassOfService()
{
  uint16_t indexTvlSeg = 0;
  const TravelSeg* pTvlSeg = getPrimarySector(indexTvlSeg);
  if(pTvlSeg == 0)
    pTvlSeg = *(_fu.travelSeg().begin());

  const PaxTypeFare::SegmentStatus& fuSegStat = _fu.segmentStatus()[indexTvlSeg];
  const DifferentialData* diff = farepathutils::differentialData(&_fu, pTvlSeg);

  if (diff != nullptr)
  {
    const PaxTypeFare::SegmentStatus& diffSegStat = farepathutils::diffSegStatus(diff, pTvlSeg);
    if (diffSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
        !(diffSegStat._bkgCodeReBook.empty()))
    {
      _rbd = diffSegStat._bkgCodeReBook;
      return;
    }
    else
    {
      _rbd = pTvlSeg->getBookingCode();
      return;
    }
  }

  if (fuSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
      !(fuSegStat._bkgCodeReBook.empty()))
  {
    _rbd = fuSegStat._bkgCodeReBook;
    return;
  }

  _rbd = pTvlSeg->getBookingCode();
  return;
}

const TravelSeg*
CommissionsRuleValidator::getPrimarySector(uint16_t& indexTvlSeg)
{
  const TravelSeg* pTvlSeg = _fu.paxTypeFare()->fareMarket()->primarySector();
  if(pTvlSeg == 0)
    pTvlSeg = *(_fu.travelSeg().begin());

  if( _fu.travelSeg().size() == 1 )
    indexTvlSeg = 0;
  else
  {
    for( const TravelSeg* tvl : _fu.travelSeg() )
    {
      if ( tvl->segmentOrder() == pTvlSeg->segmentOrder())
        break;
      ++indexTvlSeg;
    }
  }
  return pTvlSeg;
}

bool
CommissionsRuleValidator::matchFareBasisFragment(const std::string& fbrFragment) const
{
  bool validateFragment = true;
  std::size_t nextWild = 0;
  std::size_t found = 0;

  while (validateFragment)
  {
    if(!matchFareBasisSubFragment(fbrFragment, nextWild, found, validateFragment))
      return false;
    if (nextWild == std::string::npos)
      return true;
  }
  return true;
}

// Target can be FareBasis or Excluded/Required Ticket Designator codes
bool
CommissionsRuleValidator::matchSubFragment(const std::string& fragment,
                                                    std::size_t& nextWild,
                                                    std::size_t& found,
                                                    bool& validateFragment,
                                                    const std::string& wildcard) const
{
  std::size_t prevWild = nextWild;
  std::string sub_string;
  bool percentAtBegin = false;
  const char* const wild = "%";
  // check 1st or next enter
  if(nextWild == 0 && fragment.at(0) == *wild)
  {
    percentAtBegin = true;
  }
  // find next '%' or end of string
  if(percentAtBegin)
    nextWild = fragment.find(wild, 1);
  else
    nextWild = fragment.find(wild, nextWild + 1);

  if (nextWild == std::string::npos)
  {
    if(percentAtBegin) // 1st enter and starts with %
      sub_string = fragment.substr(1, fragment.size() - 1);
    else if(prevWild == 0) // 1st enter and not starts with %
      sub_string = fragment.substr(prevWild, (fragment.size() - prevWild));
    else
      sub_string = fragment.substr(prevWild + 1, (fragment.size() - prevWild + 1));
  }
  else  // next % (not at starts) is found
  {
    if(percentAtBegin)  // 1st enter and starts with %
      sub_string = fragment.substr(1, nextWild - 1);
    else if(prevWild == 0)  // 1st enter and not starts with %
    {
      if(fragment.at(0) != wildcard.at(0))  // 1st chars of Wildcard and fragment must match
        return false;
      sub_string = fragment.substr(prevWild, nextWild);
    }
    else
      sub_string = fragment.substr(prevWild + 1 , nextWild - (prevWild + 1));
  }

  if(sub_string.size() < 1)
    return false;
  // find fragment substring in the fare basis code
  found = wildcard.find(sub_string, found);
  if(found == std::string::npos)
    return false;
  // sub string found and calculate an index for the next enter
  found += sub_string.size();
  // next % not exists but the Wildcard code has not matched leftover
  if(nextWild == std::string::npos && found < wildcard.size())
    return false;
  // next % exists and its at the end of the fragment - we DONE
  if(nextWild != std::string::npos && nextWild == (fragment.size() - 1 ))
    validateFragment = false;

  return true;
}

bool
CommissionsRuleValidator::matchFareBasisSubFragment(const std::string& fbrFragment,
                                                    std::size_t& nextWild,
                                                    std::size_t& found,
                                                    bool& validateFragment) const
{
  std::size_t prevWild = nextWild;
  std::string sub_string;
  bool percentAtBegin = false;
  const char* const wild = "%";
  // check 1st or next enter
  if(nextWild == 0 && fbrFragment.at(0) == *wild)
  {
    percentAtBegin = true;
  }
  // find next '%' or end of string
  if(percentAtBegin)
    nextWild = fbrFragment.find(wild, 1);
  else
    nextWild = fbrFragment.find(wild, nextWild + 1);

  if (nextWild == std::string::npos)
  {
    if(percentAtBegin) // 1st enter and starts with %
      sub_string = fbrFragment.substr(1, fbrFragment.size() - 1);
    else if(prevWild == 0) // 1st enter and not starts with %
      sub_string = fbrFragment.substr(prevWild, (fbrFragment.size() - prevWild));
    else
      sub_string = fbrFragment.substr(prevWild + 1, (fbrFragment.size() - prevWild + 1));
  }
  else  // next % (not at starts) is found
  {
    if(percentAtBegin)  // 1st enter and starts with %
      sub_string = fbrFragment.substr(1, nextWild - 1);
    else if(prevWild == 0)  // 1st enter and not starts with %
    {
      if(fbrFragment.at(0) != _fareBasis.at(0))  // 1st chars of FBC and fragment must match
        return false;
      sub_string = fbrFragment.substr(prevWild, nextWild);
    }
    else
      sub_string = fbrFragment.substr(prevWild + 1 , nextWild - (prevWild + 1));
  }

  if(sub_string.size() < 1)
    return false;
  // find fragment substring in the fare basis code
  found = _fareBasis.find(sub_string, found);
  if(found == std::string::npos)
    return false;
  // sub string found and calculate an index for the next enter
  found += sub_string.size();
  // next % not exists but Fare Basis code has not matched leftover
  if(nextWild == std::string::npos && found < _fareBasis.size())
    return false;
  // next % exists and its at the end of the fragment - we DONE
  if(nextWild != std::string::npos && nextWild == (fbrFragment.size() - 1 ))
    validateFragment = false;

  return true;
}
} //tse
