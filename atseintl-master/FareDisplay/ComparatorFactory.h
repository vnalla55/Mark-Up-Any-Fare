//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "FareDisplay/Group.h"

namespace tse
{
class Comparator;
class FareDisplayTrx;

class ComparatorFactory
{
public:
  ComparatorFactory(const FareDisplayTrx& trx) : _trx(trx) {}
  Comparator* getComparator(Group::GroupType& type);
  virtual ~ComparatorFactory();

private:
  const FareDisplayTrx& _trx;
};
}

