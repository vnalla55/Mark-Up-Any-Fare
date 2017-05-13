//----------------------------------------------------------------------------
//  File:        Diag800Collector.C
//  Authors:     Dean Van Decker
//  Created:     Feb 2004
//
//  Description: Diagnostic 800 formatter
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

#include "Diagnostic/Diag800Collector.h"

#include "Common/Money.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/TaxCodeReg.h"

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag800Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//         Tax Record Diagnostic Display.
//
// @param  TaxCodeData - Specific Tax Information
//
//
// </PRE>
// ----------------------------------------------------------------------------

Diag800Collector&
Diag800Collector::operator << ( const TaxCodeReg& taxCodeReg )
{
  if (!_active)
    return *this;

  DiagCollector& dc = (DiagCollector&)*this;

  dc << taxCodeReg.taxCode() << "  SEQUENCE NUMBER: " << taxCodeReg.seqNo() << "\n";

  Money moneyPayment(taxCodeReg.taxAmt(), taxCodeReg.taxCur());
  dc.precision(moneyPayment.noDec());

  dc << taxCodeReg.taxCode() << "  TAX AMOUNT: " << moneyPayment << "\n";

  return *this;
}
}
