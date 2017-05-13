// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include <map>
#include <tuple>
#include <vector>

#include "Taxes/AtpcoTaxes/DataModel/Common/Codes.h"

namespace tse
{
  class FarePath;
  class Itin;
  class TaxItem;
}

namespace tax
{
  typedef std::tuple<const tse::Itin*, const tse::FarePath*, const type::CarrierCode> ServicesFeesMapKey;
  typedef std::vector<tse::TaxItem*> ServicesFeesMapVal;
  typedef std::map<ServicesFeesMapKey, ServicesFeesMapVal> ServicesFeesMap;
}

