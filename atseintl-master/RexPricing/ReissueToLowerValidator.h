//-------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "DataModel/FareMarket.h"

#include <vector>

namespace tse
{
class RexPricingTrx;
class FarePath;
class PricingUnitRuleController;
class DateTime;
class PaxTypeFare;
class ProcessTagPermutation;
class Combinations;

class ReissueToLowerValidator
{
public:
  ReissueToLowerValidator(RexPricingTrx& trx,
                          const FarePath& fp,
                          Combinations& _combinationsController,
                          PricingUnitRuleController& controller,
                          std::map<FareMarket*, FareMarket*>& FM2PrevReissueFMCache);

  bool process(const ProcessTagPermutation& permutation);

private:
  bool validationForValueF();
  bool validationForValueR();

  typedef std::map<PaxTypeFare*, PaxTypeFare*> PaxTypeFareMap;

  bool checkFCLevelValidation(const PaxTypeFareMap& faresMap) const;
  bool checkPULevelValidation(FarePath& fPath) const;
  bool checkCombinations(FarePath& fPath) const;

  const FareMarket* getFareMarket(FareMarket* fareMarket) const;
  bool hasNonHistoricalFare(PaxTypeFareMap& faresMap) const;
  void exchangePaxTypeFares(FarePath& fPath, PaxTypeFareMap& faresMap) const;
  void recalculateTotalAmount(FarePath& fPath) const;

  RexPricingTrx& _trx;
  const FarePath& _farePath;
  Combinations& _combinationsController;
  PricingUnitRuleController& _puRuleController;
  std::map<FareMarket*, FareMarket*>& _FM2PrevReissueFMCache;
  const DateTime& _date;
  FareMarket::FareRetrievalFlags _flag;
  uint16_t _itinIndex;

  friend class ReissueToLowerValidatorTest;
};

} // end of namesapce tse

