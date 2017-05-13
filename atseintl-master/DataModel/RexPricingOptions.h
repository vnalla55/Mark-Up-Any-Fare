//-------------------------------------------------------------------
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/SpanishLargeFamilyEnum.h"
#include "DataModel/PricingOptions.h"

namespace tse
{
class RexBaseTrx;

class RexPricingOptions : public PricingOptions
{
public:
  void setTrx(RexBaseTrx* trx) { _trx = trx; }

  virtual CurrencyCode& currencyOverride();
  virtual const CurrencyCode& currencyOverride() const;

  virtual NationCode& nationality() override;
  virtual const NationCode& nationality() const override;

  virtual NationCode& excNationality() { return _excNationality; }
  virtual const NationCode& excNationality() const { return _excNationality; }

  virtual LocCode& residency() override;
  virtual const LocCode& residency() const override;

  virtual LocCode& excResidency() { return _excResidency; }
  virtual const LocCode& excResidency() const { return _excResidency; }

  virtual NationCode& employment() override;
  virtual const NationCode& employment() const override;

  virtual NationCode& excEmployment() { return _excEmployment; }
  virtual const NationCode& excEmployment() const { return _excEmployment; }

  void setSpanishLargeFamilyDiscountLevel(const SLFUtil::DiscountLevel level) override;
  SLFUtil::DiscountLevel getSpanishLargeFamilyDiscountLevel() const override;

  CurrencyCode& excBaseFareCurrency() { return _excBaseFareCurrency; }
  const CurrencyCode& excBaseFareCurrency() const { return _excBaseFareCurrency; }

  std::string& excTotalFareAmt() { return _excTotalFareAmt; }
  const std::string& excTotalFareAmt() const { return _excTotalFareAmt; }

  virtual bool isRtw() const override;
  void setExcTaggedAsRtw(bool value) { _excRtw = value; }
  bool isExcTaggedAsRtw() const { return _excRtw; }
  void setNetSellingIndicator(bool value) { _netSellingIndicator = value; }
  bool isNetSellingIndicator() const { return _netSellingIndicator; }

  virtual char& fareFamilyType() override;
  virtual const char& fareFamilyType() const override;

private:
  RexBaseTrx* _trx = nullptr;
  CurrencyCode _excCurrencyOverride;
  NationCode _excNationality;
  LocCode _excResidency;
  NationCode _excEmployment;
  CurrencyCode _excBaseFareCurrency;
  std::string _excTotalFareAmt;
  SLFUtil::DiscountLevel _excSpanishLargeFamilyDiscountLevel
      = SLFUtil::DiscountLevel::NO_DISCOUNT;
  bool _excRtw = false;
  bool _netSellingIndicator = false;
  char _excFareFamilyType = ' ';
};
} // tse namespace

