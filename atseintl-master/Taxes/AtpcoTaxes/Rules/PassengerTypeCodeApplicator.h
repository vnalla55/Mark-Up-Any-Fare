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

#include "DomainDataObjects/Itin.h"
#include "DataModel/Services/PassengerTypeCode.h"
#include "Rules/BusinessRuleApplicator.h"

#include <vector>
#include <memory>

namespace tax
{
class Itin;
class LocService;
class PassengerMapper;
class PassengerTypeCodeRule;
class Passenger;
class PaymentDetail;

class PassengerTypeCodeApplicator : public BusinessRuleApplicator
{
public:
  PassengerTypeCodeApplicator(const PassengerTypeCodeRule& rule,
                              const Itin& itin,
                              type::Date referenceDate,
                              const PassengerMapper& paxMapper,
                              std::shared_ptr<const PassengerTypeCodeItems> passengerTypeCodeItems,
                              const LocService& locService,
                              const type::AirportCode& pointOfSale);

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const Itin& _itin;
  const type::Date _referenceDate;
  const type::Date& _birthDate;
  const PassengerMapper& _paxMapper;
  std::shared_ptr<const PassengerTypeCodeItems> _passengerTypeCodeItems;
  std::vector<const PassengerTypeCodeItem*> _prescannedPassengerTypeCodeItems;
  const PassengerTypeCodeRule& _parentRule;
  const LocService& _locService;
  const type::AirportCode _pointOfSale;

  static bool ageMatches(const type::Date& ourDate,
                         const type::Date& birthDate,
                         int16_t minimumAge,
                         int16_t maxAge);

  bool locationMatches(const Passenger& passenger, const PassengerTypeCodeItem& item) const;

  bool passengerMatches(const type::PassengerCode& inputPtc,
                        const type::PassengerCode& outputPtc,
                        const type::CarrierCode& validatingCarrier) const;

  type::Nation getNation(const Passenger& passenger, const type::PassengerStatusTag& tag);

  friend class PassengerTypeCodeApplicatorTest;
};

} // namespace tax

