//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FiltersAndPipes/IPredicate.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"
#include "Pricing/VITAValidator.h"

#include <boost/utility.hpp>

namespace tse
{

namespace utils
{

// Checks the requirement "interline ticketing agreement"
// for flight-only solution
class InterlineTicketingAgreement : public IPredicate<SopCombination>, boost::noncopyable
{
public:
  // Not possible to create VITAValidator with
  // const trx. At some point later, maybe this
  // can be corrected.
  explicit InterlineTicketingAgreement(ShoppingTrx& trx) :
    _trx(trx), _vitaValidator(trx), _validatingCarrierUpdater(trx) {}

  bool operator()(const SopCombination& sopIds) override;

private:
  ShoppingTrx& _trx;
  VITAValidator _vitaValidator;
  ValidatingCarrierUpdater _validatingCarrierUpdater;
};

} // namespace utils

} // namespace tse

