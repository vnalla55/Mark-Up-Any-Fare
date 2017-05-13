//-------------------------------------------------------------------
//
//  File:        AddOnAttributes.h
//  Created:     October 6, 2005
//  Authors:     Doug Batchelor
//
//  Description: Addon items for constructed fares
//
//  Updates:
//          02/15/05 - Doug Batchelor - file created.
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
//-------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PricingOptions.h"

namespace tse
{

class AddOnAttributes : public PricingOptions
{
private:
  AddOnAttributes(const AddOnAttributes&) = delete;
  AddOnAttributes& operator=(const AddOnAttributes&) = delete;

  // Constructed fare items

  Footnote _AddonFootNote1; // S55 CString2
  Footnote _AddonFootNote2; // S64 CString2
  FareClassCode _AddonFareClass; // BJ0 CString8
  TariffNumber _AddonTariff = 0; // Q3W int
  RoutingNumber _AddonRouting; // S65 CString4
  MoneyAmount _AddonAmount = 0; // C50 double
  CurrencyCode _AddonCurrency; // C40 CString3
  Indicator _OWRT = 0; // P04 char
  bool _isAddOn = false;

public:
  AddOnAttributes() = default;

  //-------------------------------------------------------------------------
  // Accessors
  //-------------------------------------------------------------------------

  Footnote& addonFootNote1() { return _AddonFootNote1; }
  const Footnote& addonFootNote1() const { return _AddonFootNote1; }

  Footnote& addonFootNote2() { return _AddonFootNote2; }
  const Footnote& addonFootNote2() const { return _AddonFootNote2; }

  FareClassCode& addonFareClass() { return _AddonFareClass; }
  const FareClassCode& addonFareClass() const { return _AddonFareClass; }

  TariffNumber& addonTariff() { return _AddonTariff; }
  const TariffNumber& addonTariff() const { return _AddonTariff; }

  RoutingNumber& addonRouting() { return _AddonRouting; }
  const RoutingNumber& addonRouting() const { return _AddonRouting; }

  MoneyAmount& addonAmount() { return _AddonAmount; }
  const MoneyAmount& addonAmount() const { return _AddonAmount; }

  CurrencyCode& addonCurrency() { return _AddonCurrency; }
  const CurrencyCode& addonCurrency() const { return _AddonCurrency; }

  Indicator& oWRT() { return _OWRT; }
  const Indicator& oWRT() const { return _OWRT; }

  bool& isAddOn() { return _isAddOn; }
  const bool& isAddOn() const { return _isAddOn; }
};
} // tse namespace
