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
#pragma once

#include "Common/Logger.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/DiversityUtil.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

#include <boost/optional.hpp>

#include <set>

#include <tr1/functional>
#include <tr1/tuple>

namespace tse
{
class DiagCollector;
class ItinStatistic;
class Logger;

class AltDatesStatistic
{
public:
  typedef ShoppingTrx::FlightMatrix::value_type FlightMatrixSolution;
  typedef DiversityUtil::CompoundCarrier Carrier;

  static const char* const DIAG_EMPTY_MSG;
  static const char* const DIAG_EMPTY_FINAL_MSG;

  struct Stat
  {
    unsigned _fareLevel;
    unsigned _numOpt;
    MoneyAmount _firstFareLevelAmount; // 1st fare level amount (-1 if not exists)

    Carrier _govCarrier;
    boost::optional<DiversityUtil::ADSolultionKind> _solKind;

    /**
     * Fare level ordinal number, _govCarrier has been seen the first time in
     *
     * This is returned optionally to make able DiversityModelAltDates to decide,
     * whether we need solution from the same carrier on the next fare level. (NGSII-286)
     *
     * Currently, only AltDatesStatistic2 returns it.
     */
    boost::optional<unsigned> _firstCxrFareLevel;

    /**
     * Snowman option will be included,
     * if the number of segment is equal or less than number of online/interline option.
     *
     * So, here we are collecting min. travel segment count for online/interline options
     * (also including online/interline snowmans as well)
     */
    boost::optional<std::size_t> _minTravelSegCount;

    /**
     * It is empty unless if same connecting city pass,
     * otherwise contains sops of option is being check counterpart for diagnostic
     */
    shpq::SopIdxVec _sameConnectingCityAndCxrOpt;

    Stat() : _fareLevel(0), _numOpt(0), _firstFareLevelAmount(-1.) {}

    Stat(unsigned fl, unsigned numOpt, MoneyAmount ffla, Carrier cxr)
      : _fareLevel(fl), _numOpt(numOpt), _firstFareLevelAmount(ffla), _govCarrier(cxr)
    {
    }

    Stat(unsigned fl,
         unsigned numOpt,
         MoneyAmount ffla,
         unsigned firstCxrFareLevel,
         boost::optional<std::size_t> minTravelSegCount,
         shpq::SopIdxVecArg sameConnectingCitySops = shpq::SopIdxVec())
      : _fareLevel(fl),
        _numOpt(numOpt),
        _firstFareLevelAmount(ffla),
        _firstCxrFareLevel(firstCxrFareLevel),
        _minTravelSegCount(minTravelSegCount),
        _sameConnectingCityAndCxrOpt(sameConnectingCitySops)
    {
    }

    bool isSnowman() const
    {
      if (UNLIKELY(!_solKind))
        return false;
      bool result = (*_solKind & DiversityUtil::Snowman);
      return result;
    }
  };

  struct CxrSPInfo
  {
    Carrier _carrier;
    bool _isSnowman;

    CxrSPInfo(Carrier cxr, bool snowman) : _carrier(cxr), _isSnowman(snowman) {}
  };

  struct DumpToDiagParam
  {
    const Carrier* _onlyForCarrier;
    const std::set<DatePair>* _datePairsToFulfill;
    bool _displaySops;
    const char* _emptyMsg;

    DumpToDiagParam(const char* emptyMsg = DIAG_EMPTY_MSG)
      : _onlyForCarrier(nullptr), _datePairsToFulfill(nullptr), _displaySops(false), _emptyMsg(DIAG_EMPTY_MSG)
    {
    }

    DumpToDiagParam& displayCarrier(const Carrier* cxr)
    {
      _onlyForCarrier = cxr;
      return *this;
    }

    DumpToDiagParam& datePairsToFulfill(const std::set<DatePair>* dp)
    {
      _datePairsToFulfill = dp;
      return *this;
    }

    DumpToDiagParam& displaySops(bool display)
    {
      _displaySops = display;
      return *this;
    }

    DumpToDiagParam& setEmptyMsg(const char* msg)
    {
      _emptyMsg = msg;
      return *this;
    }
  };

  AltDatesStatistic(ShoppingTrx&, ItinStatistic&);
  virtual ~AltDatesStatistic() {}

  virtual void addSolution(const FlightMatrixSolution& solution, const DatePair& datePair) = 0;

  virtual Stat getStat(const DiversityModel::SOPCombination& comb, MoneyAmount score) const = 0;

  virtual Stat getStat(const FlightMatrixSolution& solution, const DatePair& datePair) const = 0;

  /**
   * Filters datePairSet in place, keeps the elements, that does not meet search criteria.
   * If there is no fareLevelToCheck is defined to specific date, that date will be also kept
   * in the result.
   *
   * @param fareCutoffCoef to filter out date pairs by as well,
   *        if fareLevelIsNotLessThan reaches fare cut-off amount for that date pair.
   *        fareCutoffCoef is ignored, if is < 1.0
   * @return std::tr1::tuple<filtered out with reason fare level at fareLevelToCheck
   *                          is not less than arg. 1,
   *                         filtered out with fare cut off coef.>
   * @see allDatePairsSet
   */
  virtual std::tr1::tuple<std::size_t, std::size_t>
  filterDatePairsBy(MoneyAmount pqScore,
                    unsigned maxFareLevel,
                    float datePairFareCutoffCoef,
                    std::set<DatePair>& datePairSet) const = 0;

  typedef std::tr1::function<bool(const Stat&)> DatePairSkipCallback;
  virtual bool findAnyDatePair(MoneyAmount findUsingPQScoreStat,
                               CxrSPInfo findUsingCxrSPInfoStat,
                               const DatePairSkipCallback& skipCallback,
                               const std::set<DatePair>& datePairsToCheck) const
  {
    // a stub - does nothing
    return true;
  }

  MoneyAmount getFareLevelDelta() const { return _fareLevelDelta; }

  virtual void
  dumpToDiag(DiagCollector& dc, const DumpToDiagParam& param = DumpToDiagParam()) const = 0;

protected:
  class FareLevel
  {
  public:
    FareLevel(MoneyAmount price, MoneyAmount delta) : _price(price), _delta(delta) {}
    const MoneyAmount price() const { return _price; }

    bool operator<(const FareLevel& rhs) const
    {
      // less than within delta inaccuracy boundaries
      return _price < (rhs._price - _delta);
    }

    bool operator==(const FareLevel& rhs) const { return !(*this < rhs) && !(rhs < *this); }

    bool operator<=(const FareLevel& rhs) const { return (*this < rhs) || (rhs == *this); }

  private:
    MoneyAmount _price, _delta;
  };

  ShoppingTrx& _trx;
  ItinStatistic& _itinStatistic;
  MoneyAmount _fareLevelDelta;

  FareLevel buildFareLevel(MoneyAmount price) const { return FareLevel(price, _fareLevelDelta); }

  static Logger _logger;

private:
  AltDatesStatistic(const AltDatesStatistic&); // non-copiable
};

} // ns tse

