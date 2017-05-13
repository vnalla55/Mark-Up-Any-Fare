//-------------------------------------------------------------------
//
//  File:        PaxTypeBucket.h
//  Created:     July 3, 2006
//  Design:      Doug Steeb, Jeff Hoffman
//  Authors:
//
//  Description: a collection of fares associated with one pax type
//
//  Updates:
//          07/03/06 - JH - Moved from FareMarket::PaxTypeCortege
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

#include "Common/YQYR/YQYRTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/CxrPrecalculatedTaxes.h"
#include "Util/FlatMap.h"

#include <unordered_map>
#include <vector>

namespace tse
{
class FareMarket;
class PaxType;
class PaxTypeFare;
class PricingTrx;

class PaxTypeBucket
{
public:
  PaxTypeBucket(PaxType* pt) : _requestedPaxType(pt) {}

  PaxTypeBucket() : _inboundCurrency("USD"), _outboundCurrency("USD") {}

  bool operator==(const PaxTypeBucket& rhs) const;

  PaxType*& requestedPaxType() { return _requestedPaxType; }
  const PaxType* requestedPaxType() const { return _requestedPaxType; }

  uint16_t& paxIndex() { return _paxIndex; }
  const uint16_t& paxIndex() const { return _paxIndex; }

  std::vector<PaxType*>& actualPaxType() { return _actualPaxType; }
  const std::vector<PaxType*>& actualPaxType() const { return _actualPaxType; }

  CurrencyCode& inboundCurrency() { return _inboundCurrency; }
  const CurrencyCode& inboundCurrency() const { return _inboundCurrency; }

  CurrencyCode& outboundCurrency() { return _outboundCurrency; }
  const CurrencyCode& outboundCurrency() const { return _outboundCurrency; }
  CurrencyCode& fareQuoteOverrideCurrency() { return _fareQuoteOverrideCurrency; }
  const CurrencyCode& fareQuoteOverrideCurrency() const { return _fareQuoteOverrideCurrency; }

  std::vector<PaxTypeFare*>& paxTypeFare() { return _paxTypeFare; }
  const std::vector<PaxTypeFare*>& paxTypeFare() const { return _paxTypeFare; }

  CurrencyCode& equivAmtOverrideCurrency() { return _equivAmtOverrideCurrency; }
  const CurrencyCode& equivAmtOverrideCurrency() const { return _equivAmtOverrideCurrency; }

  bool isMarketCurrencyPresent() const { return _isMarketCurrencyPresent; }
  void setMarketCurrencyPresent(bool state) { _isMarketCurrencyPresent = state; }
  void setMarketCurrencyPresent(PricingTrx& trx, FareMarket& fm);

  CxrPrecalculatedTaxes& mutableCxrPrecalculatedTaxes() { return _cxrPrecalculatedTaxes; }
  const CxrPrecalculatedTaxes& cxrPrecalculatedTaxes() const { return _cxrPrecalculatedTaxes; }

  bool isPaxTypeInActualPaxType(const PaxTypeCode) const;
  bool isBucketFilteredSFR() const { return _isBucketFilteredSFR; }
  void setBucketFilteredSFR() { _isBucketFilteredSFR = true; }

  void removeAllFares() { _paxTypeFare.clear(); }

  YQYR::CarrierStorage& getYqyrCarrierStorage(const CarrierCode& carrier)
  {
    return _yqyrsPerCarrierShopping[carrier];
  }

  void clearYqyrCarrierStorage() { _yqyrsPerCarrierShopping.clear(); }

  bool hasAnyFareValid() const;

private:
  PaxType* _requestedPaxType = nullptr;
  std::vector<PaxType*> _actualPaxType;
  uint16_t _paxIndex = 0;
  bool _isMarketCurrencyPresent = false;
  bool _isBucketFilteredSFR = false;
  CurrencyCode _inboundCurrency;
  CurrencyCode _outboundCurrency;
  CurrencyCode _fareQuoteOverrideCurrency;
  CurrencyCode _equivAmtOverrideCurrency;
  std::vector<PaxTypeFare*> _paxTypeFare;
  CxrPrecalculatedTaxes _cxrPrecalculatedTaxes;

  typedef std::unordered_map<CarrierCode, YQYR::CarrierStorage> FeesPerCarrierMap;
  FeesPerCarrierMap _yqyrsPerCarrierShopping;

  bool isFareSkippedByOptionXO(PricingTrx& _trx, PaxTypeFare& paxTypeFare);
  bool isFareSkippedByOptionXC(PricingTrx& _trx, PaxTypeFare& paxTypeFare);
}; // endclass - PaxTypeBucket
} // tse namespace
