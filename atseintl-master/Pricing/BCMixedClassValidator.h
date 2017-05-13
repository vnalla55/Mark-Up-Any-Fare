// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#pragma once

#include <string>

namespace tse
{
class FarePath;
class FarePathValidatorTest;
class PricingTrx;
class FarePathValidatorTest;
class BCMixedClassValidator
{
  friend class ::tse::FarePathValidatorTest;

public:
  BCMixedClassValidator(PricingTrx& trx) : _trx(trx) {}
  bool validate(std::string& localRes, FarePath& farePath);

private:
  PricingTrx& _trx;

  void checkReissueExchangeIndForRebookFP(FarePath& fp);
  bool checkRebookedClassesForRex(FarePath& fp);
  void determineIfRebookClassesExist(FarePath& fp);
};
} // tse namespace
