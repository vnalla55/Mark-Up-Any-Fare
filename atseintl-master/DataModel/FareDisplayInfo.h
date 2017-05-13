//-------------------------------------------------------------------
//
//  File:        FareDisplayInfo.h
//  Author:      Doug Batchelor
//  Created:     March 1, 2005
//  Description: A class to provide an interface API for the
//               Fare Display Template processing to use to retrieve
//               the data the Display Template must show.
//
//  Updates:
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//
//---------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FBDisplay.h"
#include "DataModel/InfoBase.h"

namespace tse
{
class AddonFareInfo;
class DateTime;
class FarePath;
class SurchargeData;
class FareDisplayInfo;
class SeasonsInfo;
class TaxItem;
class TaxRecord;
class TravelInfo;
class TicketInfo;
class BrandProgram;

// -------------------------------------------------------------------
// <PRE>
//
// @class FareDisplayInfo
//
// A class to provide an interface API for the Fare Display Template processing to use
// to retrieve the data the Display Template must show.
//
//
// </PRE>
// -------------------------------------------------------------------------

class FareDisplayInfo : public InfoBase
{
  friend class FareControllerTest;

public:
  FareDisplayInfo();

  // Category 12 Surcharge Data for Inbound leg
  std::vector<SurchargeData*> _inboundSurchargeData;

  const std::vector<SurchargeData*>& inboundSurchargeData() const { return _inboundSurchargeData; }
  std::vector<SurchargeData*>& inboundSurchargeData() { return _inboundSurchargeData; }

  // Category 12 Surcharge Data for outbound leg
  std::vector<SurchargeData*> _outboundSurchargeData;

  const std::vector<SurchargeData*>& outboundSurchargeData() const
  {
    return _outboundSurchargeData;
  }
  std::vector<SurchargeData*>& outboundSurchargeData() { return _outboundSurchargeData; }

  // Sector Surcharge Rule Text
  std::string _secSurchargeText;

  const std::string& secSurchargeText() const { return _secSurchargeText; }
  std::string& secSurchargeText() { return _secSurchargeText; }

  // Call this method after instancing an InfoBase object to
  // initialize it properly.
  void initialize(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare) override;

  // Fare path pointer.
  FarePath*& farePath() { return _farePath; }
  const FarePath* farePath() const { return _farePath; }

  // Tax Record Vector
  const std::vector<TaxRecord*>& taxRecordVector() const { return _taxRecordVector; }
  std::vector<TaxRecord*>& taxRecordVector() { return _taxRecordVector; }

  // Tax Item Vector
  const std::vector<TaxItem*>& taxItemVector() const { return _taxItemVector; }
  std::vector<TaxItem*>& taxItemVector() { return _taxItemVector; }

  // Tax Record Vector for One Way FT Taxes that require both Round Trip and OneWay Taxes
  const std::vector<TaxRecord*>& taxRecordOWRTFareTaxVector() const
  {
    return _taxRecordOWRTFareTaxVector;
  }
  std::vector<TaxRecord*>& taxRecordOWRTFareTaxVector() { return _taxRecordOWRTFareTaxVector; }

  // Tax Item Vector for One Way FT Taxes that require both Round Trip and OneWay Taxes
  const std::vector<TaxItem*>& taxItemOWRTFareTaxVector() const
  {
    return _taxItemOWRTFareTaxVector;
  }
  std::vector<TaxItem*>& taxItemOWRTFareTaxVector() { return _taxItemOWRTFareTaxVector; }

  // Unique identifier for each object.
  int& myObjId() { return _myObjId; }
  const int& myObjId() const { return _myObjId; }

  // Pointer to my FareDisplayTransaction.
  const FareDisplayTrx& fareDisplayTrx() const { return *_trx; }
  FareDisplayTrx& fareDisplayTrx() { return *_trx; }
  void setFareDisplayTrx(FareDisplayTrx* fp) { _trx = fp; }

  // Pointer to my PaxTypeFare.
  const PaxTypeFare& paxTypeFare() const { return *_paxTypeFare; }

