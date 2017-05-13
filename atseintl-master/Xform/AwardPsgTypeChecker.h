#pragma once

#include "Common/TseCodeTypes.h"

#include <string>
#include <vector>

namespace tse
{
class PricingTrx;
class PaxType;

class AwardPsgTypeChecker
{
public:
  void checkPsgType(const PricingTrx& trx);

private:
  std::vector<PaxTypeCode> _paxTypeCodeVec;

  void getPaxTypeCodeVec();
  void checkPsgType(const std::vector<PaxType*>& paxTypeVec);
  void checkPsgType(const PaxType* const paxType);
  bool isSupported(const PaxType* const paxType) const;
  std::string name()
  {
    return "AWARD_PSG_TYPE";
  };
  std::string group()
  {
    return "SHOPPING_OPT";
  };
};
}
