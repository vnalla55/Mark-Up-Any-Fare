//----------------------------------------------------------------------------
//  File:        Diag312Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 312 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2004
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

namespace tse
{
class PaxTypeFare;
class SurchargesInfo;
class SurchargeData;

class Diag312Collector : public DiagCollector
{
  friend class Diag312CollectorTest;

public:
  //@TODO will be removed, once the transition is done
  explicit Diag312Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag312Collector() {}

  void diag312Collector(PricingTrx& trx,
                        const PaxTypeFare& paxTypeFare,
                        const SurchargesInfo* surchRule);

  void displaySurchargeData(PricingTrx& trx,
                            const SurchargeData* surchargeData,
                            const SurchargesInfo& surchargeInfo,
                            bool paperTktSurchargeMayApply);

  void displaySideTripSurchargeData(const SurchargeData* surchargeData, const LocCode& city);

private:
  void writeHeader(const PaxTypeFare& paxTypeFare, const SurchargesInfo& srr);

  std::string formatSurchargeApplication(const Indicator& surrAppl, const VendorCode& vendor) const;
  std::string formatNegativeSurchargeApplication(const Indicator& surrAppl) const;
  std::string formatTravelPortion(const Indicator& tvlPortion) const;
  std::string formatTimeApplication(const Indicator& timeAppl) const;
};

} // namespace tse

