/*---------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "Diagnostic/Diag990Collector.h"

namespace tse
{
class DiagCollector;
class FPPQItem;
class GroupFarePath;
class Itin;
class PaxType;
struct SavedFPPQItem;

namespace similaritin
{
class NoDiagnostic
{
public:
  NoDiagnostic() {}
  NoDiagnostic(PricingTrx&, DiagCollector&) {}

  void itinNotApplicableRule(Itin&, const uint16_t) {}
  void itinNotApplicable(Itin&, const Diag990Collector::Diag990NotAppReason) {}
  void printNotApplicable() {}
  void itinPopulated(Itin&, const std::vector<FarePath*>&) {}
  void printAttempt(const uint16_t, const GroupFarePath&) {}
  void printFamily(const Itin&, const std::vector<FPPQItem*>&) {}
  void printSurchargesNotApplied() {}
  void clearChildBookingClass(const Itin*) {}
  void updateChildBookingClass(const Itin*, const std::vector<std::vector<ClassOfService*>*>&) {}
  void processingSavedFPPQItems(const IntIndex&) {}
  void processingSimilarItinNo(const IntIndex&) {}
  void savedItems(const PaxType&, const std::vector<SavedFPPQItem>&) {}
  void processingNewFPPQItem(const FPPQItem&, const uint16_t) {}
  void savedFPPQItemFailed() {}
  void savedFPPQItemPassed() {}
  void returningItem(const uint16_t) {}
  void pushingBack(const uint16_t, const MoneyAmount) {}
  void gfpPassed() {}
  void gfpFailed() {}
  void printHeader() {}
  void revalidationPassed() {}
  void revalidationFailed() {}
  Diag990Collector* get990Diag() { return nullptr; }
};

class DiagnosticWrapper
{
public:
  DiagnosticWrapper(PricingTrx& trx, DiagCollector& diag);

  void itinNotApplicableRule(Itin&, const uint16_t);
  void itinNotApplicable(Itin&, const Diag990Collector::Diag990NotAppReason);
  void printNotApplicable();
  void itinPopulated(Itin&, const std::vector<FarePath*>&);
  void printAttempt(const uint16_t, const GroupFarePath&);
  void printFamily(const Itin& motherItin, const std::vector<FPPQItem*>&);
  void printSurchargesNotApplied();
  void clearChildBookingClass(const Itin* itin);
  void updateChildBookingClass(const Itin* child,
                               const std::vector<std::vector<ClassOfService*>*>& cosVec);

  void processingSavedFPPQItems(const IntIndex& itinNum);
  void processingSimilarItinNo(const IntIndex& itinNum);
  void savedItems(const PaxType& paxType, const std::vector<SavedFPPQItem>& items);
  void processingNewFPPQItem(const FPPQItem& fppqItem, const uint16_t itemIndex);
  void savedFPPQItemFailed();
  void savedFPPQItemPassed();
  void returningItem(const uint16_t);
  void pushingBack(const uint16_t, const MoneyAmount amount);
  void gfpPassed();
  void gfpFailed();
  void printHeader();
  void revalidationPassed();
  void revalidationFailed();
  Diag990Collector* get990Diag() { return _diag990; }

private:
  void listItems(const std::vector<SavedFPPQItem>& items);

  PricingTrx& _trx;
  DiagCollector& _diag;
  Diag990Collector* _diag990 = nullptr;
};
}
}
