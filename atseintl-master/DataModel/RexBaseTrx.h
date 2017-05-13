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

#include "Common/ExchShopCalendarUtils.h"
#include "Common/Money.h"
#include "Common/VCTR.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareRetrievalState.h"
#include "RexPricing/OptimizationMapper.h"

namespace tse
{

class PaxType;
class MergedFareMarket;
class RexPricingOptions;
class RexPricingRequest;

enum RexDateSeqStatus : uint8_t
{
  PREVIOUS_EXCHANGE_DATE = 1,
  ORIGINAL_TICKET_DATE,
  REISSUE_TICKET_DATE,
  COMMENCE_DATE,
  BEFORE_AFTER,
  ONE_DAY_BEFORE_PREV_EXC_DATE,
  ONE_DAY_AFTER_PREV_EXC_DATE,
  TWO_DAYS_BEFORE_PREV_EXC_DATE,
  TWO_DAYS_AFTER_PREV_EXC_DATE,
  ONE_DAY_BEFORE_ORIG_TKT_DATE,
  ONE_DAY_AFTER_ORIG_TKT_DATE,
  TWO_DAYS_BEFORE_ORIG_TKT_DATE,
  TWO_DAYS_AFTER_ORIG_TKT_DATE,
  ONE_DAY_BEFORE_REISSUE_TKT_DATE,
  ONE_DAY_AFTER_REISSUE_TKT_DATE,
  TWO_DAYS_BEFORE_REISSUE_TKT_DATE,
  TWO_DAYS_AFTER_REISSUE_TKT_DATE,
  ONE_DAY_BEFORE_COMMENCE_DATE,
  ONE_DAY_AFTER_COMMENCE_DATE,
  TWO_DAYS_BEFORE_COMMENCE_DATE,
  TWO_DAYS_AFTER_COMMENCE_DATE,
  FARE_COMPONENT_DATE
};

struct VCTRInfo
{
  VCTR _vctr;
  DateTime _retrievalDate;

  VCTR& vctr() { return _vctr; }
  VendorCode& vendor() { return _vctr.vendor(); }
  CarrierCode& carrier() { return _vctr.carrier(); }
  TariffNumber& fareTariff() { return _vctr.tariff(); }
  RuleNumber& ruleNumber() { return _vctr.rule(); }
  DateTime& retrievalDate() { return _retrievalDate; }
};

struct Cat31Info
{
  uint32_t rec3ItemNo = 0;
  ExchShopCalendar::DateRange rec3DateRange;

  std::set<int> tab988SeqNo;
  std::map<int, ExchShopCalendar::DateRange> tab988SeqNoToDateRangeMap;
};

struct FareComponentInfo
{
  uint16_t _fareCompNumber = 0;
  VCTRInfo* _vctrInfo = nullptr;
  std::vector<Cat31Info*> _cat31Info;

  uint16_t& fareCompNumber() { return _fareCompNumber; }
  uint16_t fareCompNumber() const { return _fareCompNumber; }

  VCTRInfo*& vctrInfo() { return _vctrInfo; }
  const VCTRInfo* vctrInfo() const { return _vctrInfo; }

  std::vector<Cat31Info*>& cat31Info() { return _cat31Info; }
  const std::vector<Cat31Info*>& cat31Info() const { return _cat31Info; }
};

class RexBaseTrx : public BaseExchangeTrx
{
  friend class RexBaseTrxTest;

public:
  enum RexPricingPhase : uint8_t
  {
    IDLE_PHASE = 0,
    REPRICE_EXCITIN_PHASE,
    MATCH_EXC_RULE_PHASE,
    PRICE_NEWITIN_PHASE
  };

  using DateSeqType = std::pair<RexDateSeqStatus, DateTime>;
  using RequestTypes = std::pair<std::string, std::string>;

  virtual void prepareRequest() = 0;
  virtual void setUpSkipSecurityForExcItin() = 0;
  virtual bool repriceWithSameFareDate() = 0;
  virtual void set(const RequestTypes& reqTypes) = 0;

