//----------------------------------------------------------------------------
//
//  File:        RTGController.h
//
//	Copyright Sabre 2004
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

#include "BookingCode/RBData.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/XMLConstruct.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "RTG/GenerateRuleRequestFormatter.h"

#include <sstream>
#include <string>
#include <vector>

namespace tse
{
using RuleTextMapKey = std::pair<CatNumber, int>;
using RuleTextMap = std::map<RuleTextMapKey, std::string>;

class FareDisplayTrx;
class Agent;
class Billing;
class ClientSocket;
class GenerateRuleRequestFormatter;
class FBCategoryRuleRecord;

class RTGController
{
public:
  friend class RuleResponseContentHandlerTest;
  RTGController(FareDisplayTrx& trx) : _trx(trx) {}

  //-------------------------------------------------------------------------
  // @function RTGController::getRuleText
  //
  // Description: Get rule text for the arugment category
  //
  // @param ruleRecord - a valid pointer to a FBCategoryRuleRecord
  // @param agent - reference to an agent record
  // @param billing - reference to a billing record
  // @param repository - where to place the fetched rule text
  // @param Cat - the main category to fetch text for
  // @param subCat - the sub-category (optional) to fetch text for
  //-------------------------------------------------------------------------
  bool getRuleText(FBCategoryRuleRecord* ruleRecord,
                   PaxTypeFare& ptf,
                   const Agent& agent,
                   const Billing& billing,
                   std::string& repository,
                   CatNumber cat,
                   bool isMinMaxFare,
                   CatNumber subCat = 0);

  // -------------------------------------------------------------------
  // @function RTGController::buildBundledRequest
  //
  // Description: Build RTG request for all mapped categories
  //
  // @param ruleRecord - a valid pointer to a FBCategoryRuleRecord
  // @param ptf - a reference to a paxTypeFare object.
  // @param Cat - the main category to fetch text for
  // @param subCat - the sub-category (optional) to fetch text for
  // -------------------------------------------------------------------------
  void buildBundledRequest(FBCategoryRuleRecord* ruleRecord,
                           PaxTypeFare& ptf,
                           CatNumber cat,
                           bool isMinMaxFare,
                           bool& hasRule,
                           FBDisplay& fbDisplay,
                           CatNumber subCat = 0);

  void initializeBundledRequest(FareDisplayTrx& trx, PaxTypeFare& ptf);

  void finalizeBundledRequest();

  void sendBundledRequest(std::string& response);

  void updateFareDisplayRuleTextMap(FBDisplay& fbDisplay, bool haveSubCats, bool isFBRfare);

  /**
   * interface for RB entry.
   */
  bool getRBText();

protected:
  RuleTextMap _ruleTextMap;

private:
  bool getResponse(const std::string& request, std::string& response);

  bool formatOneRuleItem(XMLConstruct& construct,
                         GenerateRuleRequestFormatter& formatter,
                         const CategoryRuleInfo* catRuleInfo,
                         const CategoryRuleItemInfo* ruleUsed);

  bool formatDiscFareRuleItem(XMLConstruct& construct,
                              GenerateRuleRequestFormatter& formatter,
                              const PaxTypeFare& paxTypeFare,
                              const CategoryRuleInfo* catRuleInfo);

  bool formatRuleInfo(XMLConstruct& construct,
                      GenerateRuleRequestFormatter& formatter,
                      const CategoryRuleInfo* catRuleInfo,
                      PaxTypeFare* ptf);

  template <class T>
  bool formatAllRuleItems(XMLConstruct& construct,
                          GenerateRuleRequestFormatter& formatter,
                          const T* catRuleInfo);

  PaxTypeCode basePaxtype(const PaxTypeFare* baseFare);

  bool getFareByRuleText(bool isDom,
                         bool isMinMaxFare,
                         const Agent& agent,
                         const Billing& billing,
                         const FareByRuleCtrlInfo* catRuleInfo,
                         PaxTypeFare& ptf,
                         std::string& response);

  bool getNegText(GenerateRuleRequestFormatter formatter,
                  PaxTypeFare& ptf,
                  XMLConstruct& construct,
                  CatNumber cat);

  bool getFootnoteText(bool isDom,
                       const Agent& agent,
                       const Billing& billing,
                       const FootNoteCtrlInfo* catRuleInfo,
                       PaxTypeFare& ptf,
                       std::string& response);

  bool getFareText(bool isDom,
                   const Agent& agent,
                   const Billing& billing,
                   const FareRuleRecord2Info* catRuleInfo,
                   PaxTypeFare& ptf,
                   std::string& response);

