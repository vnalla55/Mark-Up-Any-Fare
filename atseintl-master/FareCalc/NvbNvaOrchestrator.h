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
//----------------------------------------------------------------------------
#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/VecMap.h"

namespace tse
{
class PricingTrx;
class FarePath;
class CalcTotals;
class FareUsage;
class PricingUnit;
class NvbNvaInfo;
class Diag861Collector;
class Itin;
class TravelSeg;
class DataHandle;
class EndOnEnd;

class NvbNvaOrchestrator
{
  friend class NvbNvaOrchestratorTest;
  friend class NvbNvaRuleTest;

public:
  NvbNvaOrchestrator(PricingTrx& trx, const FarePath& farePath, CalcTotals& calcTotals);
  virtual ~NvbNvaOrchestrator();

  void process();
  static void processNVANVBDate(PricingTrx& trx, const FarePath& farePath, CalcTotals& totals);
  enum EOEAllSegmentIndicator
  {
    AllSegment,
    CommonPoint,
    Adjacent
  };
  static const char EOE_IND_RESTRICTIONS;

private:
  void processNvbNvaTable();
  void processPu(const std::vector<FareUsage*>& fus, bool isPuReal);
  void processNvbNvaSuppression();
  void suppressNvbNva(int16_t startSegmentOrder, int16_t endSegmentOrder);
  void findNvbNva(const NvbNvaInfo& nvbNvaInfo,
                  Indicator& nvb,
                  Indicator& nva,
                  const FareBasisCode& fareBasis) const;
  void setWinningNvbNvaFlag(Indicator& nvb, Indicator alternateNvb) const;
  void buildVirtualPu(std::vector<FareUsage*>& fus) const;
  void processNvb(const std::vector<FareUsage*>& fus, const Indicator& nvb);
  void processNva(const std::vector<FareUsage*>& fus, const Indicator& nva);
  void applyNvb1stSector(const std::vector<FareUsage*>& fus);
  void applyNvb1stIntlSector(const std::vector<FareUsage*>& fus);
  void applyNvbEntireOutbound(const std::vector<FareUsage*>& fus);
  void applyNvbEntireJourney(const std::vector<FareUsage*>& fus);
  void applyNva1stSectorEarliest(const std::vector<FareUsage*>& fus);
  void applyNva1stIntlSectorEarliest(const std::vector<FareUsage*>& fus);
  void applyNvaEntireOutboundEarliest(const std::vector<FareUsage*>& fus);
  const DateTime& getMostRestrictiveNvaDate(const std::vector<FareUsage*>& fus) const;
  bool areAnyFaresWithoutPenalties(const std::vector<FareUsage*>& fus) const;
  bool areAllFaresSmf(const std::vector<FareUsage*>& fus) const;
  bool eoeRequired(const FareUsage* fu);
  void getEndOnEnds(const FareUsage& fu, std::vector<const EndOnEnd*>& endOnEnds);

  static void processNVANVBDate(PricingTrx& trx, PricingUnit& pu, const Itin& itin);
  static void processNVANVBDate(PricingUnit& pu, FareUsage& fu, bool isWQTrx);
  static void
  applyNVANVBEndOnEndRestr(PricingTrx& trx, const Itin& itin, std::vector<FareUsage*>& allFUVec);
  typedef VecMap<int16_t, DateTime>::iterator SegnoDTMapI;
  static bool isAllSegmentsOpen(const std::vector<TravelSeg*>& tvlSegs);
  static void nvabApplyFUNoChange(PricingTrx& trx,
                                  PricingUnit& pu,
                                  bool& isThereAnyNotConfirmed,
                                  const Itin& itin,
                                  FareUsage& fareUsage);
  static void applyMostRestrictedNVAToSegsInFront(PricingUnit& pu);
  static void applyNVAData(std::map<uint16_t, const DateTime*>& nvaData, PricingUnit& pu);
  static bool getEndOnEndRequiredFareUsages(DataHandle& dataHandle,
                                            const std::vector<FareUsage*>::iterator& curFUIter,
                                            const std::vector<FareUsage*>& allFUVec,
                                            std::vector<const FareUsage*>& eoeFUVec);
  static bool getEndOnEndCombinationsFU(const EndOnEnd& eoeRule,
                                        EOEAllSegmentIndicator& allSegmentIndicator,
                                        const std::vector<FareUsage*>::iterator& curFUIter,
                                        const std::vector<FareUsage*>& allFUVec,
                                        std::vector<const FareUsage*>& eoeFUVec);
  int16_t segmentId(const TravelSeg* seg) const;

  PricingTrx& _trx;
  const FarePath& _farePath;
  CalcTotals& _calcTotals;
  Diag861Collector& _diag;
};
}

