//-------------------------------------------------------------------
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

#include "Common/OcTypes.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ItinHelperStructs.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "ServiceFees/SliceAndDice.h"

#include <map>

namespace tse
{
class PricingTrx;
class Itin;
class TravelSeg;
class MerchActivationInfo;
class MerchCarrierPreferenceInfo;
class ServiceGroupInfo;
class OCFees;
class MerchActivationValidator;
class OptionalServicesActivationInfo;

class OptionalFeeCollector : public SliceAndDice
{
  friend class OptionalFeeCollectorTest;

public:
  static const Indicator S5_INDCRXIND_CARRIER;
  static const Indicator S5_INDCRXIND_INDUSTRY;

  OptionalFeeCollector(PricingTrx& trx);

  void collect();
  virtual void process() {}

  OptionalFeeCollector(const OptionalFeeCollector& rhs);

protected:
  OptionalFeeCollector& operator=(const OptionalFeeCollector& rhs);

  bool _shoppingOCF;
  mutable int16_t _numberOfOcFeesForItinOCF; // counter for Shopping
  mutable bool _stopMatchProcessOCF; // StopProcessing
  bool _needFirstMatchOnlyOCF; // Pricing entry w/o group codes

  std::vector<ServiceGroupInfo*> _allGroupCodesOCF;
  Ts2ss _ts2ssOCF;
  std::map<const CarrierCode, std::vector<ServiceGroup*> > _cXrGrpOCF;
  std::map<const CarrierCode, std::vector<ServiceGroup*> > _cXrDispOnlyGrp;
  mutable boost::mutex _mutexOCF;

  bool _checkMerchCxrPref;
  mutable bool _timeOutOCF;

  ItinBoolMap _isInternationalOCF;
  ItinBoolMap _isRoundTripOCF;

  void processOCFees();

  virtual bool isAgencyActive();
  bool setsUpIndForShopping();
  virtual void identifyGroupCodesToBeProcessed(std::vector<ServiceGroup>& grNV,
                                               std::vector<ServiceGroup>& grNA,
                                               std::vector<ServiceGroup>& grNP);
  void filterOutForACS(std::vector<ServiceGroup>& grValid,
                       std::vector<ServiceGroup>& grNotValidForTicketDate);

  virtual void
  multiThreadSliceAndDice(std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                                std::vector<TravelSeg*>::const_iterator> >& routes,
                          int unitNo);

  template <typename T>
  void multiThreadSliceAndDiceImpl(
      std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                            std::vector<TravelSeg*>::const_iterator> >& routes,
      int unitNo)
  {
    std::vector<T> sliceAndDices(routes.size(), *((T*)this));
    TseRunnableExecutor taskExecutor(TseThreadingConst::SVCFEESCOLLECTOR_TASK);
    size_t index = 0;
    typedef std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                  std::vector<TravelSeg*>::const_iterator> >::const_iterator
    RouteIt;
    for (RouteIt routeI = routes.begin(); routeI != routes.end(); ++routeI, ++index)
    {
      sliceAndDices[index].thRoute() = *routeI;
      sliceAndDices[index].thUnitNo() = unitNo;
      taskExecutor.execute(sliceAndDices[index]);
    }
    // Wait for the tasks to complete
    taskExecutor.wait();

    for (index = 0; index < routes.size(); ++index)
      reclaimDiag(sliceAndDices[index]);
  }

  void printNoMerchActiveCxrGroup() const;
  virtual void printActiveCxrGroupHeader() const;
  virtual void printActiveMrktDrivenCxrHeader() const;
  void printActiveMrktDrivenNCxrNoData() const;
  void printActiveMrktDrivenCxrData(std::vector<MerchCarrierPreferenceInfo*>& mCxrPrefVec) const;
  void printCanNotCollect(const StatusS5Validation rc) const;
  void printPccIsDeactivated(const PseudoCityCode& pcc);
  void printNoGroupCodeProvided();
  void printNoOptionsRequested();
  void printGroupCodesRequested(const std::vector<ServiceGroup>& input,
                                const std::vector<ServiceGroup>& Invalid,
                                const std::vector<ServiceGroup>& notAct);
  void printJourneyDestination();
  void printStopAtFirstMatchMsg() const;
  void endDiag() const;

  bool checkAllSegsUnconfirmed(std::vector<TravelSeg*>::const_iterator begin,
                               std::vector<TravelSeg*>::const_iterator end) const;

  bool finalCheckShoppingPricing(Itin& itin);
  // overrides for database/external code
  virtual bool roundTripJouneyType(const Itin& itin) const;
  virtual bool internationalJouneyType(const Itin& itin) const;
  virtual void getAllATPGroupCodes();
  virtual const std::vector<OptionalServicesActivationInfo*>&
  getOptServiceActivation(Indicator crs,
                          const UserApplCode& userCode,
                          const std::string& application);
  virtual const std::vector<MerchActivationInfo*>&
  getMerchActivation(MerchActivationValidator& validator,
                     const CarrierCode& carrierCode,
                     const PseudoCityCode& pcc) const;
  virtual bool retrieveMerchActivation(MerchActivationValidator& validator,
                                       const CarrierCode& carrierCode,
                                       std::vector<ServiceGroup*>& groupCode,
                                       std::vector<ServiceGroup*>& dispOnlyGroups) const;