  RexPricingOptions* prepareOptions();
  const RexPricingOptions& getRexOptions() const;
  const RexPricingRequest& getRexRequest() const;
  RexPricingRequest& getRexRequest();

  static bool isSameFareDate(const PaxTypeFare* fare1, const PaxTypeFare* fare2);
  virtual const DateTime& getRuleApplicationDate(const CarrierCode& govCarrier) const = 0;
  virtual const std::vector<FareTypeTable*>&
  getFareTypeTables(const VendorCode& vendor,
                    uint32_t tblItemNo,
                    const DateTime& applicationDate) const;
  std::string& waiverCode() { return _waiverCode; }
  const std::string& waiverCode() const { return _waiverCode; }

  const PaxType*& exchangePaxType() { return _exchangePaxType; }
  const PaxType* exchangePaxType() const { return _exchangePaxType; }
  const PaxType& getExchangePaxType() const
  {
    return (_exchangePaxType && !_exchangePaxType->paxType().empty() ? *_exchangePaxType
                                                                  : *paxType().front());
  }

  std::string& secondaryExcReqType() { return _secondaryExcReqType; }
  const std::string& secondaryExcReqType() const { return _secondaryExcReqType; }

  virtual DateTime& ticketingDate() override
  {
    if (_dataHandle.useTLS())
      return const_cast<DateTime&>(_dataHandle.ticketDate());

    return _fareApplicationDT;
  }

  virtual const DateTime& ticketingDate() const override
  {
    if (_dataHandle.useTLS())
      return _dataHandle.ticketDate();

    return _fareApplicationDT;
  }

  virtual std::vector<Itin*>& itin() override { return *_workingItin; }
  virtual const std::vector<Itin*>& itin() const override { return *_workingItin; }

  virtual void setAnalyzingExcItin(const bool isAnalyzingExcItin) = 0;

  const bool isAnalyzingExcItin() const { return _analyzingExcItin; }

  void setupDateSeq();
  void setupMipDateSeq();

  RexDateSeqStatus getCurrTktDateSeqStatus() const { return (*_currTktDateSeq).first; }
  void resetCurrTktDateSeqStatus() { _currTktDateSeq = _tktDateSeq.begin(); }

  DateTime getCurrTktDateSeq() const { return (*_currTktDateSeq).second; }

  bool nextTktDateSeq();

  RexPricingPhase& trxPhase() { return _trxPhase; }
  const RexPricingPhase& trxPhase() const { return _trxPhase; }

  void setFareApplicationDT(const DateTime& fareApplicationDT);

  DateTime& fareApplicationDT() { return _fareApplicationDT; }
  const DateTime& fareApplicationDT() const { return _fareApplicationDT; }

  virtual std::vector<PaxType*>& paxType() override
  {
    if (isAnalyzingExcItin() && !_excPaxType.empty())
      return _excPaxType;
    return _paxType;
  }

  virtual const std::vector<PaxType*>& paxType() const override
  {
    if (isAnalyzingExcItin() && !_excPaxType.empty())
      return _excPaxType;
    return _paxType;
  }

  std::vector<PaxType*>& excPaxType() { return _excPaxType; }
  const std::vector<PaxType*>& excPaxType() const { return _excPaxType; }

  bool skipSecurityForExcItin() const { return _skipSecurityForExcItin; }
  virtual void setAdjustedTravelDate() {}
  virtual void setTktValidityDate() = 0;

  std::vector<FareComponentInfo*>& excFareCompInfo() { return _excFareCompInfo; }
  const std::vector<FareComponentInfo*>& excFareCompInfo() const { return _excFareCompInfo; }

  const Loc* currentSaleLoc() const;

  virtual void markFareRetrievalMethodHistorical(const bool setBits = true)
  {
    _fareRetrievalFlags.markHistoricalFare(setBits);
  }

  virtual void markFareRetrievalMethodTvlCommence(const bool setBits = true)
  {
    _fareRetrievalFlags.markTvlCommenceFare(setBits);
  }

  virtual void markFareRetrievalMethodKeep(const bool setBits = true)
  {
    _fareRetrievalFlags.markKeepFare(setBits);
  }

