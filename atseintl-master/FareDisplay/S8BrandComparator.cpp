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
#include "FareDisplay/S8BrandComparator.h"

#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
S8BrandComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (_priorityMap.empty())
  {
    return alphabeticalComparison(l, r);
  }

  ProgramBrand lBrand = l.fareDisplayInfo()->programBrand();
  ProgramBrand rBrand = r.fareDisplayInfo()->programBrand();

  if (brandPriority(lBrand) < brandPriority(rBrand))
    return Comparator::TRUE;
  if (brandPriority(lBrand) > brandPriority(rBrand))
    return Comparator::FALSE;

  return alphabeticalComparison(l, r);
}

Comparator::Result
S8BrandComparator::alphabeticalComparison(const PaxTypeFare& l, const PaxTypeFare& r)
{
  ProgramBrand lBrand = l.fareDisplayInfo()->programBrand();
  ProgramBrand rBrand = r.fareDisplayInfo()->programBrand();

  if (lBrand < rBrand)
    return Comparator::TRUE;
  if (lBrand > rBrand)
    return Comparator::FALSE;

  return Comparator::EQUAL;
}

//------------------------------------------------------
// S8BrandComparator::brandPriority
//------------------------------------------------------
TierNumber
S8BrandComparator::brandPriority(ProgramBrand pb)
{
  std::map<ProgramBrand, TierNumber>::const_iterator itr(_priorityMap.end());
  itr = _priorityMap.find(pb);
  TierNumber tier = 0;
  if (itr != _priorityMap.end())
  {
    tier = itr->second;
  }
  return tier;
}

//------------------------------------------------------
// S8BrandComparator::prepare
//------------------------------------------------------
void
S8BrandComparator::prepare(const FareDisplayTrx& trx)
{
  _priorityMap.clear();

  std::map<CarrierCode, std::vector<OneProgramOneBrand*> >::const_iterator spb =
      _group->programBrandMap().find(trx.fareMarket().front()->governingCarrier());
  std::vector<OneProgramOneBrand*> spbVec;
  if (spb != _group->programBrandMap().end() && !(spb->second).empty())
  {
    spbVec = spb->second;
    OneProgramOneBrandComparator spbCmp;
    std::sort(spbVec.begin(), spbVec.end(), spbCmp);

    // populate priority Map by tier number
    // prepare Fare Display Response with the Program - Brand text info
    //
    FareDisplayTrx& trxx = const_cast<FareDisplayTrx&>(trx);

    spbVec = spb->second;
    for (OneProgramOneBrand* sPB : spbVec)
    {
      _priorityMap.insert(make_pair(make_pair(sPB->programCode(), sPB->brandCode()), sPB->tier()));

      trxx.fdResponse()->programBrandNameMap().insert(
          make_pair(make_pair(sPB->programCode(), sPB->brandCode()),
                    std::make_tuple(sPB->programName(), sPB->brandName(), sPB->systemCode())));
    }
  }
}

bool
OneProgramOneBrandComparator::
operator()(const OneProgramOneBrand* spb1, const OneProgramOneBrand* spb2) const
{
  if (spb1->carrier() != spb2->carrier()) // is the same carrier ?
  {
    return spb1->carrier() < spb2->carrier();
  }

  if (spb1->programCode() != spb2->programCode()) // is the same program code ?
  {
    return spb1->programCode() < spb2->programCode();
  }

  if (spb1->brandCode() != spb2->brandCode()) // is the same brand code ?
  {
    return spb1->brandCode() < spb2->brandCode();
  }

  return spb1->tier() > spb2->tier();
}
}
