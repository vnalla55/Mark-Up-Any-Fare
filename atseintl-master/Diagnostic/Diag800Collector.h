//----------------------------------------------------------------------------
//  File:        Diag800Collector.h
//  Authors:     Dean Van Decker
//  Created:     Feb 2004
//
//  Description: Diagnostic 800 formatter will display all specific errors
//  in the Tax Service. The TaxCodeData object will be moved into this
//  specific format. Tax Fail Codes will be included in the diagnostic to
//  allow for a Developer to pinpoint the exact location
//  in the code where the Tax Failed. Tax Diagnostic Header cointains
//  the corresponding Tag to the Fail Code number produced
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

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class TaxCodeReg;

class Diag800Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag800Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag800Collector() {}

  virtual Diag800Collector& operator<<(const TaxCodeReg& data) override;
};

} // namespace tse

