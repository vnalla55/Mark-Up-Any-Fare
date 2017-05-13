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

#include "Rules/BusinessRuleApplicator.h"
#include "DataModel/Common/Types.h"

namespace tax
{

class PointOfSaleRule;
class PaymentDetail;
class LocService;

class PointOfSaleApplicator : public BusinessRuleApplicator
{
public:
  PointOfSaleApplicator(const PointOfSaleRule& rule,
                        const type::AirportCode& salePoint,
                        const LocService& locService);

  ~PointOfSaleApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const PointOfSaleRule& _pointOfSaleRule;
  const type::AirportCode& _salePoint;
  const LocService& _locService;
};
}

