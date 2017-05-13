// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <stdint.h>
#include <string>

#include "DataModel/Common/Rational64.h"
#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Codes.h"

namespace tax
{

namespace type
{

// RulesRecord - Key fields
typedef std::string SabreTaxCode;
typedef std::string TaxTypeOrSubCode;
typedef std::string TaxGroupType;
typedef std::string TktFeeSubCode;
typedef std::string ServiceGroupCode;
typedef unsigned SeqNo;
typedef char YqYrType;

// RulesRecord - Geo fields
typedef std::string LocCode;
typedef std::string StateProvinceCode;

// RulesRecord - Fares
typedef std::string FareBasisCode;
typedef std::string FareTypeCode;
typedef char OneWayRoundTrip;
typedef int16_t FareTariff;
// RulesRecord - Carriers, Flights, etc
typedef std::string EquipmentCode;
typedef std::size_t Index;
typedef int16_t FlightNumber;
typedef std::string ReservationDesignatorCode;
typedef std::string TicketCode;
typedef std::string CarrierGdsCode;


typedef std::string TaxLabel;

// Money
typedef uint32_t IntMoneyAmount;
typedef unsigned CurDecimals;
typedef Rational64 MoneyAmount;
typedef Rational64 Percent;
typedef Rational64 BSRValue;
typedef struct
{
  MoneyAmount _amount;
  CurrencyCode _currency;
} Money;

// Processing
typedef uint32_t Miles;
typedef int16_t CalcOrder;
typedef std::string RuleName;
typedef std::string DiagParamName;
typedef std::string DiagParamValue;

// Fares
typedef std::string FareRuleCode;

// Agent info
typedef std::string DutyFunctionCode;

// Nation info
typedef std::string NationName;
typedef std::string NationMessage;
} // namespace type

} // namespace tax
