// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Pricing/Shopping/PQ/AltDatesStatistic2.h"

#include "Common/Assert.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseDateTimeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/SopVecOutputDecorator.h"
#include "Pricing/Shopping/Diversity/DiversityUtil.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloADGroupFarePath.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iterator>
#include <map>
#include <sstream>

#include <boost/optional.hpp>

namespace tse
{
static Logger
_logger("atseintl.ShoppingPQ.AltDatesStatistic2");

/**
 * This auxiliary class is to detect min/max value on an option removal
 * by counting options that are equal by some Key
 */
template <class K>
class MinMaxInstanceTracker
{
public:
  void incInstanceCount(const K& k)
  {
    std::pair<typename Map::iterator, bool> insertResult(_map.insert(std::make_pair(k, 1)));
    if (!insertResult.second)
    {
      Counter& occurCount = insertResult.first->second;
      ++occurCount;
    }
  }
  void decInstanceCount(const K& k)
  {
    typename Map::iterator findIt = _map.find(k);
    if (findIt == _map.end() || (findIt->second <= 0))
    {
      LOG4CXX_ERROR(_logger, "Data discrepancy");
      return;
    }

    Counter& occurCount = findIt->second;
    if (--occurCount == 0)
    {
      _map.erase(findIt);
    }
  }
  const K& getMin() const
  {
    TSE_ASSERT(!_map.empty());

    return _map.begin()->first;
  }
  const K& getMax() const
  {
    TSE_ASSERT(!_map.empty());

    return (*--_map.end()).first;
  }
  bool isEmpty() const { return _map.empty(); }

private:
  typedef unsigned Counter;
  typedef std::map<K, Counter> Map;
  Map _map;
};

/**
 * This context is used in diag 942 only
 */
class AltDatesStatistic2::FareLevelStatNodeSharedCtx
{
public:
  const FareLevel& getMaxFareLevel() const { return _detectMaxFareLevel.getMax(); }

private:
  friend class FareLevelAuxStatNode;
  MinMaxInstanceTracker<FareLevel> _detectMaxFareLevel;
};

class AltDatesStatistic2::StatNodeFactory
{
public:
  StatNodeFactory(AltDatesStatistic2& ctx) : _ctx(ctx) {}

  StatNode* createNode(uint8_t level);

private:
  AltDatesStatistic2& _ctx;
  FareLevelStatNodeSharedCtx _flNodeCtx;
};

struct AltDatesStatistic2::DumpToDiagCtx
{
  std::ostream& _dc;
  const std::set<DatePair>* _datePairsToFulfill;
  const bool _displaySops;

  DumpToDiagCtx(std::ostream& dc,
                const std::set<DatePair>* datePairsToFulfill = nullptr,
                bool displaySops = false)
    : _dc(dc), _datePairsToFulfill(datePairsToFulfill), _displaySops(displaySops)
  {
  }
};

/**
 * StatNode organize descendant nodes into tree structure (where the key is date pair/fare
 * level/etc.)
 */
class AltDatesStatistic2::StatNode
{
public:
  struct Request
  {
    MoneyAmount _price;
    DatePair _datePair;
    boost::optional<CxrSPInfo> _cxrSPInfo;
    boost::optional<std::size_t> _travelSegCount;
    const shpq::SopIdxVec* _sops; // is used for same connection city check only

    Request(MoneyAmount price, DatePair datePair) : _price(price), _datePair(datePair), _sops(nullptr) {}
    Request(MoneyAmount price, DatePair datePair, CxrSPInfo cxrSPInfo)
      : _price(price), _datePair(datePair), _cxrSPInfo(cxrSPInfo), _sops(nullptr)
    {
    }
    Request(Carrier cxr,
            MoneyAmount price,
            DatePair datePair,
            bool snowman,
            std::size_t tsCount,
            const shpq::SopIdxVec* sops)
      : _price(price),
        _datePair(datePair),
        _cxrSPInfo(CxrSPInfo(cxr, snowman)),
        _travelSegCount(tsCount),
        _sops(sops)
    {
    }

