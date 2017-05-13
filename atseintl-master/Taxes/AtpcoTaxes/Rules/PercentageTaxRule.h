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

#include "Common/TaxableUnitTagSet.h"
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class PercentageTaxApplicator;
class Request;
class Services;

class PercentageTaxRule : public BusinessRule
{
public:
  typedef PercentageTaxApplicator ApplicatorType;
  PercentageTaxRule(type::Percent const& taxPercentage,
                    type::CurrencyCode const& taxCurrency,
                    TaxableUnitTagSet const& applicableTaxableUnits,
                    const type::Index serviceBaggageItemNo,
                    type::Vendor const& vendor,
                    type::ServiceBaggageApplTag const& serviceBaggageApplTag);

  virtual ~PercentageTaxRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  type::Percent const& taxPercentage() const { return _taxPercentage; }

  type::CurrencyCode const& taxCurrency() const { return _taxCurrency; }

  TaxableUnitTagSet const& taxableUnitSet() const { return _taxableUnitSet; }

  type::Index const& serviceBaggageItemNo() const { return _serviceBaggageItemNo; }

  type::ServiceBaggageApplTag const& serviceBaggageApplTag() const
  {
    return _serviceBaggageApplTag;
  }

private:
  type::Percent _taxPercentage;
  type::CurrencyCode _taxCurrency;
  TaxableUnitTagSet _taxableUnitSet;
  type::Index _serviceBaggageItemNo;
  type::Vendor _vendor;
  type::ServiceBaggageApplTag _serviceBaggageApplTag;
};
}

