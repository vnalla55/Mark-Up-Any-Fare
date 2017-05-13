//----------------------------------------------------------------------------
//
//  File:           TseConsts.h
//  Description:    constants those are used in only routing.
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

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
// Mileage

extern const RoutingNumber MILEAGE_ROUTING;
extern const RoutingNumber GENERIC_ROUTING;
extern const RestrictionNumber ROUTING_RESTRICTION_1;
extern const RestrictionNumber ROUTING_RESTRICTION_2;
extern const RestrictionNumber ROUTING_RESTRICTION_3;
extern const RestrictionNumber ROUTING_RESTRICTION_4;
extern const RestrictionNumber ROUTING_RESTRICTION_5;
extern const RestrictionNumber ROUTING_RESTRICTION_6;
extern const RestrictionNumber ROUTING_RESTRICTION_7;
extern const RestrictionNumber RTW_ROUTING_RESTRICTION_8;
extern const RestrictionNumber RTW_ROUTING_RESTRICTION_9;
extern const RestrictionNumber RTW_ROUTING_RESTRICTION_10;
extern const RestrictionNumber ROUTING_RESTRICTION_11;
extern const RestrictionNumber RTW_ROUTING_RESTRICTION_12;
extern const RestrictionNumber ROUTING_RESTRICTION_13;
extern const RestrictionNumber ROUTING_RESTRICTION_14;
extern const RestrictionNumber ROUTING_RESTRICTION_15;
extern const RestrictionNumber MILEAGE_RESTRICTION_16;
extern const RestrictionNumber ROUTING_RESTRICTION_17;
extern const RestrictionNumber ROUTING_RESTRICTION_18;
extern const RestrictionNumber ROUTING_RESTRICTION_19;
extern const RestrictionNumber ROUTING_RESTRICTION_21;
// Restrictions
constexpr Indicator PERMITTED = 'P';
constexpr Indicator REQUIRED = 'R';
constexpr Indicator NOT_PERMITTED = 'N';
constexpr Indicator BETWEEN_CITY1_CITY2 = 'B'; // Between City1 and City2
constexpr Indicator CITY = 'C';
constexpr Indicator NONSTOP = 'N';
constexpr Indicator DIRECT = 'D';
constexpr Indicator EITHER = 'E'; // Either Nonstop or Direct, Either Air or Surface
constexpr Indicator AIR = 'A';
constexpr Indicator SURFACE = 'S';
constexpr Indicator BLANK = ' ';

extern const CarrierCode SURFACE_CARRIER;

constexpr Indicator NO_DOMESTIC_ROUTE_VALIDATION = '1'; // Do not perform Domestic Route Validation
constexpr Indicator DOMESTIC_ROUTE_VALIDATION = ' '; // Perform Domestic Route Validation
constexpr Indicator PASS_ANY_ONLINE_POINT = '2';

constexpr uint16_t EXCEEDS_MPM = 30;
constexpr uint16_t NO_TPM = 0;

// consts for Mileage Exclusions

extern const std::string CHINA;
extern const std::string NORTH_KOREA;
extern const std::string SOUTH_KOREA;
// const for MPM Deduction
extern const std::string MACAO;
extern const std::string HONG_KONG;

// const for TPM Exclusion
extern const CarrierCode AMERICAN_AIRLINES;
extern const LocCode DALLAS;
extern const LocCode CHICAGO;
extern const LocCode NEWYORK;
extern const LocCode MIAMI;
extern const LocCode NEWARK;

// const for Mileage Equalization
extern const LocCode RIO_DE_JANEIRO;
extern const LocCode SAO_PAULO;

// consts for TPD/PSR
constexpr Indicator TPD = 'T';
constexpr Indicator PSR = 'P';

constexpr Indicator THRUVIAMKTONLY_YES = 'Y';
constexpr Indicator THRUVIAMKTONLY_NO = 'N';

constexpr Indicator VIAGEOLOCREL_AND = '&';
constexpr Indicator VIAGEOLOCREL_OR = '/';
constexpr Indicator VIAGEOLOCREL_ANDOR = '-';

constexpr Indicator THRUMKTCXREXCEPT_YES = 'Y';
constexpr Indicator THRUMKTCXREXCEPT_NO = 'N';

constexpr Indicator THRUVIAMKTSAMECXR_YES = 'Y';
constexpr Indicator THRUVIAMKTSAMECXR_NO = 'N';

constexpr Indicator STPOVRNOTALWD_YES = 'Y';

enum TpdViaGeoLocMatching
{
  NO_MATCH,
  CITY_MATCH,
  GENERAL_MATCH
};

// FareByRule RoutingConst

extern const RoutingNumber CAT25_DOMESTIC;
extern const RoutingNumber CAT25_INTERNATIONAL;
extern const RoutingNumber CAT25_EMPTY_ROUTING;

// Vendor Constants for Routing Retrieval
constexpr Indicator PUBLISHED = 'P';
constexpr Indicator TRAVEL_AGENCY = 'T';
constexpr Indicator VENDOR_CARRIER = 'C';

// Terminal points
constexpr Indicator ENTRYEXITONLY = ' ';
constexpr Indicator ANYPOINT = '1';
constexpr Indicator GETTERMPTFROMCRXPREF = '0';

// Ticketed/hidden points
constexpr Indicator TKTPTS_TKTONLY = ' ';
constexpr Indicator TKTPTS_ANY = '1';
constexpr Indicator IGNORE_TKTPTSIND = '0';

// Map directionality
constexpr Indicator MAPDIR_BOTH = ' ';
constexpr Indicator MAPDIR_L2R = '1';
constexpr Indicator MAPDIR_IGNOREIND = '0';

constexpr bool OVERFLY_APPLY = true;
constexpr bool OVERFLY_NOT_APPLY = false;
}
