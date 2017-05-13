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

#include "Common/OcTypes.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "ServiceFees/CarrierStrategy.h"
#include "ServiceFees/MerchCarrierStrategy.h"
#include "ServiceFees/ServiceFeesGroup.h"

#include <map>
#include <vector>

#include <boost/function.hpp>

namespace tse
{
class PricingTrx;
class OptionalFeeConcurValidator;
class OptionalServicesValidator;
class ServiceGroupInfo;
class Diag875Collector;
class Diag877Collector;
class Diag880Collector;

class SliceAndDice : public TseCallableTrxTask
{
  friend class SliceAndDiceTest;
protected:
  typedef std::vector<
      std::tuple<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt, bool> >::const_iterator TSResIt;
  static const uint16_t BEGIN_OF_TVL_PORTION;
  static const uint16_t END_OF_TVL_PORTION;
  static const uint16_t MARKET_DRIVEN_STRATEGY;

  static const ServiceDisplayInd DISPLAY_CAT_STORE;
  static const Indicator EMD_TYPE_ELECTRONIC_TKT;
  static const Indicator EMD_TYPE_ASSOCIATED_TKT;
  static const Indicator EMD_TYPE_STANDALONE;

public:
  SliceAndDice(
      PricingTrx& trx,
      const ItinBoolMap& isInternational,
      const ItinBoolMap& isRoundTrip,
      bool& stopMatchProcess,
      const Ts2ss& ts2ss,
      const std::map<const CarrierCode, std::vector<ServiceGroup*> >& cXrGrp,
      const std::vector<ServiceGroupInfo*>& allGroupCodes,
      const bool& shoppingOCF,
      const bool& needFirstMatchOnly,
      int16_t& numberOfOcFeesForItin,
      bool& timeOut,
      boost::mutex& mutex);
  SliceAndDice(const SliceAndDice& sd);
  virtual ~SliceAndDice() {};

  virtual void sliceAndDice(int unitNo) {}

  void performTask() override;
  void reclaimDiag(const SliceAndDice& sd);

protected:
  virtual void getJourneyDestination();
  void setJourneyDestination();
  void updateTravelParams(int unitNo, ServiceFeesGroup* srvFeesGrp, const TSResIt& begIt);
  bool processMarketDrivenFeesGroup(int unitNo,
                                    ServiceFeesGroup* srvFeesGrp,
                                    OptionalFeeConcurValidator& s6Validator);
  void defineMarketingOperatingCarriers();
  bool isOperatingInCxrMap(int unitNo, ServiceFeesGroup* srvFeesGrp) const;
  virtual bool processSingleServiceFeesGroup(int unitNo,
                                             ServiceFeesGroup* srvFeesGrp,
                                             bool needS6Validation,
                                             OptionalFeeConcurValidator& s6Validator) const;
  void processServiceFeesGroup(ServiceFeesGroup* srvFeesGrp,
                               const std::set<CarrierCode>& candidateCarriers,
                               int unitNo) const;
  void processServiceFeesGroup(ServiceFeesGroup* srvFeesGrp,
                               int unitNo,
                               OptionalFeeConcurValidator* validator) const;
  virtual void processServiceFeesGroup99(ServiceFeesGroup* srvFeesGrp,
                                         int unitNo) const;
  virtual void initializeSubCodes(ServiceFeesGroup* srvFeesGroup,
                                  const CarrierCode& carrier,
                                  const ServiceTypeCode& srvTypeCode,
                                  const ServiceGroup& srvGroup) const;
  bool getOperatingMarketingInd() const;
  virtual void processSubCodes(ServiceFeesGroup& srvFeesGrp,
                               const CarrierCode& candCarrier,
                               int unitNo,
                               bool isOneCarrier) const;
  void processSubCodes(OptionalServicesValidator& optSrvValidator,
                       ServiceFeesGroup& srvFeesGrp,
                       const CarrierCode& candCarrier,
                       int unitNo,
                       bool isOneCarrier) const;
  bool isAnyS5Found(const ServiceFeesGroup& group, const CarrierCode& cxr) const;
  bool isSubCodePassed(const ServiceFeesGroup& srvFeesGrp,
                       int unitNo,
                       const ServiceSubTypeCode& subCode,
                       const Indicator& serviceType) const;
  StatusS5Validation validateS5Data(const SubCodeInfo* subCode, int unitNo = 0, const CarrierCode& emdValidatingCarrier ="") const;
  bool checkFlightRelated(const SubCodeInfo& subCode) const;
  bool checkIndCrxInd(const Indicator& indCrxInd) const;
  bool checkDisplayCategory(const ServiceDisplayInd& displayCat) const;
  bool checkEMDType(const Indicator& emdType) const;
  ServiceFeesGroup* addNewServiceGroup(const ServiceGroup& srvGroup) const;
  virtual bool validMerchCxrGroup(const CarrierCode& carrier, const ServiceGroup& srvGroup) const;

