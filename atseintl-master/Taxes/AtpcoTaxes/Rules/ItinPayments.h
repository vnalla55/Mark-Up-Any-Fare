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

#include "Common/GroupedContainers.h"
#include "Common/PointeeComparator.h"
#include "DataModel/Common/Types.h"
#include "Rules/Payment.h"
#include "Rules/RawPayments.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <map>

namespace tax
{

class Services;

typedef
  std::map<TaxName const*, std::vector<const PaymentDetail*>, PointeeComparator>
  PaymentsMap;

class ItinPayments
{
public:
  typedef GroupedContainers<boost::ptr_vector<Payment> > Payments;
  ItinPayments(type::Index newItinId);
  ItinPayments() : _itinId{0} {}
  ~ItinPayments();

  boost::ptr_vector<Payment> const& payments(const type::ProcessingGroup processingGroup) const
  {
    return _payments.get(processingGroup);
  }

  boost::ptr_vector<Payment>& payments(const type::ProcessingGroup processingGroup)
  {
    return _payments.get(processingGroup);
  }

  type::Index const& itinId() const { return _itinId; }
  void setItinId(type::Index itinId) { _itinId = itinId; }

  type::PassengerCode const& requestedPassengerCode() const { return _requestedPassengerCode; }

  type::PassengerCode& requestedPassengerCode() { return _requestedPassengerCode; }

  const type::CarrierCode& validatingCarrier() const { return _validatingCarrier; }

  type::CarrierCode& validatingCarrier() { return _validatingCarrier; }

  void addValidTaxes(const type::ProcessingGroup, RawPayments const& taxes, Services const& services);

  void addAllTaxes(const type::ProcessingGroup, RawPayments const& taxes, Services const& services);

private:
  void addTaxes(const type::ProcessingGroup, PaymentsMap const& taxes, Services const& services);

  Payments _payments;
  type::Index _itinId;
  type::PassengerCode _requestedPassengerCode;
  type::CarrierCode _validatingCarrier;
};

} // namespace tax

