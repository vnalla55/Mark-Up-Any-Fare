//-------------------------------------------------------------------
//
//  File:        OCFees.h
//
//  Copyright Sabre 2009
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

#include "Common/SmallBitSet.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AncillaryOptions/AncillaryIdentifier.h"
#include "DataModel/AncillaryOptions/AncillaryPriceModifier.h"

#include <boost/optional.hpp>
#include <vector>

namespace tse
{

class Currency;
class DataHandle;
class Money;
class PricingTrx;
class FarePath;
class TaxNation;
class TravelSeg;
class OCFeesUsage;
class SubCodeInfo;
class OptionalServicesInfo;
class CarrierFlightSeg;
class SvcFeesCxrResultingFCLInfo;
class SvcFeesResBkgDesigInfo;


enum class EmdSoftPassIndicator : unsigned
{
  NoEmdIndicator,
  EmdSoftPass,
  EmdPassOrNoEmdValidation
};

class OCFees
{
public:
  enum SoftMatchS7Data
  {
    S7_CABIN_SOFT = 0x0001,
    S7_RBD_SOFT = 0x0002,
    S7_RULETARIFF_SOFT = 0x0004,
    S7_RULE_SOFT = 0x0008,
    S7_EQUIPMENT_SOFT = 0x0010,
    S7_RESULTING_FARE_SOFT = 0x0020,
    S7_CARRIER_FLIGHT_SOFT = 0x0040,
    S7_TIME_SOFT = 0x0080, // StopCnxDestInd, StopOverTime/Unit, Start Time/Stop Time
    S7_FREQFLYER_SOFT = 0x0100
  };

  using SoftMatchS7Status = SmallBitSet<uint16_t, SoftMatchS7Data>;

  enum BagSoftPassFlag : uint8_t
  {
    BAG_SP_RULETARIFF = 0x1,
    BAG_SP_TOURCODE = 0x2,
    BAG_SP_BTA_FARE_CHECKS = 0x4
  };

  using BagSoftPassStatus = SmallBitSet<uint8_t, BagSoftPassFlag>;

  class AmountRounder
  {
  public:
    AmountRounder(PricingTrx& trx) : _trx(trx) {}
    virtual ~AmountRounder() {}
    virtual MoneyAmount getRoundedFeeAmount(const Money &amount) const = 0;

  protected:
    PricingTrx& _trx;
  };

  class OcAmountRounder : public AmountRounder
  {
  public:
    OcAmountRounder(PricingTrx& trx) : AmountRounder(trx) {}
    virtual ~OcAmountRounder() {}
    virtual MoneyAmount getRoundedFeeAmount(const Money &amount) const;

  protected:
    bool getFeeRounding(const CurrencyCode& currencyCode,
                        RoundingFactor& roundingFactor,
                        CurrencyNoDec& roundingNoDec,
                        RoundingRule& roundingRule) const;
    virtual const Currency* getCurrency(const CurrencyCode& currencyCode) const;
    virtual const TaxNation* getTaxNation(const NationCode& nationCode) const;
  };

  class BaggageAmountRounder : public AmountRounder
  {
  public:
    BaggageAmountRounder(PricingTrx& trx) : AmountRounder(trx) {}
    virtual ~BaggageAmountRounder() {}
    virtual MoneyAmount getRoundedFeeAmount(const Money &amount) const;
  };

  class BaggageItemProperty
  {
  public:
    BaggageItemProperty(int number, const SubCodeInfo* subCode)
      : _free(false), _freeText(""), _number(number), _subCode(subCode)
    {
    }

    BaggageItemProperty(const std::string& freeText) : _free(true), _freeText(freeText) {}

    const bool isFreeText() const { return _free; }
    const FreeTextSegData& getFreeText() const { return _freeText; }

    int getNumber() const { return _number; }
    void setNumber(int number) { _number = number; }

    const SubCodeInfo* getSubCode() const { return _subCode; }
    void setSubCode(const SubCodeInfo* subCode)
    {
      _subCode = subCode;
      _free = false;
    }

  private:
    bool _free;
    FreeTextSegData _freeText;
    int _number = 0;
    const SubCodeInfo* _subCode = nullptr;
  };

  class TaxItem
  {
  public:
    const TaxCode& getTaxCode() const { return _taxCode; }
    void setTaxCode(const TaxCode& taxCode) { _taxCode = taxCode; }

    const TaxTypeCode& getTaxType() const { return _taxType; }
    void setTaxType(const TaxTypeCode& taxType) { _taxType = taxType; }

    const MoneyAmount getTaxAmount() const { return _taxAmount; }
    void setTaxAmount(const MoneyAmount taxAmount) { _taxAmount = taxAmount; }

