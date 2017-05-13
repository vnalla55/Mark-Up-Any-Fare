//-------------------------------------------------------------------
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

#include "FareDisplay/PsgTypeComparator.h"

#include "Common/TseCodeTypes.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDisplaySort.h"
#include "DBAccess/FDSPsgType.h"
#include "FareDisplay/Group.h"

#include <utility>

namespace tse
{
Comparator::Result
PsgTypeComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{

  if (_priorityMap.empty())
  {
    return alphabeticalComparison(l, r);
  }
  else
  {

    if (psgPriority(paxType(l)) < psgPriority(paxType(r)))
    {
      return _group->sortType() == Group::DESCENDING ? Comparator::FALSE : Comparator::TRUE;
    }
    else if (psgPriority(paxType(l)) > psgPriority(paxType(r)))
    {
      return _group->sortType() == Group::DESCENDING ? Comparator::TRUE : Comparator::FALSE;
    }
  }
  return alphabeticalComparison(l, r);
}

void
PsgTypeComparator::prepare(const FareDisplayTrx& trx)
{
  _priorityMap.clear();

  if (!trx.getRequest()->displayPassengerTypes().empty())
  {
    std::vector<tse::PaxTypeCode>::const_iterator i(
        trx.getRequest()->displayPassengerTypes().begin()),
        end(trx.getRequest()->displayPassengerTypes().end());
    uint16_t priority(1);

    for (; i != end; ++i)
    {
      _priorityMap.insert(std::make_pair(*i, priority));
      ++priority;
    }
  }
  else
  {
    if (_group->sortData() != nullptr)
    {
      const std::vector<FDSPsgType*> psgList =
          trx.dataHandle().getFDSPsgType(_group->sortData()->userApplType(),
                                         _group->sortData()->userAppl(),
                                         _group->sortData()->pseudoCityType(),
                                         _group->sortData()->pseudoCity(),
                                         _group->sortData()->ssgGroupNo(),
                                         _group->sortData()->fareDisplayType(),
                                         _group->sortData()->domIntlAppl(),
                                         _group->sortData()->seqno());

      if (psgList.empty())
        return;

      _priorityMap.clear();
      populatePriorityList(psgList);
    }
  }
}

void
PsgTypeComparator::populatePriorityList(const std::vector<FDSPsgType*>& gds)
{
  // lint -e{578}
  std::vector<FDSPsgType*>::const_iterator i(gds.begin()), end(gds.end());

  for (; i != end; ++i)
  {
    _priorityMap.insert(std::make_pair((*i)->psgType(), (*i)->orderNo()));
  }
}
uint16_t
PsgTypeComparator::psgPriority(const PaxTypeCode& paxType)
{
  std::map<tse::PaxTypeCode, uint16_t>::const_iterator itr(_priorityMap.end());
  if (paxType.empty())
  {

    itr = _priorityMap.find(ADULT);
  }
  else
  {
    itr = _priorityMap.find(paxType);
  }

  if (itr != _priorityMap.end())
  {
    return itr->second;
  }
  return (_priorityMap.size() + 1);
}
std::string
PsgTypeComparator::translate(const PaxTypeCode& paxCode)
{
  if (paxCode.empty())
    return ADULT;
  else
    return paxCode;
}

Comparator::Result
PsgTypeComparator::alphabeticalComparison(const PaxTypeFare& l, const PaxTypeFare& r)
{

  if (translate(l.fcasPaxType()) < translate(r.fcasPaxType()))
  {
    return _group->sortType() == Group::DESCENDING ? Comparator::FALSE : Comparator::TRUE;
  }
  else if (translate(l.fcasPaxType()) > translate(r.fcasPaxType()))
  {
    return _group->sortType() == Group::DESCENDING ? Comparator::TRUE : Comparator::FALSE;
  }
  return Comparator::EQUAL;
}

std::string
PsgTypeComparator::paxType(const PaxTypeFare& fare)
{
  return (fare.actualPaxType() != nullptr ? fare.actualPaxType()->paxType() : fare.fcasPaxType());
}
}
