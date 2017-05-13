//-------------------------------------------------------------------
//
//  File:        FareUtil.h
//  Created:     Feb 25, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Misc. util functions to process constructed fares
//
//  Copyright Sabre 2005
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"


namespace tse
{

class ConstructedFareInfo;
class ConstructedFare;
class ConstructionJob;

class FareUtil
{
public:
  // total amount & currency
  // ===== ====== = ========

  static bool calculateTotalAmount(ConstructedFareInfo& cfi, ConstructionJob& cj);

  static bool nucToFareCurrency(ConstructedFareInfo& cfi, ConstructionJob& cj);

  static bool
  calculateTotalAmount(const ConstructedFare& cf, ConstructionJob& cj, MoneyAmount& totalAmount);

protected:
  static bool addNucAmount(const MoneyAmount amount,
                           const CurrencyCode& currency,
                           const Indicator specifiedOWRT,
                           ConstructionJob& cj,
                           MoneyAmount& constructedNucAmount,
                           MoneyAmount& constructedSecondNucAmount);

private:
  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  FareUtil(const FareUtil& rhs);
  FareUtil operator=(const FareUtil& rhs);

}; // End of class FareUtil

} // End namespace tse

