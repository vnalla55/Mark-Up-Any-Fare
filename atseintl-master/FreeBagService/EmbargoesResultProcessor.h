// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include <string>
#include <vector>

namespace tse
{
class PricingTrx;
class FarePath;
class BaggageTravel;

class EmbargoesResultProcessor
{
  friend class EmbargoesResultProcessorTest;

public:
  EmbargoesResultProcessor(PricingTrx& trx);
  virtual ~EmbargoesResultProcessor();
  void process(const std::vector<FarePath*>& farePaths);

private:
  std::string formatEmbargoesText(const std::vector<const BaggageTravel*>& baggageTravels) const;

  PricingTrx& _trx;
};
}

