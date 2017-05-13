//----------------------------------------------------------------------------
//  File:         DiagCollectorGuard.h
//  Description:  Class that retains and restores the state of a DiagCollector.
//                When an object of this class goes out of scope it will restore
//                the associated DiagCollector to the state it had when the
//                DiagCollectorGuard object was instantiated.  This restoration
//                should be more efficient and should remove the need for calling
//                DiagCollector::Enable(...) so frequently throughout the code.
//  Authors:      Mike Lillis
//  Created:      April 2005
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2005
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

#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{

class DiagCollectorGuard
{
public:
  DiagCollectorGuard(DiagCollector& diagCollector)
    : _diagCollector(diagCollector), _priorState(diagCollector.isActive()) {};
  DiagCollectorGuard(DiagCollector& diagCollector, DiagnosticTypes diagNum)
    : _diagCollector(diagCollector), _priorState(diagCollector.enable(diagNum)) {};

  template <typename T>
  DiagCollectorGuard(DiagCollector& diagCollector, const T* obj, DiagnosticTypes diagNum)
    : _diagCollector(diagCollector), _priorState(diagCollector.enable(obj, diagNum)) {};

  ~DiagCollectorGuard()
  {
    _diagCollector.restoreState(_priorState);
  };

private:
  DiagCollector& _diagCollector;
  bool _priorState;
};

} // namespace tse

