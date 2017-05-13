// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         27-10-2011
//! \file         CommonSoloPQItem.h
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

#pragma once

#include "Pricing/Shopping/PQ/SoloPQItem.h"

#include <iosfwd>

namespace tse
{
namespace shpq
{

class SolutionPattern;
class SoloPQ;

//! This is base class  for SP, CR, CRC pq items. Not to be used
class CommonSoloPQItem : public SoloPQItem
{
public:
  MoneyAmount getScore() const override = 0;

  SoloPQItemLevel getLevel() const override = 0;

  const SolutionPattern* getSolPattern() const override { return &_solutionPattern; }

  void expand(SoloTrxData& soloTrxData, SoloPQ& pq) override = 0;

  std::string str(const StrVerbosityLevel strVerbosity = SVL_BARE) const override = 0;

  virtual FarePath* getFarePath() const override
  {
    return nullptr;
  };

  virtual const std::vector<CarrierCode>& getApplicableCxrs() const override
  {
    static const std::vector<CarrierCode> empty;
    return empty;
  }

  virtual void visitFareMarkets(FareMarketVisitor& visitor) const override {}

  virtual ~CommonSoloPQItem() {}

  enum LegPosition
  {
    LEG_NONE = 0,
    LEG_1ST,
    LEG_2ND,
    LEG_3RD,
    LEG_4TH
  };

protected:
  explicit CommonSoloPQItem(const SolutionPattern& solutionPattern)
    : _solutionPattern(solutionPattern), _lastFixedLeg(LEG_NONE)
  {
  }

  CommonSoloPQItem(const CommonSoloPQItem& other, const LegPosition lastFixedLeg)
    : _solutionPattern(other._solutionPattern), _lastFixedLeg(lastFixedLeg)
  {
  }

  bool isPlaceholderLeg(const LegPosition legPosition) const
  {
    return legPosition >= _lastFixedLeg;
  }

  static LegPosition getNextLegPosition(const LegPosition number)
  {
    return static_cast<LegPosition>(static_cast<int>(number) + 1);
  }

  void printBasicStr(std::ostream& stream, const char* const level) const;

  void printPlaceHolderStr(std::ostream& stream,
                           const LegPosition legPosition,
                           const bool hasNext) const;

  const SolutionPattern& _solutionPattern;
  const LegPosition _lastFixedLeg; // everything up to (and including) _lastFixedLeg
  // is fixed for same level expansion
};

} /* namespace shpq */
} /* namespace tse */

