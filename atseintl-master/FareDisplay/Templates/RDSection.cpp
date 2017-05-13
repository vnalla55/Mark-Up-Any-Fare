//-------------------------------------------------------------------
//
//  File:        RDSection.cpp
//  Authors:     Doug Batchelor
//  Created:     May 4, 2005
//  Description: This class abstracts a section.  It maintains
//               all the data and methods necessary to describe
//               and realize an RD response.
//
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
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/RDSection.h"

#include "BookingCode/FareDisplayBookingCode.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayResponseUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/TravelInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/RuleCategoryDescInfo.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/ElementFilter.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/FQCurrencyConversionSection.h"
#include "FareDisplay/Templates/ICSection.h"
#include "FareDisplay/Templates/RoutingSection.h"
#include "FareDisplay/Templates/RRSection.h"
#include "Routing/RoutingConsts.h"
#include "Rules/RuleConst.h"

#include <map>
#include <sstream>

namespace tse
{
static Logger
logger("atseintl.FareDisplay.RDSection");

namespace
{
ConfigurableValue<bool>
enableNewRDHeader("FAREDISPLAY_SVC", "NEW_RDHEADER", false);
}

// Define static footnote type names (short and long versions)
const std::string RDSection::_longFNTypeNames[] = {
    "ORIGIN ADDON", "PUBLISHED FARE", "DESTINATION ADDON"};
const std::string RDSection::_shortFNTypeNames[] = {"ORIG ADDON", "PUBLISHED", "DEST ADDON"};

RDSection::RDSection(FareDisplayTrx& trx) : Section(trx)
{
  _useNewRDHeader = enableNewRDHeader.getValue();
}

void
RDSection::buildDisplay()
{
  // Make sure we have some fares
  LOG4CXX_DEBUG(
      logger,
      "Entering RDSection::buildDisplay(), allPaxTypefare size = " << _trx.allPaxTypeFare().size());
  if (_trx.allPaxTypeFare().empty())
    return;

  std::ostringstream* oss = &_trx.response();

  FareDisplayOptions& fdo = *(_trx.getOptions());
  PaxTypeFare& paxTypeFare = *_trx.allPaxTypeFare().front();
  FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();
  if (fareDisplayInfo == nullptr)
  {
    LOG4CXX_WARN(logger, "No FareDisplayInfo!");
    return;
  }

  addPassengerTypeLine(paxTypeFare);
  addOriginDestinationLine(paxTypeFare);
  addFareBasisLine(paxTypeFare);
  addFareTypeLine(paxTypeFare);
  addSITAFareInfoLine(paxTypeFare);
  addNETFareInfoLine(paxTypeFare);

  if (_useNewRDHeader)
    addFootNotesLine(paxTypeFare);

  // Prepare for Currency Line Display
  CurrencyCode currency = paxTypeFare.currency();
  MoneyAmount amount =
      paxTypeFare.isRoundTrip() ? paxTypeFare.originalFareAmount() : paxTypeFare.fareAmount();
  CurrencyNoDec dec = paxTypeFare.numDecimal();
  RoutingNumber routing = paxTypeFare.routingNumber();
  DateTime createDate = paxTypeFare.createDate();
  DateTime fareDisc = paxTypeFare.expirationDate();

  FareDisplayResponseUtil::getEffDate(_trx, paxTypeFare, *fareDisplayInfo, _effDate);
  FareDisplayResponseUtil::getDiscDate(_trx, paxTypeFare, *fareDisplayInfo, _discDate);

  FareClassCode fareClass = paxTypeFare.fareClass();

  bool construct = paxTypeFare.hasConstructedRouting();
  bool constructInfo = paxTypeFare.hasConstructedRouting();

  if (routing.empty() && constructInfo)
  {
    routing = paxTypeFare.fare()->constructedFareInfo()->fareInfo().routingNumber();
  }

  buildCurrencyLine(currency,
                    amount,
                    dec,
                    routing,
                    createDate,
                    fareDisc,
                    _effDate,
                    _discDate,
                    fareClass,
                    paxTypeFare.fareTariff(),
                    paxTypeFare.tcrFareTariffCode(),
                    paxTypeFare.footNote1(),
                    paxTypeFare.footNote2(),
                    construct);

  if (_trx.getOptions()->routingDisplay() == TRUE_INDICATOR)
  {
    RoutingSection routingSection(_trx);
    routingSection.buildDisplay();
    return;
  }
  else
    addBlankLine();

  if (_trx.getOptions()->ruleMenuDisplay() == TRUE_INDICATOR)
  {
    buildMenu(*fareDisplayInfo);
  }
  else if (_trx.getOptions()->headerDisplay() != TRUE_INDICATOR)
  {
    bool showMX = false;
    showMX = fdo.isCombScoreboardDisplay();
    bool needMX = true;
    CatNumber crc = COMBINABILITY_RULE_CATEGORY;

    // Special case first
    if ((_trx.getOptions()->ruleCategories().empty()) && showMX)
    {
      LOG4CXX_DEBUG(logger, "MX found, putting in now... before: " << crc);
      needMX = false;
      addCatDescription(crc, *fareDisplayInfo, oss);
      FareDisplayResponseUtil::addMXText(*fareDisplayInfo, oss);
    }
    else
    {
      for (const CatNumber& catNumber : _trx.getOptions()->ruleCategories())
      {
        if (needMX && showMX)
        {
          // need to be looking for proper place in sequence to insert MX text.
          if (catNumber == COMBINABILITY_RULE_CATEGORY)
          {
            LOG4CXX_DEBUG(logger, "MX found, putting in now... with: " << catNumber);
            needMX = false;
            addCatDescription(catNumber, *fareDisplayInfo, oss);
            FareDisplayResponseUtil::addMXText(*fareDisplayInfo, oss);
            FareDisplayResponseUtil::addCatText(_trx, catNumber, *fareDisplayInfo, oss);
            continue;
          }
          else if (catNumber > COMBINABILITY_RULE_CATEGORY && catNumber != 50)
          {
            LOG4CXX_DEBUG(logger, "MX found, putting in now... before: " << catNumber);
            needMX = false;
            addCatDescription(crc, *fareDisplayInfo, oss);
            FareDisplayResponseUtil::addMXText(*fareDisplayInfo, oss);
          }
          else if (&catNumber == &(_trx.getOptions()->ruleCategories().back()))
          {
            LOG4CXX_DEBUG(logger, "MX found, putting in now... after: " << catNumber);
            needMX = false;
            addCatInfo(catNumber, *fareDisplayInfo, oss);
            addCatDescription(crc, *fareDisplayInfo, oss);
            FareDisplayResponseUtil::addMXText(*fareDisplayInfo, oss);
            continue;
          }
        }
        addCatInfo(catNumber, *fareDisplayInfo, oss);

        if (TrxUtil::isFqFareRetailerEnabled(_trx))
        {
          addRetailerCategoryDetails(catNumber, paxTypeFare);
          addIntlConstructionDetails(catNumber, paxTypeFare);
        }
      }
    }
    // Invalid Alpha codes and Categories
    addInvalidEntries();

    if (!TrxUtil::isFqFareRetailerEnabled(_trx))
      addIntlConstructionDetails(IC_RULE_CATEGORY, paxTypeFare);
  }
}

void
RDSection::addRetailerCategoryDetails(const CatNumber& catNumber, PaxTypeFare& paxTypeFare)
{
  if (!FareDisplayUtil::isFrrCustomer(_trx))
  {
    return;
  }

  if (catNumber == RETAILER_CATEGORY)
  {
    if (_trx.getOptions()->retailerDisplay() == TRUE_INDICATOR)
    {
      RRSection* rrSection = &_trx.dataHandle().safe_create<RRSection>(_trx);
      rrSection->buildDisplay(paxTypeFare);
    }
  }
}

void
RDSection::addIntlConstructionDetails(const CatNumber& catNumber, PaxTypeFare& paxTypeFare)
{
  if (catNumber == IC_RULE_CATEGORY &&
      _trx.getOptions()->IntlConstructionDisplay() == TRUE_INDICATOR)
  {
    if (!paxTypeFare.isConstructed())
    {
      _trx.response() << "   NOT A CONSTRUCTED FARE" << std::endl;
    }
    else
    {
      addBlankLine();
      // We need an IC section
      ICSection* icSection = &_trx.dataHandle().safe_create<ICSection>(_trx);
      icSection->buildDisplay(paxTypeFare);

      // Currency Conversion Section
      FQCurrencyConversionSection* fqCurrencyConversionSection =
          &_trx.dataHandle().safe_create<FQCurrencyConversionSection>(_trx);
      fqCurrencyConversionSection->buildDisplay();
    }
  }
}

void
RDSection::addNETFareInfoLine(PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.fareDisplayCat35Type() == RuleConst::NET_FARE)
  {
    std::ostringstream* oss = &_trx.response();
    *oss << "NET FARE FOR INFORMATIONAL PURPOSES ONLY" << std::endl;
  }
}

