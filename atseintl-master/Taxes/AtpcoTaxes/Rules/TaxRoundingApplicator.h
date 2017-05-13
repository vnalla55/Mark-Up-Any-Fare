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
#include "Rules/TaxRoundingRule.h"

namespace tax
{
class PaymentDetail;
class Services;

class TaxRoundingApplicator : public BusinessRuleApplicator
{
public:
  TaxRoundingApplicator(const TaxRoundingRule* taxRoundingRule,
                        const Services& services,
                        const type::Nation& posNation)
    : BusinessRuleApplicator(taxRoundingRule),
      _services(services),
      _taxRoundingRule(taxRoundingRule),
      _posNation(posNation)
  {
  }

  ~TaxRoundingApplicator() {}
  bool apply(PaymentDetail& paymentDetail) const;
  static type::MoneyAmount unitValue(type::TaxRoundingUnit unit);

  bool
  isRoundingBlank(const type::MoneyAmount& unit,
                const type::TaxRoundingDir dir) const;

  virtual void
  roundOC(PaymentDetail& paymentDetail,
          const type::MoneyAmount& unit,
          const type::TaxRoundingDir& dir) const;

  void
  standardRoundOC(PaymentDetail& paymentDetail,
                  type::MoneyAmount& unit,
                  type::TaxRoundingDir& dir) const;

protected:

  void
  doAtpcoDefaultRounding(PaymentDetail& paymentDetail) const;

  bool
  isAtpcoDefaultRoundingEnabled() const;

  const Services& _services;
  const TaxRoundingRule* _taxRoundingRule;
  const type::Nation _posNation;

  static const type::MoneyAmount _values[11];
};

} // namespace tax
