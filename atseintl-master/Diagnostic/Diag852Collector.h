//----------------------------------------------------------------------------
//  File:        Diag852Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 852 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2004
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

#include "Common/OcTypes.h"
#include "DataModel/BaggageTravel.h"
#include "Diagnostic/Diag877Collector.h"

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

#include <set>

namespace tse
{
class SubCodeInfo;
class BaggageCharge;
class BaggageTravelInfo;

namespace PrecalcBaggage
{
  struct AllowanceRecords;
  struct ChargeRecords;
  struct CxrPair;
}

class Diag852Collector : public Diag877Collector
{
  friend class Diag852CollectorTest;

public:
  enum DiagType
  { BASIC = 0,
    FBACTIVE,
    CSACTIVE,
    FLACTIVE,
    CPACTIVE,
    DCACTIVE,
    STACTIVE,
    CAACTIVE,
    CCACTIVE,
    FAACTIVE,
    FFACTIVE,
    PRACTIVE,
    PQACTIVE };

  static const std::string CODE_SHARE;
  static const std::string FARE_LINE;
  static const std::string CHECKED_PORTION;
  static const std::string DISPLAY_DETAIL;
  static const std::string BTA;
  static const std::string DISPLAY_CHARGES;
  static const std::string SUB_TYPE_CODE;
  static const std::string CARRY_ON_ALLOWANCE;
  static const std::string CARRY_ON_CHARGES;
  static const std::string EMBARGOES;
  static const std::string FEE_ASSIGNMENT;
  static const std::string FREQ_FLYER;
  static const std::string PREVALIDATION;
  static const std::string PQ;

  class Diag852ParsedParams
  {
    friend class Diag852ParsedParamsTester;

  public:
    explicit Diag852ParsedParams(const Diag852Collector& diag);

    DiagType type() const;
    uint32_t fareLine() const;
    uint32_t checkedPortion() const;
    const std::string& subTypeCode() const;

  private:
    void initialiseParams() const;

  private:
    mutable DiagType _diagType;
    mutable uint32_t _fareLine;
    mutable uint32_t _checkedPortion;
    mutable std::string _subTypeCode;

    mutable bool _initialised;
    const Diag852Collector& _parent;
  };

  class S7PrinterBase
  {
  public:
    explicit S7PrinterBase(Diag852Collector& diag) : _diag(diag) {}

    virtual ~S7PrinterBase() {}

    virtual void printS7Processing(const PricingTrx& trx, const BaggageTravel* baggageTravel) = 0;
    virtual void printTravelInfo(const BaggageTravel* baggageTravel, const Ts2ss& ts2ss) = 0;
    virtual void printS7DetailInfo(const OptionalServicesInfo* info, const PricingTrx& trx) = 0;
    virtual void printS7RecordValidationFooter(const OptionalServicesInfo& info);

  protected:
    void printS7ProcessingHeader(const PricingTrx& trx, const BaggageTravel* baggageTravel);
    void printCorporateId(const PricingTrx& trx);

    bool shouldPrintFreqFlyer(const PricingTrx& trx) const;
    void printFreqFlyerStatus(const std::vector<PaxType::FreqFlyerTierWithCarrier*>& ff,
                              const CarrierCode cxr);
    void printPTC(const PricingTrx& trx, const BaggageTravel* baggageTravel);
    void printAccountCode(const PricingTrx& trx);
    void printInputDesignator(const PricingTrx& trx);
    void printOutputDesignator(const PricingTrx& trx, const BaggageTravel* baggageTravel);
    void printOutputDesignator(const PricingTrx& trx,
                               const std::vector<TravelSeg*>::const_iterator& segB,
                               const std::vector<TravelSeg*>::const_iterator& segE,
                               const FarePath& farePath);
    void printTourCode(const BaggageTravel* baggageTravel);
    void printDayOfWeek(const BaggageTravel* baggageTravel);
    void printEquipment(const BaggageTravel* baggageTravel);

  protected:
    Diag852Collector& _diag;
  };

  class S7PrinterNoDdinfo : public S7PrinterBase
  {
  public:
    explicit S7PrinterNoDdinfo(Diag852Collector& diag) : S7PrinterBase(diag) {}

