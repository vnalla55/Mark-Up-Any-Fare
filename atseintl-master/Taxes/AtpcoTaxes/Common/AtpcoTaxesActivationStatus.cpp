// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/AtpcoTaxesActivationStatus.h"

namespace tax
{
void
AtpcoTaxesActivationStatus::setAllEnabled()
{
  _taxOnOC = true;
  _taxOnBaggage = true;
  _taxOnChangeFee = true;
  _taxOnItinYqYrTaxOnTax = true;
}
}