    // workaround to elaborate performance in FareLevelStatNode::detectMinFareLevelForSolKind
    void setIgnoreTravelSegCount() { _travelSegCount.reset(); }
    void setIgnoreSameConnectionCityCheck() { _sops = nullptr; }
  };
  struct Response
  {
    uint16_t _fareLevel;
    uint16_t _numOptions;
    MoneyAmount _firstFareLevelAmount; // -1 if not exists

    // Fare level ordinal number, _govCarrier has been seen the first time in
    uint16_t _firstCxrFareLevel;

    shpq::SopIdxVec _sameConnectingCitySops; // empty vec unless the same connecting city detected
    boost::optional<std::size_t> _minTravelSegCount;

    Response() : _fareLevel(0), _numOptions(0), _firstFareLevelAmount(-1.), _firstCxrFareLevel(0) {}

    Stat makeStat(const Request& req) const
    {
      Stat stat(_fareLevel,
                _numOptions,
                _firstFareLevelAmount,
                _firstCxrFareLevel,
                _minTravelSegCount,
                _sameConnectingCitySops);
      if (LIKELY(req._cxrSPInfo))
      {
        stat._govCarrier = req._cxrSPInfo->_carrier;
        stat._solKind =
            DiversityUtil::getSolutionKind(req._cxrSPInfo->_carrier, req._cxrSPInfo->_isSnowman);
      }
      return stat;
    }
  };

  StatNode(AltDatesStatistic2& ctx, uint8_t level) : _ctx(ctx), _level(level) {}
  virtual ~StatNode() {}

  virtual void addSolution(const FlightMatrixSolution& solution, const DatePair& datePair) = 0;
  virtual void query(const Request& request, Response& response) = 0;
  /**
   * Rebuild a node statistic upon solution removal. The solution has to be present in the node
   * or it's children nodes.
   *
   * @return true signals that the node is empty and a parent can make a decision on it's removal
   */
  virtual bool removeSolution(const FlightMatrixSolution& solution, const DatePair& datePair) = 0;

  /**
   * friendly overload
   */
  Response query(const Request& request)
  {
    Response response;
    query(request, response);
    return response;
  }

  virtual void dumpTo(DumpToDiagCtx&) = 0;

  void dumpTo(std::ostream& os)
  {
    DumpToDiagCtx ctx(os);
    dumpTo(ctx);
  }

protected:
  AltDatesStatistic2& _ctx;
  const uint8_t _level;

private:
  StatNode(const StatNode&); // non-copiable
};

class AltDatesStatistic2::DatePairStatNode : public AltDatesStatistic2::StatNode
{
public:
  DatePairStatNode(AltDatesStatistic2& ctx, uint8_t level) : StatNode(ctx, level) {}
  /**
   * @override
   */
  void addSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    getNextLevelNode(datePair).addSolution(solution, datePair);
  }
  /**
   * @override
   */
  void query(const Request& request, Response& response) override
  {
    std::map<DatePair, StatNode*>::iterator datePairIt = _map.find(request._datePair);
    if (datePairIt != _map.end())
      datePairIt->second->query(request, response);
  }
  /**
   * @override
   */
  bool removeSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    std::map<DatePair, StatNode*>::iterator datePairIt = _map.find(datePair);
    if (datePairIt == _map.end())
    {
      LOG4CXX_ERROR(_logger, "Remove solution date pair does not exist");
      return false;
    }

    if (datePairIt->second->removeSolution(solution, datePair))
    {
      LOG4CXX_ERROR(_logger,
                    "With current StatNodeFactory config DatePair removal should not happen");
      _map.erase(datePairIt);
    }

