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
#include <vector>

#include "Common/TaxableUnitTagSet.h"
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"
#include "DomainDataObjects/YqYr.h"

namespace tax
{
class BusinessRule;
class OptionalService;
class PaymentDetail;
class Services;

class FlatTaxApplicator : public BusinessRuleApplicator
{
public:
  FlatTaxApplicator(const BusinessRule* parent,
                    const Services& services,
                    const TaxableUnitTagSet& taxableUnitSet,
                    const type::CurrencyCode& paymentCurrency);
  ~FlatTaxApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  void setOcTaxEquivalentAmount(const type::Money& publishedAmount,
                                OptionalService& oc) const;

  const Services& _services;

  const TaxableUnitTagSet& _taxableUnitSet;
  const type::CurrencyCode& _paymentCurrency;
};
}

