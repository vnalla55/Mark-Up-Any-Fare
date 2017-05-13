// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
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

#include "Pricing/Shopping/Diversity/DiversityModel.h"

#include <vector>

namespace tse
{
class Diversity;
class ItinStatistic;
class Logger;

class DiversityModelPriceOnly : public DiversityModel
{
public:
  DiversityModelPriceOnly(ShoppingTrx& shoppingTrx, ItinStatistic& stats, DiagCollector* dc);

  PQItemAction getPQItemAction(const shpq::SoloPQItem* pqItem) override;

  SOPCombinationList::iterator
  getDesiredSOPCombination(SOPCombinationList& combinations, MoneyAmount score, size_t fpKey) override;
  bool getIsNewCombinationDesired(const SOPCombination& combination, MoneyAmount score) override;
  bool isNonStopNeededOnly() override { return false; }

  bool addSolution(const ShoppingTrx::FlightMatrix::value_type& solution,
                   ShoppingTrx::FlightMatrix& flightMatrix,
                   size_t farePathKey,
                   const DatePair* datePair) override;

  int getBucketStatus(const Diversity::BucketType bucket) const override { return -1; }
  bool isNonStopOptionNeeded() const override { return false; }
  bool isAdditionalNonStopEnabled() const override { return false; }
  bool isAdditionalNonStopOptionNeeded() const override { return false; }

private:
  bool continueProcessing(MoneyAmount pqScore);
  void initializeDiagnostic(const std::string& diagArg);
  void printContinueProcessing(bool result, bool isCutoffReached) const;
  void printPQItemAction(DiversityModel::PQItemAction result, const shpq::SoloPQItem* pqItem) const;
  void printParameters() const;
  void calculateFareCutOffCoef();
  ItinStatistic& _stats;
  Diversity& _diversityParams;
  DiagCollector* _dc;
  bool _newFarePath;
  bool _diagPQ;
  const size_t _numberOfSolutionsRequired;
  bool _isFareCutOffUsed;
  bool _altDates;
  static Logger _logger;
};
}