    // nobody checks this node result, so it just to conform with the contract
    return _map.empty();
  }
  /**
   * @override
   */
  void dumpTo(DumpToDiagCtx& diagCtx) override
  {
    std::ostream& dc = diagCtx._dc;

    for (const ShoppingTrx::AltDatePairs::value_type& trxDatePairStruct : _ctx._trx.altDatePairs())
    {
      const DatePair& datePair = trxDatePairStruct.first;

      std::string datePairToFulfillMark = "";
      if (diagCtx._datePairsToFulfill != nullptr)
      {
        datePairToFulfillMark = diagCtx._datePairsToFulfill->count(datePair) ? "++" : "  ";
      }

      dc << datePairToFulfillMark << datePair.first.dateToIsoExtendedString() << " "
         << datePair.second.dateToIsoExtendedString() << " : ";

      std::map<DatePair, StatNode*>::iterator datePairIt = _map.find(datePair);
      if (datePairIt != _map.end())
        datePairIt->second->dumpTo(diagCtx);

      dc << "\n";
    }
  }

  bool isEmpty() const { return _map.empty(); }

private:
  std::map<DatePair, StatNode*> _map;

  StatNode& getNextLevelNode(DatePair key)
  {
    std::map<DatePair, StatNode*>::iterator nodeIt = _map.find(key);
    if (nodeIt == _map.end())
    {
      StatNode* nextLevelNode = _ctx._statNodeFactory->createNode(_level + 1);
      nodeIt = _map.insert(std::make_pair(key, nextLevelNode)).first;
    }

    return *nodeIt->second;
  }
};

class AltDatesStatistic2::FareLevelStatNode : public AltDatesStatistic2::StatNode
{
public:
  FareLevelStatNode(AltDatesStatistic2& ctx, FareLevelStatNodeSharedCtx& flCtx, uint8_t level)
    : StatNode(ctx, level), _nodeSharedCtx(flCtx)
  {
  }
  /**
   * @override
   */
  void addSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    MoneyAmount price = solution.second->getTotalNUCAmount();
    FareLevel fareLevel = _ctx.buildFareLevel(price);

    getNextLevelNode(fareLevel).addSolution(solution, datePair);
  }
  /**
   * @override
   */
  void query(const Request& request, Response& response) override
  {
    if (UNLIKELY(_map.empty()))
    {
      // The option from the request as for the price is going to be at the 1st fare level
      response._fareLevel = 1;
      return;
    }

    response._firstFareLevelAmount = _map.begin()->first.price();

    FareLevel fareLevel = _ctx.buildFareLevel(request._price);
    Map::iterator findIt = _map.find(fareLevel);
    if (findIt == _map.end())
    {
      response._fareLevel = detectFareLevelNum(fareLevel);
    }
    else
    {
      response._fareLevel = std::distance(_map.begin(), findIt) + 1;

      findIt->second->query(request, response);
    }

    response._firstCxrFareLevel = detectMinFareLevelForSolKind(request, response._fareLevel);
  }
  /**
   * @override
   */
  bool removeSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    MoneyAmount price = solution.second->getTotalNUCAmount();
    FareLevel fareLevel = _ctx.buildFareLevel(price);

    Map::iterator findIt = _map.find(fareLevel);
    if (findIt == _map.end())
    {
      LOG4CXX_ERROR(_logger, "Remove solution fare level does not exist");
      return false;
    }

    if (findIt->second->removeSolution(solution, datePair))
    {
      // when this happens, JIRA NGSII-197 is being addressed
      _map.erase(findIt);
    }

    return _map.empty();
  }
  /**
   * @override
   */
  void dumpTo(DumpToDiagCtx& diagCtx) override
  {
    std::ostream& dc = diagCtx._dc;

    Map::const_iterator fareLevelIt = _map.begin();
    for (unsigned fareLevel = 1; fareLevelIt != _map.end(); ++fareLevel, ++fareLevelIt)
    {
      dc << "\n  FARE LEVEL " << fareLevel << " [ ";

      // print fare level price
      dc << std::setw(7) << std::fixed << std::setprecision(2);
      if (fareLevelIt != _map.end())
        dc << fareLevelIt->first.price();
      else
        dc << 0.;
      //

      // print max fare level price mark (as for the legend)
      if (fareLevelIt->first == _nodeSharedCtx.getMaxFareLevel())
      {
        dc << "*";
      }
      else
      {
        dc << " ";
      }
      //

      dc << "] : ";

      fareLevelIt->second->dumpTo(diagCtx);
    }
  }

