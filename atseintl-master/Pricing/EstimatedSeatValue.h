/*---------------------------------------------------------------------------
 *  File:    EstimatedSeatValue.h
 *  Created: Jan 10, 2008
 *  Authors:
 *
 *  Change History:
 *
 *  Copyright Sabre 2003
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "Common/ShoppingUtil.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Pricing/ESVPQDiversifier.h"

#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <vector>

#ifndef M_PI
#define M_PI 3.1415926535897932385
#endif

#ifndef SECONDS_PER_MINUTE
#define SECONDS_PER_MINUTE 60
#endif

#ifndef MINUTES_PER_DAY
#define MINUTES_PER_DAY 1440
#endif

const int outLegIdx = 0;
const int inLegIdx = 1;

namespace tse
{
class ESVPQ;
class ESVPQItem;
class Diag956Collector;
class Diag957Collector;
class Diag958Collector;
class EstimatedSeatValue;
class InterlineTicketCarrier;
class PQItemCompare;
class PQItemReverseCompare;

class ESVSopWrapper final
{
  friend class ESVPQDiversifierTest;

public:
  enum SOPFareListType
  { OW,
    RT,
    OJ,
    CT };

  void init(PaxType* paxType,
            ShoppingTrx::SchedulingOption* sop,
            SOPFareListType sopFareListType,
            int sopFareListIdx,
            std::vector<PaxTypeFare*>* paxTypeFareVec,
            const SOPFarePath* sopFarePath);

  ShoppingTrx::SchedulingOption*& sop() { return _sop; }
  const ShoppingTrx::SchedulingOption* sop() const { return _sop; }

  const SOPFareListType getSopFareListType() const { return _sopFareListType; }

  void setTotalAmt();

  const MoneyAmount getTotalAmt() const { return _totalAmt; }

  std::vector<PaxTypeFare*>* paxTypeFareVec() { return _paxTypeFareVec; }
  const std::vector<PaxTypeFare*>* paxTypeFareVec() const { return _paxTypeFareVec; }

  const bool haveCat10Restrictions() const { return _haveCat10Restrictions; }

  const SOPFarePath::CombinationType getCombinationType() const { return _combinationType; }

  double& outboundUtility() { return _outboundUtility; }
  const double getOutboundUtility() const { return _outboundUtility; }

  const int nestID()
  {
    if (-1 == _nestID)
    {
      _nestID = (int)floor(_sop->itin()->getDepartTime().totalSeconds() / SECONDS_PER_MINUTE / 120);
    }

    return _nestID;
  }

private:
  PaxType* _paxType = nullptr;
  ShoppingTrx::SchedulingOption* _sop = nullptr;
  SOPFareListType _sopFareListType = SOPFareListType::OW;
  int _sopFareListIdx = -1;
  std::vector<PaxTypeFare*>* _paxTypeFareVec = nullptr;
  bool _haveCat10Restrictions = true;
  SOPFarePath::CombinationType _combinationType = SOPFarePath::CombinationType::NOT_INITIALIZED;
  MoneyAmount _totalAmt = 0.0;

  double _outboundUtility = 0.0;
  int _nestID = -1;
};

class ESVPQItemWrapper final
{
public:
  ESVPQItemWrapper() = default;

  ESVPQItemWrapper(std::vector<int>& indices) : _indices(indices) {}

  void init(const int expandColumn, const std::vector<int>& indices);

  ESVPQItem*& esvPqItem() { return _esvPqItem; }
  const ESVPQItem* esvPqItem() const { return _esvPqItem; }

  std::vector<int>& indices() { return _indices; }
  const std::vector<int>& indices() const { return _indices; }

  int& expandColumn() { return _expandColumn; }
  const int& expandColumn() const { return _expandColumn; }

  MoneyAmount& totalAmt() { return _totalAmt; }
  const MoneyAmount& totalAmt() const { return _totalAmt; }

protected:
  ESVPQItem* _esvPqItem = nullptr;
  std::vector<int> _indices;
  int _expandColumn = 0;
  MoneyAmount _totalAmt = 0.0;
};

class EstimatedSeatValue final
{
  friend class EstimatedSeatValueTest;

public:
  enum VisBucket
  { CR,
    TB,
    ET,
    UV,
    NS,
    LFS,
    SI };

  using SopVectorsPair = std::pair<std::vector<ShoppingTrx::SchedulingOption*>,
                                   std::vector<ShoppingTrx::SchedulingOption*>>;
  static log4cxx::LoggerPtr _logger;

public:
  static void classInit();

  static bool isValidInterline(ShoppingTrx& trx, InterlineTicketCarrier* itc, Itin* outItin);

  static bool
  isValidInterline(ShoppingTrx& trx, InterlineTicketCarrier* itc, Itin* outItin, Itin* inItin);

  static bool checkCarrier(ShoppingTrx& trx,
                           CarrierCode& validatingCarrier,
                           InterlineTicketCarrier* itc,
                           std::set<CarrierCode>& cxrs);

  static bool checkNumberOfStops(ShoppingTrx::SchedulingOption*, QueueType, int);
  static bool checkNumberOfStops(ESVPQItem*, QueueType, std::string&, ShoppingTrx& _trx);
  static bool checkOnlineConfirmity(ShoppingTrx::SchedulingOption*, QueueType);
  static bool checkOnlineConfirmity(ESVPQItem*, QueueType, std::string&, ShoppingTrx& _trx);

public:
  EstimatedSeatValue(ShoppingTrx& trx);
  void process();
  void diagDisplay957(std::vector<ESVPQItem*>& esvPQItemVec);
  void diagDisplay956(ESVPQ*);
  void diagDisplay956AddInfo(const std::string&);
  void diagDisplayAllSopsInLegs(
      std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>> outPerCarrierMap,
      std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>> inbPerCarrierMap);

private:
  ShoppingTrx& _trx;
  ESVPQDiversifier* _esvDiv = nullptr;
  std::vector<ESVPQItem*> _pickedItems;
  Diag956Collector* _diag956Collector = nullptr;
  Diag957Collector* _diag957Collector = nullptr;
  Diag958Collector* _diag958Collector = nullptr;
  bool _newLogicEnabled = false;
  int _outSize = -1;
  int _inSize = -1;
  int _numFound = 0;
  bool _logEnabled = false;
  const std::vector<double>* _paramBetaOut = nullptr;
  const std::vector<double>* _paramBetaIn = nullptr;
  bool _oneWay;
  static std::set<CarrierCode> _onlineOnlyCxr;
  static std::map<CarrierCode, std::set<CarrierCode>> _restrictedCxr;

  void generateSolutions(PaxType*);
  void generateMustPriceSolutions(
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      PaxType*);
  void
  generateLFSSolutions(const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
                       const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
                       PaxType*);

  void generatePerCarrierSopVectorsMaps(
      std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      PaxType*);
  std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>
  generatePerCarrierSingleSopVectorsMap(std::vector<ShoppingTrx::Leg>::iterator legIter);
  std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>
  generateDummySingleSopVectorsMap(
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap);
  std::map<CarrierCode, SopVectorsPair> generatePerCarrierSopVectorsPairs(
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap);
  std::vector<SopVectorsPair> generateSopVectorsPairs(
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap);
  SopVectorsPair
  filterSopVectorsPairForQueue(const std::vector<ShoppingTrx::SchedulingOption*>& outVec,
                               const std::vector<ShoppingTrx::SchedulingOption*>& inbVec,
                               QueueType qType);
  std::vector<ShoppingTrx::SchedulingOption*>
  filterSingleSopVectorForQueue(const std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                                int legNo,
                                QueueType qType);

  void generatePerCarrierOnlineSolutions(
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      PaxType*,
      QueueType);
  void generatePerCarrierInterlineSolutions(
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      PaxType*,
      QueueType);
  void generateOverallOnlineSolutions(
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      PaxType*,
      QueueType);
  void generatePerCarrierLowFareSolutions(
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      PaxType*,
      QueueType);
  void generateRemainingSolutions(
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>&,
      PaxType*,
      QueueType);
  void generateRemainingOneWaySolutions(std::vector<SopVectorsPair>& sVPairs,
                                        std::vector<ESVPQ>& esvpqVec,
                                        QueueType qType);
  void generateRemainingRoundTripSolutions(std::vector<SopVectorsPair>& sVPairs,
                                           std::vector<ESVPQ>& esvpqVec,
                                           QueueType qType);
  void generateSinglePassSolutions(ESVPQ& singlePassPQ,
                                   int numberOfReqSolutionsForThePass,
                                   QueueType qtype);
  void generateSinglePassSolutions(std::vector<ESVPQ>& singlePassPQVec,
                                   int numberOfReqSolutionsForThePass,
                                   QueueType qtype);

  std::string generatePQtitle(QueueType pqType, CarrierCode carrier = "");
  std::string generatePQDivOption(QueueType pqType);
  int calcLimitForCurrPass(QueueType pqType);

  std::map<int, std::pair<ESVPQ*, ESVPQItem*>>
  buildESVPQMarkupMap(std::vector<ESVPQ>& singlePassPQVec);
  void rebuildESVPQMarkupMap(std::map<int, std::pair<ESVPQ*, ESVPQItem*>>&, int&);
  ESVPQItem* getNextItemFromESVPQVec(std::vector<ESVPQ>& singlePassPQVec,
                                     std::map<int, std::pair<ESVPQ*, ESVPQItem*>>& ESVPQMarkupMap,
                                     int& lastChosenNum);

  bool checkIfPQItemCanBePicked(ESVPQItem* pqItem,
                                std::string&,
                                bool checkIfNotAlreadyPickedFlag = true);
  void groupSolutionsInFamilies(std::vector<ESVPQItem*>& esvPQItemVec);
  void groupSolutionsByGoverningCarrier(std::vector<ESVPQItem*>& esvPQItemVec);
  void writeSolutionsToOutput(std::vector<ESVPQItem*>& esvPQItemVec);
  void regroupSolutions(std::vector<ESVPQItem*>& esvPQItemVec);
  void sendSolutions(std::vector<ESVPQItem*>& pickedItems);
  std::string generatePqItemKey(ESVPQItem* esvPQItem);
  void matchBookingInfo(ESVOption& esvOption, uint32_t legId);

  void newLogic();

  void selectLowFareAllOneWay(const std::vector<ShoppingTrx::SchedulingOption*>& allOut,
                              std::vector<ESVPQItem*>& pickedItins,
                              const int lfsNum,
                              std::vector<ESVPQItem*>& remainingFlights);

  void selectLowFareAllRoundTrip(const std::vector<ShoppingTrx::SchedulingOption*>& allOut,
                                 const std::vector<ShoppingTrx::SchedulingOption*>& allIn,
                                 std::vector<ESVPQItem*>& pickedItins,
                                 const int lfsNum,
                                 std::vector<ESVSopWrapper*>& inWrapVec);

  void copySOPptr(std::vector<ShoppingTrx::SchedulingOption>& vecSOP,
                  std::vector<ShoppingTrx::SchedulingOption*>& vecSOPptr);

  void selectInitialOBSet(const std::vector<ShoppingTrx::SchedulingOption*>& allOut,
                          const std::vector<ESVSopWrapper*>& inWrapVec,
                          std::vector<ESVPQItem*>& selectedFlights,
                          std::vector<ESVPQItem*>& reminigFlights,
                          const short noOfOptions,
                          const MoneyAmount cheapestItinerary);

  void preparePriorityVector(std::vector<std::pair<VisBucket, short>>& prioritiesVector,
                             const int legIdx);

  void findLFSforAllOut(const std::vector<ShoppingTrx::SchedulingOption*>& allOut,
                        const std::vector<ESVSopWrapper*>& inWrapVec,
                        std::vector<ESVPQItem*>& reminigFlights);

  ESVPQItem* getCheapestOption(const std::vector<ShoppingTrx::SchedulingOption*>& outVec,
                               const std::vector<ESVSopWrapper*>& inWrapVec);

  short selectByCarrier(std::vector<ESVPQItem*>& reminigFlights);

  short selectByNumStops(std::vector<ESVPQItem*>& remainingFlights,
                         const int legIdx,
                         const int outIndex,
                         const MoneyAmount cheapestItin);

  short selectByFlightTime(std::vector<ESVPQItem*>& remainingFlights,
                           const int legIdx,
                           const int outIndex);

  short selectByUtilityValue(std::vector<ESVPQItem*>& remainingFlights,
                             const int legIdx,
                             const int outIndex);

  short selectByIncrementalValue(std::vector<ESVPQItem*>& remainingFlights,
                                 const int legIdx,
                                 const int outIndex,
                                 std::vector<ESVPQItem*>& selectedFlights,
                                 const short noOfOptions);

  short
  selectByTimeBucket(std::vector<ESVPQItem*>& reminigFlights, const int legIdx, const int outIndex);

  void finalizeBucketsSelection(std::vector<ESVPQItem*>& remainingFlights,
                                const int legIdx,
                                std::vector<ESVPQItem*>& selectedFlights,
                                const short noOfOptions);

  void calculateIV(ESVPQItem* tempFlight,
                   const std::vector<ESVPQItem*>& selectedFlights,
                   const std::vector<double>* paramBeta,
                   const int legIdx,
                   const std::vector<double>& DT,
                   const std::vector<bool>& existDT);

  void prepareDTVectors(const std::vector<ESVPQItem*>& selectedFlights,
                        const int legIdx,
                        std::vector<double>& DT,
                        std::vector<bool>& existDT);

  void selectInitialIBSet(const int outIndex,
                          std::vector<ESVPQItem*>& remainingFlights,
                          std::vector<ESVPQItem*>& selectedFlights,
                          const std::vector<ShoppingTrx::SchedulingOption*>& allIn,
                          const short noOfOptions,
                          const MoneyAmount cheapestItinerary,
                          const ESVPQItem* obItin);

  void initRemainingFlights(const ESVPQItem* obItin,
                            const std::vector<ShoppingTrx::SchedulingOption*>& allIn,
                            std::vector<ESVPQItem*>& remainingFlights);

  short addLFSOptionsToFlightsSelection(const std::vector<ESVPQItem*>& lfsSolutions,
                                        const int legIdx,
                                        const ESVPQItem* obItin,
                                        std::vector<ESVPQItem*>& selectedFlights);

  bool findFlightInPQItems(const ESVPQItem* pqItem,
                           const int legIdx,
                           std::vector<ESVPQItem*>& selectedFlights);

  short selectSimpleInterlines(std::vector<ESVPQItem*>& remainingFlights,
                               const int outIndex,
                               const ESVPQItem* outboundItin);

  short selectLFSSolutions(std::vector<ESVPQItem*>& remainingFlights, const int outIndex);

  short markPQItem(ESVPQItem* pqItem, const int priority, const std::string& selectionSource) const;

  bool validBeta(const std::vector<double>* paramBeta, const bool enableMsg = false);
  const std::vector<double>* getParameterBeta(const int& timeDiff,
                                              const int& mileage,
                                              const char& direction,
                                              const char& APSatInd);
  VISTimeBin* getTimeBin(const ESVPQItem* pqItem, const int legIdx) const;
  void calculateFlightTimeMinutes(const int origDestOffset);
  void calculateFlightTimeMinutes(const int origDestOffset, const int legIdx);
};

class SOPWrapperCompare
{
public:
  bool operator()(ESVSopWrapper* lhs, ESVSopWrapper* rhs) const
  {
    return lhs->getTotalAmt() < rhs->getTotalAmt();
  }
};

class PQItemCompare
{
public:
  bool operator()(ESVPQItemWrapper* lhs, ESVPQItemWrapper* rhs)
  {
    return ((lhs->totalAmt() - rhs->totalAmt()) > EPSILON);
  }
};

class PQItemReverseCompare
{
public:
  bool operator()(ESVPQItem* lhs, ESVPQItem* rhs);
};

class PQItemGroupCompare
{
public:
  bool operator()(ESVPQItem* lhs, ESVPQItem* rhs);
};

class ESVComparator
{
public:
  ESVComparator(const int legIdx) : _legIdx(legIdx) {}
  bool operator()(ESVPQItem* lhs, ESVPQItem* rhs) const;

protected:
  const int _legIdx;
};

class FirstCarrierESVComparator : public ESVComparator
{
public:
  FirstCarrierESVComparator(const int legIdx) : ESVComparator(legIdx) {}

  bool operator()(ESVPQItem* lhs, ESVPQItem* rhs) const;
};

class NumstopsESVComparator : public ESVComparator
{
public:
  NumstopsESVComparator(const int legIdx) : ESVComparator(legIdx) {}

  bool operator()(ESVPQItem* lhs, ESVPQItem* rhs) const;
};

class ElapsedComparator : public ESVComparator
{
public:
  ElapsedComparator(const int legIdx) : ESVComparator(legIdx) {}

  bool operator()(ESVPQItem* lhs, ESVPQItem* rhs) const;
};

class UtilityValueESVComparator : public ESVComparator
{
public:
  UtilityValueESVComparator(const int legIdx) : ESVComparator(legIdx) {}

  bool operator()(ESVPQItem* lhs, ESVPQItem* rhs) const;
};

class PriorityESVComparator : public ESVComparator
{
public:
  PriorityESVComparator(const int legIdx) : ESVComparator(legIdx) {}

  bool operator()(ESVPQItem* lhs, ESVPQItem* rhs) const;
};

class IncrementalValueESVComparator : public ESVComparator
{
public:
  IncrementalValueESVComparator(const int legIdx) : ESVComparator(legIdx) {}

  bool operator()(ESVPQItem* lhs, ESVPQItem* rhs) const;
};

class PriorityComparator
{
public:
  bool operator()(const std::pair<EstimatedSeatValue::VisBucket, short>& lhs,
                  const std::pair<EstimatedSeatValue::VisBucket, short>& rhs) const
  {
    return lhs.second < rhs.second;
  }
};
} // tse namepace
