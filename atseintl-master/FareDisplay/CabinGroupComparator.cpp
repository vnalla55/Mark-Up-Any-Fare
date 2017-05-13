//-------------------------------------------------------------------
//  Created: April 2013
//  Author:
//
//  Copyright Sabre 2013
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
#include "FareDisplay/CabinGroupComparator.h"

#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
CabinGroupComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (_priorityMap.empty())
  {
    return Comparator::EQUAL;
  }

  if (inclusionNumPassed(l) < inclusionNumPassed(r))
    return Comparator::TRUE;
  if (inclusionNumPassed(l) > inclusionNumPassed(r))
    return Comparator::FALSE;

  return alphabeticalComparison(l, r);
}

Comparator::Result
CabinGroupComparator::alphabeticalComparison(const PaxTypeFare& l, const PaxTypeFare& r)
{
  uint8_t lNum = l.fareDisplayInfo()->inclusionCabinNum();
  uint8_t rNum = r.fareDisplayInfo()->inclusionCabinNum();

  if (lNum < rNum)
    return Comparator::TRUE;
  if (lNum > rNum)
    return Comparator::FALSE;

  return Comparator::EQUAL;
}

//------------------------------------------------------
// CabinGroupComparator::brandPriority
//------------------------------------------------------
uint8_t
CabinGroupComparator::inclusionNumPassed(const PaxTypeFare& ptf)
{
  std::map<uint8_t, std::pair<uint8_t, std::string> >::const_iterator itr = _priorityMap.begin();
  std::map<uint8_t, std::pair<uint8_t, std::string> >::const_iterator itrE= _priorityMap.end();
  uint8_t orderNumber = 0;
  for(; itr != itrE; ++itr)
  {
    if( ptf.isFarePassForInclCode(itr->second.first))
    {
      return orderNumber = itr->first;
    }
  }
  return (_priorityMap.size() + 1);
}

//------------------------------------------------------
// CabinGroupComparator::prepare
//------------------------------------------------------
void
CabinGroupComparator::prepare(const FareDisplayTrx& trx)
{
  _priorityMap.clear();

  std::string sub_string;
  uint8_t sizeIncl = trx.getRequest()->requestedInclusionCode().size()/2;
  // populate priority Map by the order of the Inclusion codes in the requested
  // prepare Fare Display Response with the Inclusion Code number and Inclusion code verbiage
  //
  FareDisplayTrx& trxx = const_cast<FareDisplayTrx&>(trx);
  for ( uint8_t length = 0; length < sizeIncl;  ++length)
  {
    sub_string = trxx.getRequest()->requestedInclusionCode().substr(length*2, 2);
    uint8_t inclusionNum = trxx.getRequest()->inclusionNumber(sub_string);

    _priorityMap[length] =
           std::make_pair(inclusionNum, sub_string);

    trxx.fdResponse()->multiInclusionCabins().emplace_back(inclusionNum);
  }
}

}
