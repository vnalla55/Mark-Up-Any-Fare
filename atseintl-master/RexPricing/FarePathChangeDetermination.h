//-------------------------------------------------------------------
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

#include "DataModel/RexPricingTrx.h"

namespace tse
{
class FarePath;

template <template <class, class> class V, class T>
class GenericRemoveOpens : public std::unary_function<const T, bool>
{
  V<T, std::allocator<T>> woOpens_;

public:
  V<T, std::allocator<T>>& get() { return woOpens_; }

  bool operator()(const T ts) const { return ts->changeStatus() == TravelSeg::CONFIRMOPENSEGMENT; }

  template <class Container>
  void remove(const Container& c)
  {
    std::remove_copy_if(c.begin(), c.end(), std::back_inserter(woOpens_), *this);
  }
};

class FarePathChangeDetermination
{
public:
  friend class FarePathChangeDeterminationTest;

  FarePathChangeDetermination() : _newFP(nullptr), _trx(nullptr) {}
  virtual ~FarePathChangeDetermination() {}

  void determineChanges(FarePath& newFP, RexPricingTrx& trx);
  virtual bool changedFC(const PaxTypeFare& ptf) const;
  virtual bool insideChangedPU(const PaxTypeFare& ptf) const;
  virtual bool sameInsideExtendedPU(const PaxTypeFare& ptf) const;
  const std::string applicationStatus(const PaxTypeFare& ptf) const;

protected:
  typedef std::vector<PricingUnit*>::const_iterator cPUi;
  typedef std::vector<FareUsage*>::const_iterator cFUi;
  typedef std::vector<TravelSeg*>::const_iterator cTSi;

  bool changed(const FareMarket& fm) const;
  bool fcPreviouslyNotHitButInsideHitPu(const FareUsage& newFU, const PricingUnit& newPU) const;
  bool foundSameOnNew(const FareUsage& excFC);
  void storeAllFCsOfThisPU(const PricingUnit& excPU);

  void checkRestOfNew();
  void storeCommonPartWithExcTicket(const PricingUnit& newPU);

  virtual bool sameFareComponents(const PaxTypeFare& newPtf, const PaxTypeFare& excPtf) const;
  bool sameTravelSegments(const std::vector<TravelSeg*>& newSegs,
                          const std::vector<TravelSeg*>& excSegs) const;

private:
  std::vector<const PaxTypeFare*> _changedExcFCs;
  std::vector<const PaxTypeFare*> _changedExcPuFCs;
  std::vector<const PaxTypeFare*> _extendedExcPuSameFCs;

  std::set<const PricingUnit*> _hitNewPUs;
  std::map<const FareUsage*, const FareUsage*> _newToExcFC;

  FarePath* _newFP;
  RexPricingTrx* _trx;
};

class FarePathChangeDeterminationMock : public FarePathChangeDetermination
{
  bool changedFC(const PaxTypeFare& ptf) const override { return true; };
  bool insideChangedPU(const PaxTypeFare& ptf) const override { return true; };
  bool sameInsideExtendedPU(const PaxTypeFare& ptf) const override { return true; };
};
}

