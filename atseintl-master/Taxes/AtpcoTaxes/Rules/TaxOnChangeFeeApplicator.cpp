// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "DataModel/Common/Types.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxOnChangeFeeApplicator.h"
#include "Rules/TaxOnChangeFeeRule.h"
#include "ServiceInterfaces/CurrencyService.h"
#include "ServiceInterfaces/LoggerService.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{
TaxOnChangeFeeApplicator::TaxOnChangeFeeApplicator(TaxOnChangeFeeRule const& parent,
                                                   const Services& services)
  : BusinessRuleApplicator(&parent),
    _taxOnChangeFeeRule(parent),
    _services(services)
{
}

TaxOnChangeFeeApplicator::~TaxOnChangeFeeApplicator()
{
}

bool
TaxOnChangeFeeApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (_taxOnChangeFeeRule.percentFlatTag() == type::PercentFlatTag::Percent)
  {
    paymentDetail.taxOnChangeFeeAmount() = paymentDetail.changeFeeAmount() * paymentDetail.taxAmt();
  }
  else
  {
    type::Money taxMoney = { paymentDetail.taxAmt(), paymentDetail.taxCurrency() };

    try
    {
      paymentDetail.taxOnChangeFeeAmount() =
          _services.currencyService().convertTo(paymentDetail.taxEquivalentCurrency(), taxMoney);
    }
    catch (const std::runtime_error&)
    {
      _services.loggerService().log_ERROR("TaxOnChangeFeeApplicator - conversion failed!");
      paymentDetail.taxOnChangeFeeAmount() = paymentDetail.taxAmt();
    }
  }

  return true;
}
}
