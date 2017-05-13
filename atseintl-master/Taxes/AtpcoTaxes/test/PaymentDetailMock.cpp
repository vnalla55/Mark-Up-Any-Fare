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

#include "test/PaymentDetailMock.h"
#include "DomainDataObjects/Geo.h"
#include "Common/TaxName.h"

namespace tax
{
PaymentDetailMock::PaymentDetailMock(type::TaxAppliesToTagInd taxAppliesToTag /* = type::TaxAppliesToTagInd::Blank*/)
  : PaymentDetail(
        PaymentRuleData(
            0, type::TicketedPointTag::MatchTicketedPointsOnly, TaxableUnitTagSet::none(), 0,
            type::CurrencyCode(UninitializedCode),
            taxAppliesToTag),
        _taxPointBegin,
        _taxPointLoc2,
        TaxName())
{
  _isLoc1Stopover = false;
  _isLoc1FareBreak = false;
  _myLoc1 = &_taxPointBegin;
  _mustBeTicketed = false;
  _newSeqNo = 0;
}

PaymentDetailMock::~PaymentDetailMock()
{
}

void
PaymentDetailMock::setLoc1Stopover(bool result)
{
  _isLoc1Stopover = result;
}

void
PaymentDetailMock::setLoc1FareBreak(bool result)
{
  _isLoc1FareBreak = result;
}

void
PaymentDetailMock::setTaxPointBegin(const Geo& result)
{
  setTaxPointLoc1(&result);
}

void
PaymentDetailMock::setMustBeTicketed(bool result)
{
  _mustBeTicketed = result;
}

void
PaymentDetailMock::setTaxName(const TaxName& result)
{
  _taxName = result;
}

bool
PaymentDetailMock::isLoc1Stopover() const
{
  return _isLoc1Stopover;
}

bool
PaymentDetailMock::isLoc1FareBreak() const
{
  return _isLoc1FareBreak;
}

bool
PaymentDetailMock::mustBeTicketed() const
{
  return _mustBeTicketed;
}

const TaxName&
PaymentDetailMock::taxName() const
{
  return _taxName;
}

void
PaymentDetailMock::setTaxableUnit(const type::TaxableUnit& taxableUnit)
{
  _taxableUnits.setTag(taxableUnit);
}

} // namespace tax
