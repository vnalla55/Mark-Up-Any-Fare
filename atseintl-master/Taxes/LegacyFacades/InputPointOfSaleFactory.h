// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once
#include <memory>

namespace tse
{
class PricingTrx;
class Agent;
}

namespace tax
{
class InputPointOfSale;

class InputPointOfSaleFactory
{
  InputPointOfSaleFactory() {}
  ~InputPointOfSaleFactory() {}
public:
  static std::unique_ptr<InputPointOfSale>
  createInputPointOfSale(tse::PricingTrx& trx, const tse::Agent& agent, type::Index myId);
};

}

