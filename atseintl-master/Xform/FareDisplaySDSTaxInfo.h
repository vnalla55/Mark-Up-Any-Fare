//-------------------------------------------------------------------
//
//  File:        FareDisplaySDSTaxInfo.h
//  Authors:     Hitha Alex
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize the Tax Display that
//               appear on the Sabre greenscreen.)
//
//
//  Copyright Sabre 2006
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
class FareDisplaySDSTaxInfo
{
  class Tax;
  class TaxCodeReg;

public:
  FareDisplaySDSTaxInfo() : _taxAmount(0), _feeInd(false) {}
  virtual ~FareDisplaySDSTaxInfo() {};

  const std::string& taxCode() const { return _taxCode; }
  std::string& taxCode() { return _taxCode; }

  const TaxCode& ticketingTaxCode() const { return _ticketingTaxCode; }
  TaxCode& ticketingTaxCode() { return _ticketingTaxCode; }

  const TaxDescription& taxDescription() const { return _taxDescription; }
  TaxDescription& taxDescription() { return _taxDescription; }

  const MoneyAmount& taxAmount() const { return _taxAmount; }
  MoneyAmount& taxAmount() { return _taxAmount; }

  const bool& feeInd() const { return _feeInd; }
  bool& feeInd() { return _feeInd; }

private:
  std::string _taxCode;
  MoneyAmount _taxAmount;
  bool _feeInd;
  TaxCode _ticketingTaxCode;
  TaxDescription _taxDescription;
};

} // namespace tse

