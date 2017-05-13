//----------------------------------------------------------------------------
//  File:        Diag868Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 868 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2006
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"
#include "Fares/FareRetailerRuleValidator.h"
#include "Fares/AdjustedSellingLevelFareCollector.h"
#include "Fares/AdjustedFareCalc.h"
#include "DataModel/AdjustedSellingCalcData.h"

#include <vector>

namespace tse
{
class FareMarket;
class PricingTrx;
struct FareRetailerRuleContext;

class Diag868Collector : public DiagCollector
{
public:
  explicit Diag868Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag868Collector() = default;

  void printFareMarketHeaderFRR(PricingTrx& trx, const FareMarket& fm);
  void printPaxTypeFare(PricingTrx& trx, const PaxTypeFare& paxTypeFare);
  void printDiagSecurityHShakeNotFound(const PseudoCityCode& pcc);
  void printDiagFareRetailerRulesNotFound();
  void displayStatus(PricingTrx& trx, StatusFRRuleValidation status);
  void printFareRetailerRuleLookupInfo(const FareRetailerRuleLookupInfo* frrl);
  void printPaxTypeFareAccountCode(const AccountCode& accountCode);
  void printFareRetailerRuleLookupHeader(const FareRetailerRuleLookupInfo* frrli) const;
  void printFareRetailerRuleStatus(PricingTrx& trx, StatusFRRuleValidation rc);
  //void printFareRetailerRuleInfo(const FareRetailerRuleInfo* frri);
  void printFareRetailerRuleLookupInfo(PricingTrx& trx, const FareRetailerRuleInfo* frri, StatusFRRuleValidation rc);
  void printFareRetailerRuleInfo(PricingTrx& trx,
                                 const FareRetailerRuleInfo* frri,
                                 const FareFocusSecurityInfo* ffsi,
                                 bool ddAll);
  void printSecurityTableInfo(const FareFocusSecurityInfo* ffsi, DiagCollector& dc) const;
  void printCarriersTableInfo(PricingTrx& trx,
                              const FareRetailerRuleInfo* frri,
                              DiagCollector& dc) const;
  void printMatchTravelRangeX5(PricingTrx& trx,
                               const FareRetailerRuleInfo* frri,
                               DiagCollector& dc) const;
  void printRuleCodeTableInfo(PricingTrx& trx,
                              const FareRetailerRuleInfo* frri,
                              DiagCollector& dc) const;
  void printFareClassTableInfo(PricingTrx& trx,
                               const FareRetailerRuleInfo* frri,
                               DiagCollector& dc) const;
  void printResultingTableInfo(PricingTrx& trx,
                               const FareRetailerRuleInfo* frri,
                               DiagCollector& dc) const;
  void printCalculatingTableInfo(PricingTrx& trx,
                                 const FareRetailerRuleInfo* frri,
                                 DiagCollector& dc) const;
  void printBookingCodeTableInfo(PricingTrx& trx,
                                 const FareRetailerRuleInfo* frri,
                                 DiagCollector& dc) const;
  void printAccountCodeTableInfo(PricingTrx& trx,
                                 const FareRetailerRuleInfo* frri,
                                 DiagCollector& dc) const;
  void printLocationPairExcludeInfo(PricingTrx& trx,
                                    const FareRetailerRuleInfo* frri,
                                    DiagCollector& dc) const;
  void printCalculation(PricingTrx& trx,
                                    PaxTypeFare* ptf,
                                    AdjustedFareCalc& calcObj,
                                    const FareRetailerCalcDetailInfo* calcDetail,
                                    MoneyAmount currAmt,
                                    MoneyAmount currAmtNuc,
                                    const FareRetailerRuleContext& context);
  void printFareCreation(PricingTrx* trx, PaxTypeFare* ptf) const;
  void printMinimumAmount(PricingTrx* trx,
                                      PaxTypeFare* ptf,
                                      AdjustedSellingCalcData* calcData) const;
  void printDispayCatTypeInfo(PricingTrx& trx, const FareRetailerRuleInfo* frri, DiagCollector& dc) const;
  void printPaxTypeCode(PricingTrx& trx, const FareRetailerRuleInfo* frri, DiagCollector& dc);
  void printRetailerCodeInfo(PricingTrx& trx,
                                 const FareRetailerRuleInfo* frri,
                                 DiagCollector& dc) const;
};

} // namespace tse


