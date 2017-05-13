//-------------------------------------------------------------------
//
//  File:        ReissueTable.h
//  Created:     May 22, 2007
//  Authors:     Grzegorz Cholewiak
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

#include "Common/ExchShopCalendarUtils.h"
#include "DataModel/FareCompInfo.h"
#include "Rules/RuleUtil.h"

#include <unordered_map>

namespace tse
{
class Logger;
class RexPricingTrx;
class ReissueSequence;
class DiagCollector;
struct Cat31Info;

class ReissueTable
{
  friend class ReissueTableTest;
  friend class ReissueTable_matchDepartureDateTest;
  friend class PermutationGenerator;

public:
  static constexpr Indicator MATCH_SAME_DEPARTURE_DATE = 'X';
  static constexpr Indicator MATCH_LATER_DEPARTURE_DATE = 'D';

  static constexpr Indicator NOT_APPLY = ' ';
  static constexpr Indicator FIRST_FLIGHT_COUPON = 'F';
  static constexpr Indicator FIRST_FLIGHT_COMPONENT = 'S';

  static constexpr Indicator MATCHSTOPOVERS = 'S';
  static constexpr Indicator MATCHCONNECTIONS = 'C';
  static constexpr Indicator MATCHBOTH = 'B';
  static constexpr Indicator BLANK = ' ';

public:
  ReissueTable(RexPricingTrx& trx, const Itin* itin, const PricingUnit* pu);

  ReissueTable(RexPricingTrx& trx, const Itin* itin, DiagCollector* out);
  virtual ~ReissueTable();

  virtual const std::vector<ReissueSequence*>&
  getMatchedT988Seqs(const FareMarket& fareMarket,
                     const VoluntaryChangesInfo& vcRec3,
                     bool overridenApplied,
                     FareCompInfo::SkippedValidationsSet* skippedValidations,
                     const DateTime& applDate,
                     const Cat31Info* prevalidatedCat31Info = nullptr);

protected:
  virtual const std::vector<ReissueSequence*>&
  getReissue(const VendorCode& vendor, int itemNo, const DateTime& applDate);

  bool matchCancelAndStartOver(const ReissueSequence& t988Seq);

  bool matchFlightNo(const FareMarket& fareMarket, const ReissueSequence& t988Seq);

  bool matchPrevalidatedSeqTab988(const Cat31Info* cat31Info, const int& seqNo);

  bool matchPortion(const ReissueSequence& t988Seq);

  template <typename Predicate>
  bool
  validateDepDateInd(Predicate predicate,
                     const Indicator dtInd,
                     const std::vector<TravelSeg*>& newItinDepartures,
                     const ExchShopCalendar::R3ValidationResult* r3ValidationResults,
                     const int SeqNo);

  bool
  matchDepartureDate(const FareMarket& fareMarket,
                     const ReissueSequence& t988Seq,
                     const ExchShopCalendar::R3ValidationResult* r3ValidationResults = nullptr);

  bool matchOriginallyScheduledFlight(const FareMarket& fareMarket, const ReissueSequence& t988Seq);

  bool matchCoupon(const ReissueSequence& t988Seq);

  bool matchCarrierRestrictions(const FareMarket& fareMarket, const ReissueSequence& t988Seq);

  bool matchAgencyRestrictions(const ReissueSequence& t988Seq);

  bool matchOutboundPortionOfTravel(const FareMarket& fareMarket, const ReissueSequence& t988Seq);

  bool matchTag7Definition(const ReissueSequence& t988Seq);

  void findPotentialFCBeginsInNewItin(const LocCode& boardMultiCity,
                                      const LocCode& offMultiCity,
                                      TravelSeg::ChangeStatus changeStatus,
                                      const std::vector<TravelSeg*>& newItinSegs,
                                      std::vector<TravelSeg*>& result) const;