private:
  typedef std::map<FareLevel, StatNode*> Map;
  FareLevelStatNodeSharedCtx& _nodeSharedCtx;
  Map _map;

  StatNode& getNextLevelNode(const FareLevel& key)
  {
    Map::iterator nodeIt = _map.find(key);
    if (nodeIt == _map.end())
    {
      StatNode* nextLevelNode = _ctx._statNodeFactory->createNode(_level + 1);
      nodeIt = _map.insert(std::make_pair(key, nextLevelNode)).first;
    }

    return *nodeIt->second;
  }

  uint16_t detectMinFareLevelForSolKind(const Request& request, uint16_t currentFareLevel)
  {
    Map::iterator fareLevelIter(_map.begin());

    Request reqForNumOptPerSolKind(request);
    reqForNumOptPerSolKind.setIgnoreTravelSegCount();
    reqForNumOptPerSolKind.setIgnoreSameConnectionCityCheck();

    Response response;

    for (uint16_t fareLevel = 1; fareLevel < currentFareLevel; ++fareLevel, ++fareLevelIter)
    {
      fareLevelIter->second->query(reqForNumOptPerSolKind, response);
      if (response._numOptions > 0)
        return fareLevel;
    }

    return currentFareLevel;
  }

  uint16_t detectFareLevelNum(const FareLevel& fl) const
  {
    uint16_t result = 1;
    for (const Map::value_type& nextFLItem : _map)
    {
      const FareLevel& nextFL = nextFLItem.first;
      if (fl <= nextFL)
        return result;

      ++result;
    }

    return result;
  }
};

/**
 * This is an auxiliary statistic node per each fare level. This one responsibilities are:
 *
 * - Detect max fare level, just to display appropriate mark in diagnostic;
 *
 * - Track minimum segment count per fare level, this statistic is needed for "special snowman"
 *   logic JIRA NGSII-335
 */
class AltDatesStatistic2::FareLevelAuxStatNode : public AltDatesStatistic2::StatNode
{
public:
  FareLevelAuxStatNode(AltDatesStatistic2& ctx, FareLevelStatNodeSharedCtx& flCtx, uint8_t level)
    : StatNode(ctx, level),
      _nextNode(ctx._statNodeFactory->createNode(level + 1)),
      _nodeSharedCtx(flCtx)
  {
  }
  /**
   * @override
   */
  void addSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    _nodeSharedCtx._detectMaxFareLevel.incInstanceCount(getFareLevel(solution));
    updateMinTravelSegCount(solution);

    _nextNode->addSolution(solution, datePair);
  }
  /**
   * @override
   */
  void query(const Request& request, Response& response) override
  {
    if (LIKELY(!_detectMinTravelSegCount.isEmpty()))
      response._minTravelSegCount = _detectMinTravelSegCount.getMin();

    _nextNode->query(request, response);
  }
  /**
   * @override
   */
  bool removeSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    _nodeSharedCtx._detectMaxFareLevel.decInstanceCount(getFareLevel(solution));
    updateMinTravelSegCount(solution, /*isRemoval*/ true);

    return _nextNode->removeSolution(solution, datePair);
  }
  /**
   * @override
   */
  void dumpTo(DumpToDiagCtx& diagCtx) override { _nextNode->dumpTo(diagCtx); }

