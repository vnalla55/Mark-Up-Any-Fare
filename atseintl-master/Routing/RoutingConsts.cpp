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
#include "Routing/RoutingConsts.h"

namespace tse
{
const RoutingNumber MILEAGE_ROUTING = "0000";
const RoutingNumber GENERIC_ROUTING = "4444";
const RestrictionNumber ROUTING_RESTRICTION_1 = "01";
const RestrictionNumber ROUTING_RESTRICTION_2 = "02";
const RestrictionNumber ROUTING_RESTRICTION_3 = "03";
const RestrictionNumber ROUTING_RESTRICTION_4 = "04";
const RestrictionNumber ROUTING_RESTRICTION_5 = "05";
const RestrictionNumber ROUTING_RESTRICTION_6 = "06";
const RestrictionNumber ROUTING_RESTRICTION_7 = "07";
const RestrictionNumber RTW_ROUTING_RESTRICTION_8 = "08";
const RestrictionNumber RTW_ROUTING_RESTRICTION_9 = "09";
const RestrictionNumber RTW_ROUTING_RESTRICTION_10 = "10";
const RestrictionNumber ROUTING_RESTRICTION_11 = "11";
const RestrictionNumber RTW_ROUTING_RESTRICTION_12 = "12";
const RestrictionNumber ROUTING_RESTRICTION_13 = "13";
const RestrictionNumber ROUTING_RESTRICTION_14 = "14";
const RestrictionNumber ROUTING_RESTRICTION_15 = "15";
const RestrictionNumber MILEAGE_RESTRICTION_16 = "16";
const RestrictionNumber ROUTING_RESTRICTION_17 = "17";
const RestrictionNumber ROUTING_RESTRICTION_18 = "18";
const RestrictionNumber ROUTING_RESTRICTION_19 = "19";
const RestrictionNumber ROUTING_RESTRICTION_21 = "21";

const CarrierCode SURFACE_CARRIER = "XX";

// consts for Mileage Exclusions
const std::string CHINA = "CN";
const std::string NORTH_KOREA = "KP";
const std::string SOUTH_KOREA = "KR";
// const for MPM Deduction
const std::string MACAO = "MFM";
const std::string HONG_KONG = "HKG";

// const for TPM Exclusion
const CarrierCode AMERICAN_AIRLINES = "AA";
const LocCode DALLAS = "DFW";
const LocCode CHICAGO = "CHI";
const LocCode NEWYORK = "NYC";
const LocCode MIAMI = "MIA";
const LocCode NEWARK = "EWR";

// const for Mileage Equalization
const LocCode RIO_DE_JANEIRO = "RIO";
const LocCode SAO_PAULO = "SAO";

// FareByRule RoutingConst
const RoutingNumber CAT25_DOMESTIC = "EIGH";
const RoutingNumber CAT25_INTERNATIONAL = "SEVN";
const RoutingNumber CAT25_EMPTY_ROUTING = "";
}
