//-------------------------------------------------------------------
//  Created:Jul 18, 2005
//  Author:Abu
//  Copyright Sabre 2003
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

#include "FareDisplay/CharCombinationsComparator.h"

#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDisplaySort.h"
#include "DBAccess/FDSFareBasisComb.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
CharCombinationsComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (_priorityMap.empty())
  {
    return Comparator::EQUAL;
  }
  else
    return compareChars(l.fareClass().c_str(), r.fareClass().c_str());
}

void
CharCombinationsComparator::prepare(const FareDisplayTrx& trx)
{
  if (_group->sortData() != nullptr)
  {

    const std::vector<FDSFareBasisComb*> fbCombList =
        trx.dataHandle().getFDSFareBasisComb(_group->sortData()->userApplType(),
                                             _group->sortData()->userAppl(),
                                             _group->sortData()->pseudoCityType(),
                                             _group->sortData()->pseudoCity(),
                                             _group->sortData()->ssgGroupNo(),
                                             _group->sortData()->fareDisplayType(),
                                             _group->sortData()->domIntlAppl(),
                                             _group->sortData()->seqno());
    if (fbCombList.empty())
    {
      return;
    }
    else
    {
      _priorityMap.clear();
      populatePriorityList(fbCombList);
    }
  }
}

//------------------------------------------------------
// CharCombinationsComparator::populatePriorityList
//------------------------------------------------------
void
CharCombinationsComparator::populatePriorityList(const std::vector<FDSFareBasisComb*>& charCombs)
{
  std::vector<FDSFareBasisComb*>::const_iterator i(charCombs.begin()), end(charCombs.end());

  for (; i != end; ++i)
  {
    _priorityMap.insert(make_pair((*i)->fareBasisCharComb(), (*i)->orderNo()));
  }
}

uint16_t
CharCombinationsComparator::combPriority(const FareCombinationCode& code)
{
  std::map<FareCombinationCode, uint16_t>::const_iterator itr(_priorityMap.begin()),
      end(_priorityMap.end());
  while (end != itr)
  {
    std::string::size_type pos = 0;
    pos = code.find(itr->first);
    if (pos != std::string::npos) // lint !e530
    {
      return itr->second;
    }
    else
    {
      ++itr;
    }
  }
  return (_priorityMap.size() + 1);
}

Comparator::Result
CharCombinationsComparator::compareChars(const std::string& l, const std::string& r)
{
  uint16_t lhs = combPriority(l);
  uint16_t rhs = combPriority(r);
  if (lhs < rhs)
    return TRUE;
  else if (lhs > rhs)
    return FALSE;
  else
    return Comparator::EQUAL;
}
}
