// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         04-11-2011
//! \file         TestPQItem.h
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------

#ifndef TESTPQITEM_H_
#define TESTPQITEM_H_

#include "Pricing/Shopping/PQ/SoloPQItem.h"

#include <memory>

namespace tse
{
namespace shpq
{
namespace test
{

class TestPQItem;
typedef std::shared_ptr<TestPQItem> TestPQItemPtr;

class TestPQItem : public SoloPQItem
{
public:
  virtual MoneyAmount getScore() const { return _score; }
  virtual SoloPQItemLevel getLevel() const { return _level; }
  virtual const SolutionPattern* getSolPattern() const { return _sp; }
  virtual FarePath* getFarePath() const { return 0; }
  virtual const std::vector<CarrierCode>& getApplicableCxrs() const { return _applCxrs; }
  virtual void expand(SoloTrxData& soloTrxData, SoloPQ& pq) {}
  virtual std::string str(const StrVerbosityLevel strVerbosity = SVL_BARE) const { return ""; }
  virtual void visitFareMarkets(FareMarketVisitor& visitor) const {}

  TestPQItem(const MoneyAmount score, const SoloPQItemLevel level, const SolutionPattern* sp = 0)
    : _score(score), _level(level), _sp(sp)
  {
  }

  static TestPQItemPtr
  create(const MoneyAmount score, const SoloPQItemLevel level, const SolutionPattern* sp = 0)
  {
    return TestPQItemPtr(new TestPQItem(score, level, sp));
  }

  void addApplicableCxr(const CarrierCode& cxr) { _applCxrs.push_back(cxr); }

private:
  MoneyAmount _score;
  SoloPQItemLevel _level;
  const SolutionPattern* _sp;
  std::vector<CarrierCode> _applCxrs;
};

} /* namespace test */
} /* namespace shpq */
} /* namespace tse */
#endif /* TESTPQITEM_H_ */
