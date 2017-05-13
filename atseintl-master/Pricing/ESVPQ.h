//----------------------------------------------------------------------------
//  File:        ESVPQ.h
//  Created:     2009-05-25
//
//  Description: ESV priority queue
//
//  Updates:
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
//----------------------------------------------------------------------------

#pragma once

#include "Pricing/EstimatedSeatValue.h"

namespace tse
{

class Diag956Collector;
class InterlineTicketCarrier;

class ESVPQ
{

  friend class ESVPQItem;
  friend class SOPCompare;
  friend class SOPWrapperCompare;
  friend class ESVTest;

public:
  ESVPQ(ShoppingTrx* trx, std::string title = "Solutions Priority Queue");

  virtual ~ESVPQ() = default;

  void init(const std::vector<ShoppingTrx::SchedulingOption*>& outSopVec,
            const std::vector<ShoppingTrx::SchedulingOption*>& inSopVec,
            const QueueType queueType,
            const CarrierCode& carrier,
            const std::string& diversityOption);

  void init(const std::vector<ShoppingTrx::SchedulingOption*>& outSopVec,
            const std::vector<ESVSopWrapper*>& inSopWrapVec,
            const QueueType queueType,
            const CarrierCode& carrier,
            const std::string& diversityOption);

  void init(const std::vector<ESVSopWrapper*>& outSopWrapVec,
            const std::vector<ShoppingTrx::SchedulingOption*>& inbSopVec,
            const QueueType queueType,
            const CarrierCode& carrier,
            const std::string& diversityOption);

  void init(const std::vector<ESVSopWrapper*>& outSopWrapVec,
            const std::vector<ESVSopWrapper*>& inSopWrapVec,
            const QueueType queueType,
            const CarrierCode& carrier,
            const std::string& diversityOption);

  ESVPQItem* getNextItem(Diag956Collector*, const bool oneItemOnly = false);

  std::string& title() { return _title; }
  const std::string& title() const { return _title; }

  std::string& diversityOption() { return _diversityOption; }
  const std::string& diversityOption() const { return _diversityOption; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  QueueType& queueType() { return _queueType; }
  const QueueType& queueType() const { return _queueType; }

  std::vector<const std::vector<ESVSopWrapper*>*>& legVec() { return _legVec; }
  const std::vector<const std::vector<ESVSopWrapper*>*>& legVec() const { return _legVec; }

  static std::string getQueueTitle(QueueType qT);

  InterlineTicketCarrier*& interlineTicketCarrier() { return _interlineTicketCarrier; }
  const InterlineTicketCarrier* interlineTicketCarrier() const { return _interlineTicketCarrier; }

private:
  void setupAddPenaltyFlag();

  void generateNextPQItemSet(ESVPQItemWrapper* pqItemWrapper);

  bool preValidateCat10(ESVPQItemWrapper* pqItemWrapper);

  void setTotalAmt(ESVPQItemWrapper* pqItemWrapper);

  void initLegVec(const std::vector<ShoppingTrx::SchedulingOption*>& outSopVec,
                  const std::vector<ShoppingTrx::SchedulingOption*>& inSopVec);

  void initLegVec(const std::vector<ShoppingTrx::SchedulingOption*>& outSopVec,
                  const std::vector<ESVSopWrapper*>& inSopWrapVec);

  void initLegVec(const std::vector<ESVSopWrapper*>& outSopWrapVec,
                  const std::vector<ShoppingTrx::SchedulingOption*>& inbSopVec);

  void initLegVec(const std::vector<ESVSopWrapper*>& outSopWrapVec,
                  const std::vector<ESVSopWrapper*>& inSopWrapVec);

  void pushFirstItemIntoPQ();

  bool allLegsNotEmpty();

  ESVPQItemWrapper* generateFirstItem();

  void fillInLegVec(std::vector<ShoppingTrx::SchedulingOption*>& outVec,
                    std::vector<ShoppingTrx::SchedulingOption*>& inVec);

  void expandVecInLegVec(std::vector<ShoppingTrx::SchedulingOption*>& sopVec, const int legIdx);

  void expandSOP(std::vector<ESVSopWrapper*>& sopWrapperVec, ShoppingTrx::SchedulingOption* sop);

  void expandSopFarePaths(std::vector<ESVSopWrapper*>& sopWrapperVec,
                          ShoppingTrx::SchedulingOption* sop,
                          std::vector<SOPFarePath*>& sopFarePaths,
                          ESVSopWrapper::SOPFareListType sopFareListType);

  void sortSopWrapperVec(std::vector<ESVSopWrapper*>& sopWrapperVec);

  bool validPU(ESVPQItem* pqItem, std::string* addInfoPtr);

  bool alreadyCreated(uint32_t& sopCombinationId, std::string* addInfoPtr);

  void addToCreated(uint32_t& sopCombinationId, std::string* addInfoPtr);

  bool checkMinimumConnectTime(const ESVPQItem* pqItem) const;

  bool checkCat10(ESVPQItem* pqItem);

  bool haveCat10Restrictions();

  bool isOnline();

  bool checkCIRCLETRIP();

  bool checkROUNDTRIP();

  bool checkONEWAY();

  bool checkOPENJAW();

  bool check3componentOJ(const std::vector<PaxTypeFare*>& outPaxTypeVec,
                         const std::vector<PaxTypeFare*>& inPaxTypeVec,
                         const SOPFarePath::CombinationType outCombType,
                         const SOPFarePath::CombinationType inCombType);

  bool check4componentOJ(const std::vector<PaxTypeFare*>& outPaxTypeVec,
                         const std::vector<PaxTypeFare*>& inPaxTypeVec,
                         const SOPFarePath::CombinationType outCombType,
                         const SOPFarePath::CombinationType inCombType);

  bool checkMileage(const PaxTypeFare* oj1, const PaxTypeFare* oj2, const PaxTypeFare* connFare);

  bool checkMileage(const PaxTypeFare* oj1, const PaxTypeFare* oj2, const uint16_t connMileage);

  uint16_t mileage(const PaxTypeFare* fare);

  void clearInternalData();

  std::string _title;
  std::string _diversityOption;
  CarrierCode _carrier;
  QueueType _queueType = QueueType::UnknownQueueType;
  ShoppingTrx* _trx = nullptr;
  log4cxx::LoggerPtr& _logger;
  PaxType* _paxType = nullptr;
  PQItemCompare _pqItemComp;
  std::priority_queue<ESVPQItemWrapper*, std::vector<ESVPQItemWrapper*>, PQItemCompare> _pqItemPQ;
  std::vector<ESVSopWrapper*> _legVecOut;
  std::vector<ESVSopWrapper*> _legVecIn;
  std::vector<const std::vector<ESVSopWrapper*>*> _legVec;
  std::set<uint32_t> _alreadyCreatedSOPCombinations;
  ESVSopWrapper* _sopWprapperOut = nullptr;
  ESVSopWrapper* _sopWprapperIn = nullptr;
  std::string _addInfo;
  bool _addPenalty = false;
  InterlineTicketCarrier* _interlineTicketCarrier = nullptr;
};
} // End namespace tse

