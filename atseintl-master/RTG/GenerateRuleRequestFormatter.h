//----------------------------------------------------------------------------
//
//      File: GenerateRuleRequestFormatter.h
//      Description: Class to format an RTG GenerateRuleRequest
//      Created: May 24, 2005
//      Authors: Mike Carroll
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "Common/TseEnums.h"
#include "Common/XMLConstruct.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBCategoryRuleRecord.h"
#include "DBAccess/DBAForwardDecl.h"
#include "DBAccess/FootNoteCtrlInfo.h"

namespace tse
{
class Agent;
class Money;

class GenerateRuleRequestFormatter
{
public:
  GenerateRuleRequestFormatter() = default;
  GenerateRuleRequestFormatter(bool recordScopeDomestic)
    : _isRecordScopeDomestic(recordScopeDomestic)
  {
  }

  bool& isRecordScopeDomestic() { return _isRecordScopeDomestic; }
  const bool& isRecordScopeDomestic() const { return _isRecordScopeDomestic; }

  //--------------------------------------------------------------------------
  // @function GenerateRuleRequestFormatter::formatRequest
  //
  // Description: Prepare a GenerateRuleRequest request tagged suitably for
  //              RTG
  //
  // @param catRuleRecord - reference to a valid FBCategoryRuleRecord
  // @param agent - reference to a valid Agent record
  // @param billing - reference to a valid Billing record
  // @param request - where to deposit the request
  //--------------------------------------------------------------------------
  /*	void
     GenerateRuleRequestFormatter::formatRequest(
         FBCategoryRuleRecord& catRuleRecord,
         const Agent& agent,
         const Billing& billing,
         std::string& request,
         CatNumber	subcategory);
  */

  //--------------------------------------------------------------------------
  // @function GenerateRuleRequestFormatter::formatCannedRequest
  //
  // Description: Prepare a ANNED CAT5 request tagged suitably for
  //              RTG
  //
  // @param catRuleRecord - reference to a valid FBCategoryRuleRecord
  // @param agent - reference to a valid Agent record
  // @param billing - reference to a valid Billing record
  // @param request - where to deposit the request
  //--------------------------------------------------------------------------
  /*void
  GenerateRuleRequestFormatter::formatCannedRequest(
      const FBCategoryRuleRecord& catRuleRecord,
      const Agent& agent,
      const Billing& billing,
      std::string& request);
*/

  void addRCSHeader(const GeneralFareRuleInfo* ruleInfo, XMLConstruct& construct);
  void addRCSHeader(const FootNoteCtrlInfo* ruleInfo, XMLConstruct& construct);
  void addRCSHeader(const CombinabilityRuleInfo* ruleInfo, XMLConstruct& construct);
  void addRCSHeader(const FareByRuleCtrlInfo* ruleInfo, XMLConstruct& construct);
  void addQ4RAttribute(const RuleType& ruleType, XMLConstruct& construct);

  //-------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addAGIType
  //
  // Description: Add agent section to a working request message
  //
  // @param agent - reference to a valid agent
  // @param billing - reference to a valid billing record
  // @param construct - where the formatted agent information goes
  // @param trx - use to reference request information
  // @return void
  //-------------------------------------------------------------------------
  void addAGIType(const Agent& agent,
                  const Billing& billing,
                  XMLConstruct& construct,
                  FareDisplayTrx& trx);

  //-------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addFRIType
  //
  // Description: Add Rule/Fare information to a working
  //              request message
  //
  // @param PaxTypeFare - reference to a valid PaxTypeFare
  // @param construct - a valid XMLContruct
  // @return void
  //-------------------------------------------------------------------------
  void addRFIType(const PaxTypeFare& ptf, XMLConstruct& construct);

  //-------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addR2KType
  //
  // Description: Add Record 2 Key selection information to a working
  //              request message
  //
  // @param catRuleInfo - reference to a valid CategoryRuleInfo
  // @param construct - a valid XMLContruct
  // @return void
  //-------------------------------------------------------------------------
  template <class T>
  void addR2KType(const T& ruleInfo, XMLConstruct& construct)
  {
    std::ostringstream oss;
    char tmpBuf[20];

    construct.openElement("R2K");
    // Vendor key
    construct.addAttribute("S37", ruleInfo.vendorCode());

    // Rule tariff key
    sprintf(tmpBuf, "%d", ruleInfo.tariffNumber());
    construct.addAttribute("Q3W", tmpBuf);

    // Carrier key
    construct.addAttribute("B00", ruleInfo.carrierCode());

    // Rule key
    const FootNoteRecord2Info* footNote = dynamic_cast<const FootNoteRecord2Info*>(&ruleInfo);
    if (footNote)
    {
      construct.addAttribute("Q3T", footNote->footNote());
    }
    else
    {
      construct.addAttribute("Q3T", ruleInfo.ruleNumber());
    }

    // Category key
    sprintf(tmpBuf, "%d", ruleInfo.categoryNumber());
    construct.addAttribute("Q3V", tmpBuf);

    // Sequence number key
    oss << ruleInfo.sequenceNumber();

    //sprintf(tmpBuf, "%d", ruleInfo.sequenceNumber());

    construct.addAttribute("L00", oss.str());

    construct.closeElement();
  }





