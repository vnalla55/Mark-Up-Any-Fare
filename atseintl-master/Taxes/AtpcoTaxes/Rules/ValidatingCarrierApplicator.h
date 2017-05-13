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

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"

#include <memory>

namespace tax
{

class BusinessRule;
class CarrierApplication;
class PaymentDetail;

class ValidatingCarrierApplicator : public BusinessRuleApplicator
{
public:
  ValidatingCarrierApplicator(BusinessRule const* rule,
                              type::CarrierCode const& validatingCarrier,
                              std::shared_ptr<CarrierApplication const> carrierApplication);
  ~ValidatingCarrierApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  type::CarrierCode const& _validatingCarrier;
  std::shared_ptr<CarrierApplication const> _carrierApplication;

  bool hasMatchingEntry(type::CarrierCode const&) const;
  type::CarrierCode const& getValidatingCarrier(OptionalService const&) const;
};

} // namespace tax
