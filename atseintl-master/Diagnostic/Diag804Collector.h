//----------------------------------------------------------------------------
//  File:        Diag804Collector.h
//  Authors:     Dean Van Decker
//  Created:     Feb 2004
//
//  Description: Diagnostic 804 formatter - Tax Customer Diagnostic Display
//  TaxOut Object is formated to produce the new WPQ/*804(previousely WPQ/*24)
//  display. The TaxOut Object will be overriden to allow for this specific
//  format to be produced.
//
//  Updates:
//          02/28/04 - DVD - Intitial Development
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

class Diag804Collector : public DiagCollector
{

public:
  static constexpr char PERCENTAGE = 'P';

  //@TODO will be removed, once the transition is done
  explicit Diag804Collector(Diagnostic& root)
    : DiagCollector(root), _taxDisplayTotal(0), _skipCommonHeader(false)
  {
  }

  Diag804Collector() : _taxDisplayTotal(0), _skipCommonHeader(false) {}

  virtual Diag804Collector& operator<<(const TaxResponse& Data) override;

private:
  uint32_t _taxDisplayTotal;
  bool     _skipCommonHeader;

  enum TaxDisplayHeaderType
  {
    DIAGNOSTIC_HEADER = 1
  };

  void
  buildTaxDisplayHeader(const TaxResponse& taxResponse, TaxDisplayHeaderType taxDisplayHeaderType);
};

} // namespace tse

