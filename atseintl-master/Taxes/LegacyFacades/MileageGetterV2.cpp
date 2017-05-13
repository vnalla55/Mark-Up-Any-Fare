// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include <algorithm>
#include <stdexcept>

#include "Common/Assert.h"
#include "Common/GlobalDirectionFinder.h"
#include "Common/Singleton.h"
#include "Common/TseUtil.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MileageSubstitution.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Flight.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/FlightUsage.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/GeoPath.h"
#include "Taxes/Common/TaxMileageUtil.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/MileageGetterV2.h"

namespace tse
{

namespace
{

tax::type::GlobalDirection
translateGlobalDir(const tse::GlobalDirection& globalDir)
{
  switch(globalDir)
  {
  case GlobalDirection::NO_DIR: /* None */
    return tax::type::GlobalDirection::NO_DIR;
  case GlobalDirection::AF: /* via Africa */
    return tax::type::GlobalDirection::AF;
  case GlobalDirection::AL: /* FBR - AllFares incl. EH/TS */
    return tax::type::GlobalDirection::AL;
  case GlobalDirection::AP: /* via Atlantic and Pacific */
    return tax::type::GlobalDirection::AP;
  case GlobalDirection::AT: /* via Atlantic */
    return tax::type::GlobalDirection::AT;
  case GlobalDirection::CA: /* Canada */
    return tax::type::GlobalDirection::CA;
  case GlobalDirection::CT: /* circle trip */
    return tax::type::GlobalDirection::CT;
  case GlobalDirection::DI: /* special USSR - TC3 app. British Airways */
    return tax::type::GlobalDirection::DI;
  case GlobalDirection::DO: /* domestic */
    return tax::type::GlobalDirection::DO;
  case GlobalDirection::DU: /* special USSR - TC2 app. British Airways */
    return tax::type::GlobalDirection::DU;
  case GlobalDirection::EH: /* within Eastern Hemisphere */
    return tax::type::GlobalDirection::EH;
  case GlobalDirection::EM: /* via Europe - Middle East */
    return tax::type::GlobalDirection::EM;
  case GlobalDirection::EU: /* via Europe */
    return tax::type::GlobalDirection::EU;
  case GlobalDirection::FE: /* Far East */
    return tax::type::GlobalDirection::FE;
  case GlobalDirection::IN: /* FBR for intl. incl. AT/PA/WH/CT/PV */
    return tax::type::GlobalDirection::IN;
  case GlobalDirection::ME: /* via Middle East (other than Aden) */
    return tax::type::GlobalDirection::ME;
  case GlobalDirection::NA: /* FBR for North America incl. US/CA/TB/PV */
    return tax::type::GlobalDirection::NA;
  case GlobalDirection::NP: /* via North or Central Pacific */
    return tax::type::GlobalDirection::NP;
  case GlobalDirection::PA: /* via South, Central or North Pacific */
    return tax::type::GlobalDirection::PA;
  case GlobalDirection::PE: /* TC1 - Central/Southern Africa via TC3 */
    return tax::type::GlobalDirection::PE;
  case GlobalDirection::PN: /* between TC1 and TC3 via Pacific and via North America */
    return tax::type::GlobalDirection::PN;
  case GlobalDirection::PO: /* via Polar Route */
    return tax::type::GlobalDirection::PO;
  case GlobalDirection::PV: /* PR/VI - US/CA */
    return tax::type::GlobalDirection::PV;
  case GlobalDirection::RU: /* Russia - Area 3 */
    return tax::type::GlobalDirection::RU;
  case GlobalDirection::RW: /* round the world */
    return tax::type::GlobalDirection::RW;
  case GlobalDirection::SA: /* South Atlantic only */
    return tax::type::GlobalDirection::SA;
  case GlobalDirection::SN
      : /* via South Atlantic (routing permitted in 1 direc. via N./Mid-Atlantic) */
    return tax::type::GlobalDirection::SN;
  case GlobalDirection::SP: /* via South Polar */
    return tax::type::GlobalDirection::SP;
  case GlobalDirection::TB: /* transborder */
    return tax::type::GlobalDirection::TB;
  case GlobalDirection::TS: /* via Siberia */
    return tax::type::GlobalDirection::TS;
  case GlobalDirection::TT: /* Area 2 */
    return tax::type::GlobalDirection::TT;
  case GlobalDirection::US: /* intra U.S. */
    return tax::type::GlobalDirection::US;
  case GlobalDirection::WH: /* within Western Hemisphere */
    return tax::type::GlobalDirection::WH;
  case GlobalDirection::XX: /* Universal */
    return tax::type::GlobalDirection::XX;
  case GlobalDirection::ZZ: /* Any Global */
    return tax::type::GlobalDirection::ZZ;
  default:
    return tax::type::GlobalDirection::NO_DIR;
  }
}

class MatchesGlobalDirection : public std::unary_function<Mileage*, bool>
{
public:
  MatchesGlobalDirection(const tax::type::GlobalDirection &globalDir)
      : _globalDir(globalDir) {}

