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


#include "Taxes/LegacyFacades/FacadesUtils.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputGeoPathMapping.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"

namespace tse
{
bool
FacadesUtils::isYqYr(const TaxCode& taxCode)
{
  if (taxCode.size() < 3)
  {
    return false;
  }

  return ((taxCode[0] == 'Y') &&
          (taxCode[1] == 'Q' || taxCode[1] == 'R') &&
          (taxCode[2] == 'F' || taxCode[2] == 'I'));
}

bool
FacadesUtils::findYqyr(const tax::InputYqYr& newYqyr,
                       const tax::InputRequest::InputYqYrs& inputYqYrs,
                       tax::type::Index& foundId)
{
  for (const tax::InputYqYr& yqyr : inputYqYrs)
  {
    if (newYqyr == yqyr)
    {
      foundId = yqyr._id;
      return true;
    }
  }

  return false;
}

bool
FacadesUtils::findGeoPathMapping(
    const tax::InputGeoPathMapping& newMapping,
    const tax::InputRequest::InputGeoPathMappings& inputGeoPathMappings,
    tax::type::Index& foundId)
{
  for (const tax::InputGeoPathMapping& mapping : inputGeoPathMappings)
  {
    if (newMapping == mapping)
    {
      foundId = mapping._id;
      return true;
    }
  }

  return false;
}
}
