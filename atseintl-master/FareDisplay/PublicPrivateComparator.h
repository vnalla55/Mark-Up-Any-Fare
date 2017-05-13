#pragma once

#include "FareDisplay/Comparator.h"
#include "FareDisplay/FDConsts.h"

namespace tse
{

class PaxTypeFare;

class PublicPrivateComparator : public Comparator
{
public:
  Comparator::Result compare(const PaxTypeFare& l, const PaxTypeFare& r) override;

private:
  static constexpr int PRIVATE_TARIFF = 1;
  void prepare(const FareDisplayTrx& trx) override;
};

} // namespace tse