  bool operator()(const Mileage* const mileage) const
  {
    return translateGlobalDir(mileage->globaldir()) == _globalDir;
  }
private:
  const tax::type::GlobalDirection _globalDir;
};

class MileagesComparator
{
public:
  MileagesComparator(const tax::type::GlobalDirection& globalDir) : _matcher(globalDir) {}

  bool operator()(tse::Mileage* mileage1, tse::Mileage* mileage2) const
  {
    TSE_ASSERT(mileage1 != nullptr && mileage2 != nullptr);
    if (!_matcher(mileage1))
      return true;
    if (!_matcher(mileage2))
      return false;

    return (mileage1->mileage() < mileage2->mileage());
  }
private:
  MatchesGlobalDirection _matcher;
};

} // namespace

MileageGetterV2::MileageGetterV2(const tax::GeoPath& geoPath,
                                 const std::vector<tax::FlightUsage>& flightUsages,
                                 const tax::type::Timestamp& travelDate,
                                 PricingTrx& trx,
                                 bool isRtw)
  : _geoPath(geoPath),
    _flightUsages(flightUsages),
    _travelDate(travelDate),
    _trx(trx),
    _dataHandle(trx.dataHandle()),
    _isRtw(isRtw)
{
}

tax::type::Miles
MileageGetterV2::getSingleDistance(const tax::type::Index& from,
                                   const tax::type::Index& to) const
{
  TSE_ASSERT(from < _geoPath.geos().size() && to < _geoPath.geos().size());
  tax::type::AirportCode city1 = _geoPath.geos()[from].locCode();
  tax::type::AirportCode city2 = _geoPath.geos()[to].locCode();

  if (city1 == city2)
    return tax::type::Miles(0);

  tse::DateTime tseTravelDate(_travelDate.year(),
                              _travelDate.month(),
                              _travelDate.day(),
                              _travelDate.hour(),
                              _travelDate.min());

  tse::GlobalDirection globalDir = getTseGlobalDir(from, to);
  const tse::Loc& loc1 = getLoc(city1, getOriginDate(to / 2));
  const tse::Loc& loc2 = getLoc(city2, getDestinationDate(to / 2));

  return tax::type::Miles(_isRtw ?
      TaxMileageUtil::getGCMDistance(loc1, loc2) :
      TaxMileageUtil::getDistance(loc1, loc2, globalDir, tseTravelDate, _dataHandle));
}

tse::GlobalDirection
MileageGetterV2::getTseGlobalDir(const tax::type::Index& from, const tax::type::Index& to) const
{
  typedef tse::GlobalDirectionFinder::Location Location;
  std::vector<Location> locations;
  std::set<tse::CarrierCode> carriers;

  tse::DateTime tseTravelDate(_travelDate.year(),
                              _travelDate.month(),
                              _travelDate.day(),
                              _travelDate.hour(),
                              _travelDate.min());

  tax::type::AirportCode prevLoc(tax::UninitializedCode);
  for (tax::type::Index i = from; i <= to; ++i)
  {
    const tax::Geo& curGeo = _geoPath.geos()[i];

    if (!prevLoc.empty() && prevLoc == curGeo.locCode())
      continue;

    const tse::Loc *loc = _dataHandle.getLoc(toTseAirportCode(curGeo.locCode()),
                                              _dataHandle.ticketDate());
    if (UNLIKELY(!loc))
      return GlobalDirection::NO_DIR;

    locations.emplace_back(loc, curGeo.isUnticketed());
    const tax::Flight* flight = curGeo.getFlight(_flightUsages);
    if (UNLIKELY(!flight))
      return GlobalDirection::NO_DIR;
    carriers.insert(toTseCarrierCode(flight->operatingCarrier()));

    prevLoc = curGeo.locCode();
  }
  tse::GlobalDirection globalDir;
  GlobalDirectionFinder gdf(locations);
  gdf.getGlobalDirection(&_trx, tseTravelDate, carriers, globalDir);
  return globalDir;
}

tax::type::GlobalDirection
MileageGetterV2::getSingleGlobalDir(const tax::type::Index& from, const tax::type::Index& to) const
{
  return translateGlobalDir(getTseGlobalDir(from, to));
}

const tse::Loc&
MileageGetterV2::getLoc(const tax::type::AirportCode& city, const DateTime& tseTravelDate) const
{
  const tse::Loc* loc = _dataHandle.getLoc(toTseAirportCode(city), tseTravelDate);

  if (LIKELY(loc))
    return *loc;

  const MileageSubstitution* citySubst =
      _dataHandle.getMileageSubstitution(toTseAirportCode(city), tseTravelDate);

  TSE_ASSERT(citySubst);

  loc = _dataHandle.getLoc(citySubst->publishedLoc(), tseTravelDate);
  TSE_ASSERT(loc);
  return *loc;
}

tse::DateTime
MileageGetterV2::getOriginDate(unsigned int segmentIndex) const
{
  return _trx.ticketingDate();
}

tse::DateTime
MileageGetterV2::getDestinationDate(unsigned int segmentIndex) const
{
  return _trx.ticketingDate();
}

} // namespace tse
