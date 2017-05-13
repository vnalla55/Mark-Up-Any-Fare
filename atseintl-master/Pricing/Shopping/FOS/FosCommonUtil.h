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

#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class ShoppingTrx;

namespace fos
{
class FosStatistic;

class FosCommonUtil
{
public:
  typedef std::map<CarrierCode, uint32_t> CarrierMap;
  typedef std::pair<CarrierCode, uint32_t> CarrierMapElem;

  const static CarrierCode INTERLINE_CARRIER;

  static CarrierCode detectCarrier(const ShoppingTrx& trx, const SopIdVec& combination);

  static void collectOnlineCarriers(const ShoppingTrx& trx,
                                    std::set<CarrierCode>& onlineCarriers,
                                    bool& isInterlineApplicable);

  static std::size_t calcNumOfValidSops(const ShoppingTrx& trx);

  // Check if there is any SOP with number of travel segments greater than numTS
  static bool
  checkNumOfTravelSegsPerLeg(const ShoppingTrx& trx, const SopIdVec& comb, std::size_t numTS);

  // Check if total number of travel segments is greater than numTS
  static bool
  checkTotalNumOfTravelSegs(const ShoppingTrx& trx, const SopIdVec& comb, std::size_t numTS);

  // Check connecting cities of RT solution
  static bool checkConnectingCitiesRT(const ShoppingTrx& trx, const SopIdVec& comb);

  static void diversifyOptionsNumPerCarrier(uint32_t totalOptionsRequired,
                                            std::vector<CarrierMapElem>& availableVec,
                                            CarrierMap& optionsMap);

  static bool applyNonRestrictionFilter(const ShoppingTrx& trx,
                                        const FosStatistic& stats,
                                        bool isPqOverride,
                                        bool isNoPricedSolutions);
};

} // fos
} // tse
