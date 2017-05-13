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
#include "DataModel/Common/Code.h"

namespace tax { namespace type {

// RulesRecord - Key fields
//
struct NationTag{};
typedef Code<NationTag, 2> Nation;

struct TaxCodeTag{};
typedef Code<TaxCodeTag, 2> TaxCode;

struct TaxTypeTag{};
typedef Code<TaxTypeTag, 3> TaxType;

struct OcSubCodeTag{};
typedef Code<OcSubCodeTag, 3> OcSubCode;

// RulesRecord - Geo fields
struct LocZoneTextTag{};
typedef Code<LocZoneTextTag, 1, 8> LocZoneText;

struct AirportCodeTag{};
typedef Code<AirportCodeTag, 3> AirportCode; // Airport or Multi-Airport code

struct CityCodeTag{};
typedef Code<CityCodeTag, 3> CityCode;

typedef Code<CodeMultiTag<AirportCodeTag, CityCodeTag>, 3> AirportOrCityCode;

// RulesRecord - Fares
struct FareTariffIndTag{};
typedef Code<FareTariffIndTag, 3> FareTariffInd;

// RulesRecord - Carriers, Flights, etc
struct CarrierCodeTag{};
typedef Code<CarrierCodeTag, 2, 3> CarrierCode;

// Money
struct CurrencyCodeTag{};
typedef Code<CurrencyCodeTag, 3> CurrencyCode;

struct VendorTag{};
typedef Code<VendorTag, 1, 4> Vendor;

// RulesRecord - tags
struct TaxMatchingApplTagTag{};
typedef Code<TaxMatchingApplTagTag, 1, 2> TaxMatchingApplTag;

struct TaxProcessingApplTagTag{};
typedef Code<TaxProcessingApplTagTag, 1, 2> TaxProcessingApplTag; // 1 in DB but 2 in TSE

struct StopoverTimeTagTag{};
typedef Code<StopoverTimeTagTag, 1, 3> StopoverTimeTag;

struct PseudoCityCodeTag{};
typedef Code<PseudoCityCodeTag, 3, 5> PseudoCityCode; // it is 5 in TSE

// Passenger code
struct PassengerCodeTag{};
typedef Code<PassengerCodeTag, 3> PassengerCode;

} } // namespace tax::type

