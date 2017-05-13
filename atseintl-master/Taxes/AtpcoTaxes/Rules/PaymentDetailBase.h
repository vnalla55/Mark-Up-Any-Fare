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

#include "Common/CalcDetails.h"
#include "Common/ConnectionTagSet.h"
#include "Common/Consts.h"
#include "Common/TaxableUnitTagSet.h"
#include "Common/TaxName.h"
#include "DataModel/Common/TaxPointProperties.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/OptionalService.h"
#include "DomainDataObjects/TicketingFee.h"
#include "Rules/TaxSubjectData.h"
#include "Rules/TaxableData.h"

#include <boost/optional.hpp>
#include <vector>

namespace tax
{
struct PaymentRuleData;

struct ExchangeDetails
{
  ExchangeDetails() : isMixedTax(false), isPartialTax(false) {}
  bool isMixedTax;
  bool isPartialTax;
  boost::optional<type::MoneyAmount> minTaxAmount;
  boost::optional<type::MoneyAmount> maxTaxAmount;
  boost::optional<type::CurrencyCode> minMaxTaxCurrency;
  boost::optional<type::CurDecimals> minMaxTaxCurrencyDecimals;
};

class PaymentDetailBase
{
public:
  void setTaxPointLoc1(const Geo* loc1) { _taxPointLoc1 = loc1; }
  const Geo& getTaxPointLoc1() const;

  void setTaxPointLoc2(const Geo* loc2) { _taxableData._taxPointLoc2 = loc2; }
  const Geo& getTaxPointLoc2() const;

  void setLoc3(const Geo* loc3) { _taxableData._taxPointLoc3 = loc3; }
  const Geo& getLoc3() const;
  bool hasLoc3() const { return _taxableData._taxPointLoc3 != nullptr; }

  const Geo& getJourneyLoc1() const;
  void setJourneyLoc1(const Geo* geo) { _journeyLoc1 = geo; }
  bool hasJourneyLoc1() const { return _journeyLoc1 != nullptr; }

  const Geo& getJourneyLoc2() const;
  void setJourneyLoc2(const Geo* geo) { _journeyLoc2 = geo; }
  bool hasJourneyLoc2() const { return _journeyLoc2 != nullptr; }

  bool& specUS_RTOJLogic() { return _specUS_RTOJLogic; }
  const bool& specUS_RTOJLogic() const { return _specUS_RTOJLogic; }

  PaymentDetailBase(const PaymentRuleData& paymentRuleData,
                    const Geo& taxPointBegin,
                    const Geo& taxPointEnd,
                    const TaxName& taxName,
                    const type::CarrierCode& marketingCarrier = BLANK_CARRIER);
  virtual ~PaymentDetailBase();

  type::SeqNo const& seqNo() const { return _seqNo; }

  virtual const TaxName& taxName() const { return _taxName; }

  const Geo& getTaxPointBegin() const { return getTaxPointLoc1(); }

  void setTaxPointEnd(const Geo& loc) { _taxableData._taxPointEnd = &loc; }
  const Geo& getTaxPointEnd() const { return *_taxableData._taxPointEnd; }

  type::UnticketedTransfer const& unticketedTransfer() const { return _unticketedTransfer; }

  type::TaxLabel& taxLabel() { return _taxLabel; }
  type::TaxLabel const& taxLabel() const { return _taxLabel; }

  type::TaxAppliesToTagInd const& taxAppliesToTagInd() const { return _taxAppliesToTagInd; }

  const type::Money& getPublishedAmount() const { return _publishedAmount; }
  // TODO remove those methods!
  type::MoneyAmount& taxAmt() { return _publishedAmount._amount; }
  type::MoneyAmount const& taxAmt() const { return _publishedAmount._amount; }

  type::CurrencyCode& taxCurrency() { return _publishedAmount._currency; }
  type::CurrencyCode const& taxCurrency() const { return _publishedAmount._currency; }
  //-----------

  type::TaxApplicationLimit& taxApplicationLimit() { return _taxApplicationLimit; }
  type::TaxApplicationLimit const& taxApplicationLimit() const { return _taxApplicationLimit; }

  type::TicketedPointTag const& ticketedPointTag() const { return _ticketedPoint; }

  virtual bool mustBeTicketed() const
  {
    return _ticketedPoint == type::TicketedPointTag::MatchTicketedPointsOnly;
  }