  //-------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addCSBType
  //
  // Description: Add Combinability Scoreboard information to a working
  //              request message
  //
  // @param catRuleInfo - reference to a valid CategoryRuleInfo
  // @param construct - a valid XMLContruct
  // @return void
  //-------------------------------------------------------------------------
  void addCSBType(const CombinabilityRuleInfo& catRuleInfo, XMLConstruct& construct);


  template <class T>
  void addSSIType(const T& catRuleInfo, XMLConstruct& construct,
                  CatNumber subCategory)
  {
    const auto& categoryRuleItemInfoSet = catRuleInfo.categoryRuleItemInfoSet();
    bool foundCat10Item = false;

    if (!categoryRuleItemInfoSet.empty())
    {
      for (const auto& setPtr: categoryRuleItemInfoSet)
      {
        for (const auto& info: *setPtr)
        {
          if (subCategory == static_cast<CatNumber>(info.itemcat()))
          {
            makeSSITags(construct, info);
            foundCat10Item = true;
          }
        }
      }
    }
    if (!foundCat10Item)
    {
      LOG4CXX_WARN(getLogger(), "Sub-Category = " << subCategory << " not found!!");
      return;
    }
  }

  //-------------------------------------------------------------------------
  // @function GenerateRuleRequestFormatter::makeSSITags
  //
  // Description: make a single SSI XML tag with all attributes.
  //
  // @param construct - a valid XMLContruct
  // @param categoryRuleInfo - a reference to the Cat 10 rule info
  // @return void
  //-------------------------------------------------------------------------
  template <class T>
  void makeSSITags(XMLConstruct& construct, const T& info,
                   bool forceTHEN = false)
  {
    std::string relationalTranslation;
    char tmpBuf[10];

    construct.openElement("SSI");

    // Relational indicator
    if (info.relationalInd() == CategoryRuleItemInfo::IF)
      relationalTranslation = ":";
    else if (info.relationalInd() == CategoryRuleItemInfo::THEN)
      relationalTranslation = "=";
    else if (info.relationalInd() == CategoryRuleItemInfo::OR)
      relationalTranslation = forceTHEN ? "=" : "/";
    else if (info.relationalInd() == CategoryRuleItemInfo::AND)
      relationalTranslation = "&amp;"; //"&";
    else // ELSE
      relationalTranslation = "*";

    construct.addAttribute("N0X", relationalTranslation);

    // IN/OUT indicator
    sprintf(tmpBuf, "%c", info.inOutInd());
    construct.addAttribute("N0Y", tmpBuf);

    // Directionality
    sprintf(tmpBuf, "%c", info.directionality());
    construct.addAttribute("N0Z", tmpBuf);

    // Item category
    sprintf(tmpBuf, "%d", info.itemcat());
    construct.addAttribute("Q3V", tmpBuf);

    // Item number
    sprintf(tmpBuf, "%d", info.itemNo());
    construct.addAttribute("L00", tmpBuf);

    construct.closeElement();
  }

  void
  addC25Type(std::string& fareBasis, CurrencyCode curr, Indicator fareInd, XMLConstruct& construct);

  // info on base fare used for discount (19-22, 25)
  void addDISType(std::string& fareBasis,
                  Indicator owrt, // (raw ATPCO in TseConst)
                  const PaxTypeCode& paxTypeCode,
                  XMLConstruct& construct);

  int asFixedPoint(double x, int numDecimals);

  //-------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addC35Type
  //
  // Description: Add CAT35 selection information to a working request
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLContruct
  // @return void
  //-------------------------------------------------------------------------
  void addC35Type(PseudoCityCode& creatorPCC,
                  Indicator calcInd,
                  Money& ruleMoney,
                  int noDecAmt,
                  Percent percent,
                  int noDecPercent,
                  bool needCalcData,
                  Indicator displayCatType,
                  Indicator canSell,
                  Indicator canTkt,
                  Indicator canUpd,
                  Indicator canRedist,
                  Indicator locale,
                  XMLConstruct& construct);

  void addTVLType(const FareDisplayTrx& trx, XMLConstruct& construct, bool skipDiag854 = false);

private:
  bool _isRecordScopeDomestic = false;

  //-------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addSSIType
  //
  // Description: Add selection information  to a working request message
  //
  // @param catRuleInfo - reference to a valid CategoryRuleInfo
  // @param construct - a valid XMLContruct
  // @return void
  //-------------------------------------------------------------------------
  template <class T>
  void addSSIType(const T& catRuleInfo, XMLConstruct& construct)
  {
    // There may be more than one of these sections!
    for (const auto& setPtr: catRuleInfo.categoryRuleItemInfoSet())
    {
      for (const auto& info: *setPtr)
      {
        makeSSITags(construct, info);
      }
    }
  }

  static Logger& getLogger();

}; // End class GenerateRuleRequestFormatter
} // End namespace tse
