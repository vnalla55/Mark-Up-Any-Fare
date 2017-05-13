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
#pragma once

#include "Rules/BusinessRuleApplicator.h"
#include "DataModel/Common/Types.h"

namespace tax
{

class BusinessRule;
class PaymentDetail;
class GeoPath;

class ReturnToOriginApplicator : public BusinessRuleApplicator
{
public:
  ReturnToOriginApplicator(BusinessRule const* parent,
                           GeoPath const& geoPath,
                           type::RtnToOrig const& rtnToOrig);

  ~ReturnToOriginApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  GeoPath const& _geoPath;
  type::RtnToOrig const& _rtnToOrig;
};
}
