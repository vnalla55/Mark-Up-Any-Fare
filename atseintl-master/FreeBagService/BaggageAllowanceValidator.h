//-------------------------------------------------------------------
//  Copyright Sabre 2010
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

#include "FreeBagService/BaggageItinAnalyzer.h"
#include "FreeBagService/BaggageS7SubvalidatorInterfaces.h"
#include "FreeBagService/IS7RecordFieldsValidator.h"
#include "ServiceFees/CommonSoftMatchValidator.h"

#include <memory>

namespace tse
{
class BaggageTravel;
class BagValidationOpt;
class TravelSeg;
class OptionalServicesInfo;
class SubCodeInfo;
class SvcFeesResBkgDesigInfo;
class SvcFeesDiagCollector;
class DiagCollector;

class BaggageAllowanceValidator : public CommonSoftMatchValidator, public IS7RecordFieldsValidator
{
public:
  static constexpr Indicator DEFER_BAGGAGE_RULES_FOR_MC = 'D';
  static constexpr Indicator DEFER_BAGGAGE_RULES_FOR_OPC = 'O';
  static constexpr Indicator BTA_APPLICATION_TYPE_EMPTY = ' ';

  BaggageAllowanceValidator(const BagValidationOpt& opt);

  void setBtaSubvalidator(IBtaSubvalidator*);
  void setNonBtaFareSubvalidator(INonBtaFareSubvalidator*);
  void setDeferTargetCxr(const CarrierCode& cxr) { _deferTargetCxr = cxr; }
  void setFarePath(FarePath* fp) { _farePath = fp; }

  void collectAllowance(const SubCodeInfo& s5, uint32_t startSeqNo, IAllowanceCollector& out);
  OCFees& validate(const SubCodeInfo& subCodeInfo, uint32_t startSeqNo = 0);

protected:
  virtual bool isInLoc(const VendorCode& vendor,
                       const LocKey& locKey,
                       const Loc& loc,
                       CarrierCode carrier = EMPTY_STRING()) const override;

  class ScopeSwitcher
  {
  public:
    ScopeSwitcher(BaggageAllowanceValidator* validator,
                  const OptionalServicesInfo& optSrvInfo);
    ~ScopeSwitcher();

  protected:
    BaggageAllowanceValidator* _validator;
    bool _restore;
    TravelSegPtrVecCI _segStartItToRestore;
    TravelSegPtrVecCI _segEndItToRestore;
  };

  class T183FieldDiagBuffer
  {
  public:
    T183FieldDiagBuffer(PricingTrx& trx);
    bool bufferingT183TableDiagRequired() const;
    SvcFeesDiagCollector* getT183DiagBuffer() const;
    void flush(DiagCollector* bufferedDataTarget) const;

  private:
    SvcFeesDiagCollector* _bufferForT183Table;
  };

  static constexpr Indicator SEC_POR_IND_JOURNEY = 'J';
  static constexpr Indicator TIME_DAILY = 'D';
  static constexpr Indicator TIME_RANGE = 'R';
  static constexpr Indicator TIME_UNIT_DAY = 'D';
  static constexpr Indicator TIME_UNIT_MONTH = 'M';

  virtual const std::vector<OptionalServicesInfo*>&
  getOptionalServicesInfo(const SubCodeInfo& subCode) const override;

  uint16_t getTvlDepartureDOW() const;

  virtual StatusS7Validation validateS7Data(OptionalServicesInfo& optSrvInfo, OCFees& ocFees);

  virtual bool validateVia(const OptionalServicesInfo& optSrvInfo,
                           std::vector<TravelSeg*>& passedLoc3Dest) const;
  virtual bool validateCnxOrStopover(const OptionalServicesInfo& optSrvInfo,
                                     const std::vector<TravelSeg*>& passedLoc3Dest,
                                     bool validateCnx) const;
  bool validateStopoverWithGeo(const OptionalServicesInfo& optSrvInfo) const;
  bool validateStartAndStopTime(const OptionalServicesInfo& info) const;
  bool validateStartAndStopTimeRange(const OptionalServicesInfo& info) const;

