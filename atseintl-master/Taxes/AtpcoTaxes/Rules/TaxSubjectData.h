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
#include "DomainDataObjects/YqYr.h"
#include "DomainDataObjects/OptionalService.h"
#include "DomainDataObjects/TicketingFee.h"

#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{
class PaymentDetail;

class TaxSubjectData
{
  typedef std::vector<TicketingFee> TicketingFeeVector;
  typedef std::vector<PaymentDetail*> PaymentDetailVec;

public:
  TaxSubjectData()
    : _totalFareAmount(boost::none),
      _totalFareWithMarkupAmount(boost::none),
      _totalTaxOnTaxAmount(),
      _totalTaxOnTaxWithMarkupAmount(),
      _totalTaxableAmount(),
      _totalTaxableWithMarkupAmount(),
      _changeFeeAmount(),
      _taxOnChangeFeeAmount(),
      _optionalServiceItems(),
      _taxOnTaxItems()
  {
  }

  void setTotalFareAmount(const type::MoneyAmount amount) { _totalFareAmount = amount; }
  boost::optional<type::MoneyAmount> const& getTotalFareAmount() const { return _totalFareAmount; }

  void setTotalFareWithMarkupAmount(const type::MoneyAmount amount) { _totalFareWithMarkupAmount = amount; }
  boost::optional<type::MoneyAmount> const& getTotalFareWithMarkupAmount() const { return _totalFareWithMarkupAmount; }

  type::MoneyAmount& totalTaxOnTaxAmount() { return _totalTaxOnTaxAmount; }
  type::MoneyAmount const& totalTaxOnTaxAmount() const { return _totalTaxOnTaxAmount; }

  type::MoneyAmount& totalTaxOnTaxWithMarkupAmount() { return _totalTaxOnTaxWithMarkupAmount; }
  type::MoneyAmount const& totalTaxOnTaxWithMarkupAmount() const { return _totalTaxOnTaxWithMarkupAmount; }

  type::MoneyAmount& totalTaxableAmount() { return _totalTaxableAmount; }
  type::MoneyAmount const& totalTaxableAmount() const { return _totalTaxableAmount; }

  type::MoneyAmount& totalTaxableWithMarkupAmount() { return _totalTaxableWithMarkupAmount; }
  type::MoneyAmount const& totalTaxableWithMarkupAmount() const { return _totalTaxableWithMarkupAmount; }

  type::MoneyAmount& changeFeeAmount() { return _changeFeeAmount; }
  const type::MoneyAmount& changeFeeAmount() const { return _changeFeeAmount; }

  type::MoneyAmount& taxOnChangeFeeAmount() { return _taxOnChangeFeeAmount; }
  const type::MoneyAmount& taxOnChangeFeeAmount() const { return _taxOnChangeFeeAmount; }

  PaymentDetailVec& taxOnTaxItems() { return _taxOnTaxItems; }
  const PaymentDetailVec& taxOnTaxItems() const
  {
    return _taxOnTaxItems;
  }

  boost::ptr_vector<OptionalService>& optionalServiceItems() { return _optionalServiceItems; }

  const boost::ptr_vector<OptionalService>& optionalServiceItems() const
  {
    return _optionalServiceItems;
  }

  TicketingFeeVector& ticketingFees() { return _ticketingFees; }
  const TicketingFeeVector& ticketingFees() const { return _ticketingFees; }

private:
  boost::optional<type::MoneyAmount> _totalFareAmount;
  boost::optional<type::MoneyAmount> _totalFareWithMarkupAmount;

  type::MoneyAmount _totalTaxOnTaxAmount;
  type::MoneyAmount _totalTaxOnTaxWithMarkupAmount;

  type::MoneyAmount _totalTaxableAmount;
  type::MoneyAmount _totalTaxableWithMarkupAmount;

  type::MoneyAmount _changeFeeAmount;
  type::MoneyAmount _taxOnChangeFeeAmount;

  boost::ptr_vector<OptionalService> _optionalServiceItems;
  PaymentDetailVec _taxOnTaxItems;
  TicketingFeeVector _ticketingFees;
};
}

