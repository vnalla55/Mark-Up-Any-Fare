//-------------------------------------------------------------------
//  Created: December 26, 2006
//  Author: Marco Cartolano
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

#include "FareDisplay/BrandComparator.h"

#include "DataModel/PaxTypeFare.h"
#include "DBAccess/Brand.h"
#include "DBAccess/BrandedFareApp.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
BrandComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (l.carrier() == INDUSTRY_CARRIER && r.carrier() == INDUSTRY_CARRIER)
  {
    // No brand to compare
    return Comparator::EQUAL;
  }

  if (_priorityMap.empty())
  {
    return alphabeticalComparison(l, r);
  }

  BrandCode lBrand = l.fareDisplayInfo()->brandCode();
  BrandCode rBrand = r.fareDisplayInfo()->brandCode();

  if (brandPriority(lBrand) < brandPriority(rBrand))
    return Comparator::TRUE;
  if (brandPriority(lBrand) > brandPriority(rBrand))
    return Comparator::FALSE;

  return alphabeticalComparison(l, r);
}

Comparator::Result
BrandComparator::alphabeticalComparison(const PaxTypeFare& l, const PaxTypeFare& r)
{
  BrandCode lBrand = l.fareDisplayInfo()->brandCode();
  BrandCode rBrand = r.fareDisplayInfo()->brandCode();

  if (lBrand < rBrand)
    return Comparator::TRUE;
  if (lBrand > rBrand)
    return Comparator::FALSE;

  return Comparator::EQUAL;
}

//------------------------------------------------------
// BrandComparator::brandPriority
//------------------------------------------------------
uint64_t
BrandComparator::brandPriority(BrandCode bc)
{
  std::map<BrandCode, uint64_t>::const_iterator itr(_priorityMap.end());
  itr = _priorityMap.find(bc);

  if (itr != _priorityMap.end())
  {
    return itr->second;
  }

  return (0);
}

//------------------------------------------------------
// BrandComparator::prepare
//------------------------------------------------------
void
BrandComparator::prepare(const FareDisplayTrx& trx)
{
  _priorityMap.clear();

  if (!_group->brandedFareApps().empty())
  {
    std::vector<BrandedFareApp*>::const_iterator itr = _group->brandedFareApps().begin();

    const std::vector<Brand*> brands = trx.dataHandle().getBrands(
        (*itr)->userApplType(), (*itr)->userAppl(), (*itr)->carrier(), trx.travelDate());

    if (!brands.empty())
    {
      populatePriorityList(brands);
    }
  }
  else if (!_group->brandResponseItemVec().empty())
  {
    std::vector<BrandResponseItem*>::const_iterator itr = _group->brandResponseItemVec().begin();
    std::vector<BrandResponseItem*>::const_iterator itrEnd = _group->brandResponseItemVec().end();
    uint16_t priority = 0;
    while (itr != itrEnd)
    {
      _priorityMap.insert(make_pair((*itr)->_brandCode, priority));
      ++priority;
      ++itr;
    }
  }
}

void
BrandComparator::populatePriorityList(const std::vector<Brand*>& brands)
{
  std::vector<Brand*>::const_iterator i(brands.begin()), end(brands.end());

  for (; i != end; ++i)
  {
    _priorityMap.insert(make_pair((*i)->brandId(), (*i)->seqno()));
  }
}
}
