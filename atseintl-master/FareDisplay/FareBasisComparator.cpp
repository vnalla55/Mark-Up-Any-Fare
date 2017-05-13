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

#include "FareDisplay/FareBasisComparator.h"

#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDisplaySort.h"
#include "DBAccess/FDSSorting.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
FareBasisComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (_fdsItem == nullptr)
  {
    return EQUAL;
  }

  FareClassCode lFareBasis, rFareBasis;

  removeCorpId(l.createFareBasis(nullptr).c_str(), lFareBasis);
  removeCorpId(r.createFareBasis(nullptr).c_str(), rFareBasis);

  if (lFareBasis.empty() || rFareBasis.empty())
    return EQUAL; // TODO :ERROR

  uint16_t stepNo(0);
  Comparator::Result result = EQUAL; // lint !e578

  while (result == EQUAL && stepNo < 3)
  {
    if (lessBySize(lFareBasis, rFareBasis, stepNo))
    {
      result = resolveSizeInequality(lFareBasis, rFareBasis, stepNo);
    }
    else
    {

      result = compareFareBasis(lFareBasis, rFareBasis, stepNo);
    }

    ++stepNo;
  }

  return result;
}

void
FareBasisComparator::prepare(const FareDisplayTrx& trx)
{
  _fdsItem = nullptr; // each trx, needs to initialize it to zero first to ensure we dont carry over the
                // "last" trx pointer.

  if (_group->sortData() != nullptr)
  {
    const std::vector<FDSSorting*> fdsList =
        trx.dataHandle().getFDSSorting(_group->sortData()->userApplType(),
                                       _group->sortData()->userAppl(),
                                       _group->sortData()->pseudoCityType(),
                                       _group->sortData()->pseudoCity(),
                                       _group->sortData()->ssgGroupNo(),
                                       _group->sortData()->fareDisplayType(),
                                       _group->sortData()->domIntlAppl(),
                                       _group->sortData()->seqno());

    if (fdsList.empty())
    {
      return;
    }
    else
    {
      _fdsItem = fdsList.front();
    }
  }
}

Comparator::Result
FareBasisComparator::compareFareBasis(const FareClassCode& l,
                                      const FareClassCode& r,
                                      uint16_t stepNo)
{
  uint16_t lhs = priority(l, stepNo);
  uint16_t rhs = priority(r, stepNo);
  return result(lhs, rhs, sortType(stepNo));
}

uint16_t
FareBasisComparator::priority(const FareClassCode& code, uint16_t stepNo)
{
  const std::string& str = getCharLists(stepNo);
  if (str.empty())
  {
    return code[stepNo];
  }
  size_t pos = 0;
  pos = str.find(code[stepNo]);
  if (pos != std::string::npos) // lint !e530
  {
    return (uint16_t)pos;
  }
  else
    return str.size() + 1;
}

const std::string&
FareBasisComparator::getCharLists(uint16_t stepNo)
{
  // PRECOND : There exist a fdsItem
  switch (stepNo + 1)
  {
  case 1:
    return _fdsItem->fareBasisChar1();
  case 2:
    return _fdsItem->fareBasisChar2();
  case 3:
    return _fdsItem->fareBasisChar3();
  default:
    break;
  }
  return EMPTY_STRING();
}

const Indicator
FareBasisComparator::sortType(uint16_t stepNo)
{

  switch (stepNo + 1)
  {
  case 1:
    return _fdsItem->sortFareBasisChar1();
  case 2:
    return _fdsItem->sortFareBasisChar2();
  case 3:
    return _fdsItem->sortFareBasisChar3();
  default:
    break;
  }
  return BLANK;
}

bool
FareBasisComparator::lessBySize(const FareClassCode& l, const FareClassCode& r, uint16_t stepNo)
{
  if ((uint16_t)l.size() < (stepNo + 1) || (uint16_t)r.size() < (stepNo + 1))
    return true;
  else
    return false;
}

Comparator::Result
FareBasisComparator::resolveSizeInequality(const FareClassCode& l,
                                           const FareClassCode& r,
                                           uint16_t stepNo)
{
  uint16_t lhs = l.size();
  uint16_t rhs = r.size();
  if (lhs == rhs)
    return EQUAL;
  else if (lhs < stepNo + 1)
  {
    const std::string& str = getCharLists(stepNo);
    if (str.empty())
    {
      return sortType(stepNo) == Group::DESCENDING ? Comparator::FALSE : Comparator::TRUE;
    }
    else
    {
      lhs = priority(str);
      rhs = priority(str, r[stepNo]);
      return result(lhs, rhs, sortType(stepNo));
    }
  }
  else if (rhs < stepNo + 1)
  {
    const std::string& str = getCharLists(stepNo);
    if (str.empty())
    {
      return sortType(stepNo) == Group::DESCENDING ? Comparator::TRUE : Comparator::FALSE;
    }
    else
    {

      lhs = priority(str, l[stepNo]);
      rhs = priority(str);
      return result(lhs, rhs, sortType(stepNo));
    }
  }
  return EQUAL;
}

uint16_t
FareBasisComparator::priority(const std::string& str, const char value)
{
  size_t pos = 0;
  pos = str.find(value);
  if (pos != std::string::npos)
  {
    return (uint16_t)pos;
  }
  else
    return str.size() + 1;
}

Comparator::Result
FareBasisComparator::result(uint16_t lhs, uint16_t rhs, const Indicator& sortType)
{ // lint !e578

  if (lhs < rhs)
    return sortType == Group::DESCENDING ? FALSE : TRUE;
  else if (lhs > rhs)
    return sortType == Group::DESCENDING ? TRUE : FALSE;
  else
    return Comparator::EQUAL;
}
}
