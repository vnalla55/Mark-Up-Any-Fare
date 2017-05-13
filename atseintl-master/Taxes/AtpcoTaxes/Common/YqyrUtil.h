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

#include "DataModel/Common/Types.h"

namespace tax
{
class Itin;
class ProrateCalculator;
class Request;
class TaxableYqYrs;
class GeoPath;

type::MoneyAmount
getTaxableAmount(const ProrateCalculator& prorateCalculator,
                 const type::TaxAppliesToTagInd& taxAppliesToTagInd,
                 const type::Index& paymentBegin,
                 TaxableYqYrs& yqYrDetails,
                 const GeoPath& geoPath,
                 bool skipHiddenPoints = true);

type::MoneyAmount
getYqyrAmountForItin(const Itin& itin,
                     const Request& request);

} // namespace tax