  bool checkTravelDate(const OptionalServicesInfo& optSrvInfo) const;
  bool
  checkFrequentFlyerStatus(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const override;
  bool checkFrequentFlyerStatusImpl(const OptionalServicesInfo& optSrvInfo,
                                    OCFees& ocFees) const;

  virtual StatusS7Validation
  checkServiceNotAvailNoCharge(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  virtual bool checkGeoFtwInd(const OptionalServicesInfo& optSrvInfo) const;
  virtual bool checkIntermediatePoint(const OptionalServicesInfo& optSrvInfo,
                                      std::vector<TravelSeg*>& passedLoc3Dest) const;
  virtual bool checkStopCnxDestInd(const OptionalServicesInfo& optSrvInfo,
                                   const std::vector<TravelSeg*>& passedLoc3Dest) const;
  virtual bool checkSecurity(const OptionalServicesInfo& optSrvInfo) const;
  bool checkEquipmentType(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const override;
  bool checkCabin(const Indicator cabin, const CarrierCode& carrier, OCFees& ocFees) const;
  bool checkCabinData(AirSeg& seg, const CabinType& cabin) const;
  bool checkTourCode(const OptionalServicesInfo& info) const;
  bool checkStartStopTime(const OptionalServicesInfo& info, bool& checkDOW) const;
  bool checkDOW(const OptionalServicesInfo& info) const override;

  bool checkCarrierFlightApplT186(const OptionalServicesInfo& info,
                                  const uint32_t itemNo,
                                  OCFees& ocFees) const;
  const TaxCarrierFlightInfo*
  checkCarrierFlightApplT186Preconditions(const uint32_t itemNo, const VendorCode& vendor) const;
  bool checkT186(const AirSeg* seg, const TaxCarrierFlightInfo* cxrFltT186) const;

  StatusT186 isValidCarrierFlight(const AirSeg& air, CarrierFlightSeg& t186) const override;
  StatusT186 isValidCarrierFlightNumber(const AirSeg& air, CarrierFlightSeg& t186) const override;
  virtual StatusT171 isValidFareClassFareType(const PaxTypeFare* ptf,
                                              SvcFeesCxrResultingFCLInfo& fclInfo,
                                              OCFees& ocFees) const override;
  virtual bool matchRuleTariff(const uint16_t& ruleTariff,
                               const PaxTypeFare& ptf,
                               OCFees& ocFees) const override;
  bool isStopover(const OptionalServicesInfo& optSrvInfo,
                  const TravelSeg* seg,
                  const TravelSeg* next) const override;
  std::vector<TravelSeg*>::const_iterator
  findFirstStopover(const OptionalServicesInfo& optSrvInfo,
                    const std::vector<TravelSeg*>::const_iterator& begin,
                    const std::vector<TravelSeg*>::const_iterator& end) const;
  bool isRBDValid(AirSeg* seg,
                  const std::vector<SvcFeesResBkgDesigInfo*>& rbdInfos,
                  OCFees& ocFees) const override;
  virtual StatusT171 isValidResultingFareClass(const PaxTypeFare* ptf,
                                               SvcFeesCxrResultingFCLInfo& fclInfo,
                                               OCFees& ocFees) const override;

  virtual void printT183TableBufferedData() const override;
  virtual void printResultingFareClassInfo(const PaxTypeFare& ptf,
                                           const SvcFeesCxrResultingFCLInfo& info,
                                           StatusT171 status) const override;

  OCFees* buildFees(const SubCodeInfo& subCodeInfo) const;
  void supplementFees(const SubCodeInfo& subCodeInfo, OCFees* ocFees) const;

  // IS7RecordFieldsValidator interface
  virtual const TravelSeg* determineSpecialSegmentForT186(const BaggageTravel& bt) const override;

  virtual bool checkCabinInSegment(TravelSeg* segment, const Indicator cabin) const override;
  virtual bool
  checkRBDInSegment(TravelSeg* segment,
                    OCFees& ocFees,
                    uint32_t serviceFeesResBkgDesigTblItemNo,
                    const std::vector<SvcFeesResBkgDesigInfo*>& rbdInfos) const override;
  virtual bool checkResultingFareClassInSegment(
      const PaxTypeFare* paxTypeFare,
      uint32_t serviceFeesCxrResultingFclTblItemNo,
      OCFees& ocFees,
      const std::vector<SvcFeesCxrResultingFCLInfo*>& resFCLInfo) const override;
  virtual bool checkOutputTicketDesignatorInSegment(TravelSeg* segment,
                                                    const PaxTypeFare* ptf,
                                                    const OptionalServicesInfo& s7) const override;
  virtual bool checkRuleInSegment(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                                  const RuleNumber& rule,
                                  OCFees& ocFees) const override;
  virtual bool checkRuleTariffInSegment(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                                        uint16_t ruleTariff,
                                        OCFees& ocFees) const override;
  virtual bool checkFareIndInSegment(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                                     const Indicator& fareInd) const override;
  virtual bool checkCarrierFlightApplT186InSegment(const TravelSeg* segment,
                                                   const VendorCode& vendor,
                                                   uint32_t itemNo) const override;
  StatusS7Validation checkBTA(OptionalServicesInfo& optSrvInfo, OCFees& ocFees);
  bool isCancelled(const OptionalServicesInfo& optSrvInfo) const;
  bool shouldModifySegmentRange(const BaggageTravel& baggageTravel) const;

  const BaggageTravel& _baggageTravel;
  const CheckedPoint& _furthestCheckedPoint;
  CarrierCode _deferTargetCxr;
  bool _isUsDot;
  bool _allowSoftMatch = false;
  bool _isAncillaryBaggage = false;
  T183FieldDiagBuffer _t183TableBuffer;
  std::unique_ptr<INonBtaFareSubvalidator> _nonBtaFareSubvalidator;
  std::unique_ptr<IBtaSubvalidator> _btaSubvalidator;

  friend class BaggageAllowanceValidatorTest;

private:
  StatusS7Validation validateS7Data_DEPRECATED(OptionalServicesInfo& optSrvInfo, OCFees& ocFees);
};
} // tse
