//----------------------------------------------------------------------------
//  File:           TaxInfoRequest.h
//  Description:    TaxInfoRequest header file for ATSE International Project
//  Created:        09/Dec/2008
//  Authors:        Jakub Kubica
//
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "Common/DateTime.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/Loc.h"

#include <string>
#include <vector>

namespace tse
{

//------------------------------------------------
class TaxInfoItem
{
public:
  TaxCode& taxCode() { return _taxCode; }
  const TaxCode& taxCode() const { return _taxCode; }

  std::vector<LocCode>& airports() { return _airports; }
  const std::vector<LocCode>& airports() const { return _airports; }

private:
  TaxCode _taxCode;
  std::vector<LocCode> _airports;
};

//------------------------------------------------

class TaxInfoRequest /*: public PricingRequest */
{
public:
  std::vector<NationCode>& countryCodes() { return _countryCodes; }
  const std::vector<NationCode>& countryCodes() const { return _countryCodes; }

  std::vector<TaxInfoItem>& taxItems() { return _taxItems; }
  const std::vector<TaxInfoItem>& taxItems() const { return _taxItems; }

  DateTime& overrideDate() { return _overrideDate; }
  const DateTime& overrideDate() const { return _overrideDate; }

private:
  std::vector<TaxInfoItem> _taxItems;
  std::vector<NationCode> _countryCodes;
  DateTime _overrideDate;
};
} // namespace tse
