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
#include "test/MakePaymentDetail.h"
#include "Rules/PaymentRuleData.h"

namespace tax
{
namespace
{
TaxName
makeTaxName(type::TaxCode code,
            type::TaxType type,
            type::Nation nation,
            tax::type::PercentFlatTag percentFlatTag)
{
  TaxName taxName;
  taxName.taxCode() = code;
  taxName.taxType() = type;
  taxName.nation() = nation;
  taxName.percentFlatTag() = percentFlatTag;
  return taxName;
}

OptionalService&
addOptionalService(PaymentDetail& detail)
{
  detail.optionalServiceItems().push_back(new OptionalService);
  return detail.optionalServiceItems().back();
}

} // anonymous namespace

MakePaymentDetail::MakePaymentDetail(type::PercentFlatTag percentFlatTag)
  : _detail(PaymentRuleData(1,
                            type::TicketedPointTag::MatchTicketedAndUnticketedPoints,
                            TaxableUnitTagSet::none(),
                            0,
                            type::CurrencyCode(UninitializedCode),
                            type::TaxAppliesToTagInd::Blank),
            Geo(),
            Geo(),
            makeTaxName(type::TaxCode(UninitializedCode),
                        type::TaxType(UninitializedCode),
                        type::Nation(UninitializedCode),
                        percentFlatTag))
{
}

MakePaymentDetail::MakePaymentDetail(type::TaxCode code,
                                     type::TaxType type,
                                     type::Nation nation,
                                     type::PercentFlatTag percentFlatTag)
  : _detail(PaymentRuleData(1,
                            type::TicketedPointTag::MatchTicketedAndUnticketedPoints,
                            TaxableUnitTagSet::none(),
                            0,
                            type::CurrencyCode(UninitializedCode),
                            type::TaxAppliesToTagInd::Blank),
            Geo(),
            Geo(),
            makeTaxName(code, type, nation, percentFlatTag))
{
}

MakePaymentDetail&
MakePaymentDetail::amount(int64_t nom, int64_t den)
{
  _detail.taxEquivalentAmount() = type::MoneyAmount(nom, den);
  return *this;
}

MakePaymentDetail&
MakePaymentDetail::currency(type::CurrencyCode cur)
{
  _detail.taxEquivalentCurrency() = cur;
  return *this;
}

MakePaymentDetail&
MakePaymentDetail::taxCurrency(type::CurrencyCode cur)
{
  _detail.taxCurrency() = cur;
  return *this;
}

MakePaymentDetail&
MakePaymentDetail::taxEquivalentCurrency(type::CurrencyCode cur)
{
  _detail.taxEquivalentCurrency() = cur;
  return *this;
}

MakePaymentDetail&
MakePaymentDetail::rounding(const type::MoneyAmount& unit, type::TaxRoundingDir dir)
{
  _detail.calcDetails().roundingUnit = unit;
  _detail.calcDetails().roundingDir = dir;
  return *this;
}

MakePaymentDetail&
MakePaymentDetail::withOptionalService()
{
  _detail.optionalServiceItems().push_back(new OptionalService);
  return *this;
}

MakePaymentDetail&
MakePaymentDetail::taxAmt(const type::MoneyAmount& taxAmt)
{
  _detail.taxAmt() = taxAmt;
  return *this;
}

MakePaymentDetail&
MakePaymentDetail::changeFeeAmount(const type::MoneyAmount& changeFeeAmount)
{
  _detail.changeFeeAmount() = changeFeeAmount;
  return *this;
}

MakePaymentDetail&
MakePaymentDetail::addTicketingFee(const type::Index& index,
                                   const type::MoneyAmount& amount,
                                   const type::MoneyAmount& taxAmount,
                                   const type::TktFeeSubCode& subCode)
{
  _detail.ticketingFees().push_back(TicketingFee(index, amount, taxAmount, subCode));
  return *this;
}

MakeOptionalService&
MakeOptionalService::amount(int64_t nom, int64_t den)
{
  _optionalService.amount() = type::MoneyAmount(nom, den);
  return *this;
}

MakeOptionalService&
MakeOptionalService::taxAmount(int64_t nom, int64_t den)
{
  _optionalService.taxAmount() = type::MoneyAmount(nom, den);
  return *this;
}

MakeOptionalService&
MakeOptionalService::failedRule(BusinessRule& rule)
{
  _optionalService.setFailedRule(&rule);
  return *this;
}

MakeOptionalService&
MakeOptionalService::subCode(type::OcSubCode subCode)
{
  _optionalService.subCode() = subCode;
  return *this;
}

MakeOptionalService&
MakeOptionalService::flightRelated()
{
  _optionalService.type() = type::OptionalServiceTag::FlightRelated;
  return *this;
}

MakeOptionalService&
MakeOptionalService::baggageCharge()
{
  _optionalService.type() = type::OptionalServiceTag::BaggageCharge;
  return *this;
}

MakeOptionalService&
MakeOptionalService::group(type::ServiceGroupCode groupCode, type::ServiceGroupCode subGroupCode)
{
  _optionalService.serviceGroup() = groupCode;
  _optionalService.serviceSubGroup() = subGroupCode;
  return *this;
}

MakeOptionalService&
MakeOptionalService::owner(type::CarrierCode carrier)
{
  _optionalService.ownerCarrier() = carrier;
  return *this;
}

AddOptionalService::AddOptionalService(PaymentDetail& detail)
  : MakeOptionalService(addOptionalService(detail))
{
}

} // namespace tax