// -------------------------------------------------------------------------
// RDSection::addPassengerTypeLine()
// -------------------------------------------------------------------------
void
RDSection::addPassengerTypeLine(PaxTypeFare& paxTypeFare)
{
  std::ostringstream oss[2];
  initializeLine(&oss[0], 1);

  oss[0].seekp(0, std::ios_base::beg);

  oss[0] << "PASSENGER TYPE-";
  if (paxTypeFare.actualPaxType())
  {
    if (paxTypeFare.actualPaxType()->paxType().empty())
    {
      oss[0] << ADULT;
    }
    else
    {
      oss[0] << paxTypeFare.actualPaxType()->paxType();
    }
    // Handle up to 5 pax type codes
    FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();
    if (fareDisplayInfo != nullptr && !fareDisplayInfo->passengerTypes().empty())
    {
      std::set<PaxTypeCode>::const_iterator iter = fareDisplayInfo->passengerTypes().begin();
      std::set<PaxTypeCode>::const_iterator iterEnd = fareDisplayInfo->passengerTypes().end();
      for (uint16_t i = 0; iter != iterEnd && i < 4; iter++, ++i)
      {
        if (paxTypeFare.actualPaxType()->paxType() != *iter)
        {
          oss[0] << SLASH;
          oss[0] << (*iter);
        }
      }
    }
  }

  ElementField field;
  oss[0].seekp(35, std::ios_base::beg);
  ElementFilter::autoPrice(field, paxTypeFare, _trx);
  oss[0] << "AUTO PRICE-" << field.strValue();

  if (!oss[0].str().empty())
  {
    _trx.response() << oss[0].str();
  }
  if (!oss[1].str().empty())
  {
    _trx.response() << oss[1].str() << std::endl;
  }
}

