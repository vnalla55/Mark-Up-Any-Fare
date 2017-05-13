//----------------------------------------------------------------------------
//  File:        Diag860Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 860 formatter
//
//  Updates:
//          01/23/06 - WPA - Intitial Development
//
//  Copyright Sabre 2006
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

#include "Diagnostic/Diag860Collector.h"

#include "Common/Money.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FarePath.h"

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag860Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//         Tax Record Diagnostic Display.
//
// @param  TaxCodeData - Specific Tax Information
//
//
// </PRE>
// ----------------------------------------------------------------------------

Diag860Collector&
Diag860Collector::operator << (const FarePath& x)
{
  if (!_active)
    return *this;

  return *this;
}
}
