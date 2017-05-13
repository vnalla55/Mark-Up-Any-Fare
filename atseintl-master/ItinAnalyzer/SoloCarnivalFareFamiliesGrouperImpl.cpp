// Copyright Sabre 2011
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.

#include "ItinAnalyzer/SoloCarnivalFareFamiliesGrouperImpl.h"

#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"

#include <functional>

namespace
{

typedef tse::SoloCarnivalFareFamiliesGrouperImpl::ItinVec ItinVec;

struct FareFamiliesItinComparator : std::binary_function<const tse::Itin*, const tse::Itin*, bool>
{
public:
  struct SegmentComparator
      : std::binary_function<const tse::TravelSeg*, const tse::TravelSeg*, bool>
  {
    bool isArunk(const tse::TravelSeg* segment) const { return !segment->isAir(); }

    bool operator()(const tse::TravelSeg* lhs, const tse::TravelSeg* rhs) const
    {
      if (isArunk(lhs))
      {
        // for the sake of sorting, let's say arunks go before normal segments
        return !isArunk(rhs);
      }

      if (lhs->origAirport() < rhs->origAirport())
        return true;
      if (rhs->origAirport() < lhs->origAirport())
        return false;

      if (lhs->destAirport() < rhs->destAirport())
        return true;
      if (rhs->destAirport() < lhs->destAirport())
        return false;

      const tse::AirSeg* alhs = dynamic_cast<const tse::AirSeg*>(lhs);
      const tse::AirSeg* arhs = dynamic_cast<const tse::AirSeg*>(rhs);

      if (!alhs || !arhs)
      {
        return static_cast<bool>(alhs) < static_cast<bool>(arhs);
      }

      if (alhs->carrier() < arhs->carrier())
        return true;
      if (arhs->carrier() < alhs->carrier())
        return false;

      return false;
    }
  };

  bool operator()(const tse::Itin* lhs, const tse::Itin* rhs) const
  {
    return std::lexicographical_compare(lhs->travelSeg().begin(),
                                        lhs->travelSeg().end(),
                                        rhs->travelSeg().begin(),
                                        rhs->travelSeg().end(),
                                        SegmentComparator());
  }
};

} // anon namespace

namespace tse
{

void
SoloCarnivalFareFamiliesGrouperImpl::groupWithinFamilies(ItinVec& input)
{
  typedef int FareFamilyType;
  typedef std::map<FareFamilyType, ItinVec> FamiliesType;

  FamiliesType families;

  for (tse::Itin* i : input)
  {
    families[i->getItinFamily()].push_back(i);
  }

  ItinVec result;

  for (FamiliesType::value_type& i : families)
  {
    createFareFamilies(i.second, result);
  }

  input.swap(result);
}

// This will group itins not within original (IS) families only,
// but thru all of the families in common.
void
SoloCarnivalFareFamiliesGrouperImpl::groupThruFamilies(ItinVec& input)
{
  ItinVec result;
  createFareFamilies(input, result);
  input.swap(result);
}

void
SoloCarnivalFareFamiliesGrouperImpl::createFareFamilies(const ItinVec& input, ItinVec& globalResult)
{
  ItinVec result;
  result.reserve(input.size());

  for (tse::Itin* itin : input)
  {
    typedef ItinVec::iterator iterator;
    std::pair<iterator, iterator> range =
        std::equal_range(result.begin(), result.end(), itin, FareFamiliesItinComparator());

    if (range.first == range.second)
    {
      result.insert(range.first, itin);
    }
    else
    {
      (*range.first)->addSimilarItin(itin);
    }
  }

  globalResult.insert(globalResult.end(), result.begin(), result.end());
}

} // namespace tse
