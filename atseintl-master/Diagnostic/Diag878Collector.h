//----------------------------------------------------------------------------
//  File:        Diag878Collector.h
//  Created:     2010-01-15
//
//  Description: Diagnostic 878 formatter
//
//  Updates:
//
//  Copyright Sabre 2010
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

#include "Diagnostic/DiagCollector.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"

namespace tse
{
class PricingTrx;

class Diag878Collector : public DiagCollector
{
public:
  explicit Diag878Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag878Collector() {}

  virtual Diag878Collector& operator<<(const PricingTrx& pricingTrx) override;

  void printSkipOcCollectionReason(std::string& reason);

private:
  virtual Diag878Collector& operator<<(const Itin& itin) override;
  virtual Diag878Collector&
  operator<<(const std::map<const FarePath*, std::vector<OCFees*> >& ocFeesMap);

  void printOCFees(const ServiceFeesGroup* serviceFeesGroup);

  void printSortedOCFees(const ServiceFeesGroup* serviceFeesGroup,
                         const std::vector<PaxType*>& reqPaxTypes);

  virtual Diag878Collector& operator<<(const std::vector<PaxOCFees>& paxFeesVec);
  virtual Diag878Collector& operator<<(const std::vector<PaxOCFeesUsages>& paxFees);
  virtual Diag878Collector& operator<<(const PaxOCFees& paxFees);
  virtual Diag878Collector& operator<<(const PaxOCFeesUsages& paxFees);
  std::string getCommercialName(const OCFeesUsage* ocFeesUsage);
  void printDiagHeader();
};

} // namespace tse

