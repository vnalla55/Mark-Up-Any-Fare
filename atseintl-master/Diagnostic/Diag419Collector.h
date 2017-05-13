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

#include "Common/TseBoostStringTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Diagnostic;
class PricingTrx;
class Fare;
class FareMarket;

class Diag419Collector : public DiagCollector
{
  friend class Diag419CollectorTest;

  typedef std::vector<std::vector<ClassOfService*>*>::const_iterator COSPtrVecIC;
  typedef std::vector<ClassOfService*>::const_iterator COSInnerPtrVecIC;
  typedef std::vector<TravelSeg*>::const_iterator TravelSegPtrVecIC;
  typedef std::vector<PaxTypeFare::SegmentStatus> SegmentStatusVec;
  typedef std::vector<PaxTypeFare::SegmentStatus>::iterator SegmentStatusVecI;
  typedef std::vector<PaxTypeFare::SegmentStatus>::const_iterator SegmentStatusVecCI;

public:
  enum Diag419FTD
  {
    FIRST_CLASS = 'F',
    BUSINESS_CLASS = 'B',
    ECONOMY_CLASS = 'E',
  };
  enum Diag419MixStatusInd
  {
    STAT_DIFF = 'D',
  };
  enum Diag419_OWRT
  {
    ONEWAY_IND = 'O',
    ROUND_TRIP_IND = 'R',
  };

  explicit Diag419Collector(Diagnostic& root) : DiagCollector(root), _lineCount(0) {}
  Diag419Collector() : _lineCount(0) {}

  virtual Diag419Collector& operator<<(const PaxTypeFare& paxTfare) override;
  bool finalDiag(PricingTrx& trx, const FareMarket& mkt) override;

private:
  int _lineCount; // total number of lines if necessary

  void travelSegmentHeader(); // Display header for the fares in the travel segment
  void showFareType(const PaxTypeFare& paxTfare);
};

} // namespace tse

