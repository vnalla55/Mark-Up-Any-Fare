//----------------------------------------------------------------------------
// File:        Diag997Collector.cpp
//
// Authors:     Bartosz Kolpanowicz
//              Oleksiy Shchukin
//
// Created:     Sep 2011
//
// Description: Diagnostic 997 formatter
//
// Copyright Sabre 2011
//
//              The copyright to the computer program(s) herein
//              is the property of Sabre.
//              The program(s) may be used and/or copied only with
//              the written permission of Sabre or in accordance
//              with the terms and conditions stipulated in the
//              agreement/contract under which the program(s)
//              have been supplied.
//
// Updates:     2011-10-xx  sg925220  SC13 - Write function which compares prices
//                                    of all groups of itienraries and selects
//                                    the cheapest one.
//
//----------------------------------------------------------------------------
#include "Diagnostic/Diag997Collector.h"

#include "Common/Assert.h"
#include "DataModel/SOLItinGroups.h"

namespace tse
{
namespace
{

const std::string WORD_ITIN = "ITINERARY";
const std::string WORD_GROUP = "GROUP";
const std::string WORD_SUBITIN = "SUBITIN";

const std::string HEADER = "\n"
                           "******************************************************\n"
                           "*  997 : SOLO CARNIVAL - GROUPS OF SUB-ITINERARIES   *\n"
                           "******************************************************\n";
const std::string HEADER_SUMMARY = "\n"
                                   "******************************************************\n"
                                   "*  997 : SOLO CARNIVAL - PRICE SUMMARY               *\n"
                                   "******************************************************\n";
const std::string SEPARATOR = "------------------------------------------------------\n";

/**
 * prints Itin price as
 *
 * GROUP 1 - SUB-ITINERARY 1 - 2115.56 EUR
 *
 * or as
 *
 * GROUP 1 - SUB-ITINERARY 1 - CANNOT BE PRICED
 */
struct ItinPrice
{
  const SOLItinGroups::ItinGroup& itinGroup;
  const int itinIndex;
  const int itinGroupOrdNo;

  ItinPrice(const SOLItinGroups::ItinGroup& arg1, int arg2, int arg3)
    : itinGroup(arg1), itinIndex(arg2), itinGroupOrdNo(arg3)
  {
  }

  friend std::ostream& operator<<(std::ostream& dc, const ItinPrice& op)
  {
    if (boost::optional<Money> price = op.itinGroup.getItinPrice(op.itinIndex))
    {
      dc << WORD_GROUP << " " << op.itinGroupOrdNo << " - " << WORD_SUBITIN << " "
         << (op.itinIndex + 1) << " TOTAL - " << *price << "\n";
    }
    else
    {
      dc << WORD_GROUP << " " << op.itinGroupOrdNo << " - " << WORD_SUBITIN << " "
         << (op.itinIndex + 1) << " - CANNOT BE PRICED\n";
    }

    return dc;
  }
};

/**
 * prints ItinGroup price as
 *
 * GROUP 1 TOTAL - 2115.56 EUR
 *
 * or
 *
 * GROUP 2 TOTAL - 2115.56 EUR, *** THE CHEAPEST ONE ***
 *
 * or
 *
 * GROUP 3 - CANNOT BE PRICED
 */
struct GroupPrice
{
  const SOLItinGroups& itinGroups;
  const std::size_t itinGroupIdx;

  GroupPrice(const SOLItinGroups& arg1, std::size_t arg2) : itinGroups(arg1), itinGroupIdx(arg2) {}

  friend std::ostream& operator<<(std::ostream& dc, const GroupPrice& op)
  {
    const SOLItinGroups::ItinGroup* itinGroup = op.itinGroups.itinGroups()[op.itinGroupIdx];
    TSE_ASSERT(itinGroup);

    if (boost::optional<Money> price = itinGroup->getTotalPrice())
    {
      dc << WORD_GROUP << " " << (op.itinGroupIdx + 1) << " TOTAL - " << *price;

      boost::optional<SOLItinGroups::GroupType> cheapestGroup =
          op.itinGroups.getCheapestItinGroup();
      if (cheapestGroup && static_cast<SOLItinGroups::GroupType>(op.itinGroupIdx) ==
                               *cheapestGroup) // the same instance check
      {
        dc << ", *** THE CHEAPEST ONE ***";
      }

      dc << "\n";
    }
    else
    {
      dc << WORD_GROUP << " " << (op.itinGroupIdx + 1) << " - CANNOT BE PRICED\n";
    }

    return dc;
  }
};

template <typename T>
struct OptionalToStream
{
  const boost::optional<T> value;
  const std::string novalue_string;

  OptionalToStream(const boost::optional<T>& arg1, const std::string& arg2)
    : value(arg1), novalue_string(arg2)
  {
  }

