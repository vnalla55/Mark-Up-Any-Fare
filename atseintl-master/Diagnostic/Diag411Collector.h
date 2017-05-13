//----------------------------------------------------------------------------
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

#include "Common/TseCodeTypes.h"
#include "DataModel/Fare.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FareMarket;
class CarrierMixedClass;

class Diag411Collector : public DiagCollector
{
public:
  explicit Diag411Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag411Collector() {}

  Diag411Collector& operator<<(const FareMarket& mkt) override;
  Diag411Collector& operator<<(const PaxTypeFare::BookingCodeStatus& bkgStatus) override;
  Diag411Collector& operator<<(const std::vector<BookingCode>& bookingCodeVec) override;
  Diag411Collector& operator<<(const CarrierMixedClass& iter) override;
  Diag411Collector& operator<<(const PaxTypeFare::SegmentStatus& segStat) override;
  Diag411Collector& operator<<(const std::pair<PricingTrx*, PaxTypeFare*>& output) override;

  void printMessages(int, char = 0) override;
  void lineSkip(int) override;
  void displayHierarchy(char hierarchy) override;
  virtual void table999ItemNumber(uint32_t);
  virtual void displayTagNSegs(const PaxTypeFare& ptf);
  void printIbfErrorMessage(IbfErrorMessage msg) override;

private:
  void bkg411Header();
};

} // namespace tse

