//-------------------------------------------------------------------
//
//  File:        FareDisplayInfo.h
//  Author:      Doug Batchelor
//  Created:     March 10, 2005
//  Description: A class to provide an interface API for the
//               Fare Display Template processing to use to retrieve
//               the data the Display Template must show.
//
//  Updates:
//
//  Copyright Sabre 2007
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
//////////////////////////////////////////////////////////////////////

#include "DataModel/FareDisplayInfo.h"

#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/SeasonsInfo.h"
#include "DataModel/TicketInfo.h"
#include "DataModel/TravelInfo.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"

#include <algorithm>
#include <vector>

namespace tse
{
using namespace std;

namespace
{
// this value defines the initial value of min/max data
static const string MIN_MAX_INITIAL_VALUE = "///";
// this value defines the initial value of Cat 5 res/tkt data
static const string RES_TKT_INITIAL_VALUE = "///";
static const string ADV_TKT_PERIOD_ZERO = "000";
static const string ADV_TKT_UNIT_N = "N";
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int FareDisplayInfo::_nextObjId;

FareDisplayInfo::FareDisplayInfo()
  : _lastResPeriod(RES_TKT_INITIAL_VALUE),
    _lastResUnit("  "),
    _advTktPeriod(RES_TKT_INITIAL_VALUE),
    _advTktUnit(" "),
    _minStay(MIN_MAX_INITIAL_VALUE),
    _maxStay(MIN_MAX_INITIAL_VALUE),
    _maxStayUnit(" ")
{
  _myObjId = _nextObjId++; // Temporary for debugging
}

// Initialize the object with correct pointers
// lint -e{578}
void
FareDisplayInfo::initialize(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare)
{
  _trx = &trx; // Point to my transaction object
  _paxTypeFare = &paxTypeFare; // Point to my paxTypeFare object.

  // Cat 6
  _minStay = MIN_MAX_INITIAL_VALUE;
  _maxStay = MIN_MAX_INITIAL_VALUE;
  _maxStayUnit = " ";

  // Cat 5 (Advance Reservations and Ticketing) Info
  _lastResPeriod = RES_TKT_INITIAL_VALUE;
  _lastResUnit = "  ";
  _ticketed = ' ';
  _advTktPeriod = RES_TKT_INITIAL_VALUE;
  _advTktUnit = " ";

  _displayOnly = false;
  _incompleteR3Rule = false;
  _unavailableR1Rule = false;
  _unavailableR3Rule = false;
  _unsupportedPaxType = false;
}

bool
FareDisplayInfo::isMultiAdvRes()
{
  return (!(_lastResPeriod == RES_TKT_INITIAL_VALUE));
}

bool
FareDisplayInfo::isMultiAdvTkt()
{
  return (!(_advTktPeriod == RES_TKT_INITIAL_VALUE));
}

// Check the Cat 3 (seasons) vector for duplicates
bool
FareDisplayInfo::duplicateSeason(Indicator dir, // Inbound ('I') or Outbound ( 'O')
                                 int startYear, // START: The first date on which travel is
                                                // permitted.
                                 int startMonth, // If year is blank(0), any year is assumed.
                                 int startDay, // If day is blank(0), entire month is assumed.
                                 int stopYear, // STOP: The last date on which travel is permitted.
                                 int stopMonth, // If year is blank (0), any year is assumed.
                                 int stopDay) // If day is blank(0), entire month is assumed.
{
  for (const SeasonsInfo* seasonsInfo : seasons())
  {
    if ((seasonsInfo->tvlstartDay() == startDay) && (seasonsInfo->tvlstartmonth() == startMonth) &&
        (seasonsInfo->tvlstartyear() == startYear) && (seasonsInfo->tvlStopDay()) == stopDay &&
        (seasonsInfo->tvlStopmonth() == stopMonth) && (seasonsInfo->tvlStopyear() == stopYear) &&
        (seasonsInfo->direction() == dir))
    {
      return true;
    }
  }
  return false;
}

// Add an entry into the Cat 3 (seasons) vector
void
FareDisplayInfo::addSeason(Indicator dir, // Inbound ('I') or Outbound ( 'O')
                           int startYear, // START: The first date on which travel is permitted.
                           int startMonth, // If year is blank(0), any year is assumed.
                           int startDay, // If day is blank(0), entire month is assumed.
                           int stopYear, // STOP: The last date on which travel is permitted.
                           int stopMonth, // If year is blank (0), any year is assumed.
                           int stopDay) // If day is blank(0), entire month is assumed.
{
  if (!duplicateSeason(dir, startYear, startMonth, startDay, stopYear, stopMonth, stopDay))
  {
    SeasonsInfo* seasonsInfo(nullptr);

    if (_trx != nullptr)
    { // lint --e{413}
      _trx->dataHandle().get(seasonsInfo);
      seasonsInfo->initialize(dir, startYear, startMonth, startDay, stopYear, stopMonth, stopDay);
      _seasons.push_back(seasonsInfo);
    }
  }
}

// Add an entry into the Cat 14 vector
void
FareDisplayInfo::addTravelInfo(
    const DateTime& earliestTvlStartDate, // COMM:The earliest date on which outbound travel may
                                          // commence.
    const DateTime& latestTvlStartDate, // EXP: The last date on which outbound travel may commence
    const DateTime& stopDate, // COMP:The last date on which travel must commence or be completed.
    Indicator returnTvlInd) // RET: A tag indicating whether return travel must commence or be
//     completed by the following Date/Time.
{
  if (_trx)
  {
    TravelInfo* tiPtr = &_trx->dataHandle().safe_create<TravelInfo>(
        earliestTvlStartDate, latestTvlStartDate, stopDate, returnTvlInd);
    _travelInfo.push_back(tiPtr);
  }
}

// Add an entry into the Cat 15 vector
// Earliest date for ticketing on any one sector.int stopMonth
// Last date for ticketing on all sectors.
void
FareDisplayInfo::addTicketInfo(const DateTime& earliestTktDate, const DateTime& latestTktDate)
{
  if (_trx)
  {
    TicketInfo* tiPtr =
        &_trx->dataHandle().safe_create<TicketInfo>(earliestTktDate, latestTktDate, _ruleType);
    _ticketInfo.push_back(tiPtr);
  }
}

void
FareDisplayInfo::clone(FareDisplayInfo* lhs, const PaxTypeFare* pFare) const
{
  lhs->_trx = this->_trx;
  lhs->_paxTypeFare = pFare == nullptr ? this->_paxTypeFare : pFare;
  lhs->_farePath = this->_farePath;
  lhs->_minStay = this->_minStay;
  lhs->_maxStay = this->_maxStay;
  lhs->_maxStayUnit = this->_maxStayUnit;
  lhs->_lastResPeriod = this->_lastResPeriod;
  lhs->_lastResUnit = this->_lastResUnit;
  lhs->_ticketed = this->_ticketed;
  lhs->_advTktPeriod = this->_advTktPeriod;
  lhs->_advTktUnit = this->_advTktUnit;
  lhs->_routingSequence = this->_routingSequence;

  lhs->_taxRecordVector.insert(
      lhs->_taxRecordVector.end(), this->_taxRecordVector.begin(), this->_taxRecordVector.end());
  lhs->_taxItemVector.insert(
      lhs->_taxItemVector.end(), this->_taxItemVector.begin(), this->_taxItemVector.end());
  lhs->_taxRecordOWRTFareTaxVector.insert(lhs->_taxRecordOWRTFareTaxVector.end(),
                                          this->_taxRecordOWRTFareTaxVector.begin(),
                                          this->_taxRecordOWRTFareTaxVector.end());
  lhs->_taxItemOWRTFareTaxVector.insert(lhs->_taxItemOWRTFareTaxVector.end(),
                                        this->_taxItemOWRTFareTaxVector.begin(),
                                        this->_taxItemOWRTFareTaxVector.end());
  lhs->_inboundSurchargeData.insert(lhs->_inboundSurchargeData.end(),
                                    this->_inboundSurchargeData.begin(),
                                    this->_inboundSurchargeData.end());
  lhs->_outboundSurchargeData.insert(lhs->_outboundSurchargeData.end(),
                                     this->_outboundSurchargeData.begin(),
                                     this->_outboundSurchargeData.end());

  lhs->_seasons.insert(lhs->_seasons.end(), this->_seasons.begin(), this->_seasons.end());
  lhs->_travelInfo.insert(
      lhs->_travelInfo.end(), this->_travelInfo.begin(), this->_travelInfo.end());
  lhs->_ticketInfo.insert(
      lhs->_ticketInfo.end(), this->_ticketInfo.begin(), this->_ticketInfo.end());

  lhs->_displayOnly = this->_displayOnly;
  lhs->_incompleteR3Rule = this->_incompleteR3Rule;
  lhs->_unavailableR1Rule = this->_unavailableR1Rule;
  lhs->_unavailableR3Rule = this->_unavailableR3Rule;
  lhs->_unsupportedPaxType = this->_unsupportedPaxType;
  lhs->_routingVendor = this->_routingVendor;
  lhs->_routingTariff1 = this->_routingTariff1;
  lhs->_routingTariff2 = this->_routingTariff2;
  lhs->_brandCode = this->_brandCode;
  lhs->_isMinMaxFare = this->_isMinMaxFare;
  lhs->_programBrand = this->_programBrand;
  lhs->_inclusionCabinNum = _inclusionCabinNum;
}

// These method determines if fare is auto-priceable
bool
FareDisplayInfo::isAutoPriceable()
{
  return (!_displayOnly && !_incompleteR3Rule && !_unavailableR1Rule && !_unavailableR3Rule &&
          !_unsupportedPaxType);
}

void
FareDisplayInfo::setIncompleteR3Rule(const uint16_t& cat)
{
  _incompleteR3Rule = true;
  _incompleteRuleCatNbrs.insert(cat);
}

void
FareDisplayInfo::setUnavailableR3Rule(const uint16_t& cat)
{
  _unavailableR3Rule = true;
  _unavailableRuleCatNbrs.insert(cat);
}

bool
FareDisplayInfo::isRuleCategoryIncomplete(const uint16_t& cat)
{
  return (_incompleteRuleCatNbrs.find(cat) != _incompleteRuleCatNbrs.end());
}

bool
FareDisplayInfo::isRuleCategoryUnavailable(const uint16_t& cat)
{
  return (_unavailableRuleCatNbrs.find(cat) != _unavailableRuleCatNbrs.end());
}

const char
FareDisplayInfo::getCategoryApplicability(const bool isDutyCode7Or8, const int16_t cat)
{
  return _fbDisplay.getCategoryApplicability(isDutyCode7Or8, cat);
}

bool
FareDisplayInfo::convertHoursIn24ToDays(const std::string& advTktPeriod, char& days)
{
  int32_t aTP = atoi(advTktPeriod.c_str());

  days = CROSS_LORRAINE;

  if (aTP <= 9)
    return false;

  if (aTP > 216 || aTP % 24)
    return true;

  days = static_cast<char>((aTP / 24) + '0');

  return true;
}

bool
FareDisplayInfo::isDefaultTime(string& period)
{
  return (period == ADV_PUR_TKT_DEFAULT || period == ADV_PUR_TKT_3BLANKSPACE);
}

// When invalid period/unit given, return false and set response string to Cross-Lor
// set canConvert to true for advTkt ("48H" shows as "2", "0H" not allowed)
bool
FareDisplayInfo::getAPFormatTime(string& period, string& unit, string& result, bool canConvert)
{
  result = CROSS_LORRAINE;
  //  result.insert(0, " ");

  // special chars to denote multiple periods in rule
  if (period == "$$$")
    return true;

  if (canConvert && unit == ADV_PUR_TKT_UNITS_HOUR)
  {
    char days;
    // If 24H, 48H..216H ( multiple by 24 ) convert it in no of days.
    // Hours more than 9 but not divisible by 24 should display cross of Lorraine.
    if (convertHoursIn24ToDays(advTktPeriod(), days))
    {
      result = days;
      return true;
    }
    // handle unconvered hours below
  }
  // convertable _HOUR (see above) is the only case with 3 digits
  if (period[0] != CHAR_ZERO)
    return false;

  string lastDigit(1, period[2]);

  if (unit == ADV_PUR_TKT_UNITS_DAY || unit == ADV_PUR_TKT_2BLANKSPACE)
  {
    if (period[1] != CHAR_ZERO)
      result = period.substr(1, 2);
    else
      result = lastDigit;
    return true;
  }

  // _MONTH and unconverted _HOUR can only have 1 digit
  if (period[1] != CHAR_ZERO)
    return false;
  if (unit == ADV_PUR_TKT_UNITS_MONTH)
    result = lastDigit + CHAR_M;

  else if (unit == ADV_PUR_TKT_UNITS_HOUR)
  {
    if (lastDigit[0] == CHAR_ZERO)
      result = CROSS_LORRAINE;
    else
      result = lastDigit + CHAR_H;
  }
  else
    result = CHAR_BLANK;

  return true;
}

// false = returned string doesn't need advTkt section
// // (e.g. SIML, NP, ... )
bool
FareDisplayInfo::getAPStringRes(string& field)
{
  field.clear();

  // Check for NP condition (Not auto-Priceable)
  if (!isAutoPriceable())
  {
    field = "NP";
    return false;
  }
  // explicitly filed as simultaneous
  if (advTktPeriod() == ADV_TKT_PERIOD_ZERO && advTktUnit() == ADV_TKT_UNIT_N)
  {
    field = "SIML ";
    return false;
  }
  // can only assume simultaneous if no periods given & ticket flag
  if (isDefaultTime(lastResPeriod()) && isDefaultTime(advTktPeriod()))
  {
    if (ticketed() == 'X')
    {
      field = "SIML ";
      return false;
    }
    // special case when no ticket flag
    if (advTktPeriod() == ADV_PUR_TKT_3BLANKSPACE && (!ticketed() || ticketed() == ' '))
    {
      field = CHAR_DASH;
      return false;
    }
  }

  return true;
}

// for 2-char AP field (Abacus)
void
FareDisplayInfo::getAPStringShort(string& field)
{
  string res, tkt;

  field = CROSS_LORRAINE;
  field.insert(0, 1, CHAR_BLANK); // pad in front

  // string without tkt data
  if (!getAPStringRes(res))
  {
    if (res.length() < 2)
      field = " " + res;
    else if (res.length() == 2)
      field = res;
    return;
  }

  if (ticketed() && ticketed() != ' ')
    return;

  if (getAPFormatTime(advTktPeriod(), advTktUnit(), tkt, true))
    return;

  if (isDefaultTime(lastResPeriod()))
    field = BLANK_DASH;
  else if (getAPFormatTime(lastResPeriod(), lastResUnit(), res, false))
    field = res;

  if (field.length() == 1)
    field.insert(0, 1, CHAR_BLANK); // pad in front
}

void
FareDisplayInfo::getAPString(string& field)
{
  string res, tkt;
  // string without tkt data
  if (!getAPStringRes(res))
  {
    if (res.length() < 3)
      field = " " + res;
    else
      field = res;
  }
  // string with tkt data
  else
  {
    if (isDefaultTime(lastResPeriod()))
      res = BLANK_DASH;
    else if (!getAPFormatTime(lastResPeriod(), lastResUnit(), res, false))
      field = res;

    if (advTktPeriod() == ADV_PUR_TKT_3BLANKSPACE && ticketed())
      tkt = CHAR_S;
    else if (!getAPFormatTime(advTktPeriod(), advTktUnit(), tkt, true))
      tkt.clear();

    // center justify
    if (res.length() == 1)
      res.insert(0, 1, CHAR_BLANK); // pad in front

    field = res;
    if (!tkt.empty())
      field += CHAR_SLASH + tkt;
  }

  // pad in back
  while (field.length() < 5)
    field += " ";
}

// Set up brand code
void
FareDisplayInfo::setBrandCode(const BrandCode& brandCode)
{
  _brandCode = brandCode;
}

// Branded Fares project 2013
void
FareDisplayInfo::setProgramBrand(const ProgramBrand& pb)
{
  _programBrand = pb;
}

// Price By Cabin
void
FareDisplayInfo::setPassedInclusion(uint8_t inclusionNum)
{
  _inclusionCabinNum = inclusionNum;
}

bool
FareDisplayInfo::hasPaxType(const PaxTypeCode& requestedPTC)
{
  if (_paxTypeFare->actualPaxType() == nullptr)
  {
    if (requestedPTC == "ADT")
      return true;
  }
  else
  {
    if (requestedPTC == _paxTypeFare->actualPaxType()->paxType())
      return true;
  }
  return passengerTypes().find(requestedPTC) != passengerTypes().end();
}

void
FareDisplayInfo::addRDTravelInfo(const DateTime& earliestTvlStartDate, // COMM:
                                 const DateTime& latestTvlStartDate, // EXP:
                                 const DateTime& stopDate, // COMP:
                                 Indicator returnTvlInd) // RET:
{
  if (_trx)
  {
    TravelInfo* tiPtr = &_trx->dataHandle().safe_create<TravelInfo>(
        earliestTvlStartDate, latestTvlStartDate, stopDate, returnTvlInd);
    _travelInfoRD.push_back(tiPtr);
  }
}
} // tse
