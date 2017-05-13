//-------------------------------------------------------------------
//
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
//-------------------------------------------------------------------

#pragma once

#include "DBAccess/AdvResTktInfo.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/IObserver.h"
#include "Rules/RuleApplicationBase.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/SubjectObserved.h"

namespace tse
{
class PaxTypeFare;
class FareMarket;
class PricingTrx;
class Itin;
class DiagCollector;
class TravelSeg;
class DataHandle;
class RexPricingTrx;
class Logger;

using AdvResTktUnitName = const char*;

class AdvanceResTkt : public RuleApplicationBase,
                      public SubjectObserved<IObserver<NotificationType, const DateTime&>>
{
  friend class AdvResTkt_ExcPricingTest;
  friend class AdvResTkt_a2fare;
  friend class AdvResTktTest;

public:
  AdvanceResTkt()
    : _diagFromToDate(false),
      _ignoreTktAfterResRestriction(false),
      _ignoreTktDeforeDeptRestriction(false),
      _isPTFBeginJxr(false),
      _tktToDateOverride(DateTime::emptyDate()),
      _tktFromDateOverride(DateTime::emptyDate()),
      _advTktTod(0),
      _advTktPeriod(""),
      _advTktUnit(""),
      _advTktOpt(' '),
      _advTktDepart(0),
      _advTktDepartUnit(' '),
      _advTktBoth(' '),
      _matchedTktExptTime(false),
      _pricingTrx(nullptr),
      _utcOffset(0),
      _newTicketIssueDate(DateTime::emptyDate()),
      _dataHandle(nullptr),
      _useFakeFP(false),
      _farePath(nullptr),
      _itin(nullptr),
      _cat05overrideDisplayFlag(false)
  {
  }

  virtual ~AdvanceResTkt() {}

  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      const PaxTypeFare& fare,
                                      const RuleItemInfo* ruleInfo,
                                      const FareMarket& fareMarket) override;

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* ruleInfo,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage) override;

  static constexpr int MAX_NUM_ELIGIBLE_TKTDT = 2;

  // Following are used to understand AdvResTktInfo
  static constexpr Indicator notPermitted = 'X'; // ADV RES.PERM
  static constexpr Indicator notApply = ' ';
  static constexpr Indicator earlierTime = 'E'; // ADV TKTG.BOTH
  static constexpr Indicator laterTime = 'L'; // ADV TKTG.BOTH
  static constexpr Indicator allSector = 'X'; // CONF.SECTOR
  static constexpr Indicator returnSectorNoOpen = 'N'; // CONF.SECTOR
  static constexpr Indicator firstSector = 'Y'; // CONF.SECTOR
  static constexpr Indicator firstTimePermit = 'P'; // ADV TKTG.OPT

  // For ADV RES.STANDBY
  static constexpr Indicator noWaitAndStandby = 'A';
  static constexpr Indicator noWait = 'B';
  static constexpr Indicator noOrigWaitAndStandby = 'C';
  static constexpr Indicator noOrigStandby = 'D';
  static constexpr Indicator noOrigWait = 'E';
  static constexpr Indicator noStandbyOrOrigWait = 'F';
  static constexpr Indicator noWaitOrOrigStandby = 'G';
  static constexpr Indicator onlyStandbyOnSameDayFlightNoOthers = 'H';
  static constexpr Indicator onlyStandbyOnSameDayFlightNoOtherStandby = 'I';
  static constexpr Indicator onlyStandbyOnSameDayFlightNoOthersOnOrig = 'J';
  static constexpr Indicator onlyWaitOnSameDayFlightNoOthersOnOrig = 'K';
  static constexpr Indicator standbyOnSameDateTicketed = 'L';
  static constexpr Indicator noStandby = 'X';

  static constexpr Indicator MINUTE_UNIT_INDICATOR = 'N';
  static constexpr Indicator DAY_UNIT_INDICATOR = 'D';
  static constexpr Indicator HOUR_UNIT_INDICATOR = 'H';
  static constexpr Indicator MONTH_UNIT_INDICATOR = 'M';

  static const TimeDuration SAMETIME_WINDOW;
  enum BeforeOrAfter
  {
    BEFORE_REF_TIME = 0,
    AFTER_REF_TIME
  };

  static bool getLimitDateTime(DateTime& limitTimeReturn,
                               const DateTime& referenceDT,
                               const int16_t& resTod,
                               const ResPeriod& resPeriod,
                               const ResUnit& resUnit,
                               const BeforeOrAfter& beforeOrAfter,
                               const bool forLatestTOD = true,
                               const bool keepTime = false);

  void initialize(const PricingTrx& trx,
                  const AdvResTktInfo& advResTktInfo,
                  const PaxTypeFare& paxTypeFare,
                  const PricingUnit* pricingUnit,
                  const Itin* itin);

  bool& useFakeFP() { return _useFakeFP; }

  using RuleApplicationBase::validateUnavailableDataTag;

  static const Loc* getCommonReferenceLoc(const PricingTrx& trx);

