//-------------------------------------------------------------------
//
//  File:        ConstructionDefs.h
//  Created:     Jun 14, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description:
//
//  Copyright Sabre 2005
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

#include "Common/ArrayVector.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/HashKey.h"
#include "DBAccess/TSEDateInterval.h"

namespace tse
{

// public enums
// ====== =====

enum ConstructionPoint
{ CP_ORIGIN,
  CP_DESTINATION };

enum AddonZoneStatus
{ AZ_PASS,
  AZ_FAIL,
  AZ_UNACCEPTABLE };

enum FareMatchCode
{
  // good match codes
  FM_GOOD_MATCH,
  FM_COMB_FARE_EXACT_MATCH,

  // date interval mismatch
  FM_DATE_INTERVAL_MISMATCH,
  FM_TRF_COMB_INTERVAL_MISMATCH,

  // common ATPCO & SITA codes
  FM_COMB_FARE_CLASS,
  FM_FARE_TARIFF,
  FM_OWRT,
  FM_ADDON_DIRECTION,

  // SITA codes
  FM_SITA_TRULE,
  FM_SITA_TMPM,
  FM_SITA_TRTG,
  FM_SITA_BMPM_CANT_WITH_MILEAGE,
  FM_SITA_BRTG,
  FM_SITA_BMPM_CANT_WITH_ROUTING,
  FM_SITA_BMPM_UNKNOWN,
  FM_SITA_ROUTE_CODE,
  FM_SITA_TARIFF_FAMILY,
  FM_SITA_FARE_QUALITY_INCLUDE,
  FM_SITA_FARE_QUALITY_EXCLUDE,
  FM_SITA_FARE_QUALITY_UNKNOWN,
  FM_SITA_RTAR,
  FM_SITA_ARULE_INCLUDE,
  FM_SITA_ARULE_EXCLUDE,
  FM_SITA_ARULE_UNKNOWN,
  FM_SITA_CLFB_FBC,
  FM_SITA_CLFB_UNKNOWN,
  FM_SITA_GLOBAL_DBE,
  FM_SITA_GLOBAL,
  FM_SITA_GLOBAL_UNKNOWN
};

// common add-on construction types
// ====== ====== ============ =====

class AddonZoneInfo;

typedef std::vector<AddonZoneInfo*> AddonZoneInfoVec;

class AddonFareCortege;

typedef std::vector<AddonFareCortege*> AddonFareCortegeVec;

class GatewayPair;

typedef std::vector<GatewayPair*> GatewayPairVec;

typedef std::shared_ptr<GatewayPair> CacheGatewayPair;
typedef std::vector<CacheGatewayPair> CacheGatewayPairVec;

class ConstructedFare;

typedef std::vector<ConstructedFare*> ConstructedFareList;

class ConstructedFareInfo;

typedef std::vector<std::shared_ptr<ConstructedFareInfo>> CacheConstructedFareInfoVec;
typedef std::vector<ConstructedFareInfo*> ConstructedFareInfoVec;

class AddonCombFareClassInfo;

typedef std::vector<AddonCombFareClassInfo*> AddonCombFareClassInfoVec;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
typedef HashKey<LocCode, LocCode, CarrierCode, VendorCode, GlobalDirection> CacheKey;
#else
typedef HashKey<LocCode, LocCode, CarrierCode, VendorCode> CacheKey;
#endif

class TSEDateInterval;

typedef std::vector<TSEDateInterval*> TSEDateIntervalVec;

class InhibitedDateInterval;

typedef ArrayVector<InhibitedDateInterval*> InhibitedDateIntervalVec;

typedef std::vector<CacheKey>* FlushVec;
typedef std::vector<CacheKey>::iterator FlushVecIter;

const Indicator ADDON_FLUSH_IND = 'A';
const Indicator SPECIFIED_FLUSH_IND = 'S';

typedef HashKey<VendorCode, CarrierCode, LocCode, LocCode, Indicator> FlushKey;

typedef std::map<FlushKey, FlushVec> FlushMap;

typedef FlushMap::iterator FlushMapIter;

typedef FlushMap::value_type flushValueType;

const int ADDON_CACHE_SIZE = 3000;

const Indicator INHIBIT_P = 'P';

class ConstructionJob;

typedef std::vector<ConstructionJob*> ConstructionJobs;
typedef ConstructionJobs::iterator ConstructionJobsIter;
typedef ConstructionJobs::reverse_iterator ConstructionJobsReverseIter;

} // End of namespace tse

