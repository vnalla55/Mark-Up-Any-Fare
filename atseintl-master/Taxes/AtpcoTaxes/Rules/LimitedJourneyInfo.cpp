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

#include "Rules/LimitedJourneyInfo.h"

namespace tax
{

std::vector<type::Index>
ItineraryInfo::findPassedRules() const
{
  std::vector<type::Index> result;

  std::vector<TaxLimitInfoIter> blanks;
  if (isLimit(getItinFirst(), getItinLast(), type::TaxApplicationLimit::Unlimited))
  {
    blanks = findAllBlank(getItinFirst(), getItinLast());
  }

  for (auto& blank : blanks)
  {
    result.push_back(blank->getId());
  }

  if (isLimit(getItinFirst(), getItinLast(), type::TaxApplicationLimit::OnceForItin))
  {
    std::vector<type::Index> highest = findHighestForOnceForItin(getItinFirst(), getItinLast(), _taxPointMap, blanks);
    result.insert(result.end(), highest.begin(), highest.end());
  }

  return result;
}

std::vector<type::Index>
ContinuousJourneyInfo::findPassedRules() const
{
  if (isLimitOnly(getItinFirst(), getItinLast(), type::TaxApplicationLimit::FirstTwoPerContinuousJourney)
      || isLimit(getItinFirst(), getItinLast(), type::TaxApplicationLimit::OnceForItin))
    return findFirstTwo(getFirst(), getLast());
  else if (isLimit(getItinFirst(), getItinLast(), type::TaxApplicationLimit::Unlimited))
    return findFirstTwoAndBlank(getFirst(), getLast());

  return std::vector<type::Index>();
}

std::vector<type::Index>
SingleJourneyInfo::findPassedRules() const
{
  if (isLimitOnly(getItinFirst(), getItinLast(), type::TaxApplicationLimit::OncePerSingleJourney))
    return findFirst(getFirst(), getLast());
  else if(isLimit(getItinFirst(), getItinLast(), type::TaxApplicationLimit::OnceForItin))
    return findHighest(getFirst(), getLast());
  else if(isLimit(getItinFirst(), getItinLast(), type::TaxApplicationLimit::Unlimited))
    return findHighestAndBlank(getFirst(), getLast());

  return std::vector<type::Index>();
}

std::vector<type::Index>
UsOneWayTripInfo::findPassedRules() const
{
  if (isLimitOnly(getItinFirst(), getItinLast(), type::TaxApplicationLimit::Unused)
      || isLimit(getItinFirst(), getItinLast(), type::TaxApplicationLimit::OnceForItin))
    return findFirstTwo(getFirst(), getLast());
  else if (isLimit(getItinFirst(), getItinLast(), type::TaxApplicationLimit::Unlimited))
    return findFirstTwoAndBlank(getFirst(), getLast());

  return std::vector<type::Index>();
}

std::vector<type::Index>
UsRoundTripInfo::findPassedRules() const
{
  if (isLimitOnly(getItinFirst(), getItinLast(), type::TaxApplicationLimit::FirstTwoPerUSRoundTrip)
      || isLimit(getItinFirst(), getItinLast(), type::TaxApplicationLimit::OnceForItin))
    return findFirstTwo(getFirst(), getLast());
  else if (isLimit(getItinFirst(), getItinLast(), type::TaxApplicationLimit::Unlimited))
    return findFirstTwoAndBlank(getFirst(), getLast());

  return std::vector<type::Index>();
}
}
