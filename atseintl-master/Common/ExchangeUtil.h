//-------------------------------------------------------------------
//
//  File:        ExchangeUtil.h
//  Created:     September 16, 2007
//  Authors:     Simon Li
//
//  Updates:
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/TseEnums.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"


#include <vector>

namespace tse
{
class TravelSeg;
class PaxTypeFare;
class ExcItin;
class FarePath;

namespace ExchangeUtil
{

struct SetPaxTypeFareRetrievalInfo : std::unary_function<PaxTypeFare*, void>
{
  SetPaxTypeFareRetrievalInfo(FareMarket::RetrievalInfo* info) : _info(info) {}

  void operator()(PaxTypeFare* paxTypeFare)
  {
    if (!paxTypeFare->retrievalInfo())
      paxTypeFare->retrievalInfo() = _info;
  }
  FareMarket::RetrievalInfo* _info;
};

struct SetFaresRetrievalInfo : std::unary_function<FareMarket*, void>
{
  SetFaresRetrievalInfo(FareMarket::RetrievalInfo* info) : _info(info) {}
  void operator()(FareMarket* fareMarket)
  {
    std::for_each(fareMarket->allPaxTypeFare().begin(),
                  fareMarket->allPaxTypeFare().end(),
                  SetPaxTypeFareRetrievalInfo(_info));
  }
  FareMarket::RetrievalInfo* _info;
};

struct UpdateRetrievalInfo
{
  UpdateRetrievalInfo(const PricingTrx& trx,
                      const DateTime& retrievalDate,
                      const FareMarket::FareRetrievalFlags& flags)
    : _trx(trx), _retrievalDate(retrievalDate), _flags(flags)
  {
  }

  void operator()(PaxTypeFare* ptf) const
  {
    ptf->retrievalInfo() = FareMarket::RetrievalInfo::construct(
        _trx,
        _retrievalDate,
        ((FareMarket::FareRetrievalFlags)(ptf->retrievalInfo()->_flag | _flags)));
    ptf->fareMarket()->retrievalInfo() = ptf->retrievalInfo();
  }

private:
  const PricingTrx& _trx;
  const DateTime& _retrievalDate;
  const FareMarket::FareRetrievalFlags& _flags;
};

FCChangeStatus
getChangeStatus(const std::vector<TravelSeg*>& tvlSegs, const int16_t& pointOfChgSegmentOrder);

FCChangeStatus
getChangeStatus(const std::vector<TravelSeg*>& tvlSegs,
                const int16_t& pointOfChgFirst,
                const int16_t& pointOfChgSecond);

void
avoidValidationOfCategoriesInMinFares(RexBaseTrx& trx,
                                      std::vector<PaxTypeFare*>& faresForMinFares);

Money
convertCurrency(const PricingTrx& trx,
                const Money& source,
                const CurrencyCode& targetCurr,
                bool rounding);

void
setRetrievalInfo(PricingTrx& trx, FareMarket::RetrievalInfo* info);


bool
validateFixedLegsInCEXS(const PricingTrx& trx, const std::vector<ShoppingTrx::Leg>& legs);

class RaiiProcessingDate final
{
public:
  RaiiProcessingDate(RexBaseTrx& trx, const FarePath& fp, bool isFallback);
  RaiiProcessingDate(RexBaseTrx& trx, const DateTime& date, bool isFallback);
  RaiiProcessingDate(PricingTrx& trx, bool isFallback);
  ~RaiiProcessingDate();

  void useOriginalTktIssueDT();

protected:
  bool skipSettingDate(const PricingTrx::ExcTrxType& excType);
  void setSecondROEConversionDate(const FarePath& fp);

  const DateTime _savedDate;
  const bool _prevROEvalue;
  const bool _isFallback = false;
  const bool _isRexBaseTrx;
  bool _restoreOriginalDate = false;

  RexBaseTrx* _trxOld; //to remove with fallback exchangeRefactorRaiiDate
  PricingTrx& _trx;
};
} // ExchangeUtil

class NonRefundableUtil
{
  // this class is specyfically prepared to bind all nonrefundable functionality
  // - in FarePath.*, RepriceSolutionValidator.*, RexPricingTrx.*

public:
  explicit NonRefundableUtil(RexPricingTrx& trx) : _trx(trx) {}

  MoneyAmount excTotalNucAmt() const;
  MoneyAmount excTotalBaseCurrAmt() const;
  MoneyAmount excNonRefundableNucAmt() const;
  MoneyAmount excNonRefundableBaseCurrAmt() const;

  void calculateNonRefundableAmount(FarePath& newFarePath);

private:
  MoneyAmount excXmlNucAmt() const;
  MoneyAmount excNucConvert(const MoneyAmount& srcAmt, const CurrencyCode& srcCurr) const;
  RexPricingTrx& _trx;
};

} //tse

