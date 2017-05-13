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

class GeoPath;
class GeoPathMapping;
class Itin;
class MileageService;
class PaymentDetail;
class TaxMatchingApplTagRule;

class TaxMatchingApplTagApplicator : public BusinessRuleApplicator
{
public:
  TaxMatchingApplTagApplicator(const TaxMatchingApplTagRule* rule,
                               const Itin& itin,
                               const MileageService& mileageService);

  ~TaxMatchingApplTagApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  bool isDomesticFare(type::Index start, type::Index end) const;

  const TaxMatchingApplTagRule* _rule;
  const Itin& _itin;
  const GeoPath& _geoPath;
  const GeoPathMapping& _geoPathMapping;
  const MileageService& _mileageService;
};

} // namespace tax
