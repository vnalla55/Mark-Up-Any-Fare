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
#pragma once

#include "Common/Timestamp.h"
#include "DataModel/Services/PassengerTypeCode.h"
#include "Rules/BusinessRuleApplicator.h"

#include <memory>

namespace tax
{
class Itin;
class LocService;
class Passenger;
class PassengerMapper;
class PassengerTypeCodeRule;
class PaymentDetail;

class PassengerTypeCodeApplicatorFacade : public BusinessRuleApplicator
{
public:
  PassengerTypeCodeApplicatorFacade(
      const PassengerTypeCodeRule& rule,
      const Itin& itin,
      type::Date referenceDate,
      const PassengerMapper& paxMapper,
      const std::shared_ptr<const PassengerTypeCodeItems>& passengerTypeCodeItems,
      const LocService& locService,
      const type::AirportCode& pointOfSale);

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const Itin& _itin;
  const type::Date _referenceDate;
  const PassengerMapper& _paxMapper;
  std::shared_ptr<const PassengerTypeCodeItems> _passengerTypeCodeItems;
  const PassengerTypeCodeRule& _rule;
  const LocService& _locService;
  const type::AirportCode _pointOfSale;
};
}

