//-------------------------------------------------------------------
// File:    GroupFarePathFactory.h
// Created: July 2004
// Authors: Mohammad Hossan
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
//-------------------------------------------------------------------

#pragma once

#include "Common/Thread/TseCallableTrxTask.h"
#include "Pricing/GroupFarePath.h"

#include <boost/heap/priority_queue.hpp>

#include <vector>

namespace tse
{

class AccompaniedTravel;
class DiagCollector;
class FareMarket;
class FarePath;
class FarePathFactory;
class PaxFarePathFactory;
class PaxFarePathFactoryBase;
class PaxType;
class PricingTrx;
class Logger;

class GroupFarePathFactory
{
  friend class GroupFarePathFactoryTest;
  friend class GroupFarePathFactoryExpansionTest;

public:
  class GroupFarePathPQ
  {
    typedef boost::heap::priority_queue<GroupFarePath*,
      boost::heap::compare<GroupFarePath::Greater> > Queue;

  public:
    void enqueue(GroupFarePath* groupFarePath) { _queue.push(groupFarePath); }

    GroupFarePath* dequeue()
    {
      _lastDequeued = _queue.top();
      _queue.pop();
      return _lastDequeued;
    }

    GroupFarePath* top() const { return _queue.top(); }
    bool empty() const { return _queue.empty(); }
    std::size_t size() const { return _queue.size(); }
    GroupFarePath* lastDequeued() const { return _lastDequeued; }

  private:
    Queue _queue;
    GroupFarePath* _lastDequeued = nullptr;
  };

  struct GETFPInput : public TseCallableTrxTask
  {
    GETFPInput()
    {
      desc("GET FP TASK");
    }

    void performTask() override;
    bool getAlreadyBuiltFPPQItem();

    uint32_t factIdx = 0;
    uint32_t idx = 0;

    // for searching SameFareBreak solution for other PaxTypes
    const FPPQItem* primaryFPPQItem = nullptr;
    PaxFarePathFactoryBase* pfpf = nullptr;
    GroupFarePathFactory* gfpf = nullptr;
    FPPQItem* fppqItem = nullptr; // output of the getFarePathThrFunc
    DiagCollector* diag = nullptr;
    ErrorResponseException::ErrorResponseCode errResponseCode =
        ErrorResponseException::ErrorResponseCode::NO_ERROR;
    std::string errResponseMsg;
    bool done = false;
  };

  explicit GroupFarePathFactory(PricingTrx& trx) : _trx(trx) {}
  GroupFarePathFactory& operator=(const GroupFarePathFactory&) = delete;
  GroupFarePathFactory(const GroupFarePathFactory&) = delete;

  ~GroupFarePathFactory();

  void clearFactories();

  bool initGroupFarePathPQ();

  GroupFarePath* getGroupFarePath(DiagCollector& diag, const MoneyAmount lastAmount = 0.0);

  const PricingTrx& getTrx() const { return _trx; }

  void setThroughFarePricing();

  const std::vector<PaxFarePathFactoryBase*>& getPaxFarePathFactoryBucket() const
  {
    return _paxFarePathFactoryBucket;
  }

  void setPaxFarePathFactoryBucket(const std::vector<PaxFarePathFactoryBase*>& bucket)
  {
    _paxFarePathFactoryBucket = bucket;
  }

  void setPaxFarePathFactoryBucket(const std::vector<PaxFarePathFactory*>& bucket);

  const AccompaniedTravel* accompaniedTravel() const { return _accompaniedTravel; }
  AccompaniedTravel*& accompaniedTravel() { return _accompaniedTravel; }

  bool buildNextGroupFarePathSet(bool initStage,
                                 const GroupFarePath& groupFarePath,
                                 DiagCollector& diag);

  bool& usePooling() { return _usePooling; }

  const int32_t& multiPaxShortCktTimeOut() const { return _multiPaxShortCktTimeOut; }
  int32_t& multiPaxShortCktTimeOut() { return _multiPaxShortCktTimeOut; }

  const int32_t& numTaxCallForMipAltDate() const { return _numTaxCallForMipAltDate; }
  int32_t& numTaxCallForMipAltDate() { return _numTaxCallForMipAltDate; }

  void startTimer() { _startTime = time(nullptr); }

  bool isEqualToTopOrLastGroupFarePath(const MoneyAmount lastAmount) const;

private:
  static const uint32_t INVALID_FP_INDEX;

  void buildGroupFarePath(const bool initStage,
                          const std::vector<uint32_t>& gfpIndices,
                          uint16_t xPoint,
                          DiagCollector& diag);

  bool buildNextGroupFarePathSetMIP(const GroupFarePath& gfpath, DiagCollector& diag);
  const GroupFarePathPQ& groupFarePathPQ() const { return _groupFarePathPQ; }
  GroupFarePathPQ& groupFarePathPQ() { return _groupFarePathPQ; }

  GroupFarePath* getSFBGroupFarePath(GroupFarePath*& gfpathReserved, DiagCollector& diag);

