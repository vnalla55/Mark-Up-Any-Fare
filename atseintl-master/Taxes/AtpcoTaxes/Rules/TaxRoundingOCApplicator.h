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

#include "DataModel/Common/Types.h"
#include "Rules/TaxRoundingApplicator.h"
#include "Rules/TaxRoundingOCRule.h"

namespace tax
{
class PaymentDetail;
class Request;
class Services;

class TaxRoundingOCApplicator : public TaxRoundingApplicator
{
public:
  TaxRoundingOCApplicator(const TaxRoundingOCRule* taxRoundingRule,
                          const Services& services,
                          const type::Nation& posNation,
                          RawPayments& rawPayments,
                          const Request& request)
    : TaxRoundingApplicator(taxRoundingRule,
                            services,
                            posNation),
       _rawPayments(rawPayments), _request(request)
  {
  }

  bool apply(PaymentDetail& paymentDetail) const;

private:
  void computeTaxesOnOc(OptionalService& paymentDetail,
                        RoundingInfo& roundingInfo) const;
  RoundingInfo computeRoundingUnitAndDir(PaymentDetail& paymentDetail) const;
  bool shouldComputeOcAmount(const OptionalService& optionalService) const;
  bool shouldComputeTaxesOnOc(const OptionalService& optionalService) const;
  void computeOcAmount(OptionalService& optionalService,
                       PaymentDetail& paymentDetail,
                       RoundingInfo& roundingInfo) const;

  RawPayments& _rawPayments;
  const Request& _request;
};

} // namespace tax
