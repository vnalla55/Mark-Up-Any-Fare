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
#include <boost/format.hpp>

#include "DataModel/Common/CodeIO.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/SectorDetailService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/RuleDescriptionFormater.h"
#include "Rules/SectorDetailApplicator.h"
#include "Rules/SectorDetailRule.h"

namespace tax
{
SectorDetailRule::SectorDetailRule(type::SectorDetailApplTag const& sectorDetailApplTag,
                                   type::Index const& itemNo,
                                   type::Vendor const& vendor)
  : _sectorDetailApplTag(sectorDetailApplTag), _itemNo(itemNo), _vendor(vendor)
{
}

SectorDetailRule::~SectorDetailRule()
{
}

SectorDetailRule::ApplicatorType
SectorDetailRule::createApplicator(type::Index const& itinIndex,
                                   const Request& request,
                                   Services& services,
                                   RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  FarePath const& farePath = request.farePaths()[itin.farePathRefId()];
  GeoPath const& geoPath = request.geoPaths()[itin.geoPathRefId()];
  GeoPathMapping const* geoPathMapping = itin.geoPathMapping();
  assert (geoPathMapping);
  std::shared_ptr<SectorDetail const> sectorDetail =
      services.sectorDetailService().getSectorDetail(_vendor, _itemNo);

  return ApplicatorType(this,
                        sectorDetail,
                        _sectorDetailApplTag,
                        itin.flightUsages(),
                        request.flights(),
                        request.fares(),
                        farePath.fareUsages(),
                        *geoPathMapping,
                        geoPath);
}

std::string
SectorDetailRule::getDescription(Services& services) const
{
  std::ostringstream buf;
  buf << "RESTRICTIONS FROM SECTOR DETAIL ITEM NUMBER " << _itemNo << " FROM\n"
      << " VENDOR " << _vendor << " MUST BE SATISFIED BY\n";

  if (_sectorDetailApplTag == type::SectorDetailApplTag::AnySectorLoc1Loc2)
    buf << " ANY SECTOR BETWEEN PAYMENT DETAIL BEGIN AND END\n";
  else if (_sectorDetailApplTag == type::SectorDetailApplTag::AnySectorOrginDestination)
    buf << " ANY SECTOR BETWEEN ORIGIN AND DESTINATION\n";
  else if (_sectorDetailApplTag == type::SectorDetailApplTag::FirstInternationalSector)
    buf << " FIRST INTERNATIONAL SECTOR\n";
  else if (_sectorDetailApplTag == type::SectorDetailApplTag::EverySectorInItin)
    buf << " EVERY SECTOR IN ITIN\n";
  else if (_sectorDetailApplTag == type::SectorDetailApplTag::AllSectorLoc1Loc2)
    buf << " ALL SECTORS BETWEEN PAYMENT DETAIL BEGIN AND END\n";

  buf << " ITEM " << _itemNo;
  RuleDescriptionFormater::format(buf,
                                  services.sectorDetailService().getSectorDetail(_vendor, _itemNo));

  return buf.str();
}
}
