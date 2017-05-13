#pragma once

#include "Common/TseCodeTypes.h"

namespace tse
{
class PricingTrx;
class FarePath;
class PaxType;

class TruePaxType
{
public:
  TruePaxType(PricingTrx& trx, const FarePath& fp)
    : _fp(fp),
      _trx(trx)
  {
    init();
  }

  bool allInGroup() const { return _allInGroup; }
  bool atLeastOneMatch() const { return _atLeastOneMatch; }
  bool mixedPaxType() const { return _mixedPaxType; }
  bool negFareUsed() const { return _negFareUsed; }
  bool privateFareUsed() const { return _privateFareUsed; }
  const PaxTypeCode& paxType() const { return _paxType; }

private:
  void init();

  const FarePath& _fp;

  bool _allFareSamePaxType = true;
  bool _allInGroup = true;
  bool _atLeastOneMatch = false;
  bool _mixedPaxType = false;
  bool _negFareUsed = false;
  bool _privateFareUsed = false;
  PaxTypeCode _paxType;
  PaxTypeCode _oneInGroup;
  PricingTrx& _trx;
};
}