protected:
  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& paxTypeFare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket,
                              DiagManager& diag);

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage,
                              DiagManager& diag);

  Record3ReturnTypes
  validateUnavailableDataTag(PricingTrx& trx, const RuleItemInfo& rule, DiagManager& diag);

  static bool getLimitDateTime(DateTime& limitTimeReturn,
                               const DateTime& referenceDT,
                               const int16_t& resTod,
                               const uint16_t& period,
                               const Indicator& unitInd,
                               const BeforeOrAfter& beforeOrAfter,
                               const bool forLatestTOD = true,
                               const bool keepTime = false);

  Record3ReturnTypes validateTvlSegs(PricingTrx& trx,
                                     const Itin& itin,
                                     const PricingUnit* pricingUnit, // 0 on FC validation
                                     const RuleUtil::TravelSegWrapperVector& applTravelSegs,
                                     const AdvResTktInfo& advanceResTktInfo,
                                     const RuleUtil::TravelSegWrapper& refTravelSeg,
                                     const bool& needResDTCheck,
                                     const bool& needTktDTCheck,
                                     const bool& needConfStatChk,
                                     std::map<const TravelSeg*, bool>* segsRebookStat,
                                     DiagManager& diag,
                                     bool& mayPassAfterRebook,
                                     bool& displWqWarning1,
                                     bool& displWqWarning2,
                                     const DateTime& tktDT,
                                     bool skipResPermittedChk,
                                     bool& failedOnCfmStat,
                                     const PaxTypeFare* paxTypeFare) const;

  Record3ReturnTypes validateAdvanceResTime(const DateTime& bookDT,
                                            const DateTime& refStartDT,
                                            const DateTime& refEndDT,
                                            const AdvResTktInfo& advanceResTktInfo,
                                            DiagManager& diag,
                                            bool& mayPassAfterRebook) const;

  Record3ReturnTypes validateAdvanceTktTime(PricingUnit* pricingUnit,
                                            const DateTime& ticketDT,
                                            const DateTime& bookDT,
                                            const DateTime& departureDT,
                                            const AdvResTktInfo& advanceResTktInfo,
                                            DiagManager& diag,
                                            bool& mayPassAfterRebook) const;

  const DateTime& getPrevExchangeOrOriginTktIssueDT(const PricingTrx& trx) const;

  int getEligibleTktDates(PricingTrx& trx,
                          const FareMarket& fm,
                          const PricingUnit* pu,
                          DateTime* tktDate) const;

  bool checkTravelSegChange(const FareMarket& fm) const;

  bool checkConfirmedStatus(const TravelSeg& travelSeg) const;
  bool checkConfirmedStatusNotPermitted(const TravelSeg& travelSeg,
                                        const AdvResTktInfo& advanceResTktInfo,
                                        const DateTime& ticketDT,
                                        const DateTime& departureDT) const;

  bool existResTktRestr(const AdvResTktInfo& advanceResTktInfo) const;

  bool needPUValidation(const Itin& itin,
                        const FareMarket& fareMarket,
                        const AdvResTktInfo& advanceResTktInfo) const;

  void displayRuleDataToDiag(const AdvResTktInfo& advanceResTktInfo, DiagCollector& diag);
  void displayCat05OverrideDataToDiag(DiagManager& diag) const;
  void setcat05overrideDisplayFlag() { _cat05overrideDisplayFlag = true; }
  const AdvResTktUnitName unitName(Indicator unit) const;

  const int getAdvTktTod(const AdvResTktInfo& advanceResTktInfo) const
  {
    return (!_advTktTod ? advanceResTktInfo.advTktTod() : _advTktTod);
  }
  const ResPeriod getAdvTktPeriod(const AdvResTktInfo& advanceResTktInfo) const
  {
    return (_advTktPeriod.empty() ? advanceResTktInfo.advTktPeriod() : _advTktPeriod);
  }
  const ResUnit getAdvTktUnit(const AdvResTktInfo& advanceResTktInfo) const
  {
    return (_advTktUnit.empty() ? advanceResTktInfo.advTktUnit() : _advTktUnit);
  }
  const Indicator getAdvTktOpt(const AdvResTktInfo& advanceResTktInfo) const
  {
    return (_advTktOpt == ' ' ? advanceResTktInfo.advTktOpt() : _advTktOpt);
  }
  const int getAdvTktDepart(const AdvResTktInfo& advanceResTktInfo) const
  {
    return (!_advTktDepart ? advanceResTktInfo.advTktdepart() : _advTktDepart);
  }
  const Indicator getAdvTktDepartUnit(const AdvResTktInfo& advanceResTktInfo) const
  {
    return (_advTktDepartUnit == ' ' ? advanceResTktInfo.advTktDepartUnit() : _advTktDepartUnit);
  }
  const Indicator getAdvTktBoth(const AdvResTktInfo& advanceResTktInfo) const
  {
    return (_advTktBoth == ' ' ? advanceResTktInfo.advTktBoth() : _advTktBoth);
  }

  static const AdvResTktUnitName DAY_UNIT_NAME;
  static const AdvResTktUnitName MONTH_UNIT_NAME;
  static const AdvResTktUnitName MINUTE_UNIT_NAME;
  static const AdvResTktUnitName HOUR_UNIT_NAME;
  static const AdvResTktUnitName UNKNOWN_UNIT_NAME;

  static const uint32_t MINUTES_PER_HOUR = 60;
  static const uint32_t HOURS_PER_DAY = 24;
  static const uint32_t MINUTES_PER_DAY = HOURS_PER_DAY * MINUTES_PER_HOUR;

  bool _diagFromToDate;
  bool _ignoreTktAfterResRestriction;
  bool _ignoreTktDeforeDeptRestriction;
  bool _isPTFBeginJxr;
  DateTime _tktToDateOverride;
  DateTime _tktFromDateOverride;
  int _advTktTod;
  ResPeriod _advTktPeriod;
  ResUnit _advTktUnit;
  Indicator _advTktOpt;
  int _advTktDepart;
  Indicator _advTktDepartUnit;
  Indicator _advTktBoth;

  bool _matchedTktExptTime;

  const PricingTrx* _pricingTrx;
  mutable short _utcOffset;

