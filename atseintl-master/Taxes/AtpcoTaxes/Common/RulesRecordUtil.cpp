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

#include "Common/RulesRecordUtil.h"

#include "DataModel/Services/RulesRecord.h"
#include "Rules/MathUtils.h"

#include <boost/assign/std/vector.hpp>
#include <vector>

namespace tax
{

TaxableUnitTagSet
RulesRecordUtil::narrowDownTuts(const TaxableUnitTagSet& taxableUnits,
                                type::ProcessingGroup processingGroup)
{
  using namespace boost::assign;

  std::vector<type::TaxableUnit> narrowedTaxableUnits;
  switch (processingGroup)
  {
  case type::ProcessingGroup::Itinerary:
    narrowedTaxableUnits += type::TaxableUnit::YqYr, type::TaxableUnit::TaxOnTax,
        type::TaxableUnit::Itinerary;
    break;
  case type::ProcessingGroup::OC:
    narrowedTaxableUnits += type::TaxableUnit::OCFlightRelated, type::TaxableUnit::OCTicketRelated,
        type::TaxableUnit::OCMerchandise, type::TaxableUnit::OCFareRelated;
    break;
  case type::ProcessingGroup::OB:
    narrowedTaxableUnits += type::TaxableUnit::TicketingFee;
    break;
  case type::ProcessingGroup::ChangeFee:
    narrowedTaxableUnits += type::TaxableUnit::ChangeFee;
    break;
  case type::ProcessingGroup::Baggage:
    narrowedTaxableUnits += type::TaxableUnit::BaggageCharge;
    break;
  default:
    break;
  }

  TaxableUnitTagSet result = TaxableUnitTagSet::none();
  for (type::TaxableUnit taxableUnit : narrowedTaxableUnits)
  {
    if (taxableUnits.hasTag(taxableUnit))
      result.setTag(taxableUnit);
  }
  return result;
}

type::MoneyAmount
RulesRecordUtil::getTaxAmt(const RulesRecord& rulesRecord)
{
  type::MoneyAmount amount =
      rulesRecord.taxName.percentFlatTag() == type::PercentFlatTag::Flat
          ? MathUtils::adjustDecimal(rulesRecord.taxAmt, rulesRecord.taxCurDecimals)
          : (rulesRecord.taxPercent / 1000000);

  return amount;
}

std::string
RulesRecordUtil::getVendorFullStr(const type::Vendor& vendor)
{
  if (vendor == "ATP")
    return "ATPCO";

  if (vendor == "SABR")
    return "SABRE";

  return vendor.asString();
}

} /* namespace tax */
