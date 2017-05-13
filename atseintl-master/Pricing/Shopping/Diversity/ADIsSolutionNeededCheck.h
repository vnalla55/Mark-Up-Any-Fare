// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "Common/Assert.h"
#include "Common/TseDateTimeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/PQ/AltDatesStatistic.h"

#include <boost/logic/tribool.hpp>

namespace tse
{
class DiagCollector;

class ADIsSolutionNeededCheck
{
public:
  using Stat = AltDatesStatistic::Stat;

  struct DiversityModelCallback
  {
    virtual Stat getStat(const DiversityModel::SOPCombination& comb, MoneyAmount score) const = 0;

    virtual Stat getStat(const ShoppingTrx::FlightMatrix::value_type& solution,
                         const DatePair& datePair) const = 0;

    /**
     * @return true if date pair shall be skipped from processing and diagnostic output
     */
    virtual bool checkDatePairFilt(DatePair datePair) const = 0;

    /**
     * @param datePairFirstFareLevelAmount < 0. means has not initialized yet
     */
    virtual bool isDatePairFareCutoffReached(MoneyAmount datePairFirstFareLevelAmount,
                                             MoneyAmount price) const = 0;

    virtual bool isFirstCxrFareLevel(const Stat&) const = 0;

    virtual bool isNumOptionNeeded(const Stat&) const = 0;

    virtual const ShoppingTrx* getTrx() const = 0;

  protected:
    virtual ~DiversityModelCallback() {}
  };

  /**
   * Examine and store result within
   */
  ADIsSolutionNeededCheck& examine(const DiversityModelCallback& divCtx,
                                   const DiversityModel::SOPCombination& comb,
                                   MoneyAmount price);
  /**
   * Examine and store result within
   *
   * @param isPresent if solution has been already added to flight matrix
   */
  ADIsSolutionNeededCheck& examine(const DiversityModelCallback& ctx,
                                   const ShoppingTrx::FlightMatrix::value_type& solution,
                                   DatePair datePair,
                                   bool isPresent = false);

  virtual ~ADIsSolutionNeededCheck() = default;

  bool isNeeded() const
  {
    TSE_ASSERT(!boost::indeterminate(_isNeeded) ||
               !"Querying object state prior it was properly initialized");

    return (bool)_isNeeded;
  }

  /**
   * too obvious to explain when is skipped using DATEPAIR diagarg filter
   */
  bool shallExplain() const { return _skipReason != nullptr; }

  friend DiagCollector& operator<<(DiagCollector& dc, const ADIsSolutionNeededCheck& op);

protected:
  virtual bool
  isSnowmanNeeded(const DiversityModelCallback& ctx, shpq::SopIdxVecArg sops, const Stat& stat);

private:
  boost::tribool _isNeeded = boost::indeterminate;

  // for diag
  const char* _skipReason = nullptr;
  MoneyAmount _price = 0.0;
  DatePair _datePair;
  shpq::SopIdxVec _sops;
  Stat _stat;
  const ShoppingTrx* _trx = nullptr;
  //

  /**
   * Examine and store result within
   */
  void examine(const DiversityModelCallback& divCtx,
               MoneyAmount price,
               DatePair datePair,
               shpq::SopIdxVecArg sops,
               const Stat& stat);
};

} // ns tse