    virtual void printS7Processing(const PricingTrx& trx, const BaggageTravel* baggageTravel) override;
    virtual void printTravelInfo(const BaggageTravel* baggageTravel, const Ts2ss& ts2ss) override;
    virtual void printS7DetailInfo(const OptionalServicesInfo* info, const PricingTrx& trx) override;

    void printS7BaggageInfo(const OptionalServicesInfo* info);
  };

  class S7PrinterDdinfo : public S7PrinterNoDdinfo
  {
    friend class Diag852CollectorTest;

  public:
    explicit S7PrinterDdinfo(Diag852Collector& diag) : S7PrinterNoDdinfo(diag) {}

    virtual void printS7Processing(const PricingTrx& trx, const BaggageTravel* baggageTravel) override;
    virtual void printTravelInfo(const BaggageTravel* baggageTravel, const Ts2ss& ts2ss) override;
    virtual void printS7DetailInfo(const OptionalServicesInfo* info, const PricingTrx& trx) override;
    virtual void printS7RecordValidationFooter(const OptionalServicesInfo& info) override;

  protected:
    void printBTAInfo(const OptionalServicesInfo* info, bool allignRight);

  private:
    void printTravelInfoForAllSectors(
        std::vector<TravelSeg*>::const_iterator itBegin,
        std::vector<TravelSeg*>::const_iterator itEnd,
        std::vector<TravelSeg*>::const_iterator itMostSignificant,
        const bool isBaggageTrx,
        const BaggageTravel* baggageTravel,
        const Ts2ss& ts2ss);

    void printCabinInfo(TravelSeg* seg, const Ts2ss& ts2ss);
    void printRBDInfo(TravelSeg* seg, const Ts2ss& ts2ss);
    void printCATInfo(const PaxTypeFare* ptf, bool isBaggageTrx);
    void printFlightAppInfo(TravelSeg* seg);
  };

  class S7PrinterBTA : public S7PrinterDdinfo
  {
  public:
    explicit S7PrinterBTA(Diag852Collector& diag) : S7PrinterDdinfo(diag) {}

    virtual void printS7DetailInfo(const OptionalServicesInfo* info, const PricingTrx& trx) override;
    void printS7RecordValidationFooter(const OptionalServicesInfo& info) override;
  };

  class S7PrinterDEPRECATED : public S7PrinterNoDdinfo
  {
  public:
    explicit S7PrinterDEPRECATED(Diag852Collector& diag) : S7PrinterNoDdinfo(diag) {}

    virtual void printS7Processing(const PricingTrx& trx, const BaggageTravel* baggageTravel) override;
    virtual void printTravelInfo(const BaggageTravel* baggageTravel, const Ts2ss& ts2ss) override;
  };

  Diag852Collector& operator<<(const Header) override;
  Diag852Collector& operator<<(const Itin& itin) override;

  class Prevalidation
  {
  public:
    Prevalidation(Diag852Collector& dc) : _dc(dc) {}
    Diag852Collector& operator<<(const Itin& itin);
    Diag852Collector& operator<<(const Header);

  private:
    void printAllowanceInfo(const OptionalServicesInfo& info, const OCFees& ocFees);
    void printBaggageAllowances(const boost::container::flat_map<PrecalcBaggage::CxrPair, PrecalcBaggage::AllowanceRecords>& baggageAllowance);
    void printBaggageCharges(const boost::container::flat_map<CarrierCode, PrecalcBaggage::ChargeRecords>& baggageCharges);
    void printBaggageWeightOrPcs(const OptionalServicesInfo& info);
    void printChargeInfo(const OptionalServicesInfo* info, const BaggageCharge& ocFees, const CarrierCode carrierCode);
    void printItineraryInfo(const Itin& itin);
    void printSoftPassStatus(const OCFees& ocFees);

    Diag852Collector& _dc;
  };

  Diag852Collector() : _params(*this), _displayT196(false) {}
  explicit Diag852Collector(Diagnostic& root)
    : Diag877Collector(root), _params(*this), _displayT196(false)
  {
  }

  DiagType diagType() const { return _params.type(); }
  uint32_t fareLine() const { return _params.fareLine(); }
  uint32_t checkedPortion() const { return _params.checkedPortion(); }
  const std::string& subTypeCode() const { return _params.subTypeCode(); }
  bool hasDisplayChargesOption() const;
  bool isDisplayCarryOnAllowanceOption() const;
  bool isDisplayCarryOnChargesOption() const;
  bool isDisplayEmbargoesOption() const;

