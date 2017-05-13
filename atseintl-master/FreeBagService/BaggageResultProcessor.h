//-------------------------------------------------------------------
//  Copyright Sabre 2010
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <vector>

namespace tse
{
class PricingTrx;
class FarePath;
class BaggageTravel;

class BaggageResultProcessor
{
  friend class BaggageResultProcessorTest;

public:
  BaggageResultProcessor(PricingTrx& trx);

  void buildAllowanceText(const std::vector<BaggageTravel*>& baggageTravels, bool isUsDot) const;
  void buildBaggageResponse(const std::vector<FarePath*>& farePaths);

  void processAdditionalAllowances() const;
  void processFeesAtEachCheck() const;

private:
  std::string buildBaggageResponse(FarePath* farePath);

  PricingTrx& _trx;
  std::map<FarePath*, bool> _additionalAllowancesApply;
  std::map<FarePath*, bool> _feesAtEachCheckApply;
};
} // tse