private:
  StatNode* _nextNode;
  FareLevelStatNodeSharedCtx& _nodeSharedCtx;

  /**
   * Track number of options / travel segment count, the only necessity in keeping this
   * structure it to handle properly an option removal
   */
  MinMaxInstanceTracker<std::size_t> _detectMinTravelSegCount;

  FareLevel getFareLevel(const FlightMatrixSolution& solution) const
  {
    MoneyAmount price = solution.second->getTotalNUCAmount();
    const FareLevel& fareLevel = _ctx.buildFareLevel(price);
    return fareLevel;
  }

  void updateMinTravelSegCount(const FlightMatrixSolution& solution, bool isRemoval = false)
  {
    std::size_t solTravelSegCount = DiversityUtil::getTravelSegCount(_ctx._trx, solution.first);
    if (!isRemoval)
      _detectMinTravelSegCount.incInstanceCount(solTravelSegCount);
    else
      _detectMinTravelSegCount.decInstanceCount(solTravelSegCount);
  }
};

/**
 * Group by ADSolultionKind and by carrier code (for online solutions only)
 */
class AltDatesStatistic2::SolKindStatNode : public AltDatesStatistic2::StatNode
{
public:
  SolKindStatNode(AltDatesStatistic2& ctx, uint8_t level) : StatNode(ctx, level) {}

  /**
   * @override
   */
  void addSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    Carrier cxr = SopCombinationUtil::detectCarrierPair(_ctx._trx, solution.first);

    // consider using a map[ sopComb => snowman ] to fixup performance
    DiversityUtil::ADSolultionKind kind = DiversityUtil::getSolutionKind(_ctx._trx, solution, cxr);
    getNextLevelNode(Key(kind, cxr)).addSolution(solution, datePair);
  }
  /**
   * @override
   */
  void query(const Request& request, Response& response) override
  {
    if (!request._cxrSPInfo)
      return;

    DiversityUtil::ADSolultionKind kind = DiversityUtil::getSolutionKind(
        request._cxrSPInfo->_carrier, request._cxrSPInfo->_isSnowman);
    Key k(kind, request._cxrSPInfo->_carrier);
    std::map<Key, StatNode*>::iterator findIt = _map.find(k);
    if (findIt != _map.end())
      findIt->second->query(request, response);
  }
  /**
   * @override
   */
  bool removeSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    Carrier cxr = SopCombinationUtil::detectCarrierPair(_ctx._trx, solution.first);

    // consider using a map[ sopComb => snowman ] to fixup performance
    DiversityUtil::ADSolultionKind kind = DiversityUtil::getSolutionKind(_ctx._trx, solution, cxr);
    Key k(kind, cxr);
    std::map<Key, StatNode*>::iterator findIt = _map.find(k);
    if (findIt == _map.end())
    {
      LOG4CXX_ERROR(_logger, "Remove solution carrier node does not exist");
      return false;
    }

    if (findIt->second->removeSolution(solution, datePair))
    {
      // when this happens, JIRA NGSII-197 is being addressed
      _map.erase(findIt);
    }

    return _map.empty();
  }
  /**
   * @override
   */
  void dumpTo(DumpToDiagCtx& ctx) override
  {
    std::ostream& dc = ctx._dc;

    const char* sep = "";
    typedef std::pair<Key, StatNode*> Elem;
    for (const Elem& elem : _map)
    {
      dc << sep;

      printSolKind(dc, elem.first);
      dc << " ";
      elem.second->dumpTo(ctx);

      sep = ", ";
    }
  }

private:
  struct Key
  {
    DiversityUtil::ADSolultionKind _kind;
    Carrier _carrier;

    Key(DiversityUtil::ADSolultionKind kind, Carrier cxr = Carrier()) : _kind(kind)
    {
      // neglect carrier for snowman
      if (!(kind & DiversityUtil::Snowman))
        _carrier = cxr;
    }

    bool operator<(Key rhs) const
    {
      return (std::make_pair(_kind, _carrier) < std::make_pair(rhs._kind, rhs._carrier));
    }
  };
  std::map<Key, StatNode*> _map;

  StatNode& getNextLevelNode(Key k)
  {
    std::map<Key, StatNode*>::iterator nodeIt = _map.find(k);
    if (nodeIt == _map.end())
    {
      StatNode* nextLevelNode = _ctx._statNodeFactory->createNode(_level + 1);
      nodeIt = _map.insert(std::make_pair(k, nextLevelNode)).first;
    }

    return *nodeIt->second;
  }

  static void printSolKind(std::ostream& dc, Key k)
  {
    if (k._kind & DiversityUtil::Snowman)
      dc << k._kind;
    else
      dc << k._carrier;
  }
};

