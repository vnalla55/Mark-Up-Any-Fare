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
#pragma once

#include "DataModel/TaxTrx.h"

#include <string>

namespace tax
{
namespace display
{
struct TaxDisplayRequest;
}
}

namespace tse
{

class AtpcoTaxDisplayDriver
{
public:
  AtpcoTaxDisplayDriver(TaxTrx& trx) : _trx(trx) {}

  bool buildResponse();

private:
  void prepareRequest(tax::display::TaxDisplayRequest& request);

  TaxTrx& _trx;
};

} /* namespace tax */
