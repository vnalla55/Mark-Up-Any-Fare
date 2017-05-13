// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include <log4cxx/helpers/objectptr.h>

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/Common/TaxSplitDetails.h"
#include "Taxes/LegacyTaxes/CalculationDetails.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse

{
class CountrySettlementPlanInfo;
class TaxResponse;
class PricingTrx;
class TaxCodeReg;
class TravelSeg;
class Money;
class CurrencyConversionFacade;
class FareUsage;
class TaxSpecConfigReg;
class TaxItem;

class Tax
{
  friend class UTCHiddenTest;

public:
  static constexpr char YES = 'Y';
  static constexpr char PERCENTAGE = 'P';
  static constexpr char FIXED = 'F';
  static constexpr char APPLIED = 'C';
  static constexpr char BOOKLET = 'B';
  static constexpr bool CHECK_SPN = true;

  enum HandleHiddenPoints
  { HIDDEN_POINT_NOT_HANDLED = 0,
    HIDDEN_POINT_LOC1,
    HIDDEN_POINT_LOC2,
    HIDDEN_POINT_BOTH_LOCS };

  Tax() = default;
  Tax(const Tax&) = delete;
  Tax& operator=(const Tax&) = delete;

  virtual ~Tax() = default;

  const MoneyAmount& taxAmount() const { return _taxAmount; }
  const MoneyAmount& taxableFare() const { return _taxableFare; }
  const MoneyAmount& taxableBaseFare() const { return _taxableBaseFare; }
  void setTaxAmount(MoneyAmount taxAmount) { _taxAmount = taxAmount; }
  void setTaxableFare(MoneyAmount taxableFare) { _taxableFare = taxableFare; }

  const MoneyAmount& taxAmountAdjusted() const { return _taxAmountAdjusted; }
  void setTaxAmountAdjusted(MoneyAmount taxAmountAdjusted) { _taxAmountAdjusted = taxAmountAdjusted; }

  const MoneyAmount& taxableFareAdjusted() const { return _taxableFareAdjusted; }
  void setTaxableFareAdjusted(MoneyAmount taxableFareAdjusted) { _taxableFareAdjusted = taxableFareAdjusted; }

  const uint16_t& travelSegSpecialTaxStartIndex() const { return _travelSegSpecialTaxStartIndex; }
  const uint16_t& travelSegSpecialTaxEndIndex() const { return _travelSegSpecialTaxEndIndex; }
  const uint16_t& travelSegStartIndex() const { return _travelSegStartIndex; }
  const uint16_t& travelSegEndIndex() const { return _travelSegEndIndex; }
  const MoneyAmount& thruTotalFare() const { return _thruTotalFare; }
  const MoneyAmount& taxablePartialFare() const { return _taxablePartialFare; }
  const MoneyAmount& specialPercentage() const { return _specialPercentage; }
  const uint32_t& partialLocalMiles() const { return _partialLocalMiles; }
  const uint32_t& partialThruMiles() const { return _partialThruMiles; }
  const uint16_t& travelSegPartialStartOrder() const { return _travelSegPartialStartOrder; }
  const uint16_t& travelSegPartialEndOrder() const { return _travelSegPartialEndOrder; }
  const uint16_t& travelSegThruStartOrder() const { return _travelSegThruStartOrder; }
  const uint16_t& travelSegThruEndOrder() const { return _travelSegThruEndOrder; }
  const CalculationDetails& calculationDetails() const { return _calculationDetails; }
  const TaxSplitDetails& taxSplitDetails() const { return _taxSplitDetails; }
  CurrencyNoDec paymentCurrencyNoDec() const { return _paymentCurrencyNoDec; }
  const CurrencyCode& paymentCurrency() const { return _paymentCurrency; }
  CurrencyCode& paymentCurrency() { return _paymentCurrency; }

  const LocCode& hiddenBrdAirport() const { return _hiddenBrdAirport; }
  const LocCode& hiddenOffAirport() const { return _hiddenOffAirport; }
  const CurrencyCode& intermediateCurrency() const { return _intermediateCurrency; }
  const CurrencyNoDec& intermediateNoDec() const { return _intermediateNoDec; }
  const ExchRate& exchangeRate1() const { return _exchangeRate1; }
  const CurrencyNoDec& exchangeRate1NoDec() const { return _exchangeRate1NoDec; }
  const ExchRate& exchangeRate2() const { return _exchangeRate2; }
  const CurrencyNoDec& exchangeRate2NoDec() const { return _exchangeRate2NoDec; }
  const MoneyAmount& intermediateUnroundedAmount() const { return _intermediateUnroundedAmount; }
  const MoneyAmount& intermediateAmount() const { return _intermediateAmount; }
  const bool partialTax() const { return _partialTax; }
  const bool mixedTax() const { return _mixedTax; }
  const bool applyFeeOnTax() const { return _applyFeeOnTax; }
  bool& applyFeeOnTax() { return _applyFeeOnTax; }
  virtual Tax::HandleHiddenPoints& handleHiddenPoints() { return _handleHiddenPoints; }
  const char& specialTaxRCInd() const { return _specialTaxRCInd; }
  const char& specialIndex() const { return _specialIndex; }
  const char& failCode() const { return _failCode; }
  void setFailCode(char failCode) { _failCode = failCode; }
  const std::vector<TaxSpecConfigReg*>* taxSpecConfig() const { return _taxSpecConfig; }

  void setSkipTaxOnTaxIfNoFare(bool skipTaxOnTaxIfNoFare)
  {
    _skipTaxOnTaxIfNoFare = skipTaxOnTaxIfNoFare;
  }
  void setRequireTaxOnTaxSeqMatch(bool requireTaxOnTaxSeqMatch)
  {
    _requireTaxOnTaxSeqMatch = requireTaxOnTaxSeqMatch;
  }
  void setRequireTaxOnDomTaxMatch(bool requireTaxOnDomTaxMatch)
  {
    _requireTaxOnDomTaxMatch = requireTaxOnDomTaxMatch;
  }

  const bool& skipTaxOnTaxIfNoFare() const { return _skipTaxOnTaxIfNoFare; }
  bool& skipTaxOnTaxIfNoFare() { return _skipTaxOnTaxIfNoFare; }

  const bool& requireTaxOnTaxSeqMatch() const { return _requireTaxOnTaxSeqMatch; }
  bool& requireTaxOnTaxSeqMatch() { return _requireTaxOnTaxSeqMatch; }

  const bool& requireTaxOnDomTaxMatch() const { return _requireTaxOnDomTaxMatch; }
  bool& requireTaxOnDomTaxMatch() { return _requireTaxOnDomTaxMatch; }

  const bool& isSkipExempt() const { return _isSkipExempt; }
  bool& isSkipExempt() { return _isSkipExempt; }

  const bool& isExemptedTax() const { return _isExemptedTax; }

  void getTaxSpecConfig(PricingTrx& trx, TaxCodeReg& taxCodeReg);

  virtual bool validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  bool validateZZTax(PricingTrx& trx, const CountrySettlementPlanInfo* cspi);

  virtual void
  preparePortionOfTravelIndexes(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  bool checkAndSetExemptedTax(PricingTrx& trx,
      TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg) const;

  virtual bool validateLocRestrictions(PricingTrx& trx,
                                       TaxResponse& taxResponse,
                                       TaxCodeReg& taxCodeReg,
                                       uint16_t& startIndex,
                                       uint16_t& endIndex);

  virtual bool validateTripTypes(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 uint16_t& startIndex,
                                 uint16_t& endIndex);

  virtual bool validateGeoSpecLoc1(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex);

  virtual bool validateTransferTypeLoc1(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        uint16_t startIndex,
                                        uint16_t endIndex);

  virtual bool validateRange(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex);

  virtual bool validateTransit(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t travelSegIndex);

  virtual bool validateCarrierExemption(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        uint16_t travelSegIndex);

  virtual bool validateEquipmentExemption(PricingTrx& trx,
                                          TaxResponse& taxResponse,
                                          TaxCodeReg& taxCodeReg,
                                          uint16_t travelSegIndex);

  virtual bool validateFareClass(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 uint16_t travelSegIndex);

  virtual bool validateCabin(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t travelSegIndex);

  virtual bool validateTicketDesignator(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        uint16_t travelSegIndex);
  virtual bool validateSequence(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t& travelSegStartIndex,
                                uint16_t& travelSegEndIndex,
                                bool checkSpn = false);

  virtual bool validateFinalGenericRestrictions(PricingTrx& trx,
                                                TaxResponse& taxResponse,
                                                TaxCodeReg& taxCodeReg,
                                                uint16_t& startIndex,
                                                uint16_t& endIndex);

  virtual bool
  validateTaxOnChangeFees(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  virtual bool validateBaseTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  virtual void taxCreate(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegStartIndex,
                         uint16_t travelSegEndIndex);

  virtual void applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  virtual void adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  virtual void doTaxRange(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          uint16_t& startIndex,
                          uint16_t& endIndex,
                          TaxCodeReg& taxCodeReg);

  virtual void doTaxRound(PricingTrx& trx, TaxCodeReg& taxCodeReg);

  void internationalFareRounding(CurrencyConversionFacade& ccFacade,
                                 PricingTrx& trx,
                                 Money& targetMoney,
                                 MoneyAmount& sourceMoneyAmt);

  virtual MoneyAmount calculateFareDependendTaxableAmount(PricingTrx& trx,
                                                          TaxResponse& taxResponse,
                                                          TaxCodeReg& taxCodeReg,
                                                          const bool adjusted = false);

  void setupLandToAirStopover(PricingTrx& trx, TaxResponse& taxResponse);

  void updateCalculationDetails(TaxItem* taxItem);

  std::vector<TaxItem*>& taxOnTaxItems() { return _taxOnTaxItems; }
  const std::vector<TaxItem*>& taxOnTaxItems() const { return _taxOnTaxItems; }

  static bool shouldSplitPercentageTax(const PricingTrx& trx, const TaxCode& taxCode);

  virtual const std::vector<TravelSeg*>& getTravelSeg(const TaxResponse& taxResponse) const;

  virtual bool shouldCheckTravelDate() const {return true;}

protected:
  virtual MoneyAmount fareAmountInNUCForCurrentSegment(PricingTrx& trx, const FarePath* farePath,
      const TaxCodeReg& taxCodeReg) const;
  virtual MoneyAmount fareAmountInNUC(const PricingTrx& trx, const TaxResponse& taxResponse);

  bool isValidFareClassToAmount(const TaxCodeReg& taxCodeReg, const FareUsage& fareUsage) const;

  char _failCode = 0;
  char _specialIndex = 0;
  char _specialTaxRCInd = 0;
  bool _partialTax = false;
  bool _mixedTax = false;

  MoneyAmount _taxAmount = 0;
  MoneyAmount _taxAmountAdjusted = 0;
  MoneyAmount _taxableBaseFare = 0;
  MoneyAmount _taxableFare = 0;
  MoneyAmount _taxableFareAdjusted = 0;

  CalculationDetails _calculationDetails;
  TaxSplitDetails _taxSplitDetails;

  CurrencyCode _paymentCurrency;
  CurrencyNoDec _paymentCurrencyNoDec = 0;

  uint16_t _travelSegSpecialTaxStartIndex = 0;
  uint16_t _travelSegSpecialTaxEndIndex = 0;
  uint16_t _travelSegStartIndex = 0;
  uint16_t _travelSegEndIndex = 0;

  MoneyAmount _thruTotalFare = 0;
  MoneyAmount _taxablePartialFare = 0;
  MoneyAmount _specialPercentage = 0;

  uint32_t _partialLocalMiles = 0;
  uint32_t _partialThruMiles = 0;

  uint16_t _travelSegPartialStartOrder = 0;
  uint16_t _travelSegPartialEndOrder = 0;
  uint16_t _travelSegThruStartOrder = 0;
  uint16_t _travelSegThruEndOrder = 0;

  LocCode _hiddenBrdAirport;
  LocCode _hiddenOffAirport;

  CurrencyCode _intermediateCurrency;
  CurrencyNoDec _intermediateNoDec = 0;
  ExchRate _exchangeRate1 = 0;
  CurrencyNoDec _exchangeRate1NoDec = 0;
  ExchRate _exchangeRate2 = 0;
  CurrencyNoDec _exchangeRate2NoDec = 0;
  MoneyAmount _intermediateUnroundedAmount = 0;
  MoneyAmount _intermediateAmount = 0;

  bool _applyFeeOnTax = true;
  Tax::HandleHiddenPoints _handleHiddenPoints = HandleHiddenPoints::HIDDEN_POINT_NOT_HANDLED;

  TaxLocIterator _locIt;
  bool _locIteratorInitialized = false;

  bool _landToAirStopover = true;

  int16_t _furthestFareBreakSegment = -1; //store turnaround point
  std::vector<std::pair<uint16_t, uint16_t>> _portionOfTravelBeginEndIndexes;
  const std::vector<TaxSpecConfigReg*>* _taxSpecConfig = nullptr;

  bool _skipTaxOnTaxIfNoFare = false;
  bool _requireTaxOnTaxSeqMatch = false;
  bool _requireTaxOnDomTaxMatch = false;
  bool _isSkipExempt = false;
  bool _isExemptedTax = false;

  std::vector<TaxItem*> _taxOnTaxItems;

  virtual TaxLocIterator* getLocIterator(FarePath& farePath);
  bool validateTransitOnHiddenPoints(const TaxCodeReg& taxCodeReg) const;
  bool validateFromToWithHiddenPoints(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      uint16_t& startIndex,
                                      uint16_t& endIndex);

private:
  static log4cxx::LoggerPtr _logger;

  void caclulateFurthesFareBreak(PricingTrx& trx, TaxResponse& taxResponse);

  void adjustTax(
      PricingTrx& trx,
      TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg,
      MoneyAmount& amount);

  void doAtpcoDefaultTaxRounding(PricingTrx& trx);

  void doTaxRound(PricingTrx& trx, TaxCodeReg& taxCodeReg, MoneyAmount& amount);
};
}
