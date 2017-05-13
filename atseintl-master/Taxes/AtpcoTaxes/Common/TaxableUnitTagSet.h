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
#include "Common/TagSet.h"
#include "DataModel/Common/SafeEnums.h"

namespace tax
{

struct TaxableUnitTagToBitmask
{
  typedef type::TaxableUnit enum_type;

  static unsigned toBit(enum_type val)
  {
    return 1U << static_cast<unsigned>(val);
  }

  static unsigned all()
  {
    static const unsigned ans = toBit(type::TaxableUnit::YqYr) |
                                toBit(type::TaxableUnit::TicketingFee) |
                                toBit(type::TaxableUnit::OCFlightRelated) |
                                toBit(type::TaxableUnit::OCTicketRelated) |
                                toBit(type::TaxableUnit::OCMerchandise) |
                                toBit(type::TaxableUnit::OCFareRelated) |
                                toBit(type::TaxableUnit::BaggageCharge) |
                                toBit(type::TaxableUnit::TaxOnTax) |
                                toBit(type::TaxableUnit::Itinerary) |
                                toBit(type::TaxableUnit::ChangeFee);
    return ans;
  }
};

class TaxableUnitTagSet : public TagSet<TaxableUnitTagToBitmask>
{
public:
  TaxableUnitTagSet(TagSet<TaxableUnitTagToBitmask> const& s) : TagSet<TaxableUnitTagToBitmask>(s) {}

  bool hasItineraryTags() const
  {
    return (hasTag(type::TaxableUnit::Itinerary) || hasTag(type::TaxableUnit::TaxOnTax) ||
        hasTag(type::TaxableUnit::YqYr));
  }

  bool hasChangeFeeTags() const
  {
    return hasTag(type::TaxableUnit::ChangeFee);
  }

  bool hasTicketingFeeTags() const
  {
    return hasTag(type::TaxableUnit::TicketingFee);
  }

  bool hasAncillaryTags() const
  {
    return
      (hasTag(type::TaxableUnit::OCFlightRelated) || hasTag(type::TaxableUnit::OCTicketRelated) ||
       hasTag(type::TaxableUnit::OCMerchandise) || hasTag(type::TaxableUnit::OCFareRelated));
  }

  bool hasBaggageTags() const
  {
    return hasTag(type::TaxableUnit::BaggageCharge);
  }
};

} // namespace tax

