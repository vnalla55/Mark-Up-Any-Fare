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
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/MileageService.h"

namespace tax
{
class ApplicationTag01Rule;
class Itin;
class LocService;
class PaymentDetail;

class ApplicationTag01Applicator : public BusinessRuleApplicator
{
public:
  ApplicationTag01Applicator(const ApplicationTag01Rule& parent,
                           const Itin& itin,
                           const LocService& locService,
                           const MileageService& mileageService,
                           const type::Timestamp& ticketingDate);

  ~ApplicationTag01Applicator() override {}

  bool apply(PaymentDetail& paymentDetail) const;

private:
  bool matchesPreconditions(PaymentDetail& paymentDetail) const;
  bool considerOJ(const Itin& itin, PaymentDetail& paymentDetail) const;
  bool considerOW(const Itin& itin, PaymentDetail& paymentDetail) const;
  bool considerOWDistanceValidation(const Itin& itin, PaymentDetail& paymentDetail) const;
  bool considerOWIataSegments(const Itin& itin) const;

  bool originAndDestinationBothInBufferZone() const;
  bool hasStopoverIn50USStates(PaymentDetail& paymentDetail) const;
  bool sectorBetweenAreas(const Geo& geo1,
                          const Geo& geo2,
                          const LocZone& area1,
                          const LocZone& area2) const;

  const Itin& _itin;
  const LocService& _locService;
  const MileageService& _mileageService;
  const ApplicationTag01Rule& _applicationTagRule;
  const type::Timestamp _ticketingDate;
};

} // namespace tax
