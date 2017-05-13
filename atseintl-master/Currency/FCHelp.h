//----------------------------------------------------------------------------
//
//        File: FCHelp.h
// Description: processing FCHELP command
//
//     Created: 21/11/07
//     Authors: Tomasz Karczewski
//
//  Copyright Sabre 2007
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Currency/FCCommand.h"
#include "DataModel/CurrencyTrx.h"

namespace tse
{
class FCHelp : public FCCommand
{

public:
  FCHelp(CurrencyTrx& _trx) : FCCommand(_trx) {}

  virtual ~FCHelp() {}

  void process() override;
};
}
