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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputGeoPathMapping.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"

namespace tax
{
class InputYqYr;
}

namespace tse
{
class FacadesUtils
{
public:
  static bool isYqYr(const TaxCode& taxCode);

  static bool findYqyr(const tax::InputYqYr& newYqyr,
                       const tax::InputRequest::InputYqYrs& inputYqYrs,
                       tax::type::Index& foundId);

  static bool
  findGeoPathMapping(const tax::InputGeoPathMapping& newMapping,
                     const tax::InputRequest::InputGeoPathMappings& inputGeoPathMappings,
                     tax::type::Index& foundId);
};
}
