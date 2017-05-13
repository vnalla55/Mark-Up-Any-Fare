//-------------------------------------------------------------------
//
//  File:        RTGController
//  Copyright Sabre 2004
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

#include "RTG/RTGController.h"

#include "BookingCode/FareDisplayBookingCodeRB.h"
#include "BookingCode/RBData.h"
#include "ClientSocket/ClientSocket.h"
#include "Common/ErrorResponseException.h"
#include "Common/FareDisplayUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/Logger.h"
#include "Common/TseEnums.h"
#include "Common/TSEException.h"
#include "Common/TseSrvStats.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/BookingCodeExceptionSegment.h"
#include "DBAccess/BookingCodeExceptionSequence.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/NegFareSecurityInfo.h"
#include "RTG/RBRequestFormatter.h"
#include "RTG/RuleResponseContentHandler.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

#include <sstream>
#include <string>
#include <vector>

namespace tse
{
namespace
{
Logger
logger("atseintl.RTG.RTGController");

ClientSocketConfigurableValues cscv("RTG_SERVER");
}

const std::string RTGController::SEPARATOR_LINE =
    "---------------------------------------------------------------";

const std::string RTGController::FARE_BY_RULE = "**FARE BY RULE**";
const std::string RTGController::BASE_FARE = "**BASE FARE**";
const std::string RTGController::FARE_RULE = "  FARE RULE";
const std::string RTGController::FOOTNOTE_RULE = "  FOOTNOTE RULE";
const std::string RTGController::GENERAL_RULE = "  GENERAL RULE - APPLY UNLESS OTHERWISE SPECIFIED";
const std::string RTGController::ADDITIONAL = "  ADDITIONALLY, THE FOLLOWING RULES APPLY-";
const std::string RTGController::CAT_10 = "  THIS CAT 10 RECORD 2 HAS NO SUB-CATEGORIES";
const std::string RTGController::NOT_VALIDATED_1 =
    "  NOTE - THE FOLLOWING TEXT IS INFORMATIONAL AND NOT VALIDATED";
const std::string RTGController::NOT_VALIDATED_2 = "  FOR AUTOPRICING.";
const std::string RTGController::FARE_RULE_HEADER = "   FARE RULE";
const std::string RTGController::GENERAL_RULE_HEADER = "  GENERAL RULE";

bool
RTGController::getRuleText(FBCategoryRuleRecord* ruleRecord,
                           PaxTypeFare& ptf,
                           const Agent& agent,
                           const Billing& billing,
                           std::string& repository,
                           CatNumber cat,
                           bool isMinMaxFare,
                           CatNumber subCat)
{
  LOG4CXX_INFO(logger, "Getting rule text for category=" << cat);
  bool isDom = ruleRecord->isRecordScopeDomestic();
  uint16_t ruleCount = 0; // headers needed for two more rules in a category
  bool needHeader = false; // for certain categories a rule header is required in RD display
  bool needBlankLine = false; // need a blank line for second rule  if a header already returned
  if (cat == 5 || cat == 8 || cat == 9 || cat == 11 || cat == 12 || cat == 15 || cat == 16)
  { // only these categories need rule headings
    if (ruleRecord->hasFootNoteRecord2Info())
    {
      ++ruleCount;
    }
    if (ruleRecord->hasFareRuleRecord2Info())
    {
      if (ruleCount == 1)
        needBlankLine = true; // need extra blank line if already returned footnote
      ++ruleCount;
    }
    if (ruleRecord->hasGeneralRuleRecord2Info())
    {
      ++ruleCount;
    }
    if (ruleCount > 1)
    {
      needHeader = true; // need header for these categories because of multiple rules
    }
  }

  // TODO similar vector for Cat 19 pax types

  repository.clear();

  // cat25 can skip precendence stuff (only one rule)
  if (ptf.isFareByRule() && ruleRecord->hasFareByRuleRecord2Info())
  {
    LOG4CXX_INFO(logger, "\tAttempt to get FareByRule rule text");
    if (!getFareByRuleText(isDom,
                           isMinMaxFare,
                           agent,
                           billing,
                           ruleRecord->fareByRuleRecord2Info(),
                           ptf,
                           repository))
      return false;
    return (!repository.empty());
  }

  // footnote first (precedence reqs)
  if (ruleRecord->hasFootNoteRecord2Info())
  {
    LOG4CXX_INFO(logger, "\tAttempt to get footnote rule text");
    if (needHeader)
    {
      repository += "\n   FOOTNOTE RULE";
    }
    if (!getFootnoteText(isDom, agent, billing, ruleRecord->footNoteRecord2Info(), ptf, repository))
      return false;
    if (!repository.empty() && cat != 11 && cat != 12 && cat != 15)
      return true;
  }
  // then fare rule
  std::string repo2;
  if (ruleRecord->hasFareRuleRecord2Info())
  {
    LOG4CXX_INFO(logger, "\tAttempt to get fare rule text");
    if (!getFareText(isDom, agent, billing, ruleRecord->fareRuleRecord2Info(), ptf, repo2))
      return false;
    if (!repo2.empty())
    {
      if (needHeader)
      {
        if (needBlankLine)
          repository += "\n "; // extra blank line before header
        repository += "\n   FARE RULE";
      }
      repository += repo2;
      // TODO Cat19-22 needs to check general for paxtypes not found
      if (cat != 5 && cat != 8 && cat != 9 && cat != 11 && cat != 12 && cat != 15 &&
          cat != 16 /*&& cat != 19*/)
        return true;
    }
  }
  // then combinality
  if (ruleRecord->hasCombinabilityRecord2Info())
  {
    LOG4CXX_INFO(logger, "\tAttempt to get combinality rule text");
    repo2.clear();
    if (!getCombinalityText(
            isDom, agent, billing, ruleRecord->combinabilityRecord2Info(), repo2, subCat, ptf))
      return false;

    repository += repo2;
    // only attempt general if none other found
    if (!repository.empty() /*&& cat != 19*/)
      return true;
  }

  // general rule
  if (ruleRecord->hasGeneralRuleRecord2Info())
  {
    LOG4CXX_INFO(logger, "\tAttempt to get general rule text");
    bool processGeneralRule(true);

    if (ruleRecord->hasFareRuleRecord2Info())
    {
      const GeneralRuleRecord2Info* genRuleInfo = ruleRecord->generalRuleRecord2Info();

      const FareRuleRecord2Info* fareRuleInfo = ruleRecord->fareRuleRecord2Info();

      if (genRuleInfo->vendorCode() == fareRuleInfo->vendorCode() &&
          genRuleInfo->carrierCode() == fareRuleInfo->carrierCode() &&
          genRuleInfo->tariffNumber() == fareRuleInfo->tariffNumber() &&
          genRuleInfo->ruleNumber() == fareRuleInfo->ruleNumber() &&
          genRuleInfo->sequenceNumber() == fareRuleInfo->sequenceNumber() &&
          genRuleInfo->categoryNumber() == fareRuleInfo->categoryNumber())
      {
        LOG4CXX_INFO(logger, "\tGeneral rule is duplicate of fare rule. Skipping...");
        processGeneralRule = false;
      }
    }

    if (processGeneralRule)
    {
      repo2.clear();
      if (!getGeneralText(isDom, agent, billing, ruleRecord->generalRuleRecord2Info(), ptf, repo2))
        return false;

      if (needHeader)
      {
        repository += "\n "; // always extra blank line for this header
        repository += "\n   GENERAL RULE - APPLY UNLESS OTHERWISE SPECIFIED";
      }

      repository += repo2;
    }
    if (!repository.empty() /*&& cat != 19*/)
      return true;
  }
  LOG4CXX_INFO(logger, "\tNo rule text found");
  return true;
}

bool
RTGController::formatRuleInfo(XMLConstruct& construct,
                              GenerateRuleRequestFormatter& formatter,
                              const CategoryRuleInfo* catRuleInfo,
                              PaxTypeFare* ptf)
{
  if (ptf == nullptr)
    return false;
  uint16_t cat = catRuleInfo->categoryNumber();
  bool dispAll = true;
  bool foundSet = false;

  switch (cat)
  {
  case 19:
  case 20:
  case 21:
  case 22:
    if (ptf->isDiscounted())
    {
      dispAll = false;
      // extra tag for base fare discounted
      PaxTypeFare* baseFare = ptf->baseFare(19);
      std::string baseFareBasis = baseFare->createFareBasis(_trx);
      formatter.addDISType(
          baseFareBasis, baseFare->owrt(), baseFare->actualPaxType()->paxType(), construct);
      foundSet = formatDiscFareRuleItem(construct, formatter, *ptf, catRuleInfo);
      if (!foundSet)
      {
        LOG4CXX_ERROR(logger, "Rule Set not found for category " << cat);
      }
    }
    break;

  case 25:
  case 35:
  {
    dispAll = false;
    // only do one rule (w/ qualifiers)
    PaxTypeFareRuleData* ptfRuleData = ptf->paxTypeFareRuleData(cat);
    if (!ptfRuleData)
    {
      LOG4CXX_ERROR(logger, "Missing ptfRuleData for category " << cat);
      return false;
    }
    // use cat rule data from PTF since it has results of rule matching
    foundSet = formatOneRuleItem(
        construct, formatter, ptfRuleData->categoryRuleInfo(), ptfRuleData->categoryRuleItemInfo());
    if (!foundSet)
    {
      LOG4CXX_ERROR(logger, "Rule Set not found for category " << catRuleInfo->categoryNumber());
    }
  }
  break;

  default:
    break;
  }

  // when we want all items or failed to find single item, request all
  if (dispAll || !foundSet)
  {
    if (!formatAllRuleItems(construct, formatter, catRuleInfo))
      return false;
  }

  formatter.addR2KType(*catRuleInfo, construct);
  return true;
}

bool
RTGController::formatOneRuleItem(XMLConstruct& construct,
                                 GenerateRuleRequestFormatter& formatter,
                                 const CategoryRuleInfo* catRuleInfo,
                                 const CategoryRuleItemInfo* ruleUsed)
{
  bool hasItem = false;
  uint32_t ruleUsedItemNo = ruleUsed->itemNo();
  uint32_t ruleUsedItemCat = ruleUsed->itemcat();

  for (const auto& setPtr: catRuleInfo->categoryRuleItemInfoSet())
  {
    bool isQual = false;
    bool setHasItem = false;
    for (const auto& item: *setPtr)
    {
      // the IF and everything after it is a qualifier and should be in the request
      if (setHasItem && item.relationalInd() == CategoryRuleItemInfo::IF)
      {
        isQual = true;
        setHasItem = false;
      }

      if ((item.itemNo() == ruleUsedItemNo) && (item.itemcat() == ruleUsedItemCat))
      {
        hasItem = true;
        setHasItem = true;
        isQual = false;
        formatter.makeSSITags(construct, item, true); // true makes OR into THEN
      }
      else if (isQual)
        formatter.makeSSITags(construct, item, false);
    } // endif- all items in set

    // If not processing Category 25,
    // quit once found set with used item (don't want qualifiers from other sets)
    if (hasItem && ruleUsed->itemcat() != 25)
      return true;

  } // endif - all sets

  if (hasItem)
    return true;
  else
    return false;
}

bool
RTGController::formatDiscFareRuleItem(XMLConstruct& construct,
                                      GenerateRuleRequestFormatter& formatter,
                                      const PaxTypeFare& paxTypeFare,
                                      const CategoryRuleInfo* catRuleInfo)
{
  bool hasItem = false;
  // collect are passenger types for (merged) fare
  std::set<PaxTypeCode> farePaxTypes;
  farePaxTypes.insert(paxTypeFare.actualPaxType()->paxType());
  const FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();
  if (fareDisplayInfo && !fareDisplayInfo->passengerTypes().empty())
  {
    std::set<PaxTypeCode>::const_iterator iterI = fareDisplayInfo->passengerTypes().begin();
    std::set<PaxTypeCode>::const_iterator iterE = fareDisplayInfo->passengerTypes().end();
    for (; iterI != iterE; iterI++)
    {
      farePaxTypes.insert(*iterI);
    }
  }

  bool hasItems = false;
  for (const auto& setPtr: catRuleInfo->categoryRuleItemInfoSet())
  {
    bool isQual = false;
    for (const auto& itemInfo: *setPtr)
    {
      // the IF and everything after it is a qualifier and should be in the request
      if (hasItem && itemInfo.relationalInd() == CategoryRuleItemInfo::IF)
        isQual = true;

      const DiscountInfo* discountInfo = _trx.dataHandle().getDiscount(
          catRuleInfo->vendorCode(), itemInfo.itemNo(), itemInfo.itemcat());
      if (discountInfo && (farePaxTypes.find(discountInfo->paxType()) != farePaxTypes.end()))
      {
        formatter.makeSSITags(construct, itemInfo, !hasItems); // true makes OR into THEN
        hasItems = true;
      }
      else
      {
        // LOG4CXX_DEBUG(logger, "Skip discount fare item " <<(*itemI)->itemNo() <<" , Categor " <<
        // (*itemI)->itemcat()<<" , fare passenger type "<<
        // discountInfo?discountInfo->paxType():"(EMPTY)");
        if (isQual)
          formatter.makeSSITags(construct, itemInfo, false);
      }
    }
  }
  return hasItems;
}

template <class T>
bool
RTGController::formatAllRuleItems(XMLConstruct& construct,
                                  GenerateRuleRequestFormatter& formatter,
                                  const T* catRuleInfo)
{
  if (catRuleInfo->categoryRuleItemInfoSet().empty())
    return false;

  bool hasItems = false;

  // for all items in all sets
  for (const auto& setPtr: catRuleInfo->categoryRuleItemInfoSet())
  {
    if (!setPtr->empty())
      hasItems = true;
    for (const auto& item: *setPtr)
    {
      formatter.makeSSITags(construct, item);
    }
  }
  return hasItems;
}

PaxTypeCode
RTGController::basePaxtype(const PaxTypeFare* baseFare)
{
  if (baseFare->actualPaxType())
    return baseFare->actualPaxType()->paxType();
  if (baseFare->fareClassAppSegInfo())
    return baseFare->fareClassAppSegInfo()->_paxType;
  return "ADT";
}

/****************************************/
/***     F A R E   B Y   R U L E      ***/
/****************************************/
bool
RTGController::getFareByRuleText(bool isDom,
                                 bool isMinMaxFare,
                                 const Agent& agent,
                                 const Billing& billing,
                                 const FareByRuleCtrlInfo* catRuleInfo,
                                 PaxTypeFare& ptf,
                                 std::string& response)
{
  std::string request;
  XMLConstruct construct;
  GenerateRuleRequestFormatter formatter(isDom);
  std::vector<uint32_t> goodSeqs;

  construct.openElement("GenerateRuleRequest");
  formatter.addTVLType(_trx, construct);
  formatter.addAGIType(agent, billing, construct, _trx);
  formatter.addRFIType(ptf, construct);
  const PaxTypeFare* baseFare = nullptr;
  try { baseFare = ptf.fareWithoutBase(); }
  catch (tse::TSEException& e)
  {
    LOG4CXX_ERROR(logger, "ptf.fareWithoutBase() Exception... " << e.what() << e.where());
    return false;
  }

  std::string baseFareBasis = baseFare->createFareBasis(_trx);
  const FareByRuleItemInfo& fbrii = ptf.fareByRuleInfo();
  // TODO save 'real' curr used in FbrCtrl
  CurrencyCode curr =
      (fbrii.specifiedCur2() == ptf.currency()) ? fbrii.specifiedCur2() : fbrii.specifiedCur1();

  // when fare might be from hi/lo limit,
  // tell RTG if fare is 'S'pecified limit or 'C'alculated in range
  Indicator fareInd = fbrii.fareInd();
  if (fareInd == 'H' || fareInd == 'L')
    fareInd = isMinMaxFare ? 'S' : 'C';

  bool sentDISTag;

  sentDISTag = fareInd == 'C' || fareInd == 'B' || fareInd == 'G';


  if (sentDISTag)
  {
    formatter.addDISType(baseFareBasis, baseFare->owrt(), basePaxtype(baseFare), construct);
  }

  formatter.addC25Type(baseFareBasis, curr, fareInd, construct);

  construct.openElement("RCS");
  formatter.addRCSHeader(catRuleInfo, construct);

  if (!formatRuleInfo(construct, formatter, catRuleInfo, &ptf))
    return true; // Do not send request to RTG

  construct.closeElement(); // RCS
  construct.closeElement(); // GenerateRuleRequest

  return getResponse(construct.getXMLData(), response);
}

/****************************************/
/***       N E G O T I A T E D        ***/
/****************************************/
// This only adds the C35 tag if needed
// It should be called as a part of creating a complete RTG request
bool
RTGController::getNegText(GenerateRuleRequestFormatter formatter,
                          PaxTypeFare& ptf,
                          XMLConstruct& construct,
                          CatNumber cat)
{
  // skip if doesn't apply
  if (cat != 35 || !ptf.hasCat35Filed())
    return true;

  // dig out the data
  NegPaxTypeFareRuleData* negRuleData = ptf.getNegRuleData();
  if (!negRuleData)
  {
    LOG4CXX_INFO(logger, "bad cast for expected cat35 rule data");
    return false;
  }
  // still need to pass currency even if 979 blank
  if (negRuleData->ruleAmt().code().empty())
    negRuleData->ruleAmt() = ptf.currency();

  const NegFareSecurityInfo* secRec = negRuleData->securityRec();

  // controls which rule text message to use
  Indicator textType = ptf.fcaDisplayCatType();
  bool needCalcData = true;
  if (ptf.fareDisplayCat35Type() == RuleConst::SELLING_CARRIER_FARE ||
      ptf.fareDisplayCat35Type() == RuleConst::SELLING_MARKUP_FARE ||
      negRuleData->calcInd() == RuleConst::NF_NO_CALC_DATA)
    needCalcData = false;

  // format data
  if (secRec)
  {
    formatter.addC35Type(negRuleData->creatorPCC(),
                         negRuleData->calcInd(),
                         negRuleData->ruleAmt(),
                         negRuleData->noDecAmt(),
                         negRuleData->percent(),
                         negRuleData->noDecPercent(),
                         needCalcData,
                         textType,
                         secRec->sellInd(),
                         secRec->ticketInd(),
                         secRec->updateInd(),
                         secRec->redistributeInd(),
                         secRec->localeType(),
                         construct);
  }
  else
  {
    formatter.addC35Type(negRuleData->creatorPCC(),
                         negRuleData->calcInd(),
                         negRuleData->ruleAmt(),
                         negRuleData->noDecAmt(),
                         negRuleData->percent(),
                         negRuleData->noDecPercent(),
                         needCalcData,
                         textType,
                         BLANK,
                         BLANK,
                         BLANK,
                         BLANK,
                         BLANK,
                         construct);
  }
  return true;
}

/****************************************/
/***        F O O T N O T E           ***/
/****************************************/
bool
RTGController::getFootnoteText(bool isDom,
                               const Agent& agent,
                               const Billing& billing,
                               const FootNoteCtrlInfo* catRuleInfo,
                               PaxTypeFare& ptf,
                               std::string& response)
{
  std::string request;
  XMLConstruct construct;
  GenerateRuleRequestFormatter formatter(isDom);

  construct.openElement("GenerateRuleRequest");
  formatter.addTVLType(_trx, construct);
  formatter.addAGIType(agent, billing, construct, _trx);
  formatter.addRFIType(ptf, construct);
  getNegText(formatter, ptf, construct, catRuleInfo->categoryNumber());

  construct.openElement("RCS");
  formatter.addRCSHeader(catRuleInfo, construct);

  if (!formatRuleInfo(construct, formatter, catRuleInfo, &ptf))
    return true; // Do not send request to RTG

  construct.closeElement(); // RCS
  construct.closeElement(); // GenerateRuleRequest

  return getResponse(construct.getXMLData(), response);
}

/****************************************/
/***             F A R E              ***/
/****************************************/
bool
RTGController::getFareText(bool isDom,
                           const Agent& agent,
                           const Billing& billing,
                           const FareRuleRecord2Info* catRuleInfo,
                           PaxTypeFare& ptf,
                           std::string& response)
{
  std::string request;
  XMLConstruct construct;
  GenerateRuleRequestFormatter formatter(isDom);

  construct.openElement("GenerateRuleRequest");
  formatter.addTVLType(_trx, construct);
  formatter.addAGIType(agent, billing, construct, _trx);
  formatter.addRFIType(ptf, construct);
  getNegText(formatter, ptf, construct, catRuleInfo->categoryNumber());

  construct.openElement("RCS");
  formatter.addRCSHeader(catRuleInfo, construct);

  if (!formatRuleInfo(construct, formatter, catRuleInfo, &ptf))
    return true; // Do not send request to RTG

  construct.closeElement(); // RCS
  construct.closeElement(); // GenerateRuleRequest

  return getResponse(construct.getXMLData(), response);
}

/****************************************/
/***     C O M B I N A L I T Y        ***/
/****************************************/
bool
RTGController::getCombinalityText(bool isDom,
                                  const Agent& agent,
                                  const Billing& billing,
                                  const CombinabilityRuleInfo* catRuleInfo,
                                  std::string& response,
                                  uint32_t subCat,
                                  PaxTypeFare& ptf)
{
  std::string request;
  XMLConstruct construct;
  GenerateRuleRequestFormatter formatter(isDom);

  construct.openElement("GenerateRuleRequest");
  formatter.addTVLType(_trx, construct);
  formatter.addAGIType(agent, billing, construct, _trx);
  formatter.addRFIType(ptf, construct);
  construct.openElement("RCS");
  formatter.addRCSHeader(catRuleInfo, construct);

  // TODO review Cat10 subcategory logic (this might skip itemFound logic)
  if (subCat != 0)
  {
    formatter.addSSIType(*catRuleInfo, construct, subCat);
  }
  else
  {
    // no SSI block in cat10 request is ok to send; don't fail if none
    formatAllRuleItems(construct, formatter, catRuleInfo);
  } // endif - zero sunCat

  formatter.addR2KType(*catRuleInfo, construct);
  formatter.addCSBType(*catRuleInfo, construct);

  construct.closeElement(); // RCS
  construct.closeElement(); // GenerateRuleRequest

  return getResponse(construct.getXMLData(), response);
}

/****************************************/
/***          G E N E R A L           ***/
/****************************************/
bool
RTGController::getGeneralText(bool isDom,
                              const Agent& agent,
                              const Billing& billing,
                              const GeneralRuleRecord2Info* catRuleInfo,
                              PaxTypeFare& ptf,
                              std::string& response)
{
  std::string request;
  XMLConstruct construct;
  GenerateRuleRequestFormatter formatter(isDom);

  construct.openElement("GenerateRuleRequest");
  formatter.addTVLType(_trx, construct);
  formatter.addAGIType(agent, billing, construct, _trx);
  formatter.addRFIType(ptf, construct);
  getNegText(formatter, ptf, construct, catRuleInfo->categoryNumber());

  construct.openElement("RCS");
  formatter.addRCSHeader(catRuleInfo, construct);

  if (!formatRuleInfo(construct, formatter, catRuleInfo, &ptf))
    return true; // Do not send request to RTG

  construct.closeElement(); // RCS
  construct.closeElement(); // GenerateRuleRequest

  return getResponse(construct.getXMLData(), response);
}

void
RTGController::initializeBundledRequest(FareDisplayTrx& trx, PaxTypeFare& ptf)
{
  _construct.openElement("GenerateBundledRuleRequest");

  GenerateRuleRequestFormatter formatter(true);
  formatter.addTVLType(trx, _construct);
  formatter.addAGIType(*trx.getRequest()->ticketingAgent(), *trx.billing(), _construct, trx);
  formatter.addRFIType(ptf, _construct);
}

void
RTGController::buildBundledRequest(FBCategoryRuleRecord* ruleRecord,
                                   PaxTypeFare& ptf,
                                   CatNumber cat,
                                   bool isMinMaxFare,
                                   bool& hasRule,
                                   FBDisplay& fbDisplay,
                                   CatNumber subCat)

{
  LOG4CXX_INFO(logger, "Building RTG request for category=" << cat);

  GenerateRuleRequestFormatter formatter(ruleRecord->isRecordScopeDomestic());

  // cat25 can skip precendence stuff (only one rule)
  if (ptf.isFareByRule() && ruleRecord->hasFareByRuleRecord2Info())
  {
    LOG4CXX_INFO(logger, "\tBuild request for FareByRule rule text");

    buildFareByRuleReq(isMinMaxFare, ruleRecord->fareByRuleRecord2Info(), ptf, formatter);
    hasRule = true;

    return;
  }

  // footnote first (precedence reqs)
  if (ruleRecord->hasFootNoteRecord2Info())
  {
    LOG4CXX_INFO(logger, "\tBuild request for footnote rule text");

    buildFootnoteReq(ruleRecord->footNoteRecord2Info(), ptf, formatter);
    hasRule = true;
  }

  // then fare rule
  if (ruleRecord->hasFareRuleRecord2Info())
  {
    LOG4CXX_INFO(logger, "\tBuild request for fare rule text");

    buildFareRuleReq(ruleRecord->fareRuleRecord2Info(), ptf, formatter);
    hasRule = true;
  }

  // then combinality
  if (ruleRecord->hasCombinabilityRecord2Info())
  {
    LOG4CXX_INFO(logger, "\tBuild request for combinality rule text");

    buildCombinalityReq(ruleRecord->combinabilityRecord2Info(), subCat, formatter);
    hasRule = true;
  }

  // general rule
  if (ruleRecord->hasGeneralRuleRecord2Info())
  {
    LOG4CXX_INFO(logger, "\tBuild request for general rule text");
    bool processGeneralRule(true);

    if (ruleRecord->hasFareRuleRecord2Info())
    {
      const GeneralRuleRecord2Info* genRuleInfo = ruleRecord->generalRuleRecord2Info();

      const FareRuleRecord2Info* fareRuleInfo = ruleRecord->fareRuleRecord2Info();

      if (genRuleInfo->vendorCode() == fareRuleInfo->vendorCode() &&
          genRuleInfo->carrierCode() == fareRuleInfo->carrierCode() &&
          genRuleInfo->tariffNumber() == fareRuleInfo->tariffNumber() &&
          genRuleInfo->ruleNumber() == fareRuleInfo->ruleNumber() &&
          genRuleInfo->sequenceNumber() == fareRuleInfo->sequenceNumber() &&
          genRuleInfo->categoryNumber() == fareRuleInfo->categoryNumber())
      {
        LOG4CXX_INFO(logger, "\tGeneral rule is duplicate of fare rule. Skipping...");
        processGeneralRule = false;
      }
    }

    if (processGeneralRule)
    {
      buildGeneralRuleReq(ruleRecord->generalRuleRecord2Info(), ptf, formatter);
      hasRule = true;
    }
  }
  // Check if there is a base fare info and build request if it is present.

  FBCategoryRuleRecord* baseFareRuleRecord = fbDisplay.getBaseFareRuleRecordData(cat);
  if (!baseFareRuleRecord)
    return;
  else if (!(baseFareRuleRecord->isAllEmpty()))
  {
    bool baseFare = true;
    if (baseFareRuleRecord->hasFootNoteRecord2Info())
    {
      LOG4CXX_INFO(logger, "\tBuild request for base fare footnote rule text");

      buildFootnoteReq(baseFareRuleRecord->footNoteRecord2Info(), ptf, formatter, baseFare);
      hasRule = true;
    }

    // then fare rule
    if (baseFareRuleRecord->hasFareRuleRecord2Info())
    {
      LOG4CXX_INFO(logger, "\tBuild request for base fare fare rule text");

      buildFareRuleReq(baseFareRuleRecord->fareRuleRecord2Info(), ptf, formatter, baseFare);
      hasRule = true;
    }
    if (baseFareRuleRecord->hasGeneralRuleRecord2Info())
    {
      LOG4CXX_INFO(logger, "\tBuild request for base fare general rule text");
      buildGeneralRuleReq(baseFareRuleRecord->generalRuleRecord2Info(), ptf, formatter, baseFare);
      hasRule = true;
    }
  }
}

/**************************************************************/
/***     F A R E   B Y   R U L E  - for Bundled Request     ***/
/**************************************************************/
void
RTGController::buildFareByRuleReq(bool isMinMaxFare,
                                  const FareByRuleCtrlInfo* catRuleInfo,
                                  PaxTypeFare& ptf,
                                  GenerateRuleRequestFormatter& formatter)
{
  // Make sure this rule has been applied
  if (!isRuleNeeded(catRuleInfo, &ptf))
  {
    return; // Do not send request to RTG for this rule
  }

  const PaxTypeFare* baseFare = nullptr;

  try { baseFare = ptf.fareWithoutBase(); }
  catch (tse::TSEException& e)
  {
    LOG4CXX_ERROR(logger, "ptf.fareWithoutBase() Exception... " << e.what() << e.where());
    return;
  }

  std::string baseFareBasis = baseFare->createFareBasis(_trx);
  const FareByRuleItemInfo& fbrii = ptf.fareByRuleInfo();

  // TODO save 'real' curr used in FbrCtrl
  CurrencyCode curr =
      (fbrii.specifiedCur2() == ptf.currency()) ? fbrii.specifiedCur2() : fbrii.specifiedCur1();

  // when fare might be from hi/lo limit,
  // tell RTG if fare is 'S'pecified limit or 'C'alculated in range
  Indicator fareInd = fbrii.fareInd();
  if (fareInd == 'H' || fareInd == 'L')
    fareInd = isMinMaxFare ? 'S' : 'C';

  _construct.openElement("RCS");
  formatter.addRCSHeader(catRuleInfo, _construct);
  formatter.addQ4RAttribute(FareByRule, _construct);

  bool sentDISTag;

  sentDISTag = fareInd == 'C' || fareInd == 'B' || fareInd == 'G';

  if (sentDISTag)
  {
    formatter.addDISType(baseFareBasis, baseFare->owrt(), basePaxtype(baseFare), _construct);
  }

  formatter.addC25Type(baseFareBasis, curr, fareInd, _construct);

  if (!formatRuleInfo(_construct, formatter, catRuleInfo, &ptf))
  {
    LOG4CXX_ERROR(logger,
                  "Should not Format Rule Info for category " << catRuleInfo->categoryNumber());
    return; // We should not get to this point
  }

  _construct.closeElement(); // RCS

  // Add an entry for this category number/rule type to the rule text map
  RuleTextMapKey mapKey = std::make_pair(catRuleInfo->categoryNumber(), FareByRule);
  _ruleTextMap.insert(std::make_pair(mapKey, "RTG REQUEST"));
}

/*********************************************************/
/***        F O O T N O T E  - for Bundled Request     ***/
/*********************************************************/
void
RTGController::buildFootnoteReq(const FootNoteCtrlInfo* catRuleInfo,
                                PaxTypeFare& ptf,
                                GenerateRuleRequestFormatter& formatter,
                                bool baseFare)
{
  // Make sure this rule has been applied
  if (!isRuleNeeded(catRuleInfo, &ptf))
  {
    return; // Do not send request to RTG for this rule
  }

  _construct.openElement("RCS");
  formatter.addRCSHeader(catRuleInfo, _construct);
  if (baseFare)
    formatter.addQ4RAttribute(BFFootNote, _construct);
  else
    formatter.addQ4RAttribute(FootNote, _construct);

  getNegText(formatter, ptf, _construct, catRuleInfo->categoryNumber());

  if (!formatRuleInfo(_construct, formatter, catRuleInfo, &ptf))
  {
    LOG4CXX_ERROR(logger,
                  "Should not Format Rule Info for category " << catRuleInfo->categoryNumber());
    return; // We should not get to this point
  }

  _construct.closeElement(); // RCS

  // Add an entry for this category number/rule type to the rule text map
  RuleTextMapKey mapKey;
  if (baseFare)
    mapKey = std::make_pair(catRuleInfo->categoryNumber(), BFFootNote);
  else
    mapKey = std::make_pair(catRuleInfo->categoryNumber(), FootNote);
  _ruleTextMap.insert(std::make_pair(mapKey, "RTG REQUEST"));
}

/******************************************************/
/***             F A R E  - for Bundled Request     ***/
/******************************************************/
void
RTGController::buildFareRuleReq(const FareRuleRecord2Info* catRuleInfo,
                                PaxTypeFare& ptf,
                                GenerateRuleRequestFormatter& formatter,
                                bool baseFare)
{
  // Make sure this rule has been applied
  if (!isRuleNeeded(catRuleInfo, &ptf))
  {
    return; // Do not send request to RTG for this rule
  }

  _construct.openElement("RCS");
  formatter.addRCSHeader(catRuleInfo, _construct);
  if (baseFare)
    formatter.addQ4RAttribute(BFFareRule, _construct);
  else
    formatter.addQ4RAttribute(FareRule, _construct);

  getNegText(formatter, ptf, _construct, catRuleInfo->categoryNumber());

  if (!formatRuleInfo(_construct, formatter, catRuleInfo, &ptf))
  {
    LOG4CXX_ERROR(logger,
                  "Should not Format Rule Info for category " << catRuleInfo->categoryNumber());
    return; // We should not get to this point
  }

  _construct.closeElement(); // RCS

  // Add an entry for this category number/rule type to the rule text map
  RuleTextMapKey mapKey;
  if (baseFare)
    mapKey = std::make_pair(catRuleInfo->categoryNumber(), BFFareRule);
  else
    mapKey = std::make_pair(catRuleInfo->categoryNumber(), FareRule);
  _ruleTextMap.insert(std::make_pair(mapKey, "RTG REQUEST"));
}

/****************************************************************/
/***     C O M B I N A B I L I T Y  - for Bundled Request     ***/
/****************************************************************/
void
RTGController::buildCombinalityReq(const CombinabilityRuleInfo* catRuleInfo,
                                   uint32_t subCat,
                                   GenerateRuleRequestFormatter& formatter)
{
  _construct.openElement("RCS");
  formatter.addRCSHeader(catRuleInfo, _construct);
  formatter.addQ4RAttribute(Combinability, _construct);

  // TODO review Cat10 subcategory logic (this might skip itemFound logic)
  if (subCat != 0)
  {
    formatter.addSSIType(*catRuleInfo, _construct, subCat);
  }
  else
  {
    // no SSI block in cat10 request is ok to send; don't fail if none
    formatAllRuleItems(_construct, formatter, catRuleInfo);
  } // endif - zero sunCat

  formatter.addR2KType(*catRuleInfo, _construct);

  formatter.addCSBType(*catRuleInfo, _construct);
  _construct.closeElement(); // RCS

  // Add an entry for this category number/rule type to the rule text map
  RuleTextMapKey mapKey = std::make_pair(catRuleInfo->categoryNumber(), Combinability);
  _ruleTextMap.insert(std::make_pair(mapKey, "RTG REQUEST"));
}

/*********************************************************/
/***          G E N E R A L  - for Bundled Request     ***/
/*********************************************************/
void
RTGController::buildGeneralRuleReq(const GeneralRuleRecord2Info* catRuleInfo,
                                   PaxTypeFare& ptf,
                                   GenerateRuleRequestFormatter& formatter,
                                   bool baseFare)
{
  // Make sure this rule has been applied
  if (!isRuleNeeded(catRuleInfo, &ptf))
  {
    return; // Do not send request to RTG for this rule
  }

  _construct.openElement("RCS");
  formatter.addRCSHeader(catRuleInfo, _construct);
  if (baseFare)
    formatter.addQ4RAttribute(BFGeneralRule, _construct);
  else
    formatter.addQ4RAttribute(GeneralRule, _construct);

  getNegText(formatter, ptf, _construct, catRuleInfo->categoryNumber());

  if (!formatRuleInfo(_construct, formatter, catRuleInfo, &ptf))
  {
    LOG4CXX_ERROR(logger,
                  "Should not Format Rule Info for category " << catRuleInfo->categoryNumber());
    return; // We should not get to this point
  }

  _construct.closeElement(); // RCS

  // Add an entry for this category number/rule type to the rule text map
  RuleTextMapKey mapKey;
  if (baseFare)
    mapKey = std::make_pair(catRuleInfo->categoryNumber(), BFGeneralRule);
  else
    mapKey = std::make_pair(catRuleInfo->categoryNumber(), GeneralRule);
  _ruleTextMap.insert(std::make_pair(mapKey, "RTG REQUEST"));
}

bool
RTGController::isRuleNeeded(const CategoryRuleInfo* catRuleInfo, PaxTypeFare* ptf)
{
  uint16_t cat = catRuleInfo->categoryNumber();
  bool dispAll = true;

  switch (cat)
  {
  case 19:
  case 20:
  case 21:
  case 22:
  {
    dispAll = !ptf->isDiscounted();
    cat = 19;
    break;
  }
  case 25:
  case 35:
  {
    dispAll = false;
    break;
  }
  default:
    break;
  }

  if (!dispAll)
  {
    // only do one rule
    PaxTypeFareRuleData* ptfRuleData = ptf->paxTypeFareRuleData(cat);

    if (!ptfRuleData)
    {
      LOG4CXX_ERROR(logger, "Missing ptfRuleData for category " << cat);
      return false;
    }

    if (ptfRuleData->categoryRuleInfo()->categoryNumber() != catRuleInfo->categoryNumber())
    {
      LOG4CXX_DEBUG(logger,
                    "Rule for category " << catRuleInfo->categoryNumber() << " not applied");
      return false;
    }
  }

  return true;
}

void
RTGController::finalizeBundledRequest()
{
  _construct.closeElement(); // GenerateBundledRuleRequest
}

void
RTGController::sendBundledRequest(std::string& response)
{
  getResponse(_construct.getXMLData(), response);
}

//----------------------------------------------------------------
// Update Fare Display Info Rule Text map - for Bundled Request
//----------------------------------------------------------------
void
RTGController::updateFareDisplayRuleTextMap(FBDisplay& fbDisplay, bool haveSubCats, bool isFBRfare)
{
  // Get the rule text for each category accumulated on the Rule Text map
  RuleTextMap::const_iterator ruleTextIter = _ruleTextMap.begin();
  RuleTextMap::const_iterator ruleTextIterEnd = _ruleTextMap.end();
  RuleTextMap::const_iterator ruleTextIterNxt;
  RuleTextMap::const_iterator ruleTextIterNxt1;

  bool needHeader = false; // for certain categories a rule header is required in RD display
  bool needBlankLine = false; // need a blank line for second rule if a header already returned
  bool hasFootnote = false;
  bool hasFareRule = false;
  bool hasCombinability = false;
  bool firstTimeBFHeader = true;
  CatNumber catNum = -1;

  bool bothFBRandBaseFare = false;
  bool fbrHeader = true;

  // Go through the Rule Text map processing each entry
  for (; ruleTextIter != ruleTextIterEnd; ruleTextIter++)
  {
    if (ruleTextIter->first.first != catNum)
    {
      needHeader = false;
      needBlankLine = false;
      hasFootnote = false;
      hasFareRule = false;
      hasCombinability = false;
      firstTimeBFHeader = true;
      catNum = ruleTextIter->first.first;
    }

    // Check whether RTG request was successful or not
    //  Assume request was unsuccessful when default text hasn't been replaced
    if (ruleTextIter->second == "RTG REQUEST")
    {
      if (_doesParseThrowException)
        addRuleTextError(fbDisplay, catNum);
      else
        addRuleTextPeakUseError(fbDisplay, catNum);
    }
    else
    {
      if (hasFootnote || hasCombinability || (hasFareRule && !bothFBRandBaseFare && catNum != 27))
      {
        continue; // Discard this rule type for this category
      }

      // Check Precedence
      //  When footnote and fare rule are applicable only
      //    only the information from footnote will aplly for
      //    certain categories
      //  When fare rule and general rule are applicable only
      //    only the information from fare rule will aplly for
      //    certain categories
      if (ruleTextIter->first.second == FootNote && !ruleTextIter->second.empty() && catNum != 11 &&
          catNum != 12 && catNum != 15)
      {
        hasFootnote = true;
      }
      else if (ruleTextIter->first.second == FareRule && !ruleTextIter->second.empty() &&
               catNum != 5 && catNum != 8 && catNum != 9 && catNum != 11 && catNum != 12 &&
               catNum != 15 && catNum != 16 && catNum != 19 && catNum != 20 && catNum != 21 &&
               catNum != 22)
      {
        hasFareRule = true;
      }
      else if (ruleTextIter->first.second == Combinability && !ruleTextIter->second.empty())
      {
        hasCombinability = true;
      }

      ruleTextIterNxt1 = ruleTextIter;
      ruleTextIterNxt1++;

      // When both fare rules of FBR fare and base fare exits and
      // ovrd indicator is "B" we need to display both fare rules
      if (ruleTextIter->first.second == FareRule && !ruleTextIter->second.empty() &&
          ruleTextIterNxt1->first.second == BFFareRule && catNum == 4)
      {
        bothFBRandBaseFare = true;
        needHeader = true; // since both rule exits might need an header
      }

      if (catNum == 27)
      {
        if (hasGeneralAndFareRule(catNum) && ruleTextIter->first.second == GeneralRule)
        {
          addBlankLine(fbDisplay, catNum);
          addLine(fbDisplay, catNum, GENERAL_RULE_HEADER);
        }

        if (ruleTextIter->first.second == GeneralRule)
        {
          addLine(fbDisplay, catNum, NOT_VALIDATED_1);
          addLine(fbDisplay, catNum, NOT_VALIDATED_2);
        }

        if (hasGeneralAndFareRule(catNum) && ruleTextIter->first.second == FareRule)
        {
          fbDisplay.ruleTextMap()[catNum] =
              FARE_RULE_HEADER + "\n" + fbDisplay.ruleTextMap()[catNum];
        }
      }

      // Only these categories need rule headings
      if (catNum == 5 || catNum == 8 || catNum == 9 || catNum == 11 || catNum == 12 ||
          catNum == 15 || catNum == 16 || catNum == 19 || catNum == 20 || catNum == 21 ||
          catNum == 22)
      {
        ruleTextIterNxt = ruleTextIter;
        ruleTextIterNxt++; // Point to next item

        if (!needHeader && ruleTextIterNxt->first.first == catNum)
        {
          needHeader = true;
        }
      }

      // Display FBR line only when both rules (from base fare and FBR) applied
      if (isFBRfare && fbrHeader && bothFBRandBaseFare)
      {
        addLine(fbDisplay, catNum, FARE_BY_RULE);
        fbrHeader = false;
      }

      if (needHeader)
      {
        int ruleType = ruleTextIter->first.second;

        if ((ruleType == BFFootNote || ruleType == BFFareRule || ruleType == BFGeneralRule) &&
            firstTimeBFHeader)
        {
          addBlankLine(fbDisplay, catNum);
          addLine(fbDisplay, catNum, ADDITIONAL);
          addBlankLine(fbDisplay, catNum);
          addLine(fbDisplay, catNum, BASE_FARE);
          firstTimeBFHeader = false;
        }

        if (needBlankLine)
          addBlankLine(fbDisplay, catNum);

        if (ruleType == FootNote || ruleType == BFFootNote)
        {
          addLine(fbDisplay, catNum, FOOTNOTE_RULE);
          needBlankLine = true;
        }
        else if (ruleType == FareRule || ruleType == BFFareRule)
        {
          addLine(fbDisplay, catNum, FARE_RULE);
          needBlankLine = true;
        }
        else if (ruleType == GeneralRule || ruleType == BFGeneralRule)
        {
          addLine(fbDisplay, catNum, GENERAL_RULE);
          needBlankLine = true;
        }
      }

      fbDisplay.ruleTextMap()[catNum] += ruleTextIter->second;

      if (catNum == 10 && !haveSubCats)
      {
        addBlankLine(fbDisplay, catNum);
        addLine(fbDisplay, catNum, CAT_10);
      } // end catNum 10 - if

    } // else ends
  } // for loop ends
}

void
RTGController::addLine(FBDisplay& fbDisplay, CatNumber catNum, const std::string& value)
{
  fbDisplay.ruleTextMap()[catNum] += "\n ";
  fbDisplay.ruleTextMap()[catNum] += value;
}

void
RTGController::addBlankLine(FBDisplay& fbDisplay, CatNumber catNum)
{
  fbDisplay.ruleTextMap()[catNum] += "\n ";
}

void
RTGController::addRuleTextError(FBDisplay& fbDisplay, CatNumber catNum)
{
  fbDisplay.ruleTextMap()[catNum] = "RULE TEXT DATA ERROR";
}

void
RTGController::addRuleTextPeakUseError(FBDisplay& fbDisplay, CatNumber catNum)
{
  fbDisplay.ruleTextMap()[catNum] = "RULE TEXT GENERATOR AT PEAK USE - RETRY 8 SECS";
}

//----------------------------------------------------------------
// Send request to RTG server and get response
//----------------------------------------------------------------
bool
RTGController::getResponse(const std::string& request, std::string& response)
{
  LOG4CXX_DEBUG(logger, "Request=" << request);
  tse::StopWatch sw;
  sw.start();

  if (FareDisplayUtil::isXMLDiagRequested(_trx, "RTGREQ"))
    FareDisplayUtil::displayXMLDiag(_trx, request, "RTG XML REQUEST");

  ClientSocket clientSocket(cscv);

  if (!clientSocket.initialize())
  {
    sw.stop();
    TseSrvStats::recordRTGCall(false, 0, 0, sw.elapsedTime(), _trx);

    LOG4CXX_FATAL(logger, "Failed To Connect RTG Server");
    throw ErrorResponseException(ErrorResponseException::WORLDFARE_AT_PEAK_USE);
  }

  if (!clientSocket.send("RQST", "0001", "0000", request))
  {
    LOG4CXX_ERROR(logger, "Failed To Send RD Request To RTG Server");
    sw.stop();
    TseSrvStats::recordRTGCall(false, request.size(), 0, sw.elapsedTime(), _trx);

    return false;
  }

  std::string command;
  std::string version;
  std::string revision;
  std::string rawResponse;
  if (!clientSocket.receive(command, version, revision, rawResponse))
  {
    sw.stop();
    TseSrvStats::recordRTGCall(false, request.size(), 0, sw.elapsedTime(), _trx);

    LOG4CXX_ERROR(logger, "No Response was recieved from RTG - FQ-RTG Time Out Error " << request);
    return false;
  }

  LOG4CXX_DEBUG(logger, "Response=" << rawResponse);

  if (FareDisplayUtil::isXMLDiagRequested(_trx, "RTGRES"))
    FareDisplayUtil::displayXMLDiag(_trx, rawResponse, "RTG XML RESPONSE");

  // Parse the response
  RuleResponseContentHandler docHandler(response, _ruleTextMap, _trx.fdResponse()->rtgDiagInfo());
  docHandler.initialize();
  if (!docHandler.parse(rawResponse.c_str()))
  {
    LOG4CXX_DEBUG(logger, "PARSE ERROR from RuleResponseContentHandler");
    _doesParseThrowException = true;

    sw.stop();
    TseSrvStats::recordRTGCall(false, request.size(), rawResponse.size(), sw.elapsedTime(), _trx);

    return false;
  }

  sw.stop();
  TseSrvStats::recordRTGCall(true, request.size(), rawResponse.size(), sw.elapsedTime(), _trx);

  return true;
}

void
RTGController::buildPrimeBookingCodes(std::ostringstream& response,
                                      const CarrierCode& carrier,
                                      const std::vector<BookingCode>& bookingCodes)
{
  if (bookingCodes.empty())
  {
    return;
  }
  else
  {
    response << carrier << "  PRIME BOOKING CODE                  ";

    size_t i = bookingCodes.size();
    std::ostringstream primeBookingCodes;
    for (size_t j = 0; j < i; ++j)
    {
      if (bookingCodes[j].empty() == false)
      {
        if (j > 0 && j != i)
        {
          primeBookingCodes << "/";
        }
        primeBookingCodes << bookingCodes[j];
      }
    }
    primeBookingCodes << std::endl;

    int padLength = 24 - static_cast<int>(primeBookingCodes.str().length());

    while (padLength > 0)
    {
      response << " ";
      padLength--;
    }
    response << primeBookingCodes.str();
  }
}

void
RTGController::buildRBHeader(std::ostringstream& response,
                             PaxTypeFare& paxTypeFare,
                             AirSeg* airSeg,
                             RBData& rbData)
{
  response << "CXR  FARE CLASS   MARKET              TICKET DATE   TRAVEL DATE" << std::endl;
  response << paxTypeFare.carrier() << "   " << paxTypeFare.fare()->fareInfo()->fareClass();
  size_t fareClassLength = paxTypeFare.fare()->fareInfo()->fareClass().length();
  while (fareClassLength < 13)
  {
    response << " ";
    fareClassLength++;
  }

  response << airSeg->origAirport() << airSeg->destAirport() << "                "
           << _trx.getRequest()->ticketingDT().dateToString(DDMMMYY, "") << "       "
           << airSeg->departureDT().dateToString(DDMMMYY, "") << std::endl;

  // FOR FBR we need to check if the BookingCode 2nd char is asterisk, if
  // so, this is Sistar Fare , and proper message need to be displayed
  if (paxTypeFare.isFareByRule())
  {
    FBRPaxTypeFareRuleData* fbrPTFRuleData = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);

    if (fbrPTFRuleData)
    {
      std::set<BookingCode>& baseFareInfBC = fbrPTFRuleData->baseFareInfoBookingCodes();
      if (baseFareInfBC.size())
      {
        std::set<BookingCode>::iterator it = baseFareInfBC.begin();
        std::set<BookingCode>::iterator ite = baseFareInfBC.end();
        bool firstBC = true;
        response << " " << std::endl << "****IN ADDITION TO RBD REQUIREMENT BELOW ";

        for (; it != ite; it++)
        {
          if (!firstBC)
            response << "/";
          response << *it;
          firstBC = false;
        }
        response << " INVENTORY" << std::endl << "MUST BE AVAILABLE ON ALL FLIGHTS/CARRIERS****"
                 << std::endl;
      }
    }
  }

