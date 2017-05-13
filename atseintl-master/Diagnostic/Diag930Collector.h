//----------------------------------------------------------------------------
//  Copyright Sabre 2005
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

#include "Diagnostic/Diag903Collector.h"

namespace tse
{
class Diag930Collector : public Diag903Collector
{
  friend class Diag930CollectorTest;

public:
  explicit Diag930Collector(Diagnostic& root) : Diag903Collector(root) {}
  Diag930Collector() {}

  virtual Diag930Collector& operator<<(const ShoppingTrx& trx) override;
  virtual Diag930Collector& operator<<(const ShoppingTrx::FlightMatrix& matrix) override;

private:
  MoneyAmount outputTaxData(const FarePath& path);
  void outputGroupFarePathData(const SopIdVec* fmv, const GroupFarePath& path);
  void outputFarePathData(const SopIdVec* fmv, const FarePath& path);
  void setComponentValidationForCarrier(const SopIdVec& fmv, const FarePath& path);
  void getBookingCodeCabin(const FarePath& fpath,
                           TravelSeg* tvlSeg,
                           BookingCode& bookingCode,
                           CabinType& bookedCabin,
                           const SopIdVec* fmv,
                           int sopId,
                           CabinType& bookedCabinSentBack);
  // Get booking code and cabin based on travel segment and respective segment status
  void getBookingCodeCabin(const TravelSeg& tvlSeg,
                           const PaxTypeFare::SegmentStatus* segStatus,
                           BookingCode& bookingCode,
                           CabinType& bookedCabin,
                           CabinType& bookedCabinSentBack);
  void displayFlightOnly(const ShoppingTrx::FlightMatrix& matrix);
  void outputFlightData(const SopIdVec& fmv);
  void printAltDatesStatistics(DiagCollector& dc);
};

} // namespace tse

