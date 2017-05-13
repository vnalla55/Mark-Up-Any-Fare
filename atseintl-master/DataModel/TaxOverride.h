//----------------------------------------------------------------------------
//
//  File:         TaxOverride.h
//  Created:
//  Authors:
//
//  Description:
//
//  Updates:
//          08/13/04 - Gerald LePage - Change copy and assignment constructor
//                     to public.
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{

//---------------------------------------------------------------------------
// TaxOverride is used as an Exemption object for the Tax Service..
//---------------------------------------------------------------------------

class TaxOverride
{
public:
  TaxOverride() : _taxAmt(0) {}

  const MoneyAmount& taxAmt() const { return _taxAmt; }
  MoneyAmount& taxAmt() { return _taxAmt; }

  const TaxCode& taxCode() const { return _taxCode; }
  TaxCode& taxCode() { return _taxCode; }

private:
  TaxOverride(const TaxOverride&);
  TaxOverride& operator=(const TaxOverride&);

  MoneyAmount _taxAmt;
  TaxCode _taxCode;
};
} // tse namespace
