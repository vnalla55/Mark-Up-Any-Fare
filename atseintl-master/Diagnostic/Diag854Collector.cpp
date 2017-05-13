//----------------------------------------------------------------------------
//  File:        Diag854Collector.C
//  Authors:     Dean Van Decker
//  Created:     Feb 2004
//
//  Description: Diagnostic 854 formatter
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

#include "Diagnostic/Diag854Collector.h"

#include "DataModel/FarePath.h"

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag854Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//         Tax Record Diagnostic Display.
//
// @param  TaxCodeData - Specific Tax Information
//
//
// </PRE>
// ----------------------------------------------------------------------------

Diag854Collector&
Diag854Collector::operator << (const FarePath& x)
{
  if (!_active)
    return *this;

  return *this;
}
}