void
RDSection::addOriginDestinationLine(PaxTypeFare& paxTypeFare)
{
  std::ostringstream* oss = &_trx.response();

  if (_useNewRDHeader)
  {
    *oss << "FILED " << FareDisplayUtil::getFareOrigin(&paxTypeFare);
    *oss << "-" << FareDisplayUtil::getFareDestination(&paxTypeFare);
    spaces(6);
  }
  else
  {
    *oss << "FROM-" << FareDisplayUtil::getFareOrigin(&paxTypeFare);
    spaces(1);
    *oss << "TO-" << FareDisplayUtil::getFareDestination(&paxTypeFare);
    spaces(4);
  }

  *oss << "CXR-" << paxTypeFare.carrier();
  spaces(4);

  *oss << "TVL-";
  if (_trx.travelDate().isValid())

  {
    *oss << TseUtil::getTravelDate(_trx.travelSeg()).dateToString(DDMMMYY, "");
  }
  else
  {
    spaces(7);
  }
  spaces(2);
  *oss << "RULE-" << paxTypeFare.ruleNumber();
  spaces(1);

  std::string ruleTariffCode = FareDisplayResponseUtil::getRuleTariffCode(_trx, paxTypeFare);
  if (!ruleTariffCode.empty())
    *oss << ruleTariffCode;
  if (paxTypeFare.tcrRuleTariff() != 0)
    *oss << SLASH << paxTypeFare.tcrRuleTariff();
  *oss << std::endl;
}

