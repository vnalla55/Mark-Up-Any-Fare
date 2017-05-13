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

#include "Pricing/Shopping/FOS/FosTaskScope.h"

namespace tse
{

class ShoppingTrx;
class Diversity;
class ItinStatistic;

namespace fos
{

class SolFosTaskScope : public FosTaskScope
{
public:
  SolFosTaskScope(const ShoppingTrx& trx, const ItinStatistic& stats, bool pqConditionOverride);

private:
  void prepareRegularFosParams(const ShoppingTrx& trx,
                               const ItinStatistic& stats,
                               bool pqConditionOverride);

  void prepareFosFlags(bool pqConditionOverride);

  void calculateOnlineInterlineFosParams(const ShoppingTrx& trx,
                                         const ItinStatistic& stats,
                                         uint32_t totalNumFOS);

  void prepareDirectFosParams(const ShoppingTrx& trx, const ItinStatistic& stats);

  void prepareAdditionalDirectFosParams(const Diversity& diversity, const ItinStatistic& stats);
};

} // ns fos
} // ns tse

