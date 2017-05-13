//-------------------------------------------------------------------
//  Copyright Sabre 2012
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

#include "FreeBagService/BaggageChargesValidator.h"
#include "FreeBagService/BagValidationOpt.h"

namespace tse
{
class OptionalServicesInfo;
class BaggageTravel;

typedef std::tuple<OCFees, StatusS7Validation> S7StatusInfo;

class BaggageAncillaryChargesValidator : public BaggageChargesValidator
{
  friend class BaggageAncillaryChargesValidatorTest;
  friend class BaggageAncillaryChargesValidatorByOccurrenceFilterAb240Test;
  friend class BaggageAncillaryChargesValidatorByOccurrenceFilterTest;
  friend class BaggageAncillaryChargesValidatorByWeightFilterTest;
  friend class BaggageAncillaryChargesValidatorDisclosureFinderTest;
  friend class DisclosureFinder;

private:
  class S7Filter
  {
  public:
    virtual ~S7Filter() = default;

    virtual void reset() = 0;
    virtual bool select(const OptionalServicesInfo& s7) = 0;
  };

  class ByWeightFilter : public S7Filter
  {
    friend class BaggageAncillaryChargesValidatorByWeightFilterTest;

  public:
    void reset() override
    {
      _blankWeight = false;
      _maxWeight = 0;
    }

    virtual bool select(const OptionalServicesInfo& s7) override;

  protected:
    bool _blankWeight = false;
    int32_t _maxWeight = 0;
  };

  class ByOccurrenceFilter : public S7Filter
  {
    friend class BaggageAncillaryChargesValidatorByOccurrenceFilterTest;

  public:
    virtual void reset() override { _state = MATCHING_FIRST; }

    virtual bool select(const OptionalServicesInfo& s7) override;

  protected:
    enum State
    {
      MATCHING_FIRST,
      MATCHING_BLANKBLANK,
      MATCHING_SEQUENCE,
      SKIP
    };

    virtual bool matchFirst(const OptionalServicesInfo& s7, const bool lastPcBlank, const bool blankBlank);
    bool matchBlankBlank(const bool blankBlank);
    virtual bool matchSequence(const OptionalServicesInfo& s7,
                               const bool firstPcBlank,
                               const bool lastPcBlank,
                               const bool blankBlank);

    State _state = State::MATCHING_FIRST;
    int32_t _prevLastPc = 0;
  };

  class ByOccurrenceFilterAB240 : public ByOccurrenceFilter
  {
    friend class BaggageAncillaryChargesValidatorByOccurrenceFilterAb240Test;
  public:
    ByOccurrenceFilterAB240(const PricingTrx* trx);
    bool matchFirst(const OptionalServicesInfo& s7, const bool lastPcBlank, const bool blankBlank) override;
    bool matchSequence(const OptionalServicesInfo& s7,
                       const bool firstPcBlank,
                       const bool lastPcBlank,
                       const bool blankBlank) override;
  private:
    const PricingTrx *_trx = nullptr;
  };

  class DisclosureFinder
  {
  public:
    DisclosureFinder(PricingTrx& trx, const BaggageTravel& baggageTravel);

    void processBaggageCharge(const OptionalServicesInfo& optSrv, BaggageCharge& baggageCharge);

  protected:
   int32_t getFreeBaggagePcs();
   bool isAllowanceInWeight();

   PricingTrx& _trx;
   const BaggageTravel& _baggageTravel;
   bool _lookForFdo = false;
   bool _lookForSdo = false;
   bool _lookForSdoOnly = false;
  };

public:
  BaggageAncillaryChargesValidator(const BagValidationOpt& opt)
    : BaggageChargesValidator(opt), _byOccurrenceFilterAB240(opt._bt._trx) {}

  virtual void validate(const SubCodeInfo& subCodeInfo, ChargeVector& matchedCharges) override;
  void filterSegments();
  bool hasSegments() const { return _segI != _segIE; }

protected:
  S7Filter* selectS7Filter(const ServiceSubTypeCode& subTypeCode);
  void printDiagS7Info(const ChargeVector& charges,
                       std::map<const OptionalServicesInfo*, S7StatusInfo>& s7Status) const;
  virtual StatusS7Validation validateS7Data(OptionalServicesInfo& optSrvInfo, OCFees& ocFees) override;
  virtual bool checkAdvPurchaseTktInd(const OptionalServicesInfo& optSrvInfo) const override;
  bool checkFreeBagPieces(const OptionalServicesInfo& optSrvInfo) const;
  virtual StatusS7Validation
  checkServiceNotAvailNoCharge(const OptionalServicesInfo& info, OCFees& /*ocFees*/) const override;
  virtual bool checkFeeApplication(const OptionalServicesInfo& optSrvInfo) const override;
  virtual bool checkOccurrence(const OptionalServicesInfo& optSrvInfo) const override;
  virtual const Loc& getLocForSfcValidation() const override;
  virtual bool shouldProcessAdvPur(const OptionalServicesInfo& /*info*/) const override;
  virtual bool checkBaggageWeightUnit(const OptionalServicesInfo& /*optSrvInfo*/) const override;
  bool checkTaxApplication(const OptionalServicesInfo& optSrvInfo) const;
  bool checkAvailability(const OptionalServicesInfo& optSrvInfo) const;
  bool checkRuleBusterRuleMatchingFareClass(const OptionalServicesInfo& optSrvInfo) const;
  bool checkPassengerOccurrence(const OptionalServicesInfo& optSrvInfo) const;
  const PaxTypeFare* getFare(const OptionalServicesInfo& optSrvInfo) const;
  const PaxTypeFare* getFareFromTrx(RepricingTrx* repricingTrx) const;
  bool checkFare(const PaxTypeFare* paxTypeFare) const;
  bool retrieveSpecifiedFee(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees);
  MoneyAmount getFarePercentageAmount(MoneyAmount amount, const OptionalServicesInfo& info) const;
  virtual bool checkSecurity(const OptionalServicesInfo& optSrvInfo) const override;
  bool checkForPrintFareInfo(const OptionalServicesInfo& s7) const;
  bool checkForPrintFareSelectionInfo(const OptionalServicesInfo& s7) const;
  const FareMarket* getFareMarket(const CarrierCode& carrier) const;
  bool skipBaggageDefinedByWeightForAB240(const OptionalServicesInfo& optSrv) const;

  bool _fareForFeeAlreadyQueried = false;
  const PaxTypeFare* _fareForFee = nullptr;
  ByWeightFilter _byWeightFilter;
  ByOccurrenceFilter _byOccurrenceFilter;
  ByOccurrenceFilterAB240 _byOccurrenceFilterAB240;
};

} // tse
