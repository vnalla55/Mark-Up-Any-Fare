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
#include <boost/lexical_cast.hpp>

#include "Common/MoneyUtil.h"
#include "DomainDataObjects/Request.h"
#include "Rules/PercentageTaxRule.h"
#include "Rules/PercentageTaxApplicator.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{

PercentageTaxRule::PercentageTaxRule(type::Percent const& taxPercentage,
                                     type::CurrencyCode const& taxCurrency,
                                     TaxableUnitTagSet const& applicableTaxableUnits,
                                     const type::Index serviceBaggageItemNo,
                                     type::Vendor const& vendor,
                                     type::ServiceBaggageApplTag const& serviceBaggageApplTag)
  : _taxPercentage(taxPercentage / 1000000),
    _taxCurrency(taxCurrency),
    _taxableUnitSet(applicableTaxableUnits),
    _serviceBaggageItemNo(serviceBaggageItemNo),
    _vendor(vendor),
    _serviceBaggageApplTag(serviceBaggageApplTag)
{
}

PercentageTaxRule::~PercentageTaxRule() {}

PercentageTaxRule::ApplicatorType
PercentageTaxRule::createApplicator(type::Index const& /*itinIndex*/,
                                    const Request& request,
                                    Services& services,
                                    RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, services, request.ticketingOptions().paymentCurrency());
}

std::string
PercentageTaxRule::getDescription(Services&) const
{
  if (_taxableUnitSet.isEmpty())
    return std::string("PERCENTAGETAX RULE CAN NOT BE APLICABLE IF TAXABLE UNIT TAG IS NOT SET");
  else
  {
    std::string ret =
        "TAX PERCENTAGE " +
        boost::lexical_cast<std::string>(percentToDouble(_taxPercentage)) + " ON ";

    if (_taxableUnitSet.hasTag(type::TaxableUnit::Itinerary))
      ret += "FARE, ";
    if (_taxableUnitSet.hasTag(type::TaxableUnit::YqYr))
      ret += "YQYR, ";
    if (_taxableUnitSet.hasTag(type::TaxableUnit::TaxOnTax))
      ret += "TAX, ";
    if (_taxableUnitSet.hasTag(type::TaxableUnit::OCFlightRelated))
      ret += "FLIGHTRELATED OR PREPAID OC, ";
    if (_taxableUnitSet.hasTag(type::TaxableUnit::OCTicketRelated))
      ret += "TICKETRELATED OC, ";
    if (_taxableUnitSet.hasTag(type::TaxableUnit::OCMerchandise))
      ret += "MERCHANDISE OC, ";
    if (_taxableUnitSet.hasTag(type::TaxableUnit::OCFareRelated))
      ret += "FARERELATED OC, ";
    if (_taxableUnitSet.hasTag(type::TaxableUnit::BaggageCharge))
      ret += "BAGGAGE, ";
    if (_taxableUnitSet.hasTag(type::TaxableUnit::TicketingFee))
      ret += "TICKETINGFEE, ";
    if (_taxableUnitSet.hasTag(type::TaxableUnit::ChangeFee))
      ret += "CHANGEFEE, ";
    return ret;
  }
}
}