class AltDatesStatistic2::TravelSegCountStatNode : public AltDatesStatistic2::StatNode
{
public:
  TravelSegCountStatNode(AltDatesStatistic2& ctx, uint8_t level) : StatNode(ctx, level) {}
  /**
   * @override
   */
  void addSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    std::size_t segCount = DiversityUtil::getTravelSegCount(_ctx._trx, solution.first);

    getNextLevelNode(segCount).addSolution(solution, datePair);
  }
  /**
   * Count number of options for segment count <= request._travelSegCount.
   * Returns number of options for items with less segment count,
   *  if request._travelSegCount is not specified (= 0)
   *
   * @override
   */
  void query(const Request& request, Response& response) override
  {
    if (UNLIKELY(_map.empty()))
      return;

    std::size_t tsCountUpperLimit(request._travelSegCount ? *request._travelSegCount
                                                          : _map.begin()->first);

    for (std::map<std::size_t, StatNode*>::iterator segIt(_map.begin());
         segIt != _map.end() && segIt->first <= tsCountUpperLimit;
         ++segIt)
    {
      segIt->second->query(request, response);
    }
  }
  /**
   * @override
   */
  bool removeSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    std::size_t segCount = DiversityUtil::getTravelSegCount(_ctx._trx, solution.first);

    std::map<std::size_t, StatNode*>::iterator findIt = _map.find(segCount);
    if (findIt == _map.end())
    {
      LOG4CXX_ERROR(_logger, "Remove solution node does not exist");
      return false;
    }

    if (findIt->second->removeSolution(solution, datePair))
    {
      _map.erase(segCount);
    }

    return _map.empty();
  }
  /**
   * @override
   */
  void dumpTo(DumpToDiagCtx& diagCtx) override
  {
    std::ostream& dc = diagCtx._dc;

    const char* sep = "";
    typedef std::pair<std::size_t, StatNode*> Elem;
    for (const Elem& elem : _map)
    {
      dc << sep;

      dc << elem.first << "-seg ";
      elem.second->dumpTo(diagCtx);

      sep = " ";
    }
  }

private:
  std::map<std::size_t, StatNode*> _map;

  StatNode& getNextLevelNode(std::size_t segCount)
  {
    std::map<std::size_t, StatNode*>::iterator nodeIt = _map.find(segCount);
    if (nodeIt == _map.end())
    {
      StatNode* nextLevelNode = _ctx._statNodeFactory->createNode(_level + 1);
      nodeIt = _map.insert(std::make_pair(segCount, nextLevelNode)).first;
    }

    return *nodeIt->second;
  }
};

/**
 * The node also reports if it's options share the same connecting city with the option in the
 * request
 */