  bool getCombinalityText(bool isDom,
                          const Agent& agent,
                          const Billing& billing,
                          const CombinabilityRuleInfo* catRuleInfo,
                          std::string& response,
                          uint32_t subCat,
                          PaxTypeFare& ptf);

  bool getGeneralText(bool isDom,
                      const Agent& agent,
                      const Billing& billing,
                      const GeneralRuleRecord2Info* catRuleInfo,
                      PaxTypeFare& ptf,
                      std::string& response);

  bool isRuleNeeded(const CategoryRuleInfo* catRuleInfo, PaxTypeFare* ptf);
  /**
   * For bundled RTG request
   */

  void buildFareByRuleReq(bool isMinMaxFare,
                          const FareByRuleCtrlInfo* catRuleInfo,
                          PaxTypeFare& ptf,
                          GenerateRuleRequestFormatter& formatter);

  void buildFootnoteReq(const FootNoteCtrlInfo* catRuleInfo,
                        PaxTypeFare& ptf,
                        GenerateRuleRequestFormatter& formatter,
                        bool baseFare = false);

  void buildFareRuleReq(const FareRuleRecord2Info* catRuleInfo,
                        PaxTypeFare& ptf,
                        GenerateRuleRequestFormatter& formatter,
                        bool baseFare = false);

  void buildCombinalityReq(const CombinabilityRuleInfo* catRuleInfo,
                           uint32_t subCat,
                           GenerateRuleRequestFormatter& formatter);

  void buildGeneralRuleReq(const GeneralRuleRecord2Info* catRuleInfo,
                           PaxTypeFare& ptf,
                           GenerateRuleRequestFormatter& formatter,
                           bool baseFare = false);

  /**
   * connects to the RTG server. Precondition : A valid srver name with IP and Port
   */

  bool connect(ClientSocket& socket);

  /**
   * gets the response from the RTG server.
   */

  bool getText(std::string& xmlRequest, std::string& xmlResponse, std::string& rtgText);

  /**
   * build prime booking codes text
   */
  void buildPrimeBookingCodes(std::ostringstream& response,
                              const CarrierCode& carrier,
                              const std::vector<BookingCode>& bookingCodes);
  /**
   * build the RB header text.
   */
  void buildRBHeader(std::ostringstream& response,
                     PaxTypeFare& fare,
                     AirSeg* airSeg,
                     RBData& rbData);

  /**
   * Append RB PROCESSING ERROR DETECTED and return false.
   */
  bool failRBRequest();

  /**
   * process and contruct rule text for RB Secondary Market request
   */
  bool processRBSecondaryMarket(PaxTypeFare* fare, std::string& xmlResponse, bool skipDiag854);

  /**
   * Parses the response using the RuleResponseContentHandler.
   */

  bool parse(std::string& xmlResponse, std::string& rtgText);

  /**
   * reads the response from the recieving end.
   */

  bool receive(std::string& response, ClientSocket& clientSocket);

  /**
   * sends the request to the connected server.
   */
  bool send(std::string& request, ClientSocket& clientSocket);

  /**
   * sends the request to the connected server.
   */
  bool isDomestic()
  {
    GeoTravelType type = _trx.itin().front()->geoTravelType();
    return (type == GeoTravelType::Domestic || type == GeoTravelType::Transborder);
  }

  void addLine(FBDisplay& fbDisplay, CatNumber catNum, const std::string& value);

  void addBlankLine(FBDisplay& fbDisplay, CatNumber catNum);

  void addRuleTextError(FBDisplay& fbDisplay, CatNumber catNum);

  void addRuleTextPeakUseError(FBDisplay& fbDisplay, CatNumber catNum);

  bool hasGeneralAndFareRule(const int catNum) const;

  FareDisplayTrx& _trx;

  // For bundled requests
  XMLConstruct _construct;
  bool _doesParseThrowException = true;
  GlobalDirection _gd = NO_DIR;

  static const std::string HARD_CODED_DOMESTIC_RB_RESTRICTION;

  static const std::string SEPARATOR_LINE;

  static const std::string FARE_BY_RULE;
  static const std::string BASE_FARE;
  static const std::string FARE_RULE;
  static const std::string FOOTNOTE_RULE;
  static const std::string GENERAL_RULE;
  static const std::string ADDITIONAL;
  static const std::string CAT_10;
  static const std::string NOT_VALIDATED_1;
  static const std::string NOT_VALIDATED_2;
  static const std::string FARE_RULE_HEADER;
  static const std::string GENERAL_RULE_HEADER;

  using RuleSetIter = std::vector<CategoryRuleItemInfoSet*>::const_iterator;
  using RuleItemIter = std::vector<CategoryRuleItemInfo>::const_iterator;
};

} // End namespace tse