    const uint16_t getNumberOfDec() const { return _numberOfDec; }
    void setNumberOfDec(const uint16_t numberOfDec) { _numberOfDec = numberOfDec; }

    const CurrencyCode& getCurrency() const { return _currency; }
    void setCurrency(const CurrencyCode& currency) { _currency = currency; }

    const MoneyAmount getTaxAmountPub() const { return _taxAmountPub; }
    void setTaxAmountPub(const MoneyAmount taxAmountPub) { _taxAmountPub = taxAmountPub; }

    const CurrencyCode& getCurrencyPub() const { return _currencyPub; }
    void setCurrencyPub(const CurrencyCode& currencyPub) { _currencyPub = currencyPub; }

    const int getSeqNo() const { return _seqNo; }
    void setSeqNo(int seqNo) { _seqNo = seqNo; }

  private:
    TaxCode _taxCode;
    TaxTypeCode _taxType = 0;

    MoneyAmount _taxAmount = 0;
    CurrencyCode _currency;
    uint16_t _numberOfDec = 0;

    MoneyAmount _taxAmountPub = 0;
    CurrencyCode _currencyPub;
    int _seqNo = 0;
  };

  struct TaxItemComparator
  {
    bool operator()(const TaxItem& ti1, const TaxItem& ti2) const
    {
      return (ti1.getTaxCode() == ti2.getTaxCode() && ti1.getTaxAmount() == ti2.getTaxAmount() &&
              ti1.getTaxType() == ti2.getTaxType());
    }
  };

  class BackingOutTaxes
  {
    MoneyAmount _feeAmountInFeeCurrency;
    MoneyAmount _feeAmountInSellingCurrency;
    MoneyAmount _feeAmountInSellingCurrencyPlusTaxes;

  public:
    BackingOutTaxes(const MoneyAmount& feeAmountInFeeCurrency,
                    const MoneyAmount& feeAmountInSellingCurrency,
                    const MoneyAmount& feeAmountInSellingCurrencyPlusTaxes)
        : _feeAmountInFeeCurrency(feeAmountInFeeCurrency),
          _feeAmountInSellingCurrency(feeAmountInSellingCurrency),
          _feeAmountInSellingCurrencyPlusTaxes(feeAmountInSellingCurrencyPlusTaxes)
    {
    }

    const MoneyAmount& feeAmountInFeeCurrency() const { return _feeAmountInFeeCurrency; }
    const MoneyAmount& feeAmountInSellingCurrency() const { return _feeAmountInSellingCurrency; }
    const MoneyAmount& feeAmountInSellingCurrencyPlusTaxes() const { return _feeAmountInSellingCurrencyPlusTaxes; }
  };

  class OCFeesSeg
  {
  public:
    OCFeesSeg() : _purchaseByDate(time(nullptr)) {}

    const std::vector<TaxItem>&
    getTaxes() const { return _taxes; }

    void
    addTax(const TaxItem& taxItem) { _taxes.push_back(taxItem); }

    void
    setBackingOutTaxes(const MoneyAmount& feeAmountInFeeCurrency,
        const MoneyAmount& feeAmountInSellingCurrency,
        const MoneyAmount& feeAmountInSellingCurrencyPlusTaxes);

    bool
    isBackingOutTaxes() const { return static_cast<bool>(_backingOutTaxesData); }

    const BackingOutTaxes&
    getBackingOutTaxes() const { return _backingOutTaxesData.get(); }

    const OptionalServicesInfo* _optFee = nullptr;
    MoneyAmount _feeAmount = 0;
    MoneyAmount _orginalFeeAmount = 0;
    CurrencyCode _displayCurrency;
    MoneyAmount _displayAmount = 0;
    CurrencyCode _feeCurrency;
    CurrencyCode _orginalFeeCurrency;
    CurrencyNoDec _feeNoDec = 0;
    CurrencyNoDec _orginalFeeNoDec = 0;
    const FarePath* _farePath = nullptr;
    bool _displayOnly = false;
    bool _feeGuaranteed = true;
    DateTime _purchaseByDate;
    SoftMatchS7Status _softMatchS7Status;
    std::vector<SvcFeesCxrResultingFCLInfo*> _resultingFareClass; // T171
    std::vector<CarrierFlightSeg*> _carrierFlights; // T186
    std::vector<SvcFeesResBkgDesigInfo*> _rbdData; // T198
    LocCode _matchOriginAirport;
    LocCode _matchDestinationAirport;
    std::vector<OCFees::BaggageItemProperty> _baggageItemProperties;
    std::vector<SvcFeesResBkgDesigInfo*> _padisData;
    std::vector<TaxItem> _taxes;
    boost::optional<BackingOutTaxes> _backingOutTaxesData;
    boost::optional<std::pair<AncillaryIdentifier, AncillaryPriceModifier>> _ancPriceModification;
  };

