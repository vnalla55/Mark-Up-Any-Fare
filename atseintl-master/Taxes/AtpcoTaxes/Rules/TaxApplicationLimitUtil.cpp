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

#include "Rules/TaxApplicationLimitUtil.h"

namespace tax
{

namespace
{
bool
checkLimitType(TaxLimitInfoIter iter)
{
  return iter->isLimit(type::TaxApplicationLimit::OnceForItin) ||
      iter->isLimit(type::TaxApplicationLimit::OncePerSingleJourney);
}

TaxLimitInfoIter findHighestIter(TaxLimitInfoIter first,
                             TaxLimitInfoIter last)
{
  TaxLimitInfoIter result = first;
  for( ; result!= last && !checkLimitType(result); ++result);

  TaxLimitInfoIter tmp = result;
  for( ; tmp != last; ++tmp)
  {
    if (!checkLimitType(tmp))
      continue;

    if (tmp->getMoneyAmount() > result->getMoneyAmount())
      result = tmp;
  }
  return result;
}
}

bool
isLimitOnly(TaxLimitInfoIter first, TaxLimitInfoIter last,
    const type::TaxApplicationLimit& limitType)
{
  for( ; first != last; ++first)
  {
    if (!first->isLimit(limitType))
      return false;
  }

  return true;
}

bool
isLimit(TaxLimitInfoIter first, TaxLimitInfoIter last,
    const type::TaxApplicationLimit& limitType)
{
  for( ; first != last; ++first)
  {
    if (first->isLimit(limitType))
      return true;
  }

  return false;
}

bool
isLimitOnly(TaxLimitInfoIter first, TaxLimitInfoIter last,
    const type::TaxApplicationLimit& firstLimitType,
    const type::TaxApplicationLimit& secondLimitType)
{
  for( ; first != last; ++first)
  {
    if (!first->isLimit(firstLimitType) && !first->isLimit(secondLimitType))
      return false;
  }
  return true;
}


std::vector<type::Index>
findHighest(TaxLimitInfoIter first, TaxLimitInfoIter last)
{
  std::vector<type::Index> result;
  result.push_back(findHighestIter(first, last)->getId());
  return result;
}

std::vector<type::Index>
findHighestForOnceForItin(TaxLimitInfoIter first, TaxLimitInfoIter last, const TaxPointMap& taxPointMap,
                          const std::vector<TaxLimitInfoIter>& blanks)
{
  std::vector<type::Index> result;

  TaxLimitInfoIter highest = first;
  for(; highest!=last && !taxPointMap.isOnlyInItinerary(highest->getId()); ++highest);

  TaxLimitInfoIter tmp = highest;
  for( ; tmp != last; ++tmp)
  {
    if (!taxPointMap.isOnlyInItinerary(tmp->getId()))
      continue;

    auto blankInSamePoint = std::find_if(blanks.begin(), blanks.end(), 
                                         [&](const TaxLimitInfoIter& elem)
                                         {
                                           return elem->getId() == tmp->getId();
                                         });
    if (blankInSamePoint != blanks.end() && *blankInSamePoint < tmp)
    {
      // There is blank (unlimited) application starting in same point as currently processed
      // oncePerItin application (tmp), that has lower sequence number; thus, tmp will be
      // removed at overlapping check and should not be selected as highest oncePerItin
      continue;
    }

    if (tmp->getMoneyAmount() > highest->getMoneyAmount())
      highest = tmp;
  }
  result.push_back(highest->getId());

  return result;
}

std::vector<type::Index>
findHighestAndBlank(TaxLimitInfoIter first, TaxLimitInfoIter last)
{
  TaxLimitInfoIter highest = findHighestIter(first, last);
  std::vector<type::Index> result;

  for( ; first != last; ++first)
  {
    if (first == highest || first->isLimit(type::TaxApplicationLimit::Unlimited))
      result.push_back(first->getId());
  }
  return result;
}

std::vector<type::Index> findFirst(TaxLimitInfoIter first,
                                   TaxLimitInfoIter last)
{
  std::vector<type::Index> result;
  if(first != last)
    result.push_back(first->getId());
  return result;
}

std::vector<type::Index> findFirstTwo(TaxLimitInfoIter first,
                                      TaxLimitInfoIter last)
{
  std::vector<type::Index> result;

  type::Index appliedCount = 0;
  for(TaxLimitInfoIter tmp = first; (tmp != last) && (appliedCount < 2); ++tmp)
  {
    if (tmp->isExempt())
      continue;

    result.push_back(tmp->getId());
    ++appliedCount;
  }

  return result;
}

std::vector<type::Index> findFirstTwoAndBlank(TaxLimitInfoIter first,
                                              TaxLimitInfoIter last)
{
  std::vector<type::Index> result;

  type::Index appliedCount = 0;
  for(TaxLimitInfoIter tmp = first; tmp != last; ++tmp)
  {
    if (tmp->isExempt())
      continue;

    if ((appliedCount < 2) || (tmp->isLimit(type::TaxApplicationLimit::Unlimited)))
    {
      result.push_back(tmp->getId());
      ++appliedCount;
    }
  }

  return result;
}

std::vector<type::Index> findFirstFour(TaxLimitInfoIter first,
                                       TaxLimitInfoIter last)
{
  std::vector<type::Index> result;

  type::Index appliedCount = 0;
  for(TaxLimitInfoIter tmp = first; (tmp != last) && (appliedCount < 4); ++tmp)
  {
    if (tmp->isExempt())
      continue;

    result.push_back(tmp->getId());
    ++appliedCount;
  }

  return result;
}

std::vector<type::Index> findFirstFourAndBlank(TaxLimitInfoIter first,
                                              TaxLimitInfoIter last)
{
  std::vector<type::Index> result;

  type::Index appliedCount = 0;
  for(TaxLimitInfoIter tmp = first; tmp != last; ++tmp)
  {
    if (tmp->isExempt())
      continue;

    if ((appliedCount < 4) || (tmp->isLimit(type::TaxApplicationLimit::Unlimited)))
    {
      result.push_back(tmp->getId());
      ++appliedCount;
    }
  }

  return result;
}

std::vector<TaxLimitInfoIter>
findAllBlank(TaxLimitInfoIter first, TaxLimitInfoIter last)
{
  std::vector<TaxLimitInfoIter> result;
  for(; first != last; ++first)
  {
    if (first->isLimit(type::TaxApplicationLimit::Unlimited))
      result.push_back(first);
  }
  return result;
}

TaxLimitInfoIter findBegin(TaxLimitInfoIter first,
                           TaxLimitInfoIter last,
                           type::Index value)
{
  for ( ; first != last && first->getId() < value; ++first);
  return first;
}

TaxLimitInfoIter findEnd(TaxLimitInfoIter first,
                         TaxLimitInfoIter last,
                         type::Index value)
{
  for( ; first != last && first->getId() < value; ++first);
  return first;
}
}
