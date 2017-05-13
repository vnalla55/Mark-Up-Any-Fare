// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class DateTime;
class DiagCollector;
class Loc;
class PricingTrx;
class TaxReissue;

class ReissueTaxInfoBuilder
{
public:
  friend class ReissueTaxInfoBuilderTest;

  struct ReissueTaxInfo
  {
    void setDefaultValues();
    void setMatchedValues(const TaxReissue& taxReissue);

    bool reissueRestrictionApply;
    bool taxApplyToReissue;
    bool reissueTaxRefundable;
    CurrencyCode reissueTaxCurrency;
    MoneyAmount reissueTaxAmount;
  };

  ReissueTaxInfoBuilder();
  ~ReissueTaxInfoBuilder();

  const ReissueTaxInfo& build(PricingTrx& trx, const TaxCode& taxCode, const TaxType& taxType = "000");
  const ReissueTaxInfo& reissueTaxInfo() const { return _reissueTaxInfo; }

private:
  const tse::Loc* getReissueLocation(PricingTrx& trx);
  bool matchReissueLocation(const TaxReissue& taxReissue,
                            const tse::Loc* exchangeReissueLoc,
                            const DateTime& ticketingDate);

  bool matchTicketingCarrier(const TaxReissue& taxReissue, const CarrierCode& validatingCarrier);

  void diag806Header(PricingTrx& trx,
                     const std::vector<TaxReissue*>& taxReissueVec,
                     const TaxCode& taxCode,
                     const tse::Loc* exchangeReissueLoc);
  void diag806TaxSeq(const TaxReissue& taxReissue);
  void diag806MatchingSeq(const TaxReissue* taxReissue);

  ReissueTaxInfo _reissueTaxInfo;
  DiagCollector* _diag;
};

} // namespace tse
