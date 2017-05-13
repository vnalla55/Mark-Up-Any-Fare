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
#include <boost/noncopyable.hpp>
#include "Rules/PaymentDetail.h"

namespace tax
{
class MakePaymentDetail
{
  PaymentDetail _detail;

public:
  static MakePaymentDetail flat() { return MakePaymentDetail(type::PercentFlatTag::Flat); }
  static MakePaymentDetail flat(type::TaxCode code, type::TaxType type, type::Nation nation)
  {
    return MakePaymentDetail(code, type, nation, type::PercentFlatTag::Percent);
  }

  static MakePaymentDetail percent() { return MakePaymentDetail(type::PercentFlatTag::Percent); }

  explicit MakePaymentDetail(type::PercentFlatTag percentFlatTag = type::PercentFlatTag::Flat);
  MakePaymentDetail(type::TaxCode code,
                    type::TaxType type,
                    type::Nation nation,
                    type::PercentFlatTag percentFlatTag = type::PercentFlatTag::Flat);

  MakePaymentDetail& amount(int64_t nom, int64_t den);
  MakePaymentDetail& currency(type::CurrencyCode cur);
  MakePaymentDetail& taxCurrency(type::CurrencyCode cur);
  MakePaymentDetail& taxEquivalentCurrency(type::CurrencyCode cur);
  MakePaymentDetail& rounding(const type::MoneyAmount& unit, type::TaxRoundingDir dir);
  MakePaymentDetail& withOptionalService();
  MakePaymentDetail& taxAmt(const type::MoneyAmount& taxAmt);
  MakePaymentDetail& changeFeeAmount(const type::MoneyAmount& changeFeeAmount);
  MakePaymentDetail& addTicketingFee(const type::Index& index,
                                     const type::MoneyAmount& amount,
                                     const type::MoneyAmount& taxAmount,
                                     const type::TktFeeSubCode& subCode);
  operator PaymentDetail const&() const { return _detail; }
};

class MakeOptionalService : boost::noncopyable
{
  OptionalService _privateOptionalService;
  OptionalService& _optionalService;

public:
  explicit MakeOptionalService(OptionalService& optService) : _optionalService(optService) {}
  MakeOptionalService() : _optionalService(_privateOptionalService) {};
  MakeOptionalService& amount(int64_t nom, int64_t den);
  MakeOptionalService& taxAmount(int64_t nom, int64_t den);
  MakeOptionalService& failedRule(BusinessRule& rule);
  MakeOptionalService& subCode(type::OcSubCode subCode);
  MakeOptionalService& flightRelated();
  MakeOptionalService& baggageCharge();
  MakeOptionalService& group(type::ServiceGroupCode groupCode, type::ServiceGroupCode subGroupCode);
  MakeOptionalService& owner(type::CarrierCode carrier);
  operator OptionalService const&() const { return _optionalService; }
};

class AddOptionalService : public MakeOptionalService
{
public:
  explicit AddOptionalService(PaymentDetail& detail);
};
}
