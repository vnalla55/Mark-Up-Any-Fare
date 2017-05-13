//----------------------------------------------------------------------------
//
//  File:  NetRemitPricing.h
//  Created:  Feb 17, 2006
//  Authors:  Andrea Yang
//
//  Description: Process Fare Path for Net Remit
//
//  Copyright Sabre 2006
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseStringTypes.h"

#include <map>

namespace tse
{

class PricingTrx;
class FarePath;
class NetRemitFarePath;
class PaxTypeFare;
class CombinabilityRuleInfo;

typedef std::map<const PaxTypeFare*, const CombinabilityRuleInfo*> FareCombRuleInfoMap;
class NetRemitPricing
{
public:
  NetRemitPricing(PricingTrx& trx, FarePath& farePath);
  ~NetRemitPricing();

  bool process();
  void displayFarePathAxess(const FarePath& fp);

protected:
  void init();
  bool isNetRemit();

  void displayNetRemitFarePath(const NetRemitFarePath& netRemitFp);
  void calcSurcharges(FarePath& netRemitFp);
  void compareNetRemitFareAmount();

  void processAxessPricing();
  bool processRuleRevalidationForAxess(FarePath& axessFp);
  bool processCombinabilityForAxess(FarePath& axessFp);
  void getCat10ForNetFares(FareCombRuleInfoMap& cat10ForNetFaresMap);
  void setCat10ForNetFaresMap(FareCombRuleInfoMap& cat10ForNetFaresMap, PaxTypeFare& paxTypeFare);

private:
  PricingTrx& _trx;
  FarePath& _farePath;
};
}