void
RDSection::addFareBasisLine(PaxTypeFare& paxTypeFare)
{
  std::ostringstream* oss = &_trx.response();
  *oss << "FARE BASIS-";
  oss->setf(std::ios::left, std::ios::adjustfield);
  *oss << std::setfill(' ') << std::setw(14) << paxTypeFare.createFareBasisCodeFD(_trx);
  spaces(4);

  ElementField field;
  ElementFilter::pricingCat(field, paxTypeFare);
  *oss << std::setfill(' ') << std::setw(11) << field.strValue();
  spaces(2);

  ElementFilter::displayCatType(field, paxTypeFare);
  *oss << "DIS-" << field.strValue();
  spaces(3);

  *oss << "VENDOR-" << paxTypeFare.vendor() << std::endl;
}

void
RDSection::addFareTypeLine(PaxTypeFare& paxTypeFare)
{
  std::ostringstream* oss = &_trx.response();
  *oss << "FARE TYPE-" << paxTypeFare.fcaFareType();
  spaces(6);

  if (paxTypeFare.isRoundTrip())
    *oss << ROUND_TRIP << DASH;
  else
    *oss << ONE_WAY_TRIP << DASH;
  *oss << FareDisplayResponseUtil::getFareDescription(_trx, paxTypeFare) << std::endl;
}

void
RDSection::addSITAFareInfoLine(PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.vendor() != SITA_VENDOR_CODE)
  {
    return;
  }

  const SITAFareInfo* sitaFareInfo =
      dynamic_cast<const SITAFareInfo*>(paxTypeFare.fare()->fareInfo());
  if (sitaFareInfo == nullptr)
    return;

  std::ostringstream* oss = &_trx.response();

  *oss << "TARIFF FAMILY-";
  oss->setf(std::ios::left, std::ios::adjustfield);
  *oss << std::setfill(' ') << std::setw(1) << paxTypeFare.tariffFamily();
  spaces(4);

  *oss << "DBE-";
  *oss << std::setfill(' ') << std::setw(3) << paxTypeFare.dbeClass();
  spaces(4);

  *oss << "FARE QUALITY-";
  *oss << std::setfill(' ') << std::setw(1) << paxTypeFare.fareQualCode();
  spaces(4);

  *oss << "ROUTE CODE-";
  *oss << std::setfill(' ') << std::setw(2) << paxTypeFare.routeCode() << std::endl;
}

bool
RDSection::buildFootNotesString(const Footnote& footNote1,
                                const Footnote& footNote2,
                                std::string& output)
{
  // Check if there is at least one footnote
  if (footNote1.empty() && footNote2.empty())
    return false;

  output = footNote1;

  // Build footnotes string
  if (!footNote2.empty())
    output += SLASH + footNote2;

  return true;
}

