//-------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "DataModel/RexBaseTrx.h"
#include "RexPricing/PaxTypeFareWrapper.h"
#include "Rules/RuleConst.h"

#include <vector>

namespace tse
{

class FareMarket;
class FareCompInfo;
class PaxTypeFare;

class RexFareSelectorStrategy
{
  friend class RexFareSelectorStrategyTest;

public:
  typedef std::vector<PaxTypeFareWrapper>::iterator Iterator;

  RexFareSelectorStrategy(const RexBaseTrx& trx) : _trx(trx) {}

  virtual ~RexFareSelectorStrategy() {}

  void process(FareCompInfo& fc) const;

protected:
  static const double VARIANCE_SQ[3];

  bool processImpl(FareCompInfo& fc, std::vector<PaxTypeFareWrapper>& preSelected) const;

  virtual bool select(FareCompInfo& fc,
                      Iterator begin,
                      Iterator end,
                      std::vector<PaxTypeFareWrapper>& selected) const = 0;

  void selection(FareCompInfo& fc,
                 std::vector<PaxTypeFareWrapper>& preSelected,
                 std::vector<PaxTypeFareWrapper>& result) const;

  bool selectionNextLevel(FareCompInfo& fc,
                          Iterator begin,
                          Iterator end,
                          std::vector<PaxTypeFareWrapper>& result) const;

  void
  setInitialFaresStatus(std::vector<PaxTypeFare*>& fares, unsigned category, bool catIsValid) const;

  void preSelect(FareCompInfo& fc, std::vector<PaxTypeFareWrapper>& preSelected) const;

  bool checkForcedSideTrip(const FareMarket& fm) const;

  void updateAmount(Iterator begin, Iterator end, bool usePublishedCurrency) const;
  void updateAmount(Iterator begin, Iterator end,
                    bool usePublishedCurrency, bool considerNetAmonut) const;

  void updateFareComp(FareCompInfo& fc, std::vector<PaxTypeFareWrapper>& selected) const;

  std::vector<PaxTypeFare*>::iterator
  selectByDirectionality(FareCompInfo& fc,
                         std::vector<PaxTypeFare*>::iterator begin,
                         std::vector<PaxTypeFare*>::iterator end) const;

  uint16_t getCategory() const;

  virtual bool getPreSelectedFares(const FareCompInfo& fc,
                                   RexDateSeqStatus status,
                                   std::vector<PaxTypeFareWrapper>& preSelected) const
  {
    return false;
  }

  virtual void setPreSelectedFares(const FareCompInfo& fc,
                                   RexDateSeqStatus status,
                                   const std::vector<PaxTypeFareWrapper>& preSelected) const
  {
  }

  double getTolerance() const;

  const RexBaseTrx& _trx;
};

} // tse

