// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include <sstream>

#include "Rules/TaxPointLoc2StopoverTagApplicator.h"
#include "Rules/TaxPointLoc2StopoverTagRule.h"

#include "ServiceInterfaces/Services.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

namespace
{
static const type::TaxMatchingApplTag taxMatchingApplTag03_furthestInDirOfTravel{"03"};
static const type::TaxMatchingApplTag taxMatchingApplTag06_fareBreakMustBeStop{"06"};
static const type::TaxMatchingApplTag taxMatchingApplTag07_lastDomesticPoint{"07"};
}

TaxPointLoc2StopoverTagRule::TaxPointLoc2StopoverTagRule(const type::Loc2StopoverTag& loc2StopoverTag,
                                                         const type::TaxMatchingApplTag& taxMatchingApplTag,
                                                         const type::TaxPointTag& taxPointTag,
                                                         const type::TaxProcessingApplTag& taxProcessingApplTag,
                                                         LocZone taxPointLoc1Zone,
                                                         LocZone taxPointLoc2Zone)
  : _loc2StopoverTag(loc2StopoverTag),
    _taxMatchingApplTag(taxMatchingApplTag),
    _taxPointTag(taxPointTag),
    _taxProcessingApplTag(taxProcessingApplTag),
    _taxPointLoc1Zone(taxPointLoc1Zone),
    _taxPointLoc2Zone(taxPointLoc2Zone)
{
}

TaxPointLoc2StopoverTagRule::~TaxPointLoc2StopoverTagRule() {}

TaxPointLoc2StopoverTagRule::ApplicatorType
TaxPointLoc2StopoverTagRule::createApplicator(const type::Index& itinIndex,
                                              const Request& request,
                                              Services& services,
                                              RawPayments& /*itinPayments*/) const
{
  const Itin& itin = request.getItinByIndex(itinIndex);

  return ApplicatorType(*this, itin, services);
}

std::string
TaxPointLoc2StopoverTagRule::getDescription(Services&) const
{
  std::string afterBefore = (_taxPointTag == type::TaxPointTag::Departure) ? "AFTER" : "BEFORE";
  std::stringstream description;
  description << "LOC2 WILL BE SET TO";

  switch(_loc2StopoverTag)
  {
  case type::Loc2StopoverTag::Stopover:
    description << " FIRST STOPOVER ";
    break;
  case type::Loc2StopoverTag::FareBreak:
    description << " FIRST FARE/FEE BREAK ";
    if (_taxMatchingApplTag == taxMatchingApplTag06_fareBreakMustBeStop)
      description << "AND A STOPOVER";
    break;
  case type::Loc2StopoverTag::Furthest:
    if (_taxMatchingApplTag == taxMatchingApplTag03_furthestInDirOfTravel)
    {
      description << " FURTHEST POINT IN THE DIRECTION OF TRAVEL ";
    }
    else if (_taxMatchingApplTag == taxMatchingApplTag07_lastDomesticPoint)
    {
      description << " LAST DOMESTIC POINT BEFORE INTERNATIONAL ";
    }
    else
    {
      description << " FURTHEST POINT FARE/FEE COMPONENT ";
    }
    break;
  default: // Blank
    description << " FIRST POINT ";
  }

  description << afterBefore << " LOC1";

  return description.str();
}
}