  class Memento
  {
    OCFeesSeg* _currentSegment;
  public:
    Memento(OCFeesSeg* currentSegment) : _currentSegment(currentSegment) {}
    OCFeesSeg* getSavedState() { return _currentSegment; }
  };

  OCFees() = default;
  OCFees(const OCFees& ocfees);
  virtual ~OCFees() = default;

  Memento
  saveToMemento() { return Memento(getCurrentSeg()); }

  void
  restoreFromMemento(Memento& memento) { setCurrentSeg(memento.getSavedState()); }

  // accessors
  CarrierCode& carrierCode() { return _carrier; }
  const CarrierCode& carrierCode() const { return _carrier; }

  TravelSeg*& travelStart() { return _travelStart; }
  const TravelSeg* travelStart() const { return _travelStart; }

  TravelSeg*& travelEnd() { return _travelEnd; }
  const TravelSeg* travelEnd() const { return _travelEnd; }

  const SubCodeInfo*& subCodeInfo() { return _subCodeInfo; }
  const SubCodeInfo* subCodeInfo() const { return _subCodeInfo; }

  const OptionalServicesInfo*& optFee() { return _curSeg->_optFee; }
  const OptionalServicesInfo* optFee() const { return _curSeg->_optFee; }

  MoneyAmount& feeAmount() { return _curSeg->_feeAmount; }
  const MoneyAmount& feeAmount() const { return _curSeg->_feeAmount; }

  MoneyAmount& orginalFeeAmount() { return _curSeg->_orginalFeeAmount; }
  const MoneyAmount& orginalFeeAmount() const { return _curSeg->_orginalFeeAmount; }

  CurrencyCode& feeCurrency() { return _curSeg->_feeCurrency; }
  const CurrencyCode& feeCurrency() const { return _curSeg->_feeCurrency; }

  CurrencyCode& orginalFeeCurrency() { return _curSeg->_orginalFeeCurrency; }
  const CurrencyCode& orginalFeeCurrency() const { return _curSeg->_orginalFeeCurrency; }

  MoneyAmount& displayAmount() { return _curSeg->_displayAmount; }
  const MoneyAmount& displayAmount() const { return _curSeg->_displayAmount; }

  CurrencyCode& displayCurrency() { return _curSeg->_displayCurrency; }
  const CurrencyCode& displayCurrency() const { return _curSeg->_displayCurrency; }

  CurrencyNoDec& feeNoDec() { return _curSeg->_feeNoDec; }
  const CurrencyNoDec& feeNoDec() const { return _curSeg->_feeNoDec; }

  CurrencyNoDec& orginalFeeNoDec() { return _curSeg->_orginalFeeNoDec; }
  const CurrencyNoDec& orginalFeeNoDec() const { return _curSeg->_orginalFeeNoDec; }

  const FarePath*& farePath() { return _curSeg->_farePath; }
  const FarePath* farePath() const { return _curSeg->_farePath; }

  bool isDisplayOnly() const { return _curSeg->_displayOnly; }
  void setDisplayOnly(bool isDisplayOnly) { _curSeg->_displayOnly = isDisplayOnly; }

  bool& isFeeGuaranteed() { return _curSeg->_feeGuaranteed; }
  bool isFeeGuaranteed() const { return _curSeg->_feeGuaranteed; }

  DateTime& purchaseByDate() { return _curSeg->_purchaseByDate; }
  DateTime purchaseByDate() const { return _curSeg->_purchaseByDate; }

  LocCode& matchedOriginAirport() { return _curSeg->_matchOriginAirport; }
  LocCode& matchedOriginAirport() const { return _curSeg->_matchOriginAirport; }

  LocCode& matchedDestinationAirport() { return _curSeg->_matchDestinationAirport; }
  LocCode& matchedDestinationAirport() const { return _curSeg->_matchDestinationAirport; }

  // soft Match status and accessories

  bool isAnyS7SoftPass() const { return _curSeg->_softMatchS7Status.value() != 0; }

  SoftMatchS7Status& softMatchS7Status() { return _curSeg->_softMatchS7Status; }
  const SoftMatchS7Status& softMatchS7Status() const { return _curSeg->_softMatchS7Status; }

