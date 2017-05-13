//----------------------------------------------------------------------------
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//  Description: Diagnostic NVB/NVA processing
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class CalcTotals;
class FarePath;
class FareUsage;
class NvbNvaInfo;
class NvbNvaSeg;

class Diag861Collector : public DiagCollector
{
  friend class Diag861CollectorTest;

public:
  using DiagCollector::operator<<;
  DiagCollector& operator<<(const NegFareRestExtSeq& nfr);
  DiagCollector& operator<<(const NvbNvaInfo& info);
  DiagCollector& operator<<(const FareUsage& fareUsage) override;

  void logNvbNvaSeg(const NvbNvaSeg& seg, bool matchFlag = true);
  void logTravelSeg(int16_t idx, const FarePath& farePath);
  void logPricingUnit(const std::vector<FareUsage*>& fus, const Itin* itin, CalcTotals& calcTotals);
  void logNvb(const Indicator& nvb);
  void logNva(const Indicator& nva);
  void printSuppressionHeader();
  void printSuppressionFooter();
  void printNvbNvaTableHeader();
  void printNvbNvaTableFooter();
  void clearLogIfDataEmpty();

private:
  void decodeNvb(const Indicator& nvb);
  void decodeNva(const Indicator& nva);
  uint8_t getFareBasicWide() override { return 15; }
  int16_t segmentOrder(const TravelSeg* travelSeg, const Itin* itin) const;

  bool _isEmptySuppression = true;
  bool _isEmptyNvbNvaTable = true;
};
} // namespace tse

