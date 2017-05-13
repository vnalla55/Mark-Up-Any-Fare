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

#include <cppunit/extensions/HelperMacros.h>
#include "Common/CurrencyConversionFacade.h"

namespace tse
{

class CurrencyConversionFacadeMock : public CurrencyConversionFacade
{
public:
  CurrencyConversionFacadeMock() : _callsCounter(0), _convertResult(false) {};

  virtual bool
  convert(Money& target,
          const Money& source,
          const PricingTrx& trx,
          bool useInternationalRounding = false,
          CurrencyConversionRequest::ApplicationType applType = CurrencyConversionRequest::OTHER,
          bool reciprocalRate = false,
          CurrencyCollectionResults* results = nullptr) override
  {
    _callsCounter++;
    return _convertResult;
  }

  void setConvertResult(bool convertResult) { _convertResult = convertResult; }

  uint32_t getCallsCounter() const { return _callsCounter; }

private:
  uint32_t _callsCounter;
  bool _convertResult;
};

} // namespace tse