  response << "   " << std::endl;

  // No Prime RBD text for industry YY fare.
  if (((paxTypeFare.fare()->isIndustry() || paxTypeFare.carrier() == INDUSTRY_CARRIER) &&
       paxTypeFare.fareMarket()->governingCarrier() == INDUSTRY_CARRIER) ||
      ((paxTypeFare.fare()->isIndustry() ||
        paxTypeFare.carrier() == INDUSTRY_CARRIER) && // eg: RB9CF on YY fare
       paxTypeFare.fareMarket()->governingCarrier() != INDUSTRY_CARRIER &&
       !rbData.isCarrierMatchedTable990()))
  {
    return;
  }

  buildPrimeBookingCodes(
      response, paxTypeFare.fareMarket()->governingCarrier(), rbData.getBookingCodes());
}

/**
 * Entry point for RB entry. Instantiates the RBData object, which gets data, creates RBDataItem for
 * each valid sequence-segment combination. Finally a RB request formatter will generate the RB XML
 * request and collect the response from RTG using the generic ContenHandler.*/

bool
RTGController::failRBRequest()
{
  _trx.response() << " RB PROCESSING ERROR DETECTED " << std::endl;
  return false;
}

bool
RTGController::processRBSecondaryMarket(PaxTypeFare* fare,
                                        std::string& xmlResponse,
                                        bool skipDiag854)
{
  LOG4CXX_DEBUG(logger,
                "\nRTGController::processRBSecondaryMarket() for cxr="
                    << _trx.requestedCarrier() << " orig=" << fare->fareMarket()->boardMultiCity()
                    << " dest=" << fare->fareMarket()->offMultiCity());
  if(fare->fareMarket()->getGlobalDirection() == GlobalDirection::ZZ)
  {
    _gd = fare->fareMarket()->getGlobalDirection();
  }

  size_t secondaryCarrierSize = _trx.getRequest()->secondaryCarrier().size();
  RBData rbData;
  rbData.setSecondary(true); // Indicate process secondary market
  AirSeg airSeg;
  bool _skipDiag854 = skipDiag854;

  for (size_t i = 0; i < secondaryCarrierSize; ++i)
  {
    LOG4CXX_DEBUG(logger,
                  "\n--- Secondary market for cxr="
                      << _trx.getRequest()->secondaryCarrier()[i]
                      << " orig=" << _trx.getRequest()->secondaryCity1()[i]
                      << " dest=" << _trx.getRequest()->secondaryCity2()[i]);
    LOG4CXX_DEBUG(logger,
                  "\n--- fare Cities=" << fare->fareMarket()->governingCarrier()
                                       << " orig=" << fare->fareMarket()->boardMultiCity()
                                       << " dest=" << fare->fareMarket()->offMultiCity());
    rbData.setCarrierMatchedTable990(false);

    if ((_trx.getRequest()->secondaryCarrier()[i] == fare->fareMarket()->governingCarrier()) &&
        (_trx.getRequest()->secondaryCity1()[i] == fare->fareMarket()->boardMultiCity() ||
         _trx.getRequest()->secondaryCity1()[i] == fare->fareMarket()->origin()->loc()) &&
        (_trx.getRequest()->secondaryCity2()[i] == fare->fareMarket()->offMultiCity() ||
         _trx.getRequest()->secondaryCity2()[i] == fare->fareMarket()->destination()->loc()))
    {
      continue;
    }
    // should not repeat the primary market/carrier
    else if ((_trx.getRequest()->secondaryCity1()[i] == fare->fareMarket()->boardMultiCity() ||
              _trx.getRequest()->secondaryCity1()[i] == fare->fareMarket()->origin()->loc()) &&
             (_trx.getRequest()->secondaryCity2()[i] == fare->fareMarket()->offMultiCity() ||
              _trx.getRequest()->secondaryCity2()[i] == fare->fareMarket()->destination()->loc()))

    {
      rbData.setSecondaryCityPairSameAsPrime(true);
    }
    else
    {
      rbData.setSecondaryCityPairSameAsPrime(false);
    }

    if (i > 0 || (fare->carrier() != INDUSTRY_CARRIER ||
                  fare->fareMarket()->governingCarrier() != INDUSTRY_CARRIER))
    {
      _trx.response() << SEPARATOR_LINE << std::endl;
    }

    rbData.setAirSeg(&airSeg);
    airSeg.origAirport() = _trx.getRequest()->secondaryCity1()[i];
    airSeg.destAirport() = _trx.getRequest()->secondaryCity2()[i];
    airSeg.setMarketingCarrierCode(_trx.getRequest()->secondaryCarrier()[i]);

    airSeg.departureDT() = _trx.travelSeg()[0]->departureDT();

    // set origin and destination in the AirSeg
    const Loc* origin = _trx.dataHandle().getLoc(airSeg.origAirport(), _trx.travelDate());
    const Loc* destination = _trx.dataHandle().getLoc(airSeg.destAirport(), _trx.travelDate());
    airSeg.origin() = origin;
    airSeg.destination() = destination;

    GlobalDirection gd;
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.emplace_back(&airSeg);
    // find the global direction
    const PricingTrx* pTrx = static_cast<FareDisplayTrx*>(&_trx);
    GlobalDirectionFinderV2Adapter::getGlobalDirection(pTrx, airSeg.departureDT(), tvlSegs, gd);
    if(gd != GlobalDirection::ZZ && _gd == GlobalDirection::ZZ)
      fare->fareMarket()->setGlobalDirection(gd);

    std::string xmlRequest;
    BookingCode bookingCode;
    rbData.rbItems().clear();
    FareDisplayBookingCodeRB rbBookingCode(&rbData);
    rbBookingCode.getBookingCode(_trx, *fare, bookingCode);
    if (!rbData.rbItems().empty() > 0)
    {
      RBRequestFormatter formatter(xmlRequest, rbData);
      formatter.buildRequest(_trx, _skipDiag854);
    }

    std::string rtgText; // initialize to clear previous exceptions saved
    if (xmlRequest.empty())
    {
      _trx.response() << airSeg.marketingCarrierCode() << "  " << airSeg.origAirport() << " "
                      << airSeg.destAirport() << std::endl;
      // to do process prime booking codes SECTION 6 EROR 12
      if (_trx.requestedCarrier() == _trx.getRequest()->secondaryCarrier().at(i).c_str() &&
          fare->carrier() != INDUSTRY_CARRIER)
      {
        _trx.response() << "      NO EXCEPTION DATA EXISTS USE PRIME BOOKING CODE " << std::endl;
      }
      else
      {
        _trx.response() << "      NO RBD FOUND FOR SPECIFIED CXR AND MARKET " << std::endl;
      }

      continue;
    }
    else if (getText(xmlRequest, xmlResponse, rtgText))
    {
      if (rbData.isSecondaryCityPairSameAsPrime())
      {
        _trx.response() << "SPECIFY SECONDARY MARKET FOR COMPLETE DISPLAY" << std::endl;
      }

      _trx.response() << airSeg.marketingCarrierCode() << "  " << airSeg.origAirport() << " "
                      << airSeg.destAirport() << std::endl;
      if (rtgText.empty())
      {
        if (_trx.requestedCarrier() == _trx.getRequest()->secondaryCarrier().at(i).c_str() &&
            fare->carrier() != INDUSTRY_CARRIER)
        {
          _trx.response() << "      NO EXCEPTION DATA EXISTS USE PRIME BOOKING CODE " << std::endl;
        }
        else
        {
          _trx.response() << "      NO RBD FOUND FOR SPECIFIED CXR AND MARKET " << std::endl;
        }
        continue;
      }
      else
      {
        _skipDiag854 = true;
        _trx.response() << rtgText << std::endl;
      }
      LOG4CXX_DEBUG(logger,
                    "Secondary Market = " << airSeg.marketingCarrierCode() << "  "
                                          << airSeg.origAirport() << " " << airSeg.destAirport()
                                          << std::endl);
      if (rbData.isLastSegmentConditional() &&
          fare->fareMarket()->governingCarrier() == airSeg.marketingCarrierCode())
      {
        _trx.response() << SEPARATOR_LINE << std::endl;
        _trx.response() << "      OTHERWISE APPLY THE PRIME RBD " << std::endl;
      }
    }
  } // for
  if(fare->fareMarket()->getGlobalDirection() != GlobalDirection::ZZ && _gd == GlobalDirection::ZZ)
  {
    fare->fareMarket()->setGlobalDirection(_gd);
    _gd = GlobalDirection::NO_DIR;
  }
  return true; // Never fail, primary market has been done
}