class AltDatesStatistic2::NumOptStatNode : public AltDatesStatistic2::StatNode
{
public:
  NumOptStatNode(AltDatesStatistic2& ctx, uint8_t level) : StatNode(ctx, level) {}
  /**
   * @override
   */
  void addSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    _sopCombVec.push_back(solution.first);
  }
  /**
   * @override
   */
  void query(const Request& request, Response& response) override
  {
    response._numOptions += numOpt();

    if (!request._sops)
      return;

    for (const shpq::SopIdxVec& comb : _sopCombVec)
    {
      if (sameConnectionCity(comb, *request._sops))
      {
        response._sameConnectingCitySops = comb;
        return;
      }
    }
  }
  /**
   * @override
   */
  bool removeSolution(const FlightMatrixSolution& solution, const DatePair& datePair) override
  {
    std::deque<shpq::SopIdxVec>::iterator findIt(
        std::find(_sopCombVec.begin(), _sopCombVec.end(), solution.first));
    if (findIt == _sopCombVec.end())
    {
      LOG4CXX_ERROR(_logger, "Solution to remove does not exist");
      return false;
    }

    _sopCombVec.erase(findIt);
    return _sopCombVec.empty();
  }
  /**
   * @override
   */
  void dumpTo(DumpToDiagCtx& ctx) override
  {
    ctx._dc << "[";
    if (ctx._displaySops)
    {
      const char* sep = "";
      for (const shpq::SopIdxVec& sopComb : _sopCombVec)
      {
        ctx._dc << sep;
        ctx._dc << "(" << SopVecOutputDecorator(_ctx._trx, sopComb) << ")";

        sep = " ";
      }
    }
    else
    {
      ctx._dc << numOpt();
    }
    ctx._dc << "]";
  }

private:
  std::deque<shpq::SopIdxVec> _sopCombVec;

  std::size_t numOpt() const { return _sopCombVec.size(); }

  bool sameConnectionCity(shpq::SopIdxVecArg sops1, shpq::SopIdxVecArg sops2)
  {
    return DiversityUtil::isSolutionSimilar(_ctx._trx, sops1, sops2);
  }
};

AltDatesStatistic2::StatNode*
AltDatesStatistic2::StatNodeFactory::createNode(uint8_t level)
{
  switch (level)
  {
  case 0:
    return &_ctx._trx.dataHandle().safe_create<DatePairStatNode>(_ctx, level);
    break;
  case 1:
    return &_ctx._trx.dataHandle().safe_create<FareLevelStatNode>(_ctx, _flNodeCtx, level);
    break;
  case 2:
    return &_ctx._trx.dataHandle().safe_create<FareLevelAuxStatNode>(_ctx, _flNodeCtx, level);
    break;
  case 3:
    return &_ctx._trx.dataHandle().safe_create<SolKindStatNode>(_ctx, level);
    break;
  case 4:
    return &_ctx._trx.dataHandle().safe_create<TravelSegCountStatNode>(_ctx, level);
    break;
  case 5:
    return &_ctx._trx.dataHandle().safe_create<NumOptStatNode>(_ctx, level);
    break;
  default:
    TSE_ASSERT(!"StatNodeFactory does not support node level");
    break;
  }
  return nullptr;
}

AltDatesStatistic2::AltDatesStatistic2(ShoppingTrx& trx, ItinStatistic& itinStat)
  : AltDatesStatistic(trx, itinStat),
    _statNodeFactory(&trx.dataHandle().safe_create<StatNodeFactory>(*this))
{
  _stat = &dynamic_cast<DatePairStatNode&>(*_statNodeFactory->createNode(0));
}

void
AltDatesStatistic2::addSolution(const AltDatesStatistic::FlightMatrixSolution& solution,
                                const DatePair& datePair)
{
  _stat->addSolution(solution, datePair);
}

bool
tse::AltDatesStatistic2::findAnyDatePair(MoneyAmount findUsingPQScoreStat,
                                         CxrSPInfo findUsingCxrSPInfoStat,
                                         const DatePairSkipCallback& skipCallback,
                                         const std::set<DatePair>& datePairsToCheck) const
{
  for (DatePair datePair : datePairsToCheck)
  {
    StatNode& node = *_stat;
    StatNode::Request request(findUsingPQScoreStat, datePair, findUsingCxrSPInfoStat);
    StatNode::Response response = node.query(request);

    bool skip = skipCallback(response.makeStat(request));
    if (!skip)
      return true;
  }

  return false;
}

