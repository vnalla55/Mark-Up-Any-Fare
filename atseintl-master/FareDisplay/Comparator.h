//-------------------------------------------------------------------
//  File:        Comparator.h
//  Created:     July 01, 2005
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
#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;
class Group;

class Comparator
{
public:
  virtual ~Comparator() = default;

  enum Result
  {
    TRUE = 1,
    FALSE,
    EQUAL
  };

  /**
   * abstract interface to compare two Fares based on a set of Group/Sort priority.
   */

  virtual Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) = 0;
  virtual void prepare(const FareDisplayTrx& trx) = 0;

  Group*& group() { return _group; }
  const Group* group() const { return _group; }
  static constexpr char CORP_ID_SEPARATOR = '/';

protected:
  Group* _group = nullptr;
  /**
   * removes the corp id part of a FareBasis to ensure fare basies are only compares
   * agaings their fare class code.
   * e.g Y26/Af0023 and ERFG02/CH12 will be compared against Y26 and ERFG02.
   */
  void removeCorpId(const FareClassCode& beforeValue, FareClassCode& afterValue)
  {
    size_t pos = 0;
    pos = beforeValue.find(CORP_ID_SEPARATOR);
    if (pos == std::string::npos)
    {
      afterValue = beforeValue;
    }
    else
    {
      afterValue.assign(beforeValue, 0, pos);
    }
  }
};
}
