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

#include "DataModel/Diversity.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DesiredSopCombinationInterpreter.h"

namespace tse
{

struct DsciSwapperOperand
{
  const ShoppingTrx::SchedulingOption* _outbound;
  const ShoppingTrx::SchedulingOption* _inbound;
  MoneyAmount _price;

  DsciSwapperOperand() : _outbound(nullptr), _inbound(nullptr), _price(0.0) {}

  DsciSwapperOperand(const ShoppingTrx::SchedulingOption* outbound,
                     const ShoppingTrx::SchedulingOption* inbound,
                     MoneyAmount price)
    : _outbound(outbound), _inbound(inbound), _price(price)
  {
  }
};

struct DsciSwapperScore
{
  double _primaryScore = 0.0;
  double _secondaryScore = 0.0;
};

class DsciDiversityItinerarySwapperAdapter
    : public DesiredSopCombinationInterpreter<DsciSwapperOperand>
{
public:
  friend class DsciDiversityItinerarySwapperAdapterTest;

  DsciDiversityItinerarySwapperAdapter(const ShoppingTrx& trx,
                                       const ItinStatistic& stats,
                                       size_t newTODBucket,
                                       Diversity::BucketType bucket,
                                       bool isBrandedFaresPath)
    : DesiredSopCombinationInterpreter<DsciSwapperOperand>(trx, stats), _isTodDistanceResultLastUsed(false)
  {
    if (UNLIKELY(isBrandedFaresPath))
      addIBFPreferenceClause();

    addSolutionScoreClause(bucket);
    addTODDistClause(newTODBucket);
  }

  bool isTodDistanceResultLastUsed() const { return _isTodDistanceResultLastUsed; }

private:
  bool _isTodDistanceResultLastUsed;

  void addTODDistClause(size_t newTODBucket)
  {
    Clause clause = std::tr1::bind(&DsciDiversityItinerarySwapperAdapter::isBetterByTODDist,
                                   this,
                                   newTODBucket,
                                   std::tr1::placeholders::_1,
                                   std::tr1::placeholders::_2);
    _clauseList.push_back(clause);
  }

  void addSolutionScoreClause(Diversity::BucketType bucket)
  {
    Clause clause = std::tr1::bind(&DsciDiversityItinerarySwapperAdapter::isBetterBySolutionScore,
                                   this,
                                   bucket,
                                   std::tr1::placeholders::_1,
                                   std::tr1::placeholders::_2);
    _clauseList.push_back(clause);
  }

  void addIBFPreferenceClause()
  {
    Clause clause = std::tr1::bind(&DsciDiversityItinerarySwapperAdapter::isBetterByIBFPreference,
                                   this,
                                   std::tr1::placeholders::_1,
                                   std::tr1::placeholders::_2);
    _clauseList.push_back(clause);
  }

  int isBetterByTODDist(size_t newTODBucket, const Operand& lhs, const Operand& rhs);
  int isBetterBySolutionScore(Diversity::BucketType bucket, const Operand& lhs, const Operand& rhs);
  int isBetterByIBFPreference(const Operand& lhs, const Operand& rhs);

  DsciSwapperScore calcSolutionScore(Diversity::BucketType bucket,
                                     MoneyAmount price,
                                     const ShoppingTrx::SchedulingOption* outbound,
                                     const ShoppingTrx::SchedulingOption* inbound) const;
  int compareScores(const DsciSwapperScore& lhsScore, const DsciSwapperScore& rhsScore);

  float calcTODDistance(size_t decreasedBkt, size_t increasedBkt) const;
};

} // tse