bool
RTGController::getRBText()
{
  LOG4CXX_INFO(logger, "At RTGController::getRBText() for RB request");

  _trx.response().clear();
  if (_trx.allPaxTypeFare().empty())
  {
    _trx.response() << " RB FAILED TO RETRIEVE THE REQUESTED FARE " << std::endl;
    return false;
  }

  std::string xmlRequest;
  std::string xmlResponse;
  std::string rtgText;
  RBData rbData;
  std::vector<BookingCode> bookingCodes;
  size_t secondaryCarrierSize = _trx.getRequest()->secondaryCarrier().size();
  PaxTypeFare* fare = _trx.allPaxTypeFare().front();

  BookingCode bookingCode;
  rbData.setSecondary(false);
  rbData.setCarrierMatchedTable990(false);

  AirSeg* airSeg = dynamic_cast<AirSeg*>(fare->fareMarket()->travelSeg().front());
  if (airSeg == nullptr) // Check if casting failed
  {
    LOG4CXX_ERROR(logger, "dynamic_cast<AirSeg*>(fare->fareMarket()->travelSeg().front())");
    return failRBRequest();
  }

  airSeg->setMarketingCarrierCode(fare->fareMarket()->governingCarrier());
  rbData.setAirSeg(airSeg);
  FareDisplayBookingCodeRB rbBookingCode(&rbData);
  rbBookingCode.getBookingCode(_trx, *fare, bookingCode);
  buildRBHeader(_trx.response(), *fare, airSeg, rbData);
  RBRequestFormatter formatter(xmlRequest, rbData);
  formatter.buildRequest(_trx, false);

  if (xmlRequest.empty())
  {
    if (fare->carrier() == INDUSTRY_CARRIER &&
        fare->fareMarket()->governingCarrier() != INDUSTRY_CARRIER)
    {
      _trx.response() << SEPARATOR_LINE << std::endl;
      _trx.response() << fare->fareMarket()->governingCarrier() << "  "
                      << "EXCEPTIONS" << std::endl;
      _trx.response() << "      NO RBD FOUND FOR SPECIFIED CXR AND MARKET " << std::endl;
    }

    if (secondaryCarrierSize > 0)
    {
      return processRBSecondaryMarket(fare, xmlResponse, false);
    }
    else
    {
      return false;
    }
  }
  else if (getText(xmlRequest, xmlResponse, rtgText))
  {
    if (!rtgText.empty())
    {
      _trx.response() << SEPARATOR_LINE << std::endl;
      _trx.response() << fare->fareMarket()->governingCarrier() << "  "
                      << "EXCEPTIONS" << std::endl;
      _trx.response() << rtgText << std::endl;

      if (rbData.isLastSegmentConditional() &&
          fare->fareMarket()->governingCarrier() != INDUSTRY_CARRIER &&
          fare->fareMarket()->governingCarrier() != ANY_CARRIER)
      {
        _trx.response() << SEPARATOR_LINE << std::endl;
        _trx.response() << "      OTHERWISE APPLY THE PRIME RBD " << std::endl;
      }
    }

    if (secondaryCarrierSize > 0) // Secondary market request?
    {
      return processRBSecondaryMarket(fare, xmlResponse, true);
    }
    else
    {
      return true;
    }
  }
  else
  {
    return failRBRequest();
  }
}

