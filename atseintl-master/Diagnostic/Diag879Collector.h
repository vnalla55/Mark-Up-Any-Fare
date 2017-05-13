//----------------------------------------------------------------------------
//  File:        Diag879Collector.h
//  Created:     2010-01-26
//
//  Description: Diagnostic 879 formatter
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

#include "Common/ServiceFeeUtil.h"
#include "Diagnostic/DiagCollector.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"

namespace tse
{
class PricingTrx;

class Diag879Collector : public DiagCollector
{
public:
  explicit Diag879Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag879Collector() {}

  virtual Diag879Collector& operator<<(const PricingTrx& pricingTrx) override;

private:
  virtual Diag879Collector& operator<<(const Itin& itin) override;
  virtual Diag879Collector&
  operator<<(const std::map<const FarePath*, std::vector<OCFees*> >& ocFeesMap);
  virtual Diag879Collector& operator<<(const std::vector<PaxOCFees>& paxFeesVec);
  virtual Diag879Collector& operator<<(const PaxOCFees& paxFees);
  virtual Diag879Collector& operator<<(const std::vector<PaxOCFeesUsages>& paxFeesVec);
  virtual Diag879Collector& operator<<(const PaxOCFeesUsages& paxFees);
  std::string getCommercialName(const OCFeesUsage* ocFeesUsage);

  void printOCFees(const ServiceFeesGroup* serviceFeesGroup);
};

} // namespace tse

