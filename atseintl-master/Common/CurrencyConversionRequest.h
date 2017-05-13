//----------------------------------------------------------------------------
//
//  File: CurrencyConversionRequest.h
//
//  Created: June 2004
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"

namespace tse
{
class Money;
class DateTime;
class PricingRequest;
class DataHandle;
class PricingOptions;
class PricingTrx;

class CurrencyConversionRequest
{
public:
  enum ApplicationType
  { TAXES,
    FARES,
    SURCHARGES,
    DC_CONVERT,
    DC_DISPLAY,
    OTHER,
    FAREDISPLAY,
    NO_ROUNDING};

  CurrencyConversionRequest(Money& target,
                            const Money& source,
                            const DateTime& ticketDate,
                            const PricingRequest& request,
                            DataHandle& dataHandle,
                            bool isInternational = true,
                            ApplicationType applicationType = ApplicationType::OTHER,
                            bool reciprocalRate = false,
                            const PricingOptions* options = nullptr,
                            bool useInternationalRounding = false,
                            bool roundFare = true,
                            const PricingTrx* trx = nullptr);

  Money& target() { return _target; }
  const Money& target() const { return _target; }

  const Money& source() const { return _source; }

  const DateTime& ticketDate() const { return _ticketDate; }

  const PricingRequest& getRequest() const { return _request; }

  DataHandle& dataHandle() const { return _dataHandle; }

  bool& isInternational() { return _isInternational; }
  const bool& isInternational() const { return _isInternational; }

  ApplicationType& applicationType() { return _applicationType; }
  const ApplicationType& applicationType() const { return _applicationType; }

  bool& reciprocalRate() { return _reciprocalRate; }
  const bool& reciprocalRate() const { return _reciprocalRate; }

  bool& commonSalesLocCurrency() { return _commonSalesLocCurrency; }
  const bool& commonSalesLocCurrency() const { return _commonSalesLocCurrency; }

  CurrencyCode& salesLocCurrency() { return _salesLocCurrency; }
  const CurrencyCode& salesLocCurrency() const { return _salesLocCurrency; }

  CurrencyCode& conversionCurrency() { return _conversionCurrency; }
  const CurrencyCode& conversionCurrency() const { return _conversionCurrency; }

  const PricingOptions* getOptions() { return _options; }

  bool& useInternationalRounding() { return _useInternationalRounding; }
  const bool& useInternationalRounding() const { return _useInternationalRounding; }

  bool& roundFare() { return _roundFare; }
  const bool& roundFare() const { return _roundFare; }

  const PricingTrx* trx() { return _trx; }
  void setTrx(const PricingTrx* trx) { _trx = trx; }

private:
  Money& _target;
  const Money& _source;
  const DateTime& _ticketDate;
  const PricingRequest& _request;
  DataHandle& _dataHandle;
  bool _isInternational = true;
  ApplicationType _applicationType = ApplicationType::OTHER;
  bool _reciprocalRate = false;
  bool _commonSalesLocCurrency = false;
  CurrencyCode _salesLocCurrency;
  CurrencyCode _conversionCurrency;
  const PricingOptions* _options = nullptr;
  bool _useInternationalRounding = false;
  bool _roundFare = true;
  const PricingTrx* _trx = nullptr;
};

inline CurrencyConversionRequest::CurrencyConversionRequest(Money& target,
                                                            const Money& source,
                                                            const DateTime& ticketDate,
                                                            const PricingRequest& request,
                                                            DataHandle& dataHandle,
                                                            bool isInternational,
                                                            ApplicationType applicationType,
                                                            bool reciprocalRate,
                                                            const PricingOptions* options,
                                                            bool useInternationalRounding,
                                                            bool roundFare,
                                                            const PricingTrx* trx)
  : _target(target),
    _source(source),
    _ticketDate(ticketDate),
    _request(request),
    _dataHandle(dataHandle),
    _isInternational(isInternational),
    _applicationType(applicationType),
    _reciprocalRate(reciprocalRate),
    _options(options),
    _useInternationalRounding(useInternationalRounding),
    _roundFare(roundFare),
    _trx(trx)
{
}
}