  TaxPointsProperties& getMutableTaxPointsProperties()
  {
    if (!_myTaxPointsProperties)
    {
      if (BOOST_LIKELY(_taxPointsProperties != nullptr))
        _myTaxPointsProperties.reset(new TaxPointsProperties(*_taxPointsProperties));
      else
        _myTaxPointsProperties.reset(new TaxPointsProperties());
    }
    return *_myTaxPointsProperties;
  }
  void setTaxPointsProperties(const std::shared_ptr<const TaxPointsProperties>& tpp)
  {
    _taxPointsProperties = tpp;
  }
  const TaxPointsProperties& taxPointsProperties() const
  {
    return (_myTaxPointsProperties) ? *_myTaxPointsProperties : *_taxPointsProperties;
  }

  type::MoneyAmount& taxEquivalentAmount() { return _taxEquivalentAmount; }
  const type::MoneyAmount& taxEquivalentAmount() const { return _taxEquivalentAmount; }

  type::MoneyAmount& taxEquivalentWithMarkupAmount() { return _taxEquivalentWithMarkupAmount; }
  const type::MoneyAmount& taxEquivalentWithMarkupAmount() const { return _taxEquivalentWithMarkupAmount; }

  type::CurrencyCode& taxEquivalentCurrency() { return _taxEquivalentCurrency; }
  const type::CurrencyCode& taxEquivalentCurrency() const { return _taxEquivalentCurrency; }

  std::string& applicatorFailMessage() { return _applicatorFailMessage; }
  const std::string& applicatorFailMessage() const { return _applicatorFailMessage; }

  bool& roundTripOrOpenJaw() { return _roundTripOrOpenJaw; }
  const bool& roundTripOrOpenJaw() const { return _roundTripOrOpenJaw; }

  const TaxableUnitTagSet& taxableUnits() const { return _taxableUnits; }

  virtual bool isLoc1Stopover() const;
  virtual bool isLoc1FareBreak() const;

  bool isOpen(type::Index index) const;
  bool isStopover(type::Index index) const;
  bool isUSStopover(type::Index index) const;
  bool isUSLimitStopover(type::Index index) const;
  bool isFareBreak(type::Index index) const;
  bool bothBeginAndEndAreFareBreaks() const;
  bool isRangeBetweenBeginAndEnd(type::Index rangeStart, type::Index rangeEnd) const;

  CalcDetails& calcDetails() { return _calcDetails; }
  const CalcDetails& calcDetails() const { return _calcDetails; }

  ExchangeDetails& exchangeDetails() { return _exchangeDetails; }
  const ExchangeDetails& exchangeDetails() const { return _exchangeDetails; }

  type::SabreTaxCode& sabreTaxCode() { return _sabreTaxCode; }
  const type::SabreTaxCode& sabreTaxCode() const { return _sabreTaxCode; }

  type::CarrierCode& marketingCarrier() { return _marketingCarrier; }
  type::CarrierCode const& marketingCarrier() const { return _marketingCarrier; }

  type::SeqNo const& spn() const { return _spn; }

  bool isTaxAndEquivalentCurrencyEqual() const;
  bool isFlatTax() const;

  type::StopoverTag const& loc1StopoverTag() const { return _loc1StopoverTag; }
  type::StopoverTag& loc1StopoverTag() { return _loc1StopoverTag; }

  type::Loc2StopoverTag const& loc2StopoverTag() const { return _loc2StopoverTag; }
  type::Loc2StopoverTag& loc2StopoverTag() { return _loc2StopoverTag; }

protected:
  TaxableData _taxableData;
  ExchangeDetails _exchangeDetails;

  const type::SeqNo _seqNo;
  const TaxName _taxName;
  const type::TicketedPointTag _ticketedPoint;
  const type::UnticketedTransfer _unticketedTransfer;
  type::TaxApplicationLimit _taxApplicationLimit;
  type::SabreTaxCode _sabreTaxCode; // guaranteed to be 3 or less characters
  type::CarrierCode _marketingCarrier;
  type::SeqNo _spn;
  CalcDetails _calcDetails;

  const Geo* _taxPointLoc1;
  const Geo* _journeyLoc1;
  const Geo* _journeyLoc2;
  bool _specUS_RTOJLogic;

  std::shared_ptr<const TaxPointsProperties> _taxPointsProperties;
  std::shared_ptr<TaxPointsProperties> _myTaxPointsProperties;
  std::string _applicatorFailMessage;

  type::TaxLabel _taxLabel;
  type::TaxAppliesToTagInd _taxAppliesToTagInd;
  type::Money _publishedAmount;

  type::MoneyAmount _taxEquivalentAmount;
  type::MoneyAmount _taxEquivalentWithMarkupAmount;
  type::CurrencyCode _taxEquivalentCurrency;
  TaxableUnitTagSet _taxableUnits;
  type::StopoverTag _loc1StopoverTag;
  type::Loc2StopoverTag _loc2StopoverTag;
  bool _isOmitted;
  bool _roundTripOrOpenJaw;
};
}

