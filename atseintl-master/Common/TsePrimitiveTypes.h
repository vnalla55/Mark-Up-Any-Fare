//-------------------------------------------------------------------
//
//  File:        TseTypes.h
//  Created:     March 8, 2004
//  Authors:
//
//  Description: Common TSE types
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

#include <cstdint>
// To keep small number of dependencies please do not add new includes.
// See existing files Common/Tse*Types.h or create new one.

namespace tse
{
class TravelSeg;

typedef char Indicator;
typedef double CurrencyFactor;
typedef double ExchRate;
typedef double MoneyAmount;
typedef double Percent;
typedef double RoundingFactor;
typedef double RoundUnit;
typedef Indicator AlaskaZoneCode;
typedef Indicator FareDirectionType;
typedef Indicator LocTypeCode;
typedef Indicator PseudoCityType;
typedef Indicator SvcType;
typedef Indicator SystemCode;
typedef Indicator TaxRangeType;
typedef Indicator TaxTypeCode;
typedef Indicator TicketMethod;
typedef Indicator ZoneType;
typedef int AddonZone;
typedef int CatNumber;
typedef int CurrencyNoDec;
typedef int CurrencyRoundUnit;
typedef int ISONumericCode;
typedef int LatLong;
typedef int RoundUnitNoDec;
typedef int SopId;
typedef int TariffCategory;
typedef int TariffNumber;
typedef int TierNumber;
typedef int TJRGroup;
typedef int TvlSectSiCode;
typedef int16_t IntIndex; // also used in pair with INVALID_INT_INDEX constant
typedef int16_t SetNumber;
typedef int16_t TSICode;
typedef int32_t DisplayCategory;
typedef int32_t FlightNumber;
typedef int32_t LinkNumber;
typedef int32_t SequenceNumber;
typedef int64_t SequenceNumberLong;
typedef uint32_t LegId;

using FreqFlyerTierLevel = uint16_t;

struct SegmentAttributes
{
  TravelSeg* tvlSeg;
  uint16_t attributes;
};

} // namespace tse
