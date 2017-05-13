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

#include "Rules/PassengerTypeCodeApplicatorFacade.h"

#include "DataModel/Services/PassengerTypeCode.h"
#include "DataModel/Common/CodeIO.h"
#include "Rules/DummyApplicator.h"
#include "Rules/PassengerTypeCodeApplicator.h"
#include "Rules/PassengerTypeCodeRule.h"
#include "Rules/PaymentDetail.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/PassengerMapper.h"

namespace tax
{
PassengerTypeCodeApplicatorFacade::PassengerTypeCodeApplicatorFacade(
    const PassengerTypeCodeRule& rule,
    const Itin& itin,
    type::Date referenceDate,
    const PassengerMapper& paxMapper,
    const std::shared_ptr<const PassengerTypeCodeItems>& passengerTypeCodeItems,
    const LocService& locService,
    const type::AirportCode& pointOfSale)
  : BusinessRuleApplicator(&rule),
    _itin(itin),
    _referenceDate(referenceDate),
    _paxMapper(paxMapper),
    _passengerTypeCodeItems(passengerTypeCodeItems),
    _rule(rule),
    _locService(locService),
    _pointOfSale(pointOfSale)
{
}

bool
PassengerTypeCodeApplicatorFacade::apply(PaymentDetail& paymentDetail) const
{
  if (_passengerTypeCodeItems && _passengerTypeCodeItems->empty())
  {
    std::ostringstream buf;
    buf << "VENDOR " << _rule.vendor() << ", ITEM " << _rule.itemNo()
        << " NOT FOUND IN SERVICES!";
    return DummyApplicator(_rule, false, buf.str()).apply(paymentDetail);
  }

  return PassengerTypeCodeApplicator(_rule,
                                     _itin,
                                     _referenceDate,
                                     _paxMapper,
                                     _passengerTypeCodeItems,
                                     _locService,
                                     _pointOfSale).apply(paymentDetail);
}
}
