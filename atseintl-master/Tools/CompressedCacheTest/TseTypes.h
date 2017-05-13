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

#include <stdint.h>
#include <string>
#include <set>
#include <boost/container/string.hpp>
#include "Code.h"
#include "DateTime.h"
//#include "DereferencingComparators.h"

namespace tse
{

// general types
// ======= =====

typedef char        Indicator;
typedef double      MoneyAmount;
typedef boost::container::string BoostString;

// Location related types
// ======== ======= =====

typedef std::string IATAAreaCode;        // CString4    IATAAreaCode;
typedef std::string IATASubAreaCode;     // CString2    IATASubAreaCode;
typedef Code<4>     NationCode;          // CString2    NationCode;
typedef Code<4>     StateCode;           // CString4    StateCode;
typedef std::string TransType;           // CString2    TransType;
typedef int         ISONumericCode;
typedef std::string ISOAlphaCode;        // CString3    ISOAlphaCode;

typedef Indicator   LocTypeCode;
typedef Code<8>     LocCode;             // CString5    LocCode;
typedef std::string LocDescription;

typedef Code<5>     PseudoCityCode;      // CString5    PseudoCityCode;
typedef Code<7>     AgencyPCC;           // CString7    AgencyPCC;
typedef std::string AgencyCode;          // CString4    AgencyCode;
typedef std::string AgencyIATA;          // CString8    AgencyIATA;
typedef std::string AgencyDutyCode;      // CString8    AgencyDutyCode;
typedef Indicator   PseudoCityType;      // Indicator   PseudoCityType

typedef int         LatLong;

typedef Indicator   AlaskaZoneCode;
typedef std::string DSTGrpCode;          // CString4    DSTGrpCode;

// Itinerary segment related types
// ========= ======= ======= =====

typedef Code<3>     EquipmentType;       // CString3    EquipmentType;
typedef int32_t     FlightNumber;

// Fares
// =====

typedef Indicator   FareDirectionType;

typedef int32_t     LinkNumber;
typedef int32_t     SequenceNumber;
typedef int         TariffNumber;
typedef int         TariffCategory;
typedef Code<7>     TariffCode;          // CString7    TariffCode;

typedef BoostString FareType;            // CString8    FareType;
typedef BoostString FareTypeAbbrev;      // CString3    FareTypeAbbrev;
typedef BoostString FareClassCode;

typedef std::string FareCombinationCode; // CString2    FareCombinationCode
typedef int16_t     SetNumber;

// SITA Fare fields
// ==== ==== ======

typedef std::string RouteCode;           // CString2    RouteCode;
typedef Code<2> RouteCodeC;           // CString2    RouteCode;
typedef Code<3>     DBEClass;            // CString3    DBEClass;

// Common needs
// ====== =====

typedef Code<4>     VendorCode;          // CString5    VendorCode;

typedef Code<3>     CarrierCode;         // CString3    CarrierCode;
typedef Code<3>     PaxTypeCode;         // CString3    PaxTypeCode;
typedef std::string PaxGroupTypeCode;    // CString3    PaxGroupTypeCode;
typedef std::string MessageTypeCode;     // CString10   MessageTypeCode;
typedef std::string CustomerIDCode;      // CString4    CustomerIDCode;

typedef Code<3>     CurrencyCode;        // CString3    CurrencyCode;
typedef double      CurrencyFactor;
typedef int         CurrencyNoDec;
typedef int         RoundUnitNoDec;
typedef double      ExchRate;
typedef double      RoundingFactor;
typedef int         CurrencyRoundUnit;
typedef double      RoundUnit;

typedef Code<2>     Footnote;            // CString2    Footnote;
typedef Code<4>     RuleNumber;          // CString4    RuleNumber;
typedef int         CatNumber;

typedef Code<4>     RoutingNumber;       // CString4    RoutingNumber;
typedef Code<2>     RestrictionNumber;
typedef Code<7>     Zone;                // CString7    Zone;
typedef int         AddonZone;           // CString3    AddonZone;
typedef Indicator   ZoneType;

typedef BoostString TktCode;             // CString10   TktCode;
typedef Code<3> TktCodeModifier;     // CString3    TktCodeModifier;

typedef BoostString TktDesignator;       // CString10   TktDesignator;
typedef Code<3> TktDesignatorModifier;// CString3    TktDesignatorModifier;

typedef int16_t     TSICode;

typedef Code<2>     BookingCode;         // CString2    BookingCode;

typedef Code<3>     TaxCode;             // CString3    TaxCode;
typedef std::string TaxDescription;
typedef std::string TaxOnTaxInfo;
typedef std::string FreeTextSegData;
typedef Code<12>    TaxSpecConfigName;

typedef std::string TaxMessage;
typedef double      Percent;

typedef Indicator   TaxTypeCode;
typedef Indicator   TaxRangeType;

typedef Code<4>     UserApplCode;        // CString4    UserApplCode;
typedef int         TJRGroup;
typedef std::string BookConvSeq;         // CString4    BookConvSeq;

typedef std::string AccountCode;
typedef Code<8> AccountCodeC;

typedef int         TvlSectSiCode;
typedef std::string AppendageCode;       // CString5    AppendageCode;
typedef Code<7> DayOfWeekCode;
// OptionalServicesInfo
typedef Code<15> TourCode;
// OptionalServicesInfo
typedef Code<8> RuleBusterFcl;
typedef std::string ResUnit;             // CString2    ResUnit;
typedef std::string ResPeriod;           // CString3    ResPeriod;

typedef std::string Description;

typedef std::string DifferentialTag;

typedef std::string PSSDate;
typedef std::string PSSTime;

typedef std::string InclusionCode;		// CString4

typedef Code<2>     AlphaCode;			// CString2
typedef std::string CombinabilityCode;	// CString2
typedef std::string TemplateID;			// CString2
typedef std::string BrandCode;          // CString2

typedef int32_t     DisplayCategory;
typedef int16_t     IntIndex; // also used in pair with INVALID_INT_INDEX constant

typedef std::pair<uint32_t,uint32_t>     IndexPair;
typedef std::vector<uint32_t>            IndexVector;
typedef IndexVector::iterator            IndexVectorIterator;
typedef IndexVector::const_iterator      IndexVectorConstIterator;
typedef std::vector<IndexVector>         IndexVectors;
typedef IndexVectors::iterator           IndexVectorsIterator;
typedef IndexVectors::const_iterator     IndexVectorsConstIterator;
typedef std::vector<IndexPair>           IndexPairVector;
typedef IndexPairVector::iterator        IndexPairVectorIterator;
typedef IndexPairVector::const_iterator  IndexPairVectorConstIterator;
typedef std::vector<IndexPairVector>     IndexPairVectors;
typedef IndexPairVectors::iterator       IndexPairVectorsIterator;
typedef IndexPairVectors::const_iterator IndexPairVectorsConstIterator;

typedef Code<2>  ServiceTypeCode;         // CString2    Service fee code
typedef Code<3>  ServiceSubTypeCode;      // CString3    Service sub code
typedef Code<3>  ServiceGroup;            // CString3    Service group
typedef Code<2>  ServiceGroupDescription; // CString2    Service gropup description
typedef Code<4>  SubCodeSSR;              // CString4    Special Service Request
typedef Code<2>  ServiceDisplayInd;       // CString2    Service display indicator
typedef Code<2>  ServiceBookingInd;       // CString2    Service booking indicator
typedef Code<3>  StopoverTime;            // CString3    Stopover time
typedef Code<3>  ServiceRuleTariffInd;    // CString3    Service rule tariff indicator
typedef Code<3>  ServicePurchasePeriod;   // CString3    Service Purchase period
typedef Code<2>  ServicePurchaseUnit;     // CString2    Service Purchase unit
typedef Code<15> FareBasisCode;           // CString15   fare basis code

typedef Code<6>  FopBinNumber;       // CString6    fop bin number
typedef Code<7>  TaxRestrictionLocation;

typedef Code<2>  OCFareTypeCode;
typedef Code<5>  PsgNameNumber;
typedef Code<20> TktNumber;

//typedef std::set<const DateTime*, DereferencingLess> TvlDatesSet;

} // namespace tse

#include "TseEnums.h"
