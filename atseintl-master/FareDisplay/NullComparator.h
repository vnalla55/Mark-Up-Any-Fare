#pragma once

#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{
class PaxTypeFare;

class NullComparator : public Comparator
{
public:
  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override
  {
    return Comparator::Result::EQUAL;
  }
  void prepare(const FareDisplayTrx& trx) override {}
};
} // namespace tse