  std::vector<SvcFeesCxrResultingFCLInfo*>& softMatchResultingFareClassT171()
  {
    return _curSeg->_resultingFareClass;
  }
  const std::vector<SvcFeesCxrResultingFCLInfo*>& softMatchResultingFareClassT171() const
  {
    return _curSeg->_resultingFareClass;
  }

  std::vector<CarrierFlightSeg*>& softMatchCarrierFlightT186() { return _curSeg->_carrierFlights; }
  const std::vector<CarrierFlightSeg*>& softMatchCarrierFlightT186() const
  {
    return _curSeg->_carrierFlights;
  }

  std::vector<SvcFeesResBkgDesigInfo*>& softMatchRBDT198() { return _curSeg->_rbdData; }
  const std::vector<SvcFeesResBkgDesigInfo*>& softMatchRBDT198() const { return _curSeg->_rbdData; }

  std::vector<SvcFeesResBkgDesigInfo*>& padisData() { return _curSeg->_padisData; }
  const std::vector<SvcFeesResBkgDesigInfo*>& padisData() const { return _curSeg->_padisData; }

  std::vector<OCFeesSeg*>& segments() { return _segs; }
  const std::vector<OCFeesSeg*>& segments() const { return _segs; }

  size_t segCount() const { return _segs.size(); }
  void setSeg(size_t index) { _curSeg = _segs.at(index); }

  const OCFeesSeg& getSeg(size_t index) const { return *(_segs.at(index)); }

  OCFeesSeg* getSegPtr(size_t index) const { return (_segs.at(index)); }

  OCFeesSeg* getCurrentSeg() { return _curSeg; }
  void setCurrentSeg(OCFeesSeg* segment) { _curSeg = segment; }

  void addSeg(DataHandle& dh);
  void cleanOutCurrentSeg();
  void cleanBaggageResults();
  void pointToFirstOCFee()
  {
    if (segCount() > 1)
      setSeg(0);
  }
  bool& failS6() { return _failS6; }
  bool failS6() const { return _failS6; }

  BagSoftPassStatus bagSoftPass() const { return _bagSoftPass; }
  BagSoftPassStatus& mutableBagSoftPass() { return _bagSoftPass; }

  const std::vector<TaxItem>& getTaxes() const
  {
    return _curSeg->_taxes;
  }

  void addTax(const TaxItem& taxItem)
  {
    _curSeg->_taxes.push_back(taxItem);
  }

  void addTax(const TaxItem& taxItem, size_t index)
  {
    OCFeesSeg* ocFeesSegPtr = _segs.at(index);
    if(ocFeesSegPtr)
      ocFeesSegPtr->_taxes.push_back(taxItem);
  }

  uint32_t& baggageTravelIndex() { return _baggageTravelIndex; }
  const uint32_t& baggageTravelIndex() const { return _baggageTravelIndex; }

  std::vector<OCFeesUsage*>& ocfeeUsage() { return _ocfeeUsage; }
  const std::vector<OCFeesUsage*>& ocfeeUsage() const { return _ocfeeUsage; }

  void
  setBackingOutTaxes(const MoneyAmount& feeAmountInFeeCurrency,
      const MoneyAmount& feeAmountInSellingCurrency,
      const MoneyAmount& feeAmountInSellingCurrencyPlusTaxes)
  {
    return _curSeg->setBackingOutTaxes(feeAmountInFeeCurrency,
          feeAmountInSellingCurrency, feeAmountInSellingCurrencyPlusTaxes);
  }

  bool
  isBackingOutTaxes() const
  {
    return _curSeg->isBackingOutTaxes();
  }

  const BackingOutTaxes&
  getBackingOutTaxes() const
  {
    return _curSeg->getBackingOutTaxes();
  }

  EmdSoftPassIndicator getEmdSoftPassChargeIndicator() const { return _emdChargeIndicator; }
  void setEmdSoftPassChargeIndicator(EmdSoftPassIndicator emdChargeIndicator) { _emdChargeIndicator = emdChargeIndicator; }

private:
  CarrierCode _carrier;
  TravelSeg* _travelStart = nullptr;
  TravelSeg* _travelEnd = nullptr;
  const SubCodeInfo* _subCodeInfo = nullptr;
  OCFeesSeg _ocFeesSeg;
  OCFeesSeg* _curSeg = &_ocFeesSeg;
  std::vector<OCFeesSeg*> _segs = {_curSeg};
  bool _failS6 = false;
  BagSoftPassStatus _bagSoftPass;
  uint32_t _baggageTravelIndex = 0;
  std::vector<OCFeesUsage*> _ocfeeUsage;
  EmdSoftPassIndicator _emdChargeIndicator = EmdSoftPassIndicator::NoEmdIndicator;
};
} // tse
