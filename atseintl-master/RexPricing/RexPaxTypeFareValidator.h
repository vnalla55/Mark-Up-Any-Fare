//----------------------------------------------------------------------------
//   File : PaxTypeFareValidator.cpp
// Copyright Sabre 2009
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/Money.h"

#include <vector>

namespace tse
{
class RexPricingTrx;
class PaxTypeFare;
class ExcItin;
class Diag602Collector;
class DiagCollector;
class FarePath;
class DCFactory;

class RexPaxTypeFareValidator
{
  friend class RexPaxTypeFareValidatorTest;

public:
  RexPaxTypeFareValidator(RexPricingTrx& rexTrx, DiagCollector* dc = nullptr);
  virtual ~RexPaxTypeFareValidator();

  bool validate(const PaxTypeFare& fare);
  bool validate(const FarePath& farePath);

protected:
  bool collectFullyFlownFares(std::vector<const PaxTypeFare*>& excFares, const FarePath& farePath);
  bool collectFullyFlownFaresAfterBoardPoint(std::vector<const PaxTypeFare*>& fares,
                                             const FarePath& farePath,
                                             int16_t lowestSegmentOrder);
  bool findFCsCombinationWithSameFareBreaks(
      std::vector<const PaxTypeFare*>::const_iterator& excFaresIter,
      std::vector<const PaxTypeFare*>::const_iterator& newFaresIter) const;
  MoneyAmount
  countAmountForTheSameCurrency(std::vector<const PaxTypeFare*>::const_iterator faresIterFrom,
                                const std::vector<const PaxTypeFare*>::const_iterator& faresIterTo)
      const;
  virtual DCFactory* setupDiagnostic(Diag602Collector*& dc602);
  virtual void releaseDiagnostic(DCFactory* factory, Diag602Collector*& dc602);
  void displayDiagnostic(const PaxTypeFare& paxTypeFare,
                         const MoneyAmount& newAmount,
                         const MoneyAmount& excAmount);

protected:
  RexPricingTrx& _rexTrx;

private:
  const ExcItin& _excItin;
  std::vector<const PaxTypeFare*> _excFares;
  std::vector<const PaxTypeFare*> _newFares;
  DiagCollector* _dc;
};

} // namespace tse

