//-------------------------------------------------------------------
//
//  File:        FTTax.h
//  Authors:     Abu
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize the Tax Display that
//               appear on the Sabre greenscreen.)
//
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{

class FTTax
{
  class Tax;
  class TaxCodeReg;

public:
  const TaxCode& taxCode() const { return _taxCode; }
  TaxCode& taxCode() { return _taxCode; }

  const MoneyAmount& taxAmount() const { return _taxAmount; }
  MoneyAmount& taxAmount() { return _taxAmount; }

  const bool& feeInd() const { return _feeInd; }
  bool& feeInd() { return _feeInd; }

  const bool& surchgInd() const { return _surchgInd; }
  bool& surchgInd() { return _surchgInd; }

private:
  TaxCode _taxCode;
  MoneyAmount _taxAmount = 0;
  bool _feeInd = false;
  bool _surchgInd = false;
};
} // namespace tse