  void printDiagHeader(int unitNo) const;
  bool isDDPass() const;
  void printDiagS5NotFound(const ServiceGroup& group, const CarrierCode& cxr) const;
  void printS7Header(const ServiceGroup& group, const CarrierCode& cxr) const;
  void printDiagS5Info(const SubCodeInfo* sci, const StatusS5Validation& rc) const;
  void printTimeOutMsg(int unitNo, ServiceGroup sg = BLANK_CODE) const;
  void checkDiagForS5Detail(const SubCodeInfo* subInfo) const;
  bool isMktCrxMktDrivenCrx(int unitNo, ServiceFeesGroup* srvFeesGrp) const;
  virtual bool validateS7(const OptionalServicesValidator& validator,
                          OCFees& ocFee,
                          bool stopMatch = false) const;
  virtual bool prePaidBaggageActive() const;
  bool marketingSameAsOperating(int unitNo) const;
  bool partitionSameAsOperatingAndOcFeeCarrier(int unitNo, const CarrierCode&) const;
  void processSliceAndDicePortion(int unitNo,
                                  const std::pair<std::vector<TravelSeg*>::const_iterator,
                                                  std::vector<TravelSeg*>::const_iterator>& route);
  uint64_t getFilterMask(int unitNo) const;
  void processPortionOfTvl(int unitNo);
  virtual bool checkAllSegsConfirmed(std::vector<TravelSeg*>::const_iterator begin,
                                     std::vector<TravelSeg*>::const_iterator end,
                                     bool doDiag = true) const;
  void processPortionOfTravel(int unitNo, boost::function<void(int)> fun);
  void processServiceFeesGroups(int unitNo);
  bool isTimeOut(int unitNo, ServiceGroup sg = BLANK_CODE) const;
  bool checkNumbersToStopProcessing(bool ret) const;

  void createDiag(bool header = true);

  bool checkEMDAgreement(const SubCodeInfo& subCode, int unitNo, const CarrierCode& emdValidatingCarrier) const;
  bool checkEMDAgreement_old(const Indicator& emdType, int unitNo, const CarrierCode& emdValidatingCarrier) const;

public:
  Diag875Collector* diag875() const { return _diag875; }
  Diag877Collector* diag877() const { return _diag877; }
  Diag880Collector* diag880() const { return _diag880; }

protected:
  bool isOneMarketingCxr(int unitNo) const { return _marketingCxr[unitNo].size() == 1; }
  bool isOneOperatingCxr(int unitNo) const { return _operatingCxr[unitNo].size() == 1; }
  void setStrategy(CarrierStrategy& strategy) const { _carrierStrategy = &strategy; }
  void inline accumulatePassedServices() const
  {
    const boost::lock_guard<boost::mutex> guard(_mutex);
    _numberOfOcFeesForItin++;
  }
  void printGroupDescriptionEmpty(const ServiceGroup sg) const;

protected:
  PricingTrx& _trx;
  // will shared between trhreads
  const ItinBoolMap& _isInternational;
  const ItinBoolMap& _isRoundTrip;
  bool& _stopMatchProcess;
  const Ts2ss& _ts2ss;
  const std::map<const CarrierCode, std::vector<ServiceGroup*> >& _cXrGrp;
  const std::vector<ServiceGroupInfo*>& _allGroupCodes;
  const bool& _shopping;
  const bool& _needFirstMatchOnly; // Pricing entry w/o group codes
  int16_t& _numberOfOcFeesForItin; // counter for Shopping
  bool& _timeOut;
  std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*> > _carrierEmdInfoMap;
  NationCode _nation;
  boost::mutex& _mutex;

  // own copy
  FarePath* _farePath = nullptr;
  std::vector<tse::TravelSeg*>::const_iterator _beginsOfUOT[3]; // begins of unit of travel
  std::vector<tse::TravelSeg*>::const_iterator _beginsOfLargestUOT[3]; // begins of largest unit of
                                                                       // travel
  const Loc* _journeyDestination = nullptr;
  SingleSegmentStrategy _singleStrategy;
  MultipleSegmentStrategy _multipleStrategy;
  MerchCarrierStrategy* _merchCrxStrategy = &_multipleStrategy;
  TravelSeg* _first = nullptr; // first seg current portion of travel
  TravelSeg* _last = nullptr; // last seg current portion of travel
  bool _slice_And_Dice = false;
  std::set<tse::CarrierCode> _operatingCxr[2]; // for unit of travel
  std::set<tse::CarrierCode> _marketingCxr[2]; // for unit of travel

  mutable Diag875Collector* _diag875 = nullptr;
  mutable Diag877Collector* _diag877 = nullptr;
  Diag880Collector* _diag880 = nullptr;
  bool _diagInfo = false;

  ServiceFeesGroup* _singleFeesGroupValidation = nullptr;

  mutable MarketingCarrierStrategy _marketingCarrierStrategy;
  mutable CarrierStrategy* _carrierStrategy = &_marketingCarrierStrategy;
  mutable OperatingCarrierStrategy _operatingCarrierStrategy;
  mutable PartitionCarrierStrategy _partitionCarrierStrategy;
  mutable MultipleOperatingCarrierStrategy _multipleOperatingCarrierStrategy;

  ////////// Thread start parameteres
public:
  std::pair<std::vector<TravelSeg*>::const_iterator, std::vector<TravelSeg*>::const_iterator>&
  thRoute()
  {
    return _thRoute;
  }
  int& thUnitNo() { return _thUnitNo; }

protected:
  std::pair<std::vector<TravelSeg*>::const_iterator, std::vector<TravelSeg*>::const_iterator>
  _thRoute;
  int _thUnitNo = 0;
};
}


