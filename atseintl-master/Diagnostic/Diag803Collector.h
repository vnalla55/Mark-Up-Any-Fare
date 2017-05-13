//----------------------------------------------------------------------------
//  File:        Diag803Collector.h
//  Authors:     Sommapan Lathitham
//  Created:     Mar 2004
//
//  Description: Diagnostic 803 formatter - PFC Output Vector Diagnostic Display
//               TaxOut Object is formatted to produce PFC Output Vector display.
//
//  Updates:
//          03/01/04 - Sommapan - Intitial Development
//
//  Copyright Sabre 2004
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

#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class TaxResponse;

class Diag803Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag803Collector(Diagnostic& root)
    : DiagCollector(root), _pfcDisplayTotal(0), _skipCommonHeader(false)
  {
  }
  Diag803Collector() : _pfcDisplayTotal(0), _skipCommonHeader(false) {}

  virtual Diag803Collector& operator<<(const TaxResponse& Data) override;

private:
  uint16_t _pfcDisplayTotal;
  bool     _skipCommonHeader;

  enum PfcHeaderType
  {
    DIAGNOSTIC_HEADER = 1 // diagnostic header for WPQ/*24
  };

  void buildPfcHeader(const TaxResponse& taxResponse, PfcHeaderType pfcDisplayHeaderType);
};

} // namespace tse

