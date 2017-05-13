//----------------------------------------------------------------------------
//
//  File:           CurrencyConversionCache.h
//
//  Description:    Entry point to the database access layer.
//
//  Updates:
//
//  Copyright Sabre 2006
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{

class DataHandle;
class NUCInfo;

// this is a class which is responsible for caching information
// that is used in currency conversion operations. It is designed
// so that when making repeated calls to currency conversion
// operations, an instance of the cache may be passed in and
// persisted across calls.
//
// The cache is not thread-safe (intentionally, for fast performance),
// and so should only be used from one thread.
class CurrencyConversionCache
{
public:
  explicit CurrencyConversionCache(DataHandle& dh);

  const NUCInfo*
  getNUCInfo(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& ticketDate);

private:
  DataHandle& _dh;
  typedef std::map<DateTime, NUCInfo*> DateTimeMap;
  typedef std::map<CarrierCode, DateTimeMap> CarrierCodeMap;
  typedef std::map<CurrencyCode, CarrierCodeMap> NUCCurrencyMap;
  NUCCurrencyMap _nucCache;
};
}