  virtual void markFareRetrievalMethodCurrent(const bool setBits = true)
  {
    _fareRetrievalFlags.markCurrentFare(setBits);
  }

  virtual void markFareRetrievalMethodLastReissue(const bool setBits = true)
  {
    _fareRetrievalFlags.markLastReissueFare(setBits);
  }

  virtual void markAllFareRetrievalMethod(const bool setBits = true)
  {
    _fareRetrievalFlags.markAllFare(setBits);
  }

  virtual bool needRetrieveHistoricalFare() const
  {
    return _fareRetrievalFlags.needHistoricalFare();
  }

  virtual bool needRetrieveKeepFare() const { return _fareRetrievalFlags.needKeepFare(); }

  virtual bool needRetrieveTvlCommenceFare() const
  {
    return _fareRetrievalFlags.needTvlCommenceFare();
  }

  virtual bool needRetrieveCurrentFare() const { return _fareRetrievalFlags.needCurrentFare(); }

  virtual bool needRetrieveLastReissueFare() const
  {
    return _fareRetrievalFlags.needLastReissueFare();
  }

  virtual bool needRetrieveAllFare() const { return _fareRetrievalFlags.needAllFare(); }

  virtual bool needRetrieveFare() const { return _fareRetrievalFlags.needRetrieveFare(); }

  // mip only
  virtual bool needRetrieveKeepFareAnyItin() const
  {
    return _fareRetrievalFlags.isSet(FareMarket::RetrievKeep);
  }
  virtual bool needRetrieveKeepFare(uint16_t) const
  {
    return _fareRetrievalFlags.isSet(FareMarket::RetrievKeep);
  }

  virtual const OptimizationMapper& getOptimizationMapper() const
  {
    return _optimizationMapper;
  }

  virtual OptimizationMapper& optimizationMapper()
  {
    return _optimizationMapper;
  }

  virtual Money
  convertCurrency(const Money& source, const CurrencyCode& targetCurr, bool rounding) const;
  const CurrencyCode& exchangeItinCalculationCurrency() const;

  virtual FareRetrievalState& fareRetrievalFlags() { return _fareRetrievalFlags; }
  virtual const FareRetrievalState& fareRetrievalFlags() const { return _fareRetrievalFlags; }

  typedef void GetRec2Cat10Function(PricingTrx&, std::vector<MergedFareMarket*>&);

  virtual void
  getRec2Cat10WithVariousRetrieveDates(std::vector<MergedFareMarket*>& mergedFareMarketVect,
                                       GetRec2Cat10Function getRec2Cat10) = 0;

  static bool isRexTrxAndNewItin(const PricingTrx& trx)
  {
    return (trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
            trx.excTrxType() == PricingTrx::AF_EXC_TRX) &&
           (static_cast<const RexBaseTrx*>(&trx))->trxPhase() == RexBaseTrx::PRICE_NEWITIN_PHASE;
  }

  boost::mutex& dataHandleMutex()
  {
    return _dataHandleMutex;
  }

  bool& redirected() { return _redirected; }
  bool redirected() const { return _redirected; }

  bool& redirectResult() { return _redirectResult; }
  bool redirectResult() const { return _redirectResult; }

  virtual void createExchangePricingTrxForRedirect(bool forDiagnostic);

  ExchangePricingTrx*& exchangePricingTrxForRedirect() { return _exchangePricingTrxForRedirect; }
  const ExchangePricingTrx* exchangePricingTrxForRedirect() const
  {
    return _exchangePricingTrxForRedirect;
  }

  ErrorResponseException& redirectReasonError() { return _redirectReasonError; }
  const ErrorResponseException& redirectReasonError() const { return _redirectReasonError; }

  virtual void initalizeForRedirect(std::vector<FareMarket*>& fareMarket,
                                    std::vector<TravelSeg*>& travelSeg) const = 0;
  void setActionCode() override;

  bool previousExchangeDateFare() const { return _previousExchangeDateFare; }
  bool& previousExchangeDateFare() { return _previousExchangeDateFare; }

  virtual DateTime& newItinROEConversionDate() { return _newItinROEConversionDate; }
  virtual const DateTime& newItinROEConversionDate() const { return _newItinROEConversionDate; }

