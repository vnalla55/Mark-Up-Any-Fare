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

#include "DataModel/Common/SafeEnums.h"
#include "DomainDataObjects/Geo.h"
#include "Rules/BusinessRule.h"
#include "Rules/PaymentRuleData.h"
#include "Rules/PaymentDetailBase.h"
#include "Util/BranchPrediction.h"

namespace tax
{
const Geo&
PaymentDetailBase::getTaxPointLoc1() const
{
  if (LIKELY(_taxPointLoc1 != nullptr))
    return *_taxPointLoc1;
  else
    throw std::runtime_error("PaymentDetailBase::getTaxPointLoc1 - _taxPointLoc1 is null");
}

const Geo&
PaymentDetailBase::getTaxPointLoc2() const
{
  if (_taxableData._taxPointLoc2 != nullptr)
    return *_taxableData._taxPointLoc2;
  else
    throw std::runtime_error("PaymentDetailBase::getTaxPointLoc2 - _taxPointLoc2 is null");
}

const Geo&
PaymentDetailBase::getLoc3() const
{
  if (_taxableData._taxPointLoc3 != nullptr)
    return *_taxableData._taxPointLoc3;
  else
    throw std::runtime_error("PaymentDetailBase::getLoc3 - loc3 is null");
}

const Geo&
PaymentDetailBase::getJourneyLoc1() const
{
  if (_journeyLoc1 != nullptr)
    return *_journeyLoc1;
  else
    throw std::runtime_error("PaymentDetailBase::getJourneyLoc1 - journeyLoc1 is null");
}

const Geo&
PaymentDetailBase::getJourneyLoc2() const
{
  if (_journeyLoc2 != nullptr)
    return *_journeyLoc2;
  else
    throw std::runtime_error("PaymentDetailBase::getJourneyLoc2 - journeyLoc2 is null");
}

PaymentDetailBase::PaymentDetailBase(const PaymentRuleData& paymentRuleData,
                                     const Geo& taxPointBegin,
                                     const Geo& taxPointEnd,
                                     const TaxName& taxName,
                                     const type::CarrierCode& marketingCarrier)
  : _seqNo(paymentRuleData._seqNo),
    _taxName(taxName),
    _ticketedPoint(paymentRuleData._ticketedPointTag),
    _unticketedTransfer(taxPointBegin.unticketedTransfer()),
    _taxApplicationLimit(type::TaxApplicationLimit::Unlimited),
    _sabreTaxCode(),
    _marketingCarrier(marketingCarrier),
    _spn(0),
    _calcDetails(),
    _taxPointLoc1(&taxPointBegin),
    _journeyLoc1(nullptr),
    _journeyLoc2(nullptr),
    _specUS_RTOJLogic(false),
    _taxPointsProperties(nullptr),
    _myTaxPointsProperties(nullptr),
    _applicatorFailMessage(),
    _taxLabel(),
    _taxAppliesToTagInd(paymentRuleData._taxAppliesToTagInd),
    _publishedAmount(paymentRuleData._publishedAmount),
    _taxEquivalentAmount(),
    _taxEquivalentWithMarkupAmount(),
    _taxEquivalentCurrency(),
    _taxableUnits(paymentRuleData._taxableUnits),
    _loc1StopoverTag(type::StopoverTag::Blank),
    _loc2StopoverTag(type::Loc2StopoverTag::Blank),
    _isOmitted(false),
    _roundTripOrOpenJaw(false)
{
  _taxableData._taxPointLoc2 = &taxPointEnd;
  _taxableData._taxPointEnd = &taxPointEnd;
}

PaymentDetailBase::~PaymentDetailBase()
{
}

bool
PaymentDetailBase::isOpen(type::Index index) const
{
  return taxPointsProperties()[index].isOpen;
}

bool
PaymentDetailBase::isStopover(type::Index index) const
{
  TaxPointProperties const& properties = taxPointsProperties()[index];
  return properties.isFirst || properties.isLast ||
         properties.isSurfaceStopover() ||
         (properties.isTimeStopover == true) ||
         properties.isExtendedStopover ||
         properties.isOpen;
}

bool
PaymentDetailBase::isUSStopover(type::Index index) const
{
  TaxPointProperties const& properties = taxPointsProperties()[index];
  return properties.isFirst || properties.isLast ||
         (properties.isUSTimeStopover == true) ||
         properties.isOpen;
}

bool
PaymentDetailBase::isUSLimitStopover(type::Index index) const
{
  TaxPointProperties const& properties = taxPointsProperties()[index];
  return properties.isFirst || properties.isLast ||
         (properties.isUSLimitStopover == true) ||
         properties.isOpen;
}

bool
PaymentDetailBase::isFareBreak(type::Index index) const
{
  return taxPointsProperties()[index].isFareBreak;
}

bool
PaymentDetailBase::isLoc1Stopover() const
{
  return isStopover(getTaxPointLoc1().id());
}

bool
PaymentDetailBase::isLoc1FareBreak() const
{
  return isFareBreak(getTaxPointLoc1().id());
}

bool
PaymentDetailBase::bothBeginAndEndAreFareBreaks() const
{
  return isFareBreak(getTaxPointBegin().id()) && isFareBreak(getTaxPointEnd().id());
}

bool
PaymentDetailBase::isRangeBetweenBeginAndEnd(type::Index rangeStart, type::Index rangeEnd) const
{
  if (rangeStart >= rangeEnd)
  {
    std::stringstream error;
    error << "PaymentDetailBase::isRangeBetweenBeginAndEnd - rangeStart(" << rangeStart
          << ") >= rangeEnd(" << rangeEnd << ")";
    throw std::logic_error(error.str());
  }

  return ((getTaxPointBegin().loc().tag() == type::TaxPointTag::Departure &&
           rangeStart >= getTaxPointBegin().id() && rangeEnd <= getTaxPointEnd().id()) ||
          (getTaxPointBegin().loc().tag() == type::TaxPointTag::Arrival &&
           rangeStart >= getTaxPointEnd().id() && rangeEnd <= getTaxPointBegin().id()));
}

bool
PaymentDetailBase::isTaxAndEquivalentCurrencyEqual() const
{
  return taxCurrency() == taxEquivalentCurrency();
}

bool
PaymentDetailBase::isFlatTax() const
{
  return taxName().percentFlatTag() == type::PercentFlatTag::Flat;
}

}

