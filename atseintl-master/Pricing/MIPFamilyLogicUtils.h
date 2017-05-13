/*---------------------------------------------------------------------------
 *  File:    MIPFamilyLogicUtils.h
 *  Created: Jan 27, 2012
 *  Authors: Artur de Sousa Rocha
 *
 *  Copyright Sabre 2012
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/

#pragma once

#include "Common/TseEnums.h"

#include <vector>

namespace tse
{
class PricingTrx;
class Itin;
class FPPQItem;
class FarePath;
class FareMarket;
class GroupFarePath;

class MIPFamilyLogicUtils
{
public:
  static void populateSimilarItinData(const PricingTrx& trx, Itin& child, Itin& motherItin);

  static bool pricedThroughFare(const PricingTrx& trx, const FarePath& farePath);
  static bool pricedThroughFare(const PricingTrx& trx, const GroupFarePath& groupFPath);
  static bool pricedThroughFare(const PricingTrx& trx, const std::vector<FPPQItem*>& groupFPath);
  static bool sameCarriers(const Itin& motherItin, const Itin& estItin);

private:
  static void populateSimilarItinBrAllData(Itin& child, Itin& motherItin);
  static bool isThroughFareMarket(const PricingTrx& trx, const FareMarket& fareMarket);

};

} /* end namespace tse */