private:
  Record3ReturnTypes identifyAndValidateTvlSeg(PricingTrx& trx,
                                               const Itin& itin,
                                               const AdvResTktInfo& advanceResTktInfo,
                                               const FarePath* farePath,
                                               const PricingUnit* pricingUnit,
                                               const FareMarket* fareMarket,
                                               const PaxTypeFare& paxTypeFare,
                                               std::map<const TravelSeg*, bool>* segsRebookStat,
                                               DiagManager& diag,
                                               bool& mayPassAfterRebook);

  Record3ReturnTypes validateResTkt(const DateTime& bookingDT,
                                    const DateTime& tktDT,
                                    bool needResDTCheck,
                                    bool needTktDTCheck,
                                    const AdvResTktInfo& advanceResTktInfo,
                                    bool needResTktSameDayChk,
                                    bool needResTktSameTimeChk,
                                    const DateTime& refOrigDT,
                                    const DateTime& refStartDT,
                                    const DateTime& refEndDT,
                                    PricingUnit* pricingUnit,
                                    DiagManager& diag,
                                    bool& mayPassAfterRebook,
                                    bool skipResPermittedChk,
                                    const bool isWQTrx,
                                    bool& displWqWarning1,
                                    const DateTime& adjustedDateResRestriction,
                                    PricingTrx& trx) const;

  bool resCanNotBeMadeNow(const RuleUtil::TravelSegWrapper& refOrigTvlSegW,
                          const AdvResTktInfo& advanceResTktInfo) const;

  bool checkAvail(const TravelSeg& tvlSeg,
                  const std::vector<ClassOfService*>& cosVec,
                  PricingTrx& trx,
                  const PaxTypeFare& paxTypeFare) const;

  void getCurrentRebookStatus(const PaxTypeFare& paxTypeFare,
                              FareUsage* fu,
                              std::map<const TravelSeg*, bool>& segsRebookStatus) const;

  bool isFlownSectorForNewItin(const PricingTrx& trx, const TravelSeg& seg) const;

  Record3ReturnTypes
  updateRebookStatus(PricingTrx& trx,
                     const Itin* itin,
                     PaxTypeFare& paxTypeFare,
                     FareUsage* fu,
                     const std::map<const TravelSeg*, bool>& segsRebookStatus) const;

  void getCurrentRebookStatus(const FarePath& farePath,
                              std::map<const TravelSeg*, bool>& segsRebookStatus) const;
  bool updateRebookStatus(PricingTrx& trx,
                          const FarePath& farePath,
                          FareUsage& fuInProcess,
                          const std::map<const TravelSeg*, bool>& segsRebookStatus) const;
  short checkUTC(const PricingTrx& trx, const TravelSeg& tvlSeg) const;

  virtual void updateBookingDate(DateTime& bookingDT, PricingTrx& trx) const;

  void getBookingDate(PricingTrx& trx,
                      const TravelSeg& tvlSeg,
                      DateTime& bookingDT,
                      bool& isRebooked,
                      std::map<const TravelSeg*, bool>* segsBkgStat) const;

  const DateTime&
  determineTicketDate(const DateTime& ticketDate, const DateTime& departureDate) const;

  void setRebook(const TravelSeg& tvlSeg, PaxTypeFare::SegmentStatus& segStat) const;

  bool partOfLocalJourney(PricingTrx& trx, TravelSeg* tvlSeg) const;

  bool rebookNotRequired(const std::map<const TravelSeg*, bool>& segsRebookStatus,
                         const TravelSeg* tvlSeg) const;
  bool isHistorical(PricingTrx& trx) const;

  bool deleteTvlSegIgnoreCat5(PricingTrx& trx,
                              const PricingUnit* pricingUnit,
                              const RuleUtil::TravelSegWrapperVector& applTravelSegment,
                              RuleUtil::TravelSegWrapperVector& newApplTvlSegs) const;

  const FareMarket* getFlowMarket(PricingTrx& trx, const Itin* itin, TravelSeg* tvlSeg) const;

  std::vector<ClassOfService*>* getFlowAvail(TravelSeg* tvlSeg, const FareMarket* fm) const;

  PaxTypeFare::SegmentStatusVec& getSegStatusVec(PaxTypeFare& paxTypeFare, FareUsage* fu) const;

  PaxTypeFare::SegmentStatusVec&
  getSegStatusVecRule2(PaxTypeFare& paxTypeFare, FareUsage* fu) const;

  const DateTime& determineTicketDateForRex(const RexPricingTrx& exchTrx,
                                            bool isPricingUnit,
                                            const DateTime& tktDT,
                                            const DateTime& departureDT) const;

  bool nonFlexFareValidationNeeded(PricingTrx& trx) const;

  DateTime _newTicketIssueDate;

  DataHandle* _dataHandle;

  bool _useFakeFP;
  const FarePath* _farePath;
  const Itin* _itin;
  bool _cat05overrideDisplayFlag;
  static Logger _logger;
};

class AdvanceResTktWrapper
{
public:
  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              PaxTypeFare& paxTypeFare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket);

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              FareUsage& fareUsage);

  void initialize(const PricingTrx& trx,
                  const AdvResTktInfo& advResTktInfo,
                  const PaxTypeFare& paxTypeFare,
                  const PricingUnit* pricingUnit,
                  const Itin* itin)
  {
    _advanceResTkt.initialize(trx, advResTktInfo, paxTypeFare, pricingUnit, itin);
  }

  void setRuleDataAccess(RuleControllerDataAccess* ruleDataAccess)
  {
    _advanceResTkt.setRuleDataAccess(ruleDataAccess);
  }

  void setChancelor(std::shared_ptr<RuleValidationChancelor> chancelor)
  {
    _advanceResTkt.setChancelor(chancelor);
  }

  bool& useFakeFP() { return _advanceResTkt.useFakeFP(); }

private:
  AdvanceResTkt _advanceResTkt;
};
} // namespace tse

