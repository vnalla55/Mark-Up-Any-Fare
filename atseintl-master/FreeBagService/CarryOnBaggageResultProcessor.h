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

#include "DataModel/BaggageTravel.h"

#include <map>
#include <set>
#include <string>
#include <vector>

#include <stdint.h>

namespace tse
{
class FarePath;
class PricingTrx;

class CarryOnBaggageResultProcessor
{
  friend class CarryOnBaggageResultProcessorTest;

public:
  CarryOnBaggageResultProcessor(PricingTrx& trx);
  void processAllowance(const std::vector<FarePath*>& farePaths) const;
  void processCharges(const std::vector<FarePath*>& farePaths) const;

private:
  std::string formatAllowanceText(const std::vector<const BaggageTravel*>& baggageTravels) const;
  std::string formatChargesText(const std::vector<const BaggageTravel*>& baggageTravels) const;
  void getTaxTblNoSets(const std::vector<const BaggageTravel*>& baggageTravels,
                       std::vector<std::set<uint32_t> >& taxTblItemNoSets) const;
  void gatherSubCodeQuantities(const ChargeVector& charges,
                               std::map<const SubCodeInfo*, size_t>& quantities) const;

  PricingTrx& _trx;
};
} // tse

