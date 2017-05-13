//-------------------------------------------------------------------
//
//  File:        VoluntaryChanges.h
//  Created:     April 25, 2007
//  Authors:     Grzegorz Cholewiak
//
//  Description:
//
//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/ExchShopCalendarUtils.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/Waiver.h"

#include <vector>
#include <utility>

namespace tse
{
class CarrierApplicationInfo;
struct Cat31Info;
class DateOverrideRuleItem;
class DateTime;
class DiagCollector;
class ReissueTable;

class VoluntaryChanges
{
  friend class VoluntaryChangesTest;

public:
  static constexpr Indicator TICKET_VALIDITY_DATE_CHECK_REQUIRED = 'X';
  static constexpr Indicator NOT_APPLY = ' ';
  static constexpr Indicator BEFORE = 'B';
  static constexpr Indicator AFTER = 'A';
  static constexpr Indicator BEFORE_PU = 'D';
  static constexpr Indicator AFTER_PU = 'E';
  static constexpr Indicator NOT_PERMITTED = 'N';
  static constexpr Indicator ADVRSVN_ORIG_TKT_DT = 'O';
  static constexpr Indicator ADVRSVN_REISSUE_DT = 'R';
  static constexpr Indicator ADVRSVN_JOURNEY = 'J';
  static constexpr Indicator ADVRSVN_PRICING_UNIT = 'P';
  static constexpr Indicator ADVRSVN_FARE_COMPONENT = 'F';
  static constexpr Indicator DOMINTL_COMB_BLANK = ' ';
  static constexpr Indicator DOMINTL_COMB_APPLY = 'Y';
  static constexpr Indicator DOMINTL_COMB_NOT_APPLY = 'N';
  static constexpr Indicator CHG_IND_P = 'P';
  static constexpr Indicator CHG_IND_J = 'J';

  VoluntaryChanges(RexPricingTrx& trx, const Itin* itin, const PricingUnit* pu);
  virtual ~VoluntaryChanges();

  VoluntaryChanges() = delete;
  VoluntaryChanges(const VoluntaryChanges&) = delete;
  VoluntaryChanges& operator=(const VoluntaryChanges&) = delete;

  Record3ReturnTypes validate(const FareUsage& fareUsage, const VoluntaryChangesInfo& vcRec3);

  bool matchWaiver(const VoluntaryChangesInfo& vcRec3, const DateTime& applDate);
  bool matchPTC(const CarrierCode& carrier, const VoluntaryChangesInfo& vcRec3);
  bool matchTicketValidity(const VoluntaryChangesInfo& vcRec3);
  virtual bool matchT988(PaxTypeFare& ptf,
                         const VoluntaryChangesInfo& vcRec3,
                         bool overridenData,
                         ReissueTable& t988Validator,
                         const Cat31Info* prevalidatedCat31Info = nullptr);
  bool matchOverrideDateTable(const VoluntaryChangesInfo& vcRec3,
                              const DateTime& applDate,
                              const DateTime& newTravelDate);
  bool matchDeparture(const FareMarket& fareMarket, const VoluntaryChangesInfo& vcRec3);
  bool oneCarrierTicket();
  bool segsNotChanged(FareMarket& fareMarket, const VoluntaryChangesInfo& vcRec3);
  bool chkNumOfReissue(FareMarket& fareMarket, const VoluntaryChangesInfo& vcRec3);
  bool matchAdvanceReservation(const FareMarket& fareMarket, const VoluntaryChangesInfo& vcRec3);
  bool matchTktTimeLimit(const VoluntaryChangesInfo& vcRec3);
  bool matchSameAirport(FareCompInfo& fareComponentInfo, const VoluntaryChangesInfo& vcRec3);
  bool
  getConstrainsFromT988(PaxTypeFare& ptf, const VoluntaryChangesInfo& vcRec3, bool overridenData);
  std::pair<bool, const Cat31Info*>
  isInPreselectedRec3(const PaxTypeFare& paxTypeFare,
                      const VoluntaryChangesInfo& vcRec3) const;

protected:
  virtual bool matchR3(PaxTypeFare& paxTypeFare,
                       const VoluntaryChangesInfo& vcRec3);

  bool matchOverrideDateRule(const DateOverrideRuleItem& dorItem,
                             const DateTime& travelDate,
                             const DateTime& ticketingDate);
  bool matchJourneyTktTimeLimit(const Indicator tktTimeLimitInd, const DateTime& ticketDT);
  bool matchPUTktTimeLimit(const Indicator beforeAfter,
                           const PricingUnit* pu,
                           const DateTime& ticketDT);

  bool shouldOverrideWithIntlFc(FareCompInfo* fc, const VoluntaryChangesInfo& vcRec3);
  void storeOverridenIntlFc(FareCompInfo& domFc,
                            FareCompInfo& intlFc,
                            const VoluntaryChangesInfo& vcRec3);
  bool validateOverridenIntlFc(const FareCompInfo& fc,
                               const CarrierCode& domCarrier,
                               bool domIntlCombApply,
                               bool anyCarrier,
                               std::vector<CarrierApplicationInfo*>& carrierLst);

  bool validateIntlCarrierWithCarrApplData(const CarrierCode& carrier,
                                           std::vector<CarrierApplicationInfo*>& carierLst);
  virtual const std::vector<CarrierApplicationInfo*>&
  getCarrierApplication(const VendorCode& vendor, int itemNo, const DateTime& applDate);
  virtual bool isInternationalItin();
  virtual std::vector<FareCompInfo*>* getAllFc();

  void updateFailByPrevReissued(FareCompInfo& fcInfo, const PaxTypeFare* paxTypeFare);
private:
  bool validateWithoutTravelDate(PaxTypeFare& paxTypeFare, const VoluntaryChangesInfo& vcRec3);
  bool validateUsingTravelDate(PaxTypeFare& paxTypeFare, const VoluntaryChangesInfo& vcRec3);

  DiagCollector* _dc = nullptr;
  bool _isSoftPass = false;
  FareCompInfo::SkippedValidationsSet* _skippedValidations = nullptr;
  bool _failByPrevReissued = false;
  bool _isEXSCalendar = false;
  RexPricingTrx& _trx;
  const Itin* _itin;
  const PricingUnit* _pu;

  ExchShopCalendar::DateRange _firstSegment;
  ExchShopCalendar::DateRange _lastSegment;
  std::unique_ptr<ExchShopCalendar::R3ValidationResult> _r3DateValidationResults;
};
}