AltDatesStatistic::Stat
AltDatesStatistic2::getStat(const DiversityModel::SOPCombination& comb, MoneyAmount score) const
{
  Carrier cxr = SopCombinationUtil::detectCarrierPair(_trx, comb.oSopVec);

  // consider using a map[ sopComb => snowman ] to fixup performance
  bool isSnowman = DiversityUtil::detectSnowman(_trx, comb.oSopVec);

  StatNode& node = *_stat;
  StatNode::Request request(cxr,
                            score,
                            *comb.datePair,
                            isSnowman,
                            DiversityUtil::getTravelSegCount(_trx, comb.oSopVec),
                            &comb.oSopVec);
  StatNode::Response response = node.query(request);

  return response.makeStat(request);
}

AltDatesStatistic::Stat
AltDatesStatistic2::getStat(const FlightMatrixSolution& solution, const DatePair& datePair) const
{
  const shpq::SopIdxVec sops = solution.first;
  Carrier cxr = SopCombinationUtil::detectCarrierPair(_trx, sops);
  MoneyAmount price = solution.second->getTotalNUCAmount();

  bool isSnowman = DiversityUtil::detectSnowman(_trx, solution.first);

  StatNode& node = *_stat;
  StatNode::Request request(
      cxr, price, datePair, isSnowman, DiversityUtil::getTravelSegCount(_trx, sops), &sops);
  StatNode::Response response = node.query(request);

  return response.makeStat(request);
}

std::tr1::tuple<std::size_t, std::size_t>
AltDatesStatistic2::filterDatePairsBy(MoneyAmount pqScore,
                                      unsigned maxFareLevel,
                                      float datePairFareCutoffCoef,
                                      std::set<DatePair>& datePairSet) const
{
  // Will check fare level price and
  std::size_t numMaxFareLevelAmountIsLessThanPQScore(0), numFareCutoffAmountReached(0);

  for (std::set<DatePair>::iterator setIter = datePairSet.begin(); setIter != datePairSet.end();)
  {
    StatNode& stat = *_stat;
    StatNode::Request request(pqScore, *setIter);
    StatNode::Response response = stat.query(request);

    bool erase = false;

    // Check if response._fareLevel (as it is detected on fareLevelIsNotLessThan)
    // exceeds max fare level fareLevelToCheck
    if (response._fareLevel > maxFareLevel)
    {
      numMaxFareLevelAmountIsLessThanPQScore++;
      erase = true;
    }

    // Check if fare cut-off amount was reached for this date pair (TODO: move to DMAD and use via
    // callback?)
    if ((datePairFareCutoffCoef > 1.) && (response._firstFareLevelAmount > 0.))
    {
      MoneyAmount fareCutoffAmount = response._firstFareLevelAmount * datePairFareCutoffCoef;
      if (pqScore >= fareCutoffAmount)
      {
        numFareCutoffAmountReached++;
        erase = true;
      }
    }

    if (erase)
      datePairSet.erase(setIter++);
    else
      setIter++;
  }

  return std::tr1::make_tuple(numMaxFareLevelAmountIsLessThanPQScore, numFareCutoffAmountReached);
}

void
AltDatesStatistic2::dumpToDiag(DiagCollector& dc, const DumpToDiagParam& param) const
{
  if (_stat->isEmpty())
  {
    dc << param._emptyMsg << "\n";
    return;
  }

  dc << " -- SOLUTIONS DISTRIBUTION PER DATEPAIRS, FARE LEVELS AND CARRIERS --\n";

  DumpToDiagCtx ctx(dc, param._datePairsToFulfill, param._displaySops);
  _stat->dumpTo(ctx);

  dc << "LEGEND:\t* - max fare level price\n";

  if (param._datePairsToFulfill != nullptr)
    dc << "\t++ - date pairs to fulfill\n";
}

void
tse::AltDatesStatistic2::removeSolution(const FlightMatrixSolution& solution,
                                        const DatePair& datePair)
{
  _stat->removeSolution(solution, datePair);
}

} // namespace tse