void
RDSection::addFootNotesLine(const PaxTypeFare& paxTypeFare)
{
  std::map<FootNoteType, std::string> footNotesMap;
  std::string footNotesString;
  std::ostringstream& oss = _trx.response();

  // Print global prefix
  oss << "FOOTNOTES-";

  // Try to build published footnotes string
  if (buildFootNotesString(paxTypeFare.footNote1(), paxTypeFare.footNote2(), footNotesString))
    footNotesMap[FootNoteType::PUBLISHED] = footNotesString;

  // Only constructed fares may have addons
  if (paxTypeFare.hasConstructedRouting())
  {
    // Try to build string from origin addon footnotes
    if (buildFootNotesString(
            paxTypeFare.origAddonFootNote1(), paxTypeFare.origAddonFootNote2(), footNotesString))
      footNotesMap[FootNoteType::ORIG_ADDON] = footNotesString;

    // Try to build string from destination addon footnotes
    if (buildFootNotesString(
            paxTypeFare.destAddonFootNote1(), paxTypeFare.destAddonFootNote2(), footNotesString))
      footNotesMap[FootNoteType::DEST_ADDON] = footNotesString;
  }

  // Get size of footnotes container
  unsigned int footNoteCount = footNotesMap.size();

  // If container is empty, there are no footnotes to display
  if (footNoteCount == 0)
  {
    oss << "NO FOOTNOTES WERE FILED ON THIS FARE";
  }
  // If container contains only one footnote, display long version of information
  else if (footNoteCount == 1)
  {
    std::map<FootNoteType, std::string>::iterator iter = footNotesMap.begin();

    oss << "FILED ON THE " << _longFNTypeNames[iter->first] << "-" << iter->second;
  }
  // If container contains several footnotes, display all of them in short version
  else
  {
    unsigned int column = 0;
    // Display all available footnote types
    for (const auto& footNote : footNotesMap)
    {
      std::string output = _shortFNTypeNames[footNote.first] + "-" + footNote.second;

      // Maintain required column formatting
      if (column == 0)
        oss << std::setw(18);
      else
        oss << std::setw(17);

      oss << std::setfill(' ') << output;

      ++column;
    }
  }
  oss << std::endl;
}

bool
RDSection::addCatDescription(const CatNumber& catNumber,
                             FareDisplayInfo& fareDisplayInfo,
                             std::ostringstream* oss)
{
  const RuleCategoryDescInfo* descriptionInfo = _trx.dataHandle().getRuleCategoryDesc(catNumber);
  if (descriptionInfo == nullptr)
  {
    LOG4CXX_ERROR(logger, "No description for cat: " << catNumber);
    return false;
  }
  fareDisplayInfo.catNum() = catNumber;

  switch (catNumber)
  {
  case IC_RULE_CATEGORY:
    fareDisplayInfo.catNum() = catNumber;
    // Show rule number and description for International Construction
    *oss << INTL_CONST_CODE << PERIOD << descriptionInfo->description() << std::endl;
    break;
  case RETAILER_CATEGORY:
    fareDisplayInfo.catNum() = catNumber;
    *oss << RETAILER_CODE << PERIOD << descriptionInfo->description() << std::endl;
    break;
  default:
    oss->setf(std::ios::right, std::ios::adjustfield);
    // Show rule number and description
    *oss << std::setfill('0') << std::setw(2) << catNumber << PERIOD
         << descriptionInfo->description() << std::endl;
    break;
  }
  return true;
}

// -------------------------------------------------------------------------
// RDSection::addCatInfo()
// -------------------------------------------------------------------------
void
RDSection::addCatInfo(const CatNumber& catNumber,
                      FareDisplayInfo& fareDisplayInfo,
                      std::ostringstream* oss)
{
  if (!FareDisplayUtil::isFrrCustomer(_trx))
  {
    if (catNumber == RETAILER_CATEGORY)
      return;
  }

  if (addCatDescription(catNumber, fareDisplayInfo, oss))
    FareDisplayResponseUtil::addCatText(_trx, catNumber, fareDisplayInfo, oss);
}

void
RDSection::addInvalidEntries()
{
  std::ostringstream& oss = _trx.response();

  std::vector<AlphaCode>& badAlphaCodes = _trx.fdResponse()->badAlphaCodes();
  std::vector<CatNumber>& badCategories = _trx.fdResponse()->badCategoryNumbers();
  if (badAlphaCodes.empty() && badCategories.empty())
  {
    LOG4CXX_DEBUG(logger, "No bad alpha codes, or categories");
    return;
  }

  addBlankLine();
  oss << "CATEGORY ";

  for (const auto& badCode : badAlphaCodes)
    oss << badCode << SLASH;

  for (const auto catNumber : badCategories)
    oss << catNumber << SLASH;

  oss.seekp(-1, std::ios_base::cur);
  oss << " INVALID" << std::endl;
}