  void getAllActiveGroupCodesForApplication(std::vector<ServiceGroup>& gValid,
                                            const Indicator crs,
                                            const UserApplCode userCode,
                                            const std::string& application); //Deprecated

  void getAllActiveGroupCodesForApplication(std::set<ServiceGroup>& gValid,
                                            const Indicator crs,
                                            const UserApplCode userCode,
                                            const std::string& application);

  bool collectOperMktCxr(std::vector<tse::CarrierCode>& cxrCodes,
                         bool callFrmMerchCxrPref = false) const;
  virtual bool processCarrierMMRecords();

  virtual void getAllActiveGroupCodes(std::vector<ServiceGroup>& grCodes); // Deprecated
  virtual void getAllActiveGroupCodes(std::set<ServiceGroup>& grCodes);
  void selectActiveNotActiveGroupCodes(std::vector<ServiceGroup>& grValid,
                                       std::vector<ServiceGroup>& grNotValidForTicketDate,
                                       std::vector<ServiceGroup>& grNotProcessedForTicketDate);

  void filterOutInputGroupCodes(std::vector<ServiceGroup>& gC,
                                std::vector<ServiceGroup>& gValid,
                                std::vector<ServiceGroup>& gNValid,
                                std::vector<ServiceGroup>& gNValidTktDate);
  bool isATPActiveGroupFound(ServiceGroup sg, bool& invalid, std::vector<ServiceGroup>& vec);

  void cleanUpOCFees(Itin& itin);
  virtual void updateServiceFeesDisplayOnlyState(OCFees* ocFee);

  bool processMerchCxrPrefData(std::vector<MerchCarrierPreferenceInfo*>& mCxrPrefVec) const;
  const MerchCarrierPreferenceInfo*
  getMerchCxrPrefInfo(const CarrierCode& carrier, const ServiceGroup& groupCode) const;

  void getValidRoutes(std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                            std::vector<TravelSeg*>::const_iterator> >& routes,
                      std::vector<TravelSeg*>::const_iterator firstSeg,
                      std::vector<TravelSeg*>::const_iterator endSeg);
  virtual int getNumberOfSegments(std::vector<TravelSeg*>::const_iterator firstSeg,
                                  std::vector<TravelSeg*>::const_iterator endSeg) const;
  virtual ServiceFeesGroup::FindSolution getProcessingMethod(ServiceFeesGroup& srvGroup);
  virtual void sliceAndDice(int unitNo) override;
  void processLargestPortionOfTvl(int unitNo);
  virtual void addInvalidGroups(Itin& itin,
                                const std::vector<ServiceGroup>& grNA,
                                const std::vector<ServiceGroup>& grNP,
                                const std::vector<ServiceGroup>& grNV);
  void sortOCFeeGroup(std::vector<ServiceGroup>& grValid);
  virtual void
  getGroupCodesNotProcessedForTicketingDate(std::vector<ServiceGroup>& grValid,
                                            std::vector<ServiceGroup>& grNotValidForTicketDate,
                                            std::vector<ServiceGroup>& grNotProcessedForTicketDate);
  void addNotAvailableGroups(Itin& itin,
                             const std::vector<ServiceGroup>& grNAP,
                             ServiceFeesGroup::StateCode);
  void timeOutPostProcessing(Itin& itin,
                             const std::vector<ServiceGroup>& grNV,
                             const std::vector<ServiceGroup>& grNA);
  void collectExcludedSFG(std::vector<ServiceGroup>& excludedsfG,
                          const std::vector<ServiceGroup>& grNV,
                          const std::vector<ServiceGroup>& grNA);
  void cleanUpGroupCode(ServiceFeesGroup& sfG);
  virtual bool shouldProcessAllGroups() const;
  virtual UserApplCode getUserApplCode() const;
  virtual bool isAncillaryPricing() const
  {
    return false;
  };
  virtual void createPseudoPricingSolution();

  bool isOCFeesProcessedForLargestPortions(Itin& itin, ServiceFeesGroup& sfG);
  bool checkLargestPortion(std::vector<TravelSeg*>::const_iterator first,
                           std::vector<TravelSeg*>::const_iterator last,
                           ServiceFeesGroup& sfG) const;
  bool checkOCFeeMap(const TravelSeg* tvl, ServiceFeesGroup& sfG) const;
  bool isTravelSegFound(const TravelSeg* tvl, std::vector<OCFees*>& vec) const;
  void cleanUpAllOCFees(Itin& itin);
  virtual void updateServiceFeesGroupState(ServiceFeesGroup* sfG);
};

} // tse namespace