  friend std::ostream& operator<<(std::ostream& dc, const OptionalToStream& op)
  {
    if (op.value)
      dc << *op.value;
    else
      dc << op.novalue_string;

    return dc;
  }
};

template <typename T>
OptionalToStream<T>
make_optional(const T value, const T novalue, const std::string& novalue_string)
{
  if (value != novalue)
    return OptionalToStream<T>(value, novalue_string);
  else
    return OptionalToStream<T>(boost::none, novalue_string);
}

void
printItinIdentification(DiagCollector& dc, const Itin& itin)
{
  dc << ", NUM: ";
  dc << make_optional<IntIndex>(itin.itinNum(),
                                /*if =*/INVALID_INT_INDEX,
                                /*then*/ "N/A");
  if (itin.isHeadOfFamily())
  {
    dc << ", HEAD OF FAMILY: ";
  }
  else
  {
    dc << ", FAMILY: ";
  }
  dc << make_optional<IntIndex>(itin.getItinFamily(),
                                /*if =*/INVALID_INT_INDEX,
                                /*then*/ "N/A");
  dc << '\n';
}

void
printSummaryStatistics(DiagCollector& dc, const PricingTrx& trx)
{
  typedef tse::PricingTrx::SOLItinGroupsMap::value_type GroupPairType;
  typedef boost::optional<SOLItinGroups::GroupType> OptGroupType;

  std::map<OptGroupType, size_t> counters;

  for (const GroupPairType& group : trx.solItinGroupsMap())
  {
    OptGroupType cheapestGroup = group.second->getCheapestItinGroup();
    ++counters[cheapestGroup];
  }

  dc << HEADER_SUMMARY;

  dc << "COULD NOT PRICE              : " << counters[OptGroupType()] << "\n";

  dc << "CHEAPEST ORIGINAL            : " << counters[SOLItinGroups::ORIGINAL] << "\n";

  dc << "CHEAPEST IL_CXR_AND_LEG      : " << counters[SOLItinGroups::INTERLINE_BY_CXR_AND_LEG]
     << "\n";

  dc << "CHEAPEST IL_CXR_GROUPING     : " << counters[SOLItinGroups::INTERLINE_BY_CXR_GROUPING]
     << "\n";

  dc << "CHEAPEST OL_LEG              : " << counters[SOLItinGroups::ONLINE_BY_LEG] << "\n";

  dc << "CHEAPEST OL_DOM_INT_GROUPING : " << counters[SOLItinGroups::ONLINE_BY_DOM_INT_GROUPING]
     << "\n";

  dc << SEPARATOR;
}

} // anonymous namespace

Diag997Collector& Diag997Collector::operator<<(const PricingTrx& trx)
{
  if (_active)
  {
    DiagCollector& dc = *this;

    typedef tse::SOLItinGroups::ItinGroup::const_iterator ItinGroupIt;
    typedef tse::SOLItinGroups::ItinGroupVec::const_iterator ItinGroupVecIt;
    typedef tse::PricingTrx::SOLItinGroupsMap::const_iterator SOLItinGroupsMapIt;

    const tse::PricingTrx::SOLItinGroupsMap& itinGroupsMap = trx.solItinGroupsMap();
    SOLItinGroupsMapIt mIt = itinGroupsMap.begin();
    SOLItinGroupsMapIt mEndIt = itinGroupsMap.end();

    dc << HEADER;

    for (int solItinMapOrdNo = 1; mIt != mEndIt; ++mIt, ++solItinMapOrdNo)
    {
      if (!mIt->first)
        continue;

      const Itin& originalItin = *mIt->first;

      // ------------------
      // ITINERARY [number]
      // ------------------
      dc << std::endl << SEPARATOR << WORD_ITIN << " " << solItinMapOrdNo;
      printItinIdentification(dc, originalItin);
      dc << SEPARATOR;

      // [main itinerary contents]
      dc << printItinMIP(originalItin);

      if (!mIt->second)
        continue;

      SOLItinGroups& itinGroups = *mIt->second;
      ItinGroupVecIt v1It = itinGroups.itinGroups().begin();
      ItinGroupVecIt v1EndIt = itinGroups.itinGroups().end();

      // Groups of sub-itineraries.
      for (int itinGroupOrdNo = 1; v1It != v1EndIt; ++v1It, ++itinGroupOrdNo)
      {
        if (!*v1It)
          continue;

        const tse::SOLItinGroups::ItinGroup& itinGroup = **v1It;
        ItinGroupIt v2It = itinGroup.begin();
        ItinGroupIt v2EndIt = itinGroup.end();

        // Sub-itineraries from a specific group.
        for (int subItinOrdNo = 1; v2It != v2EndIt; ++v2It, ++subItinOrdNo)
        {
          if (!*v2It)
            continue;

          const Itin& subItin = **v2It;
          const int subItinIdx = subItinOrdNo - 1;

          // ---------------------------------------
          // GROUP [number] - SUB-ITINERARY [number]
          // ---------------------------------------
          dc << SEPARATOR;
          dc << WORD_GROUP << " " << itinGroupOrdNo << " - " << WORD_SUBITIN << " " << subItinOrdNo;
          printItinIdentification(dc, subItin);
          dc << SEPARATOR;

          // [sub-itinerary contents]
          dc << printItinMIP(subItin);

          // [sub-itinerary farecalc line]
          if (!itinGroup.getItinFarecalcLine(subItinIdx).empty())
            dc << " \"" << itinGroup.getItinFarecalcLine(subItinIdx) << "\"" << std::endl;
          else
            dc << " NO FARECALC LINE" << std::endl;

          // [sub-itinerary price]
          dc << SEPARATOR;
          dc << ItinPrice(itinGroup, subItinIdx, itinGroupOrdNo);

        } // for subItinOrdNo

        dc << SEPARATOR;
        dc << GroupPrice(itinGroups, /*groupIndex*/(itinGroupOrdNo - 1));

      } // for j

      dc << SEPARATOR;

    } // for itinGroupOrdNo

    printSummaryStatistics(dc, trx);

    dc << std::endl << std::endl;
  } // if _active

  return *this;
} // Diag997Collector::operator<<(const PricingTrx& trx)

} // namespace tse
