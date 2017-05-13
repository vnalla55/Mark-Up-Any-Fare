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

#include "Diagnostic/DCFactoryFareDisplay.h"

#include "Diagnostic/DiagCollListInc.h"

#include <iostream>

namespace tse
{
DCFactoryFareDisplay* DCFactoryFareDisplay::_instance = nullptr;

DCFactoryFareDisplay::DCFactoryFareDisplay() : DCFactory()
{
  initializeDiagCollectorCreators();
}

DCFactoryFareDisplay::~DCFactoryFareDisplay() {}

DiagCollector*
DCFactoryFareDisplay::threadCreate(Trx& trx, Diagnostic& root)
{
  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();
  if (diagType == DiagnosticNone)
  {
    return thrDisabledDC(trx);
  }

  DiagnosticCreatorMap::const_iterator it = _diagFDOverrideCreatorMap.find(diagType);
  if (it != _diagFDOverrideCreatorMap.end())
  {
    return it->second->createDiagCollector(trx, root, diagType);
  }
  return DCFactory::threadCreate(trx, root);
}
void
DCFactoryFareDisplay::initializeDiagCollectorCreators()
{
  _diagFDOverrideCreatorMap[Diagnostic207] =
      createSharedPtr(new DiagCollectorCreator<Diag207CollectorFD>());
  _diagFDOverrideCreatorMap[Diagnostic209] =
      createSharedPtr(new DiagCollectorCreator<Diag207CollectorFD>());
  _diagFDOverrideCreatorMap[Diagnostic212] =
      createSharedPtr(new DiagCollectorCreator<Diag212CollectorFD>());
  _diagFDOverrideCreatorMap[Diagnostic291] =
      createSharedPtr(new DiagCollectorCreator<Diag291CollectorFD>());
}

DCFactoryFareDisplay*
DCFactoryFareDisplay::instance()
{
  if (!_instance)
  {
    boost::lock_guard<boost::mutex> g(_mutex);
    if (!_instance)
    {
      _instance = new DCFactoryFareDisplay;
    }
  }
  return _instance;
}
}