  // isLocationSwapped information for Surcharge Validation
  bool& isLocationSwappedForSurcharges() { return _isLocationSwappedForSurcharges; }
  const bool& isLocationSwappedForSurcharges() const { return _isLocationSwappedForSurcharges; }

  // Rule Category number
  CatNumber& catNum() { return _catNum; }
  const CatNumber& catNum() const { return _catNum; }

  // Cat 3 (Season) info follows
  const std::vector<SeasonsInfo*>& seasons() const { return _seasons; }
  std::vector<SeasonsInfo*>& seasons() { return _seasons; }

  // Cat 5 (Advance Reservations and Ticketing) Info
  ResPeriod& lastResPeriod() { return _lastResPeriod; }
  const ResPeriod& lastResPeriod() const { return _lastResPeriod; }

  ResUnit& lastResUnit() { return _lastResUnit; }
  const ResUnit& lastResUnit() const { return _lastResUnit; }

  Indicator& ticketed() { return _ticketed; }
  const Indicator& ticketed() const { return _ticketed; }

  // Rule Type - FootNote, FareRule or GeneralRule
  Indicator& ruleType() { return _ruleType; }
  const Indicator& ruleType() const { return _ruleType; }

  ResPeriod& advTktPeriod() { return _advTktPeriod; }
  const ResPeriod& advTktPeriod() const { return _advTktPeriod; }

  ResUnit& advTktUnit() { return _advTktUnit; }
  const ResUnit& advTktUnit() const { return _advTktUnit; }

  bool isMultiAdvRes();
  bool isMultiAdvTkt();

  bool isDefaultTime(std::string& period);

  bool
  getAPFormatTime(std::string& period, std::string& unit, std::string& result, bool canConvert);
  bool getAPStringRes(std::string& field);

  void getAPStringShort(std::string& field);
  void getAPString(std::string& field);

  // Cat 6 (Minimum Stay) and Cat 7 (Maximum Stay) Info

  std::string& minStay() { return _minStay; }
  const std::string& minStay() const { return _minStay; }

  std::string& routingSequence() { return _routingSequence; }
  const std::string& routingSequence() const { return _routingSequence; }

  std::string& maxStay() { return _maxStay; }
  const std::string& maxStay() const { return _maxStay; }

  std::string& maxStayUnit() { return _maxStayUnit; }
  const std::string& maxStayUnit() const { return _maxStayUnit; }

  // Note:  The following Cat14 (travel) and Cat15 (sales) info is combined into the
  //        TRAVEL_TICKET column on the display
  const std::vector<TravelInfo*>& travelInfo() const { return _travelInfo; }
  std::vector<TravelInfo*>& travelInfo() { return _travelInfo; }
  const std::vector<TicketInfo*>& ticketInfo() const { return _ticketInfo; }
  std::vector<TicketInfo*>& ticketInfo() { return _ticketInfo; }

  // The following vector contain a Cat14 (travel dates which
  // will be used for RD header
  const std::vector<TravelInfo*>& travelInfoRD() const { return _travelInfoRD; }
  std::vector<TravelInfo*>& travelInfoRD() { return _travelInfoRD; }

  // Add an entry into the Cat 3 (seasons) vector
  void addSeason(Indicator dir, // Inbound ('I') or Outbound ( 'O')
                 int32_t startYear, // START: The first date on which travel is permitted.
                 int32_t startMonth, // If year is blank(0), any year is assumed.
                 int32_t startDay, // If day is blank(0), entire month is assumed.
                 int32_t stopYear, // STOP: The last date on which travel is permitted.
                 int32_t stopMonth, // If year is blank (0), any year is assumed.
                 int32_t stopDay); // If day is blank(0), entire month is assumed.

  // Check the Cat 3 (seasons) vector for duplicates
  bool duplicateSeason(Indicator dir, // Inbound ('I') or Outbound ( 'O')
                       int32_t startYear, // START: The first date on which travel is permitted.
                       int32_t startMonth, // If year is blank(0), any year is assumed.
                       int32_t startDay, // If day is blank(0), entire month is assumed.
                       int32_t stopYear, // STOP: The last date on which travel is permitted.
                       int32_t stopMonth, // If year is blank (0), any year is assumed.
                       int32_t stopDay); // If day is blank(0), entire month is assumed.

