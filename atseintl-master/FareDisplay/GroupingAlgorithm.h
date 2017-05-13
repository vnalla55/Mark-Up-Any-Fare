
//-------------------------------------------------------------------
//  Authors:     Abu Islam
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "FareDisplay/Comparator.h"
#include "FareDisplay/Group.h"

namespace tse
{

/* @class GroupingAlgorithm
 * This class is used as a functor class for the std::sort algorithm.
 * For each paxTypeFare, it applies the grouping and sorting criteria
 * to select the appropriate position.
 * Grouping is applied in the order of sequence number(priority number).
 * It process the next group only when the previous group comparison returns EQUAL.
 */

class GroupingAlgorithm : public std::binary_function<PaxTypeFare, PaxTypeFare, bool>
{
public:
  GroupingAlgorithm(std::vector<Group*>& groups) : _groups(groups) {};

  bool operator()(const PaxTypeFare* l, const PaxTypeFare* r)
  {
    if (l == nullptr)
      return true;
    if (r == nullptr)
      return false;

    Comparator::Result _result = Comparator::EQUAL;
    std::vector<Group*>::const_iterator i(_groups.begin());

    for (; i != _groups.end(); ++i)
    {
      if ((*i)->comparator() == nullptr)
        continue;

      (*i)->comparator()->group() = *i;
      _result = (*i)->comparator()->compare(*l, *r);

      if (_result == Comparator::EQUAL)
      {
      }
      else
      {
        break;
      }
    }
    return _result == Comparator::TRUE;
  }

private:
  const std::vector<Group*>& _groups;
};

} // namespace tse

