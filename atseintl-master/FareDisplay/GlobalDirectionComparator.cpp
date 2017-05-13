//-------------------------------------------------------------------
//  Created:Jul 1, 2005
//  Author:Abu
//
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

#include "FareDisplay/GlobalDirectionComparator.h"

#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDisplaySort.h"
#include "DBAccess/FDSGlobalDir.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
GlobalDirectionComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{

  if (globalPriority(l.globalDirection()) < globalPriority(r.globalDirection()))
    return Comparator::TRUE;
  if (globalPriority(l.globalDirection()) > globalPriority(r.globalDirection()))
    return Comparator::FALSE;

  return Comparator::EQUAL;
}

uint16_t
GlobalDirectionComparator::globalPriority(GlobalDirection gd)
{
  std::map<GlobalDirection, uint16_t>::const_iterator itr(_priorityMap.end());
  itr = _priorityMap.find(gd);
  if (itr != _priorityMap.end())
  {
    return itr->second;
  }
  return (gd + _priorityMap.size());
}

void
GlobalDirectionComparator::prepare(const FareDisplayTrx& trx)
{
  if (_group->sortData() != nullptr)
  {
    const std::vector<FDSGlobalDir*> globalList =
        trx.dataHandle().getFDSGlobalDir(_group->sortData()->userApplType(),
                                         _group->sortData()->userAppl(),
                                         _group->sortData()->pseudoCityType(),
                                         _group->sortData()->pseudoCity(),
                                         _group->sortData()->ssgGroupNo(),
                                         _group->sortData()->fareDisplayType(),
                                         _group->sortData()->domIntlAppl(),
                                         _group->sortData()->versionDate(),
                                         _group->sortData()->seqno(),
                                         _group->sortData()->createDate());

    if (globalList.empty())
    {
      _priorityMap.clear();
      initialize();
      return;
    }
    else
    {
      _priorityMap.clear();
      populatePriorityList(globalList);
    }
  }
}

void
GlobalDirectionComparator::initialize()
{
  // use alphabetical priority as default
  _priorityMap.insert(std::make_pair(GlobalDirection::FE, 1));
  _priorityMap.insert(std::make_pair(GlobalDirection::RU, 2));
  _priorityMap.insert(std::make_pair(GlobalDirection::EH, 3));
  _priorityMap.insert(std::make_pair(GlobalDirection::TS, 4));
  _priorityMap.insert(std::make_pair(GlobalDirection::AP, 5));
  _priorityMap.insert(std::make_pair(GlobalDirection::PA, 6));
  _priorityMap.insert(std::make_pair(GlobalDirection::PN, 7));
  _priorityMap.insert(std::make_pair(GlobalDirection::AT, 8));
  _priorityMap.insert(std::make_pair(GlobalDirection::SA, 9));
  _priorityMap.insert(std::make_pair(GlobalDirection::CT, 10));
  _priorityMap.insert(std::make_pair(GlobalDirection::RW, 11));
  _priorityMap.insert(std::make_pair(GlobalDirection::WH, 12));
  _priorityMap.insert(std::make_pair(GlobalDirection::ZZ, 13));
}

void
GlobalDirectionComparator::populatePriorityList(const std::vector<FDSGlobalDir*>& gds)
{
  std::vector<FDSGlobalDir*>::const_iterator i(gds.begin()), end(gds.end());

  for (; i != end; ++i)
  {
    _priorityMap.insert(std::make_pair((*i)->globalDir(), (*i)->orderNo()));
  }
}
}