  // Add an entry into the Cat 14/15 travel-ticket vector
  void addTravelInfo(const DateTime& earliestTvlStartDate, // COMM: The earliest date on which
                                                           // outbound travel may commence.
                     const DateTime& latestTvlStartDate, // EXP:  The last date on which outbound
                                                         // travel may commence
                     const DateTime& stopDate, // COMP: The last date on which travel must commence
                                               // or be completed.
                     Indicator returnTvlInd); // RET:  A tag indicating whether return travel must
                                              // commence or be completed by the following
                                              // Date/Time.

  void
  addTicketInfo(const DateTime& earliestTktDate, // TKTG:  Earliest date for ticketing on any one
                                                 // sector.int stopMonth,		//If year is blank
                                                 // (0), any year is assumed.
                const DateTime& latestTktDate); // TKTG:  Last date for ticketing on all sectors.
  // Add data into Cat 14 travel vector for RD header
  void addRDTravelInfo(const DateTime& earliestTvlStartDate, // COMM: The earliest date on which
                                                             // outbound travel may commence.
                       const DateTime& latestTvlStartDate, // EXP:  The last date on which outbound
                                                           // travel may commence
                       const DateTime& stopDate, // COMP: The last date on which travel must
                                                 // commence or be completed.
                       Indicator returnTvlInd); // RET:  A tag indicating whether return travel must
                                                // commence or be completed by the following
                                                // Date/Time.

  //////
  void setFBDisplayData(uint16_t cat,
                        const FareRuleRecord2Info* fRR2,
                        const FootNoteRecord2Info* fNR2,
                        const GeneralRuleRecord2Info* gRR2,
                        const FareClassCode& fareBasis,
                        DataHandle& dataHandle)
  {
    return _fbDisplay.setData(cat, fRR2, fNR2, gRR2, fareBasis, dataHandle);
  }
  //////
  void setFBDisplayData(const CombinabilityRuleInfo* cRR2,
                        const FareClassCode& fareBasis,
                        DataHandle& dataHandle,
                        bool isDomestic)
  {
    return _fbDisplay.setData(cRR2, fareBasis, dataHandle, isDomestic);
  }

  void setFBDisplayData(const FareByRuleCtrlInfo* cFBRR2,
                        const FareClassCode& fareBasis,
                        DataHandle& dataHandle)
  {
    return _fbDisplay.setData(cFBRR2, fareBasis, dataHandle);
  }

  //////
  void setFBDisplayLocationSwapped(uint16_t cat, bool isLocationSwapped, DataHandle& dataHandle)
  {
    return _fbDisplay.setLocationSwapped(cat, isLocationSwapped, dataHandle);
  }

  TariffCode& ruleTariffCode() { return _ruleTariffCode; }
  const TariffCode& ruleTariffCode() const { return _ruleTariffCode; }

  FBDisplay& fbDisplay() { return _fbDisplay; }
  const FBDisplay& fbDisplay() const { return _fbDisplay; }

  // Category Rule Item Info for NUMBER OF DATA TABLES row
  int16_t& noOfDataTables() { return _noOfDataTables; }
  const int16_t& noOfDataTables() const { return _noOfDataTables; }

  const CategoryRuleItemInfo*& categoryRuleItemInfo() { return _catRuleItemInfo; }
  const CategoryRuleItemInfo* categoryRuleItemInfo() const { return _catRuleItemInfo; }

  // AddOn Display
  const AddonFareInfo*& addOnFareInfoPtr() { return _addOnFareInfoPtr; }
  const AddonFareInfo* addOnFareInfoPtr() const { return _addOnFareInfoPtr; }

  std::map<const AddonFareInfo*, std::string>& addOnRoutingSeq() { return _addOnRoutingSeq; }
  const std::map<const AddonFareInfo*, std::string>& addOnRoutingSeq() const
  {
    return _addOnRoutingSeq;
  }