  virtual DateTime& newItinSecondROEConversionDate() { return _newItinSecondROEConversionDate; }
  virtual const DateTime& newItinSecondROEConversionDate() const
  {
    return _newItinSecondROEConversionDate;
  }

  virtual bool& useSecondROEConversionDate() { return _useSecondROEConversionDate; }
  virtual const bool& useSecondROEConversionDate() const { return _useSecondROEConversionDate; }

  virtual DateTime& newItinROEConversionDate(uint16_t) { return _newItinROEConversionDate; }
  virtual const DateTime& newItinROEConversionDate(uint16_t) const
  {
    return _newItinROEConversionDate;
  }

  virtual void skipRulesOnExcItin(std::vector<uint16_t>& categorySequence) const;

  virtual bool isPlusUpCalculationNeeded() const = 0;

  const DateTime adjustToCurrentUtcZone(const DateTime& dateToAdjust, const Loc* locOfAdjust) const;

  MoneyAmount& totalFareCalcAmount() { return _totalFareCalcAmount; }
  const MoneyAmount& totalFareCalcAmount() const { return _totalFareCalcAmount; }

  void setTotalBaseFareAmount(const Money& amt) { _totalBaseFareAmount = amt; }
  const Money& getTotalBaseFareAmount() const { return _totalBaseFareAmount; }

  void setExcNonRefAmount(const Money& amt) { _excNonRefAmount = amt; }
  const Money& getExcNonRefAmount() const { return _excNonRefAmount; }
  bool isExcNonRefInRequest() const { return _excNonRefAmount.value() != -EPSILON; }

  virtual bool isContextShopping() const override
  {
    return (getRequest()->isContextShoppingRequest() &&
            (trxPhase() == RexBaseTrx::PRICE_NEWITIN_PHASE ||
             trxPhase() == RexBaseTrx::IDLE_PHASE));
  }

  const Percent* getExcDiscountPercentage(const FareMarket& fareMarket) const;
  const DiscountAmount* getExcDiscountAmount(const FareMarket& fareMarket) const;
  const Percent* getExcDiscountPercentage(const PricingUnit& pricingUnit) const;
  const DiscountAmount* getExcDiscountAmount(const PricingUnit& pricingUnit) const;

  void setupFootNotePrevalidation() override;

protected:
  std::string _waiverCode;
  std::string _secondaryExcReqType;

  // const PaxType* _accompanyPaxType;
  const PaxType* _exchangePaxType = nullptr; // For exchange itin when different from new itin

  std::vector<Itin*>* _workingItin = &_itin;
  std::vector<DateSeqType> _tktDateSeq{DateSeqType(ORIGINAL_TICKET_DATE, DateTime::emptyDate())};
  std::vector<DateSeqType>::const_iterator _currTktDateSeq = _tktDateSeq.cbegin();
  DateTime _fareApplicationDT; // Relevant to new itin && irrelevant to exchange itin

  boost::mutex _dataHandleMutex;
  std::vector<PaxType*> _excPaxType;

  std::vector<FareComponentInfo*> _excFareCompInfo;

  FareRetrievalState _fareRetrievalFlags;
  bool _analyzingExcItin = false;
  RexPricingPhase _trxPhase = RexPricingPhase::IDLE_PHASE;
  bool _skipSecurityForExcItin = false;
  bool _redirected = false;
  bool _redirectResult = false;
  bool _useSecondROEConversionDate = false;
  bool _previousExchangeDateFare = false;
  ExchangePricingTrx* _exchangePricingTrxForRedirect = nullptr;
  ErrorResponseException _redirectReasonError = ErrorResponseException::ErrorResponseCode::NO_ERROR;

  DateTime _newItinROEConversionDate; // QREX/BSP project
  DateTime _newItinSecondROEConversionDate;

  MoneyAmount _totalFareCalcAmount = -EPSILON;
  Money _totalBaseFareAmount{-EPSILON, NUC};
  Money _excNonRefAmount{-EPSILON, NUC};
  OptimizationMapper _optimizationMapper;
};

} // tse


