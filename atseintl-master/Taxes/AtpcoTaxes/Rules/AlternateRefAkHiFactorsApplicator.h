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
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{

class PaymentDetail;
class AKHIFactorService;
class LocService;
class AlternateRefAkHiFactorsRule;

class AlternateRefAkHiFactorsApplicator : public BusinessRuleApplicator
{
public:
  AlternateRefAkHiFactorsApplicator(const AlternateRefAkHiFactorsRule& parent,
                                    const AKHIFactorService& akhiFactorService,
                                    const LocService& locService);
  ~AlternateRefAkHiFactorsApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  type::Percent getAlaskaFactor(const type::AirportCode& loc, const type::AirportCode& zoneLoc) const;

  const AKHIFactorService& _AkHiFactorCache;
  const LocService& _locService;
};

} // namespace tax