  virtual bool validateGeoRuleItem(const int& geoTblItemNo,
                                   const VendorCode& vendor,
                                   const RuleConst::TSIScopeParamType defaultScope,
                                   const FarePath* fp,
                                   const PricingUnit* pu,
                                   const FareMarket* fm,
                                   RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                   const bool checkOrig,
                                   const bool checkDest);
  bool validateGeoRule(const int& geoTblItemNoFrom,
                       const int& geoTblItemNoTo,
                       const VendorCode& vendor,
                       const RuleConst::TSIScopeParamType defaultScope,
                       const FarePath* fp,
                       const PricingUnit* pu,
                       const FareMarket* fm,
                       const TSICode& tsiFrom,
                       const TSICode& tsiTo,
                       std::vector<TravelSeg*>& validTvlSegs);
  bool validateGeoRule(const int& itemNo,
                       const VendorCode& vendor,
                       const bool& checkOrig,
                       const bool& checkDest,
                       const RuleConst::TSIScopeType& scope,
                       RuleUtil::TravelSegWrapperVector& applTravelSegment);
  bool validateGeoRule(const int& geoTblItemNoFrom,
                       const int& geoTblItemNoTo,
                       const VendorCode& vendor,
                       const TSICode& tsiFrom,
                       const TSICode& tsiTo,
                       const RuleConst::TSIScopeType& scope,
                       std::vector<TravelSeg*>& validTvlSegs);
  bool validateChangeStatus(const int& itemNo,
                            const Indicator& portionInd,
                            const bool& checkOrigFrom,
                            const bool& checkDestFrom,
                            const bool& checkOrigTo,
                            const bool& checkDestTo,
                            const RuleUtil::TravelSegWrapperVector& applTravelSegmentFrom,
                            const RuleUtil::TravelSegWrapperVector& applTravelSegmentTo);
  bool validateChangeStatus(const int& itemNo,
                            const Indicator& portionInd,
                            std::vector<TravelSeg*>& validTvlSegs);
  virtual bool getGeoRuleItem(const uint32_t geoTblItemNo,
                              const VendorCode& vendor,
                              RexPricingTrx& trx,
                              bool& checkOrig,
                              bool& checkDest,
                              TSICode& tsi,
                              LocKey& locKey1,
                              LocKey& locKey2);

  virtual const TSIInfo* getTSI(const TSICode& tsi);

  virtual const std::vector<CarrierApplicationInfo*>&
  getCarrierApplication(const VendorCode& vendor, int itemNo);

  virtual void
  diagGeoTblItem(const uint32_t geoTblItemNo, const tse::VendorCode& vendor, RexPricingTrx& trx);

  bool checkPortionBadData(const int& itemNo,
                           const VendorCode& vendor,
                           bool& checkOrigFrom,
                           bool& checkDestFrom,
                           bool& checkOrigTo,
                           bool& checkDestTo);
  bool setupFirstFlight(const Indicator& portionInd,
                        const FareUsage*& fu,
                        std::vector<TravelSeg*>::const_iterator& tsi,
                        std::vector<TravelSeg*>::const_iterator& tsie);
  bool checkFirstFlight(const Indicator& portionInd, const FareUsage* fu);

  bool updateValidSegments(const TSICode& tsiFrom,
                           const TSICode& tsiTo,
                           const std::vector<TravelSeg*>& allTvlSegs,
                           const TravelSeg* atsFROMFrontSeg,
                           const TravelSeg* atsFROMBackSeg,
                           const std::vector<TravelSeg*>& validTOSegs,
                           std::vector<TravelSeg*>& validTvlSegs);

  virtual const CarrierCode& findPublishingCarrier(const FareMarket& fareMarket);

  bool processAgencyRestrictions(Indicator agencyRestrictionIndicator, std::string iataAgencyNo);

  struct IsStopOver : public std::unary_function<const TravelSeg*, bool>
  {
    bool operator()(const TravelSeg* seg) const
    {
      return (seg->isForcedStopOver() || seg->stopOver());
    }
  };

  struct IsConnection : public std::unary_function<const TravelSeg*, bool>
  {
    bool operator()(const TravelSeg* seg) const
    {
      return (seg->isForcedConx() || !seg->stopOver());
    }
  };

  static Logger _logger;
  DiagCollector* _dc;
  RexPricingTrx& _trx;
  const Itin* _itin;
  const PricingUnit* _pu;
  bool _isExternalOutput;

  using DateIndRange = std::unordered_map<Indicator, ExchShopCalendar::DateRange>;
  DateIndRange _dateIndRange;

private:
  ReissueTable();
  ReissueTable(const ReissueTable&);
  ReissueTable& operator=(const ReissueTable&);

};

} // tse

