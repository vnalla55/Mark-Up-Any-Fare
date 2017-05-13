//-------------------------------------------------------------------
//
//  File:        Tab988Merger.h
//  Created:     December 10, 2008
//  Authors:     Miroslaw Bartyna
//
//  Updates:
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

#include "DataModel/FareCompInfo.h"
#include "DataModel/RexShoppingTrx.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/ReissueSequence.h"
#include "Rules/ReissueTable.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

namespace tse
{
class DiagCollector;

class Tab988Merger : public ReissueTable
{
public:
  using T988Seqs = std::vector<ReissueSequence*>;

  Tab988Merger(RexPricingTrx& trx, const Itin* itin, const PricingUnit* pu);
  Tab988Merger(RexPricingTrx& trx, const Itin* itin, DiagCollector* out);
  virtual ~Tab988Merger();
  bool merge(const PaxTypeFare& ptf,
             const VoluntaryChangesInfo& vcRec3,
             bool overridenApplied,
             const DateTime& applDate,
             const ExchShopCalendar::R3ValidationResult* r3ValidationResults);

protected:
  friend class Tab988MergerTest;
  friend class TestCommon;
  typedef std::vector<int> TvlVectType;
  typedef std::vector<TvlVectType> PortionMergeTvlVectType;

  void mergePortion(const T988Seqs& t988Seqs, RexShoppingTrx::R3SeqsConstraint& dataItem);

  bool determineUnchangableSegs(const Indicator& portionInd,
                                std::vector<TravelSeg*>& validTvlSegs,
                                TvlVectType& tvlVect,
                                bool collectDiag = false);
  bool determineUnchangableSegs(const Indicator& portionInd,
                                const bool& checkOrigFrom,
                                const bool& checkDestFrom,
                                const bool& checkOrigTo,
                                const bool& checkDestTo,
                                const RuleUtil::TravelSegWrapperVector& applTravelSegmentFrom,
                                const RuleUtil::TravelSegWrapperVector& applTravelSegmentTo,
                                TvlVectType& tvlVect,
                                bool collectDiag = false);

  void diagBanner(const FareMarket& fareMarket,
                  const VoluntaryChangesInfo& vcRec3,
                  const bool table988SequenceEmpty);
  void showSegsFromExchangeItin(std::vector<TravelSeg*>::const_iterator& tsi,
                                std::vector<TravelSeg*>::const_iterator& tsie);

  void
  collectForcedConnections(const T988Seqs& t988Seqs, RexShoppingTrx::R3SeqsConstraint& dataItem);
  void collectFirstBreakRest(const T988Seqs& t988Seqs, RexShoppingTrx::R3SeqsConstraint& dataItem);
  void collectOutboundPortionRest(const T988Seqs& t988Seqs,
                                  const FareMarket& fareMarket,
                                  RexShoppingTrx::R3SeqsConstraint& dataItem);

  void mergeFareByteCxrAppl(const T988Seqs& t988Seqs,
                            const CarrierCode& governingCarrier,
                            RexShoppingTrx::R3SeqsConstraint& dataItem);
  void showCxrPreMergeData(const RexShoppingTrx::FareByteCxrAppl& fareByteCxrAppl, const int seqNo);
  void mergeFlightNumber(const T988Seqs& t988Seqs, RexShoppingTrx::R3SeqsConstraint& dataItem);
  void matchTbl988Seqs(T988Seqs* matchedT988Seqs,
                       const FareMarket& fareMarket,
                       const T988Seqs& t988Seqs,
                       const ExchShopCalendar::R3ValidationResult* r3ValidationResults);
  bool matchCxrsFromPSS(RexShoppingTrx& rsTrx,
                        const FareMarket& fareMarket,
                        const ReissueSequence& t988Seq);
  void createFMToFirstFMinPU(const FareMarket& currFareMarket, RexShoppingTrx& rsTrx);
  std::vector<T988Seqs> mergeDateRestrictions(const T988Seqs& seqs);
  void processT988Sequences(const FareMarket& fareMarket,
                            const VoluntaryChangesInfo& vcRec3,
                            const T988Seqs* matchedT988Seqs);

private:
  Tab988Merger();
  Tab988Merger(const Tab988Merger&);
  Tab988Merger& operator=(const Tab988Merger&);
};
}

