/*---------------------------------------------------------------------------
 *  Copyright Sabre 2016
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include <vector>

namespace tse
{
class PricingTrx;
class FarePath;

namespace similaritin
{
template <typename D>
class SurchargesValidator
{
public:
  SurchargesValidator(PricingTrx& trx, D& diag) : _trx(trx), _diagnostic(diag) {}
  void applySurcharges(const std::vector<FarePath*>& farePathVec);
  void applySurcharges(FarePath& farePath);

private:
  PricingTrx& _trx;
  D& _diagnostic;

};
}
}