bool
RTGController::connect(ClientSocket& clientSocket)
{

  if (!clientSocket.initialize())
  {
    LOG4CXX_FATAL(logger, "Failed To Connect RTG Server");
    return false;
  }
  return true;
}

bool
RTGController::send(std::string& request, ClientSocket& clientSocket)
{
  if (!clientSocket.send("RQST", "0001", "0000", request))
    return false;
  else
    return true;
}

bool
RTGController::receive(std::string& response, ClientSocket& clientSocket)
{
  std::string command;
  std::string version;
  std::string revision;
  if (!clientSocket.receive(command, version, revision, response))
  {
    LOG4CXX_DEBUG(logger, "Response=" << response);
    return false;
  }
  LOG4CXX_DEBUG(logger, "Response=" << response);
  return true;
}

bool
RTGController::parse(std::string& response, std::string& rtgText)
{
  LOG4CXX_DEBUG(logger, "PARSE RESPONSE ");
  RuleResponseContentHandler docHandler(rtgText, _ruleTextMap, _trx.fdResponse()->rtgDiagInfo());
  docHandler.initialize();
  if (!docHandler.parse(response.c_str()))
  {
    LOG4CXX_FATAL(logger, "PARSE FAILED ");
    return false;
  }
  else
    return true;
}

bool
RTGController::getText(std::string& xmlRequest, std::string& xmlResponse, std::string& rtgText)
{
  LOG4CXX_DEBUG(logger, "Request=" << xmlRequest);

  if (FareDisplayUtil::isXMLDiagRequested(_trx, "RTGREQ"))
    FareDisplayUtil::displayXMLDiag(_trx, xmlRequest, "RTG XML REQUEST");

  ClientSocket clientSocket(cscv);
  if (connect(clientSocket))
  {
    if (send(xmlRequest, clientSocket))
    {
      bool isReceived = receive(xmlResponse, clientSocket);

      if (isReceived && FareDisplayUtil::isXMLDiagRequested(_trx, "RTGRES"))
        FareDisplayUtil::displayXMLDiag(_trx, xmlResponse, "RTG XML RESPONSE");

      if (isReceived && xmlResponse.empty() == false)
        return parse(xmlResponse, rtgText);
    }
  }
  return false;
}

bool
RTGController::hasGeneralAndFareRule(const int catNum) const
{
  bool hasFareRule(false), hasGeneralRule(false);
  RuleTextMap::const_iterator ruleTextIter = _ruleTextMap.begin();
  RuleTextMap::const_iterator ruleTextIterEnd = _ruleTextMap.end();

  for (; ruleTextIter != ruleTextIterEnd; ruleTextIter++)
  {
    if (ruleTextIter->first.first != catNum)
      continue;

    if (ruleTextIter->first.second == FareRule)
      hasFareRule = true;

    if (ruleTextIter->first.second == GeneralRule)
      hasGeneralRule = true;
  }

  return hasFareRule && hasGeneralRule;
}
}
