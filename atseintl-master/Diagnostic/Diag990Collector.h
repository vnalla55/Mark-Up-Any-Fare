//----------------------------------------------------------------------------
//  File:        Diag990Collector.h
//  Created:     2009-10-23
//
//  Description: Diagnostic 990 formatter
//
//  Updates:
//
//  Copyright Sabre 2009
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
class FarePath;

class Diag990Collector : public DiagCollector
{
public:
  explicit Diag990Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag990Collector() {}

  enum Diag990NotAppReason : uint8_t
  { DefaultReason,
    SameCarriers,
    SameCarriersSeatAviailable,
    SeatAvailable,
    GetBookingCodeForDiffCarrier,
    FlightApplicationRule,
    StopoverRule,
    CloningFailed,
    GetBookingCodeForChildItin,
    Routing,
    Combinability,
    FinalRevalidation };
  void addItinNotApplicable(const Itin* itin, Diag990NotAppReason reason);
  void clearItinNotApplicable() { itinInfo.clear(); }

  void printBookingClass(const Itin& itin, const std::vector<FPPQItem*>& gfp);
  void printBookingClass(const Itin& itin, const GroupFarePath& gfp, const Itin& estItin);
  void printItinNotApplicable(const Itin& motherItin, const GroupFarePath& gfp);
  void setBookingCodeForChildFailedReason(const Itin* itin, std::string rs);
  void setBookingCodeForChildFailedReason(const Itin* itin, std::string rs, size_t segIndex);
  void printPnrCollocation(PricingTrx& trx, const Itin* itin);
  void printFamily(PricingTrx& trx, const std::vector<FPPQItem*>& groupFPath, const Itin& motherItin);

  Diag990Collector& operator<<(const std::vector<FPPQItem*>& x) override;
  Diag990Collector& operator<<(const Itin& itin) override;

  void printSimilarItinFarePaths(const std::vector<FarePath*>& farePaths);

  void clearChildBookingClass();
  void updateChildBookingClass(std::vector<std::vector<ClassOfService*>*>& cosVec);

  void printItinNotApplicable();
  void clearChildBookingClassNew(const Itin* child);
  void updateChildBookingClassNew(const Itin* child,
                                  const std::vector<std::vector<ClassOfService*>*>& cosVec);
  void printBookingClassChildItinNew(const Itin* child);

private:
  std::string getBookingClass(const Itin& motherItin, const GroupFarePath& gfp, const Itin& itin);
  std::string
  getBookingClass(const Itin& motherItin, const std::vector<FPPQItem*>& fpPQItem, const Itin& itin);
  std::string getInterlineBookingClass(const Itin& itin);

  class Diag990ItinInfo
  {
  public:
    Diag990ItinInfo(const Itin* i, std::string m) : itin(i), msg(m) {}

    const Itin* itin;
    std::string msg;
  };

  std::vector<Diag990ItinInfo> itinInfo;
  std::map<const Itin*, BookingCode> failedBookingCode;
  std::map<const Itin*, std::string> bookingCodeForDiffCxFailedReason;
  std::map<const Itin*, std::string> bookingCodeForChildFailedReason;
  std::map<const Itin*, std::string> bookingCodeForDiffCx;
  std::map<const Itin*, std::string> bookingCodeForChild;
  std::map<const Itin*, std::string> bookingClassForChild;

  std::stringstream _bookingclassChild;
};

} // namespace tse