  void initThreads(std::vector<GETFPInput>& thrInputVect,
                   const bool initStage,
                   const uint16_t xPoint,
                   const std::vector<uint32_t>& fpIndices);

  void
  initThreadsSameFareBreak(std::vector<GETFPInput>& thrInputVec, const FPPQItem* primaryFPPQItem);

  void startThreads(std::vector<GETFPInput>& thrInputVect) const;

  bool collectThreadResult(std::vector<GETFPInput>& thrInputVect,
                           GroupFarePath& gfpath,
                           bool noneFound,
                           DiagCollector& diag) const;

  void checkZeroAmountInfantFP(GroupFarePath& gfpath, DiagCollector& diag);

  void setPriority(const FPPQItem& fppqItem, GroupFarePath& gfpath) const;
  void recalculatePriorities(GroupFarePath& gfpath) const;

  bool isGroupFarePathValid(GroupFarePath& gfpath);
  bool isDivideParty(const GroupFarePath& gfpath) const;

  bool reserveGFPandContinue(GroupFarePath& gfpath,
                             bool cmdPricing,
                             GroupFarePath*& gfpathReserved) const;
  bool checkCurrency(const GroupFarePath& gfpath, DiagCollector& diag) const;

  bool isSameItinInEachFarePath(const GroupFarePath& gfpath) const;

  bool checkISICode(const Itin::ISICode& firstISICode, FarePath& fp, DiagCollector& diag) const;

  bool processMultiPax(const std::vector<FarePath*>& gfpVect,
                       const FareUsage& currentFU,
                       const bool needSameFareBreak,
                       const bool isInSideTripPU,
                       DiagCollector& diag) const;

  bool processMultiPaxShopping(const std::vector<FarePath*>& gfpVect,
                               const FareUsage& currentFU,
                               DiagCollector& diag) const;

  void displayGroupFP(const GroupFarePath& gfp, DiagCollector& diag) const;

  bool isMultiOptionRequest(const GroupFarePath& gfp) const;
  void checkAltDateSolution(Itin& curItin,
                            FarePath& farePath,
                            PricingTrx::AltDateInfo& altDateInfo) const;
  bool checkShopping(const GroupFarePath& gfpath) const;
  bool checkIS(const GroupFarePath& gfpath) const;
  bool checkMIPAltDateCell(const GroupFarePath& gfpath) const;
  bool currencyMatchingNeeded() const;

  bool startMultiPaxShortCkt();

  bool inhibitDivideParty(const GroupFarePath* gfpath) const;

  void getSortedFC(const FarePath& fpath, std::map<uint16_t, const PaxTypeFare*>& sortedFC) const;

  void setAltDateOptionFound(const GroupFarePath& gfpath) const;

  void disableFurtherCmdPrcSolution() const;

  void restoreCmdPrcInfo() const;
  void setAltDateOptionFound(Itin& itin) const;

  void prepareInfantFactoryIndicators();

  bool findNonDividePartySolution(const GroupFarePath& gfp) const;
  bool getAllDataFromGFP(const GroupFarePath& gfp,
                         const PaxType& primaryPaxType,
                         std::vector<PaxType*>& ptVec,
                         std::vector<MoneyAmount>& paxTotalAmountVec,
                         std::vector<FarePath*>& farePathVec) const;
  void getAllFPBookingCodes(const std::vector<FarePath*>& farePathVec,
                            std::vector<std::vector<BookingCode>>& allPaxTypesBookingCodeVec) const;
  bool findBookingCodesinFPF(FarePathFactory* fpf,
                             const MoneyAmount& topAmount,
                             const std::vector<BookingCode>& desiredBookingCode) const;
  void findBookingCodesInFM(const FareMarket* fareMarket,
                            const Itin* itin,
                            std::vector<BookingCode>& fmBookingCodes,
                            const std::vector<BookingCode>& desiredBookingCode) const;

  PricingTrx& _trx;
  AccompaniedTravel* _accompaniedTravel = nullptr;
  GroupFarePathPQ _groupFarePathPQ;

  uint32_t _totalFPFactory = 0;
  std::vector<PaxFarePathFactoryBase*> _paxFarePathFactoryBucket; // one factory per PaxType
  std::vector<char> _infantFactoryInd;

  time_t _startTime = time(nullptr);
  int32_t _multiPaxShortCktTimeOut = 30; // in number of sec

  bool _done = false;
  bool _usePooling = true;
  bool _throughFarePricing = false;
  bool _isXoRequest = false;
  bool _isXcRequest = false;
  bool _matchCurrency = false;
  bool _runningMultiPaxShortCktLogic = false;
  int32_t _numTaxCallForMipAltDate = 2;

  bool isGroupFarePathValidForSinglePaxType(const GroupFarePath& gfpath) const;
  bool isGroupFarePathValidForMultiPaxType(GroupFarePath& gfpath);
  MoneyAmount getTax(Itin& itin, FarePath* farePath) const;
  void printShutdownMessage(DiagCollector& diag) const;
  bool isValidGFPForValidatingCxr(GroupFarePath& gfpath, DiagCollector& diag) const;

  static Logger _logger;
};

} // tse namespace
