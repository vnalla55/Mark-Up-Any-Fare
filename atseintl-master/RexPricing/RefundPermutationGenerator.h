//-------------------------------------------------------------------
//
//  File:        RefundPermutationGenerator.h
//  Created:     July 29, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/LoggerPtr.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "Rules/RuleUtil.h"

#include <vector>

namespace tse
{
class DataHandle;
class Diag688Collector;
class PaxTypeFare;

class RefundPermutationGenerator
{
public:
  typedef std::vector<RefundPermutation*> Permutations;
  typedef std::map<unsigned, std::vector<RefundProcessInfo*>> FCtoSequence;

  RefundPermutationGenerator(PricingTrx& trx, log4cxx::LoggerPtr logger = nullptr);
  ~RefundPermutationGenerator();

  void process(Permutations& permutations,
               const FarePath& farePath,
               const RefundPricingTrx::Options* refundOptions = nullptr,
               smp::RecordApplication departureInd = smp::BOTH);

  void processForSMP(Permutations& permutations,
                     const FarePath& farePath,
                     FCtoSequence& seqsByFC,
                     smp::RecordApplication departureInd = smp::BOTH);

protected:
  typedef RefundPricingTrx::Options::const_iterator OptionsIt;

  void generate(const RefundPricingTrx::Options* refundOptions,
                const std::vector<const PaxTypeFare*>& fares,
                Permutations& permutations,
                smp::RecordApplication departureInd = smp::BOTH) const;

  void generateForSMP(const std::vector<const PaxTypeFare*>& fares,
                      Permutations& permutations,
                      const PricingUnit& pricingUnit,
                      FCtoSequence& seqsByFC,
                      smp::RecordApplication departureInd = smp::BOTH) const;

  void storePermutations(Permutations& permutations,
                         FCtoSequence& seqsByFC) const;

  std::pair<OptionsIt, OptionsIt>
  findInOptions(const RefundPricingTrx::Options& refundOptions, const PaxTypeFare* ptf) const;
  void missingFareError() const;

  DataHandle& _dataHandle;
  Diag688Collector* _dc;
  log4cxx::LoggerPtr _logger;
  PricingTrx* _trx;
  bool _diagEnabled;

private:
  RefundPermutationGenerator(const RefundPermutationGenerator& rhs);
  RefundPermutationGenerator& operator=(const RefundPermutationGenerator& rhs);

  friend class RefundPermutationGeneratorTest;
};
}