  void printBaggageHeader(const PricingTrx& trx, bool isCarrierOverridden = false);
  void printCarryOnBaggageHeader(bool processingCarryOn, bool processingEmbargo);
  void printItinAnalysisResults(const PricingTrx& trx,
                                const CheckedPointVector& checkedPoints,
                                const CheckedPoint& furthestCheckedPoint,
                                const TravelSeg* furthestTicketedPoint,
                                const Itin& itin,
                                bool retransitPointsExist);

  void printCheckedPoint(const CheckedPoint& checkedPoint);

  void printCheckedPoints(const CheckedPointVector& checkedPoints,
                          const TravelSeg* furthestTicketedPoint);

  void
  printFurthestCheckedPoint(const CheckedPointVector& cp, const CheckedPoint& furthestCheckedPoint);

  void printBaggageTravels(const std::vector<const BaggageTravel*>& baggageTravels, bool isUsDot);

  void printMileage(const LocCode& origin,
                    const LocCode& destination,
                    const std::vector<Mileage*>& mil,
                    const boost::container::flat_set<GlobalDirection>& gdirs,
                    Indicator mileageType);

  void printCarryOnBaggageTravels(const std::vector<const BaggageTravel*>& baggageTravels);

  void printCarryOnBaggageTravel(const BaggageTravel* baggageTravel, uint32_t index);

  void printBaggageTravel(const BaggageTravel* baggageTravel,
                          bool isUsDot,
                          uint32_t index,
                          bool defer = false);
  void printDoNotReadA03Table();
  void printReadTable(const std::vector<PaxType::FreqFlyerTierWithCarrier*>& data);
  void printDeterminedFFStatus(const uint16_t level);
  void printFFStatuses(const CarrierCode partnerCarrier,
                       const uint16_t partnerlevel,
                       const uint16_t determinedLevel);
  void setCarrierWithA03Status(CarrierCode carrier, bool has03Table)
  {
    _owningCarrierWithA03Status = std::make_pair(carrier, has03Table);
  }

  void printS5Record(const SubCodeInfo* s5, bool isMarketingCxr, bool isCarrierOverride = false);

  void printS7Processing(const PricingTrx& trx, const BaggageTravel* baggageTravel);

  void printS7ProcessingContext(const PricingTrx& trx,
                                const BaggageTravel* baggageTravel,
                                bool isUsDot,
                                uint32_t index,
                                bool isMarketingCxr,
                                const SubCodeInfo* s5,
                                bool defer = false,
                                bool isCarrierOverride = false);

  void printS7ProcessingCarryOnContext(const PricingTrx& trx,
                                       const BaggageTravel* baggageTravel,
                                       uint32_t index,
                                       const SubCodeInfo* s5);

  void printTravelInfo(const BaggageTravel* baggageTravel, const Ts2ss& ts2ss) override;

  void printS7CommonHeader() override; // from Diag877Collector

  void printS7ChargesHeader(uint32_t bagNo);

  void printS7DetailInfo(const OptionalServicesInfo* info,
                         const PricingTrx& trx) override; // from Diag877Collector

  void printChargesHeader();
  void printCharge(const BaggageCharge* baggageCharge, uint32_t occurence);

  void printTariffCarriers(const PricingTrx& trx, const Itin& itin);

  bool checkFl(const Itin& itin) const;

  bool printTable196ForCarryOnDetailSetup(uint32_t checkedPortion,
                                          const OptionalServicesInfo* s7,
                                          const FarePath& farePath,
                                          const PricingTrx& trx);

  void printTable196ForBaggageDetailSetup(uint32_t checkedPortion,
                                          const OptionalServicesInfo* s7,
                                          const FarePath& farePath,
                                          const PricingTrx& trx);

