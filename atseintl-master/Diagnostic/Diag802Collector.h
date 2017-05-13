//----------------------------------------------------------------------------
//  File:        Diag802Collector.h
//  Authors:     Dean Van Decker
//  Created:     Feb 2004
//
//  Description: Diagnostic 802 formatter for Tax Record Display.
//  Request to display all Tax Ticketing records for Debugging.
//  The TaxOut Object will be overriden to allow for this specific
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

class Diag802Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag802Collector(Diagnostic& root)
    : DiagCollector(root), _taxRecordDisplayTotal(0), _skipCommonHeader(false)
  {
  }
  Diag802Collector() : _taxRecordDisplayTotal(0), _skipCommonHeader(false) {}

  virtual Diag802Collector& operator<<(const TaxResponse& Data) override;

private:
  uint16_t _taxRecordDisplayTotal;
  bool     _skipCommonHeader;

  enum TaxRecordHeaderType
  {
    DIAGNOSTIC_HEADER = 1 // diagnostic header for WPQ/*24
  };

  void buildTaxRecordHeader(const TaxResponse& taxResponse,
                            TaxRecordHeaderType taxRecordDisplayHeaderType);
};

} // namespace tse