  // Indicators to determine whether or not the fare is auto-priceable
  bool& displayOnly() { return _displayOnly; }
  const bool& displayOnly() const { return _displayOnly; }
  bool& incompleteR3Rule() { return _incompleteR3Rule; }
  const bool& incompleteR3Rule() const { return _incompleteR3Rule; }
  bool& unavailableR1Rule() { return _unavailableR1Rule; }
  const bool& unavailableR1Rule() const { return _unavailableR1Rule; }
  bool& unavailableR3Rule() { return _unavailableR3Rule; }
  const bool& unavailableR3Rule() const { return _unavailableR3Rule; }
  bool& unsupportedPaxType() { return _unsupportedPaxType; }
  const bool& unsupportedPaxType() const { return _unsupportedPaxType; }
  bool& isMinMaxFare() { return _isMinMaxFare; }
  const bool& isMinMaxFare() const { return _isMinMaxFare; }

  void setDisplayOnly() { _displayOnly = true; }
  void setUnavailableR1Rule() { _unavailableR1Rule = true; }
  void setUnavailableR3Rule(const uint16_t& cat);
  void setIncompleteR3Rule(const uint16_t& cat);
  void setUnsupportedPaxType() { _unsupportedPaxType = true; }
  void setMinMaxFare(bool b) { _isMinMaxFare = b; }
  bool isAutoPriceable();

  bool isRuleCategoryIncomplete(const uint16_t& cat);
  bool isRuleCategoryUnavailable(const uint16_t& cat);

  const char getCategoryApplicability(const bool isDutyCode7Or8, const int16_t cat);

  std::set<PaxTypeCode>& passengerTypes() { return _passengerTypes; }
  const std::set<PaxTypeCode>& passengerTypes() const { return _passengerTypes; }

  // FBR data
  const VendorCode& routingVendor() const { return _routingVendor; }
  VendorCode& routingVendor() { return _routingVendor; }
  const TariffNumber& routingTariff1() const { return _routingTariff1; }
  TariffNumber& routingTariff1() { return _routingTariff1; }
  const TariffNumber& routingTariff2() const { return _routingTariff2; }
  TariffNumber& routingTariff2() { return _routingTariff2; }

  const BrandCode& brandCode() const { return _brandCode; }
  BrandCode& brandCode() { return _brandCode; }

  void setBrandCode(const BrandCode& brandCode);
  bool hasPaxType(const PaxTypeCode& ptc);

  // Branded Fares project 2013
  typedef std::pair<ProgramCode, BrandCode> ProgramBrand;
  const ProgramBrand& programBrand() const { return _programBrand; }
  void setProgramBrand(const ProgramBrand& pb);

  const QualifiedBrand& getBrandProgramPair() const
  {
    return _brandProgramPair;
  }

  void setBrandProgramPair(const QualifiedBrand& brandProgramPair)
  {
    _brandProgramPair = brandProgramPair;
  }

  //Price by Cabin
  uint8_t inclusionCabinNum() const { return _inclusionCabinNum; }
  void setPassedInclusion(uint8_t inclusionNum);

  // Clone method
  void clone(FareDisplayInfo* lhs, const PaxTypeFare* pFare = nullptr) const;

private:
  // Prevent accidental or malicious instantiation
  // FareDisplayInfo(const FareDisplayInfo& rhs);
  // FareDisplayInfo& operator=(const FareDisplayInfo& rhs);

  bool convertHoursIn24ToDays(const std::string& advTktPeriod, char& days);

  static int32_t _nextObjId;

  FareDisplayTrx* _trx = nullptr; // Point to my transaction object
  const PaxTypeFare* _paxTypeFare = nullptr; // Point to my paxTypeFare object.

  // Maintain a pointer to the FarePath associated with this Info Object.
  FarePath* _farePath = nullptr;

  // Multi Pax Types
  std::set<PaxTypeCode> _passengerTypes;

  // Maintain a Tax Record Vector
  std::vector<TaxRecord*> _taxRecordVector;

