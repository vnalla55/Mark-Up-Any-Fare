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

#include "DomainDataObjects/YqYr.h"
#include "Rules/PaymentDetailBase.h"
#include "Rules/TaxSubjectData.h"
#include "Rules/TaxableSubject.h"

#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{
struct TaxableYqYrs : public TaxableSubjectMulti<TaxableYqYr>
{
  std::vector<std::pair<type::Index, type::Index> > _ranges;
};
typedef TaxableSubjectSingle<TaxSubjectData> TaxableItinerary;

class ItineraryPaymentDetail : public PaymentDetailBase
{
  typedef std::vector<TicketingFee> TicketingFeeVector;
  typedef TaxableSubjectSingle<TaxSubjectData> TaxableItinerary;

public:
  ItineraryPaymentDetail(const PaymentRuleData& paymentRuleData,
                         const Geo& taxPointBegin,
                         const Geo& taxPointEnd,
                         const TaxName& taxName,
                         const type::CarrierCode& marketingCarrier = BLANK_CARRIER);

  void setTotalFareAmount(const type::MoneyAmount amount)
  {
    _onItinerary._subject.setTotalFareAmount(amount);
  }
  boost::optional<type::MoneyAmount> const& getTotalFareAmount() const
  {
    return _onItinerary._subject.getTotalFareAmount();
  }

  void setTotalFareWithMarkupAmount(const type::MoneyAmount amount)
  {
    _onItinerary._subject.setTotalFareWithMarkupAmount(amount);
  }
  boost::optional<type::MoneyAmount> const& getTotalFareWithMarkupAmount() const
  {
    return _onItinerary._subject.getTotalFareWithMarkupAmount();
  }

  void setFailedRule(BusinessRule const* failedRule) { _onItinerary.setFailedRule(failedRule); }
  bool isFailedRule() const { return _onItinerary.isFailedRule(); }

  std::string getFailureReason(Services& services) const;

  type::MoneyAmount& totalYqYrAmount() { return _onYqYr._taxableAmount; }
  type::MoneyAmount const& totalYqYrAmount() const { return _onYqYr._taxableAmount; }

  type::MoneyAmount& totalTaxOnTaxAmount() { return _onItinerary._subject.totalTaxOnTaxAmount(); }
  type::MoneyAmount const& totalTaxOnTaxAmount() const
  {
    return _onItinerary._subject.totalTaxOnTaxAmount();
  }

  type::MoneyAmount& totalTaxOnTaxWithMarkupAmount() { return _onItinerary._subject.totalTaxOnTaxWithMarkupAmount(); }
  type::MoneyAmount const& totalTaxOnTaxWithMarkupAmount() const
  {
    return _onItinerary._subject.totalTaxOnTaxWithMarkupAmount();
  }

  type::MoneyAmount& totalTaxableAmount() { return _onItinerary._subject.totalTaxableAmount(); }
  type::MoneyAmount const& totalTaxableAmount() const
  {
    return _onItinerary._subject.totalTaxableAmount();
  }

  type::MoneyAmount& totalTaxableWithMarkupAmount()
  {
    return _onItinerary._subject.totalTaxableWithMarkupAmount();
  }
  type::MoneyAmount const& totalTaxableWithMarkupAmount() const
  {
    return _onItinerary._subject.totalTaxableWithMarkupAmount();
  }

  type::MoneyAmount& changeFeeAmount() { return _onItinerary._subject.changeFeeAmount(); }
  const type::MoneyAmount& changeFeeAmount() const
  {
    return _onItinerary._subject.changeFeeAmount();
  }

  type::MoneyAmount& taxOnChangeFeeAmount() { return _onItinerary._subject.taxOnChangeFeeAmount(); }
  const type::MoneyAmount& taxOnChangeFeeAmount() const
  {
    return _onItinerary._subject.taxOnChangeFeeAmount();
  }

  std::vector<PaymentDetail*>& taxOnTaxItems()
  {
    return _onItinerary._subject.taxOnTaxItems();
  }
  const std::vector<PaymentDetail*>& taxOnTaxItems() const
  {
    return _onItinerary._subject.taxOnTaxItems();
  }

  boost::ptr_vector<OptionalService>& optionalServiceItems()
  {
    return _onItinerary._subject.optionalServiceItems();
  }

  const boost::ptr_vector<OptionalService>& optionalServiceItems() const
  {
    return _onItinerary._subject.optionalServiceItems();
  }

  TicketingFeeVector& ticketingFees() { return _onItinerary._subject.ticketingFees(); }
  const TicketingFeeVector& ticketingFees() const { return _onItinerary._subject.ticketingFees(); }

  bool isFailed() const;
  bool areAllOptionalServicesFailed() const;

  bool isValidForGroup(const type::ProcessingGroup& processingGroup) const;
  bool isValidForOptionalService(const type::ProcessingGroup& processingGroup) const;
  bool isValidForTicketingFee() const;
  bool isValidForItinerary() const;

  const TaxableItinerary& getItineraryDetail() const { return _onItinerary; }
  TaxableItinerary& getMutableItineraryDetail() { return _onItinerary; }

  const TaxableYqYrs& getYqYrDetails() const { return _onYqYr; }
  TaxableYqYrs& getMutableYqYrDetails() { return _onYqYr; }

  void failItinerary(const BusinessRule& rule);
  void failOptionalServices(const BusinessRule& rule);
  void failYqYrs(const BusinessRule& rule);
  void failAll(const BusinessRule& rule);

private:
  // as of now _onItinerary contains info about many subjects; extracting them is in progress
  TaxableItinerary _onItinerary;
  TaxableYqYrs _onYqYr;
};
}