namespace
{
std::string
formatCatNumber(CatNumber cat)
{
  if (cat == IC_RULE_CATEGORY)
    return INTL_CONST_CODE;

  std::ostringstream number;
  number.setf(std::ios::right, std::ios::adjustfield);
  number << std::setfill('0') << std::setw(2) << cat;
  return number.str();
}
}

std::string
RDSection::formatMenuItem(FareDisplayInfo& fareDisplayInfo, bool dutyCode78, const MenuItem& item)
    const
{
  const CatNumber& catNumber = item.first;

  std::ostringstream stream;

  // Incomplete/Unavailable indicators
  if (fareDisplayInfo.isRuleCategoryIncomplete(catNumber))
    stream << INCOMPLETE;
  else if (fareDisplayInfo.isRuleCategoryUnavailable(catNumber))
    stream << UNAVAILABLE;
  else
    stream << SPACE;

  stream << fareDisplayInfo.getCategoryApplicability(dutyCode78, catNumber)
         << formatCatNumber(catNumber) << PERIOD << *item.second;

  return stream.str();
}

std::string
RDSection::buildMenuItems(FareDisplayInfo& fareDisplayInfo,
                          bool dutyCode78,
                          const std::vector<MenuItem>& items)
{
  std::string buff;
  std::ostringstream line;

  const static unsigned width = MAX_PSS_LINE_SIZE / 3;
  const static unsigned columnWidth[3] = {width + 1, width - 1, width};
  const static unsigned doubleColumnWidth[3] = {2 * width, 2 * width - 1, 2 * width};

  std::vector<MenuItem>::const_iterator item = items.begin();

  for (unsigned i = 0; item != items.end(); ++item, ++i)
  {
    unsigned column = i % 3;

    bool longItem = item->second->size() > width - 4;

    const unsigned& itemWidth = longItem ? doubleColumnWidth[column] : columnWidth[column];

    if (longItem)
    {
      if (column == 2)
      {
        ++i;
        line << '\n';
      }
      ++i;
      ++column;
    }

    line << std::setw(itemWidth) << std::left << formatMenuItem(fareDisplayInfo, dutyCode78, *item);

    if (column == 2)
    {
      buff += line.str() + '\n';
      line.str("");
    }
  }

  if (!line.str().empty())
    buff += line.str() + '\n';

  return buff;
}

void
RDSection::buildMenu(FareDisplayInfo& fareDisplayInfo)
{
  // Get agent duty code
  Agent* agent = _trx.getRequest()->ticketingAgent();
  bool dutyCode78 =
      ((agent->agentDuty() == "7") || (agent->agentDuty() == "8") || (agent->agentDuty() == "$"));

  // Categories for menu
  std::vector<MenuItem> items;
  items.reserve(_trx.getOptions()->ruleCategories().size());

  for (const auto& ruleCatNumber : _trx.getOptions()->ruleCategories())
  {
    const RuleCategoryDescInfo* descriptionInfo =
        _trx.dataHandle().getRuleCategoryDesc(ruleCatNumber);

    if (descriptionInfo == nullptr)
    {
      LOG4CXX_WARN(logger, "No description for cat: " << ruleCatNumber);
      continue;
    }

    items.push_back(make_pair(ruleCatNumber, &descriptionInfo->shortDescription()));
  }

  _trx.response() << buildMenuItems(fareDisplayInfo, dutyCode78, items);

  addBlankLine();

  _trx.response() << " CATEGORIES WITH * REFLECT FARE RULE DATA\n"
                  << " CATEGORIES WITH $ REFLECT GENERAL RULE DATA\n"
                  << " CATEGORIES WITH - REFLECT FOOTNOTE DATA\n"
                  << " CATEGORIES WITH / REFLECT ANY COMBINATION OF ABOVE\n"
                  << " CATEGORIES WITH I REFLECT INCOMPLETE RECORD 3\n"
                  << " CATEGORIES WITH U REFLECT UNAVAILABLE RECORD 3\n";
}
} // tse namespace
