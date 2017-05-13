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

#include "Common/CurrencyConversionRequest.h"
#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/CurrencyService.h"

namespace tse
{
class PricingTrx;
class CurrencyConversionFacade;

class CurrencyServiceV2 : public tax::CurrencyService
{
  using ConversionType = CurrencyConversionRequest::ApplicationType;
  friend class CurrencyServiceV2Test;

public:
  explicit CurrencyServiceV2(PricingTrx& trx, const DateTime& ticketDate);
  ~CurrencyServiceV2();
  tax::type::MoneyAmount
  convertTo(const tax::type::CurrencyCode& targetCurrency, const tax::type::Money& source,
            tax::CalcDetails* calcDetails = nullptr) const override;

  tax::type::BSRValue getBSR(const tax::type::CurrencyCode& fromCurrency,
                             const tax::type::CurrencyCode& toCurrency) const override;

  tax::type::MoneyAmount convert(const tse::MoneyAmount amount,
                                 const tse::CurrencyCode& baseFareCurrency,
                                 const tse::CurrencyCode& calculationCurrency,
                                 const tse::CurrencyCode& paymentCurrency,
                                 ConversionType applType = CurrencyConversionRequest::OTHER,
                                 bool useInternationalRounding = false) const;

  tax::type::CurDecimals getCurrencyDecimals(const tax::type::CurrencyCode&) const override;

  tax::type::CurrencyCode getNationCurrency(const tax::type::CurrencyCode& taxCurrency,
                                            const tax::type::Nation& nation) const override;

private:
  CurrencyServiceV2(const CurrencyServiceV2&);
  CurrencyServiceV2& operator=(const CurrencyServiceV2&);

  PricingTrx& _trx;
  CurrencyConversionFacade* _conversionFacade;
  const DateTime& _ticketDate;
};

} // namespace tse