  void printTable196DetailHeader(const uint32_t itemno, const std::vector<std::string>& t196);
  void printTable196Detail(const SubCodeInfo* subCodeInfo);
  void printTable196Detail(bool selected);
  void printTable196DetailEnd(const SubCodeInfo* subCodeInfo);
  void printFareInfo(const PaxTypeFare* paxTypeFare, PricingTrx& pricingTrx);
  void printFareSelectionInfo(const FareMarket& fareMarket,
                              const CarrierCode& carrierCode,
                              PricingTrx& pricingTrx,
                              const PaxTypeFare* selectedFare = nullptr);
  void printFareCheckInfo(const CarrierCode& fareCarrier,
                          const std::string& fareCheckStatus,
                          const std::string& paxFareBasis,
                          MoneyAmount moneyAmount,
                          const CurrencyCode& currency,
                          bool isSelected);
  std::string getFareCheckStatus(const PaxTypeFare& paxTypeFare, const CarrierCode& carrier) const;
  virtual void printS7OptionalServiceStatus(StatusS7Validation rc) override;
  virtual void
  printS7RecordValidationFooter(const OptionalServicesInfo& info, const PricingTrx& trx) override;

  void printBTASegmentHeader(const TravelSeg& segment);
  void printBTASegmentFooter(bool segmentValidationPassed, const TravelSeg& segment);
  bool isBTAContextOn() const;
  void printBTAFieldStatusCabin(bool matched);
  void printBTAFieldStatusRBD(bool matched);
  void printBTAStatusTableT171(bool matched);
  void printBTAStatusOutputTicketDesignator(bool matched);
  void printBTAFieldStatusRuleTariff(bool matched);
  void printBTAFieldStatusRule(bool matched);
  void printBTAFieldStatusFareInd(bool matched);
  void printBTAFieldStatusCarrierFlightApplT186(bool matched);

  virtual bool shouldCollectInRequestedContext(ProcessingContext context) const override;
  bool validContextForBTA(ProcessingContext context) const;


  void printDetailInterlineEmdAgreementInfo(const std::vector<EmdInterlineAgreementInfo*>& eiaList,
                                            const tse::CarrierCode& validatingCarrier) const;
  void printNoInterlineDataFoundInfo();
  void printDetailInterlineEmdProcessingS5Info(const tse::NationCode& nation,
                                              const tse::CrsCode& gds,
                                              const tse::CarrierCode& validatingCarrier,
                                              const std::set<tse::CarrierCode>& marketingCarriers,
                                              const std::set<tse::CarrierCode>& operatingCarriers);
  void printEmdValidationResult(bool emdValidationResult, AncRequestPath rp, const BaggageTravel* baggageTravel, const BaggageTravelInfo& bagInfo) const;

  // "Baggage in PQ"-related methods
  void printFarePathBaggageCharge(const FarePath& fp, MoneyAmount lbound);
  void printBaggageTravelCharges(const BaggageTravel& bt);
  void printInfoAboutUnknownBaggageCharges(PricingTrx& trx);

protected:
  void printTable196DetailSetup(uint32_t checkedPortion,
                                const OptionalServicesInfo* s7,
                                const FarePath& farePath,
                                const PricingTrx& trx,
                                bool isAllowanceInfoDiag);

  void displayAmount(const OCFees& ocFees) override; // from Diag877Collector
  void appendAmount(const int32_t quantity, const std::string& unit);
  void displayFFMileageAppl(Indicator ind) override; // from Diag877Collector
  void printOriginAndDestination(const Itin& itin, bool isUsdot);
  bool isDDInfo() const;
  bool isAvEMDIA() const;
  void printCheckedSegments(const BaggageTravel* baggageTravel, bool isUSDot);
  std::string getCarrierListAsString(const std::set<tse::CarrierCode>& carriers, const std::string& separator) const;
  void printSimpleS7Id(const OptionalServicesInfo& s7);

  S7PrinterBase* s7Printer(const PricingTrx& trx);

  Diag852ParsedParams _params;
  bool _displayT196;

private:
  DataHandle _dataHandle;
  std::pair<CarrierCode, bool> _owningCarrierWithA03Status;
  static constexpr int CARRIER_LENGTH = 4;
  static constexpr int FARE_BASIS_LENGTH = 12;
  static constexpr int STATUS_LENGTH = 20;
  static constexpr int AMOUNT_LENGTH = 10;
  static constexpr int CURRENCY_LENGTH = 10;
  static constexpr int SELECTION_MARK_LENGTH = 3;

  static constexpr int FARE_CHECK_INFO_PRECISION = 9;

  static const std::string BTA_FIELD_STATUS_PASSED;
  static const std::string BTA_FIELD_STATUS_FAILED;
};

} // namespace tse

