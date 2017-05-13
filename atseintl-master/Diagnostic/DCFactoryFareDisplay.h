//----------------------------------------------------------------------------
//  File:         DCFactoryFareDisplay.h
//  Description:  New class for specified FareDisplay diagnostics
//  Authors:      Adam Szalajko
//  Created:      Dec 2008
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2008
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

#include "Diagnostic/DCFactory.h"

namespace tse
{

class DCFactoryFareDisplay : public DCFactory
{
public:
  static DCFactoryFareDisplay* instance();

protected:
  DCFactoryFareDisplay();
  virtual ~DCFactoryFareDisplay();
  virtual DiagCollector* threadCreate(Trx& trx, Diagnostic& root) override;
  void initializeDiagCollectorCreators();

private:
  static DCFactoryFareDisplay* _instance;
  DiagnosticCreatorMap _diagFDOverrideCreatorMap;
};

} // namespace tse

