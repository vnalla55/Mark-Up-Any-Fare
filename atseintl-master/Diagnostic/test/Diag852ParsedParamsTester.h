//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#ifndef DIAG_852_PARSED_PARAMS_TESTE_H
#define DIAG_852_PARSED_PARAMS_TESTE_H

#include "Diagnostic/Diag852Collector.h"

namespace tse
{

class Diag852ParsedParamsTester
{
public:
  Diag852ParsedParamsTester(Diag852Collector::Diag852ParsedParams& diag852Params)
    : _diag852Params(diag852Params)
  {
  }

  void updateDiagType(Diag852Collector::DiagType diagType)
  {
    forceInitialization();
    _diag852Params._diagType = diagType;
  }

  void updateFareLine(uint32_t fareLine)
  {
    forceInitialization();
    _diag852Params._fareLine = fareLine;
  }

  void updateCheckedPortion(uint32_t checkedPortion)
  {
    forceInitialization();
    _diag852Params._checkedPortion = checkedPortion;
  }

  void forceInitialization()
  {
    if (_diag852Params._parent.rootDiag())
    {
      _diag852Params._initialised = false;
      _diag852Params.initialiseParams();
    }
  }

private:
  Diag852Collector::Diag852ParsedParams& _diag852Params;
};
}

#endif