  // Maintain a Tax Item Vector for Tax Breakdown
  std::vector<TaxItem*> _taxItemVector;

  // Tax Record Vector FT requirements
  std::vector<TaxRecord*> _taxRecordOWRTFareTaxVector;

  // Tax Item Vector FT requirements
  std::vector<TaxItem*> _taxItemOWRTFareTaxVector;

  // Cat 3 (Season) info follows.  It's possible to have multiple sets of
  // season data for a single fare.  So, we'll gather the season related
  // data into a struct and keep a vector of them.
  std::vector<SeasonsInfo*> _seasons;

  // Cat 5 (Advance Reservations and Ticketing) Info
  ResPeriod _lastResPeriod;
  ResUnit _lastResUnit;
  ResPeriod _advTktPeriod;
  ResUnit _advTktUnit;

  // Cat 6 (Minimum Stay) and Cat 7 (Maximum Stay) Info
  std::string _minStay; // c3 - 001-999 or 3 character alpha representation of day of week
  // (MON, TUE, WED, THU, FRI, SAT, SUN.)
  // all blanks 'bbb' implies no minimum stay requirement
  // '$$$' implies multiple conditions exist
  std::string _maxStay; // c3 - 001-999 or 3 character alpha representation of day of week
  // (MON, TUE, WED, THU, FRI, SAT, SUN.)
  // all blanks 'bbb' implies no maximum stay requirement
  // '$$$' implies multiple conditions exist
  std::string _maxStayUnit; // c2 - If minimum stay field is numeric:
  //      Nb = Minutes
  //      Hb = Hours
  //      Db = Days
  //      Mb = Months
  //      If minimum stay is alpha: 01 = 1st occurrence (of day of week)
  //                                02-52 = occurrence

  // Note:  The following Cat14 and Cat15 info is combined into the
  //        TRAVEL_TICKET column on the display
  std::vector<TravelInfo*> _travelInfo;
  std::vector<TicketInfo*> _ticketInfo;

  std::vector<TravelInfo*> _travelInfoRD; // used for RD header

  int32_t _myObjId = 0;

  // Save a pointer to a Rule Category rule number
  CatNumber _catNum = 0;
  Indicator _ticketed = ' ';
  Indicator _ruleType = ' ';
  // Category Rule Item Info for NUMBER OF DATA TABLES row
  int16_t _noOfDataTables = 0;
  // Save isLocationSwapped for Surcharges Rule Validation
  bool _isLocationSwappedForSurcharges = false;

  std::string _routingSequence;

  TariffCode _ruleTariffCode; // rule tariff code

  // FB Display
  FBDisplay _fbDisplay;

  const CategoryRuleItemInfo* _catRuleItemInfo = nullptr;

  // Addon Display
  const AddonFareInfo* _addOnFareInfoPtr = nullptr;
  std::map<const AddonFareInfo*, std::string> _addOnRoutingSeq;

  // Indicators for Inhibit='D', Missing/Incomplete rule data and Unavailable Tag='X'
  // If one of these indicators is True, the fare is not auto-priceable
  bool _displayOnly = false;
  bool _incompleteR3Rule = false;
  bool _unavailableR1Rule = false;
  bool _unavailableR3Rule = false;
  bool _unsupportedPaxType = false;

  // true when cat25 fare is from a min/max limit (fareInd == H || L)
  bool _isMinMaxFare = false;

  // Sets for Incomplete/Unavailable rule category numbers
  std::set<uint16_t> _incompleteRuleCatNbrs;
  std::set<uint16_t> _unavailableRuleCatNbrs;

  TariffNumber _routingTariff1 = 0;
  TariffNumber _routingTariff2 = 0;

  // FBR data
  VendorCode _routingVendor;

  // Brand info
  BrandCode _brandCode;

  // Branded Fare project 2013
  ProgramBrand _programBrand;

  QualifiedBrand _brandProgramPair = {nullptr, nullptr};

  // Price by Cabin
  uint8_t _inclusionCabinNum =0;
};

} // namespace tse

