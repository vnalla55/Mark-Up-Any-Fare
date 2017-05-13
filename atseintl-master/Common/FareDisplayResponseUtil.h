//-------------------------------------------------------------------
//  Copyright Sabre 2009
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/RoutingRestriction.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"

#include <sstream>
#include <string>
#include <vector>

namespace tse
{

class FareDisplayTrx;
class FareDisplayInfo;
class PaxTypeFare;
class Routing;
class NUCCollectionResults;

class FareDisplayResponseUtil
{
  friend class FareDisplayResponseUtilTest;

public:
  FareDisplayResponseUtil();
  virtual ~FareDisplayResponseUtil();

  // Methods to build Routing Display
  typedef std::vector<std::string> RoutingMapStrings;
  typedef std::vector<const Routing*> Routings;
  typedef Routings::const_iterator RoutingsConstIter;
  typedef std::vector<RoutingRestriction*> RoutingRestrictions;
  typedef RoutingRestrictions::const_iterator RoutingRestrictionsConstIter;

  static bool buildRTGDisplay(tse::FareDisplayTrx& trx, const PaxTypeFare& paxTypeFare);
  static bool addGlobalDescription(FareDisplayTrx& trx, GlobalDirection global, bool inLine);
  static bool isNonstop(const FareDisplayTrx&, const RoutingMapStrings*);
  static bool isCTRW(FareDisplayTrx&, GlobalDirection gd, const RoutingInfo&);
  static bool isIncomplete(const RoutingInfo&);
  static void displayIncomplete(FareDisplayTrx&, bool);
  static bool addRouteString(FareDisplayTrx& trx,
                             const RoutingMapStrings* strings,
                             bool indent,
                             bool useLineNumbers,
                             bool fdDisplay);
  static void addRestrictions(FareDisplayTrx&, const RoutingRestrictions&, bool, bool);
  static void displayTPD(FareDisplayTrx& trx, const RoutingInfo& rtgInfo);
  static void displayPSR(FareDisplayTrx& trx, const RoutingInfo& rtgInfo);
  static bool displayDRV(FareDisplayTrx&, const Routing*, const PaxTypeFare&, bool);
  static void displayDRV(FareDisplayTrx&, const Routings&, const PaxTypeFare&, bool);
  static void displayConstructed(FareDisplayTrx&, const PaxTypeFare&, const RoutingInfo*, bool);

  // Methods to build RD Display
  static void addCatText(FareDisplayTrx& fareDisplayTrx,
                         const CatNumber& catNumber,
                         FareDisplayInfo& fareDisplayInfo,
                         std::ostringstream* oss);
  static void addMXText(FareDisplayInfo& fareDisplayInfo, std::ostringstream* oss);

  // Method to build FB Display
  bool buildFBDisplay(const CatNumber& cat,
                      FareDisplayTrx& fareDisplayTrx,
                      FareDisplayInfo* fareDisplayInfo,
                      PaxTypeFare& paxTypeFare);

  // Methods to get Rule Tariff Description (Moved from FareDisplay/FBTariffDescription.cpp)
  virtual bool getRuleTariffDescription(const FareDisplayTrx& trx,
                                        const TariffNumber& fareTariff,
                                        const GeneralRuleRecord2Info* fGR2,
                                        TariffCode& ruleTariffCode);

  virtual bool getRuleTariffDescription(const FareDisplayTrx& trx,
                                        const TariffNumber& fareTariff,
                                        const VendorCode& vendor,
                                        const CarrierCode& carrier,
                                        const TariffNumber& ruleTariff,
                                        TariffCode& ruleTariffCode);

  // Method to get Addon Tariff Code (Moved from FareDisplay/FBTariffDescription.cpp)
  static bool getAddonTariffCode(const FareDisplayTrx& trx,
                                 const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& addonTariff,
                                 TariffCode& addonTariffCode);

  static std::string getRuleTariffCode(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare);
  static std::string getFareDescription(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare);

  // Method to get dates (Copied logic from FareDisplay/Templates/RDSection.cpp)
  static void getEffDate(FareDisplayTrx& trx,
                         PaxTypeFare& paxTypeFare,
                         FareDisplayInfo& fareDisplayInfo,
                         DateTime& effDate);
  static void getDiscDate(FareDisplayTrx& trx,
                          PaxTypeFare& paxTypeFare,
                          FareDisplayInfo& fareDisplayInfo,
                          DateTime& discDate);

  // Method to convert Fare from published currency to NUC,
  // Then convert Fare from NUC to currency of specified Fare.
  // (Moved from FareDisplay/Templates/ICSection.cpp)
  static void convertFare(FareDisplayTrx& fareDisplayTrx,
                          PaxTypeFare& paxTypeFare,
                          MoneyAmount& fareAmount,
                          const CurrencyCode& displayCurrency,
                          NUCCollectionResults& nucResults);

  // precondition: src is alpha-numeric string
  // postcondition: retuns true (rn=<valid number>/false (rn=0) for numeric/alpha-numeric input str
  static bool routingNumberToNumeric(const std::string& src, int16_t& rn);

  // precondition: src is alpha-numeric string
  // postcondition: retuns a string without leading zeros or spaces
  static std::string routingNumberToString(const std::string& src);

  // precondition: src is alpha-numeric string
  // postcondition: retuns a 4 char long string with all leading zero
  //  conveted to spaces and shifted as per ff value
  static std::string
  routingNumberToStringFormat(const std::string& src, std::ios_base::fmtflags ff);

  // postcondition: retuns a 4 char long string shifted as per ff value
  static std::string routingNumberToStringFormat(uint16_t rn, std::ios_base::fmtflags ff);

private:

  // Methods to build Routing Display
  static bool
  displayAnyRoutingForRD(FareDisplayTrx&, const std::string& rtg, const GlobalDirection& glb);
  static void
  addConstructedInfo(FareDisplayTrx& trx, const PaxTypeFare& paxTypeFare, const RoutingInfo* info);
  static bool all17(const RoutingRestrictions&);
  static void
  addRTGMPM(FareDisplayTrx& trx, const PaxTypeFare& paxTypeFare, const RoutingInfo* info);
  static void displayMileageMsg(FareDisplayTrx&, const RoutingInfo*);
  static void
  splitLine(FareDisplayTrx& trx, const std::string& theLine, bool inLine, uint32_t lineNum, bool);
  static void displayConstrMsg(FareDisplayTrx&, const std::string&, const std::string&, bool);
  static std::string translateNation(const NationCode&);
  static std::string gateway(const std::string&);

  // Methods to build FB Display
  void buildBaseFareDisplay(FareDisplayTrx& fareDisplayTrx,
                            PaxTypeFare& paxTypeFare,
                            FBCategoryRuleRecord* _baseFareRuleRecord,
                            std::string _record2Type);
  static void addBaseFareLine(FareDisplayTrx& fareDisplayTrx);
  static void addFBRLine(FareDisplayTrx& fareDisplayTrx);
  static void addBlankLine(FareDisplayTrx& fareDisplayTrx);
  static void spaces(FareDisplayTrx& fareDisplayTrx, const int16_t& numSpaces);
  static void addNoRuleDataLine(FareDisplayTrx& fareDisplayTrx);
  static void addGeneralRuleRec2(FareDisplayTrx& fareDisplayTrx);
  static void addFareRuleRec2(FareDisplayTrx& fareDisplayTrx);
  static void addFootnoteRec2(FareDisplayTrx& fareDisplayTrx);

  template <class T>
  void addRecord2StringTable(FareDisplayTrx& fareDisplayTrx, const T& criiSets)
  {
    for (const auto& setPtr: criiSets)
    {
      const size_t _noOfDataTables = setPtr->size();
      addDataTableInfo(fareDisplayTrx, _noOfDataTables);

      for (const auto& itemInfo: *setPtr)
      {
        addStringTableInfo(fareDisplayTrx, itemInfo);
      }
      addBlankLine(fareDisplayTrx);
    }
  }

  static void addDataTableInfo(FareDisplayTrx& fareDisplayTrx, size_t _noOfDataTables);

  // -------------------------------------------------------------------------
  // FareDisplayResponseUtil::addStringTableInfo()
  // -------------------------------------------------------------------------
  template <class T>
  void addStringTableInfo(FareDisplayTrx& fareDisplayTrx, const T& cRII)
  {
    const std::string relInd[5] = { "IF", "THEN", "OR", "ELSE", "AND" };

    std::ostringstream* oss = &fareDisplayTrx.response();

    // Relational indicators
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << std::setfill(' ') << std::setw(5) << relInd[cRII.relationalInd()];

    // cat of item from CatRuleItemInfo (not CatRuleInfo)
    oss->setf(std::ios::right, std::ios::adjustfield);
    *oss << std::setfill('0') << std::setw(2) << cRII.itemcat();

    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << DASH << SPACE << std::setfill(' ') << std::setw(6) << cRII.itemNo();
    *oss << SPACE << cRII.directionality() << std::endl;
  }

  static void addLine3(FareDisplayTrx& fareDisplayTrx,
                       FBCategoryRuleRecord* fbRuleRecord,
                       std::string _record2Type,
                       std::string _ruleTariffCode);
  static void addLine4(FareDisplayTrx& fareDisplayTrx,
                       FBCategoryRuleRecord* fbRuleRecord,
                       std::string _record2Type);
  static void addLine5(FareDisplayTrx& fareDisplayTrx,
                       FBCategoryRuleRecord* fbRuleRecord,
                       std::string _record2Type);
  static void addLine6(FareDisplayTrx& fareDisplayTrx,
                       FBCategoryRuleRecord* fbRuleRecord,
                       std::string _record2Type);
  static void addLine7(FareDisplayTrx& fareDisplayTrx,
                       FBCategoryRuleRecord* fbRuleRecord,
                       std::string _record2Type);
  static void addLine8(FareDisplayTrx& fareDisplayTrx);

  // Methods to get Rule Tariff Description (Moved from FareDisplay/FBTariffDescription.cpp)
  virtual RecordScope getCrossRefType(const FareDisplayTrx& trx);

  const TariffCrossRefInfo*
  matchByFareTariff(const std::vector<TariffCrossRefInfo*>& tariffDescInfoList,
                    const TariffNumber& fareTariff);
  virtual DataHandle& getDataHandle(const FareDisplayTrx& trx) { return trx.dataHandle(); }

  virtual const std::vector<TariffCrossRefInfo*>&
  getTariffXRefByRuleTariff(DataHandle& dataHandle,
                            const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const RecordScope& crossRefType,
                            const TariffNumber& ruleTariff,
                            const DateTime& travelDate)
  {
    return dataHandle.getTariffXRefByRuleTariff(
        vendor, carrier, crossRefType, ruleTariff, travelDate);
  }

  virtual const std::vector<TariffCrossRefInfo*>&
  getTariffXRefByGenRuleTariff(DataHandle& dataHandle,
                               const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const RecordScope& crossRefType,
                               const TariffNumber& ruleTariff,
                               const DateTime& travelDate)
  {
    return dataHandle.getTariffXRefByGenRuleTariff(
        vendor, carrier, crossRefType, ruleTariff, travelDate);
  }

  // Method to get Addon Tariff Code (Moved from FareDisplay/FBTariffDescription.cpp)
  static const TariffCrossRefInfo*
  matchByAddonTariff(const std::vector<TariffCrossRefInfo*>& tariffDescInfoList,
                     const TariffNumber& addonTariff);

  enum AddonAndPublishType
  { AptEmpty,
    AptWithMap,
    AptWithoutMap,
    AptMileage };

  static AddonAndPublishType returnRoutingType(const Routing* routing);
  static uint16_t countAddon(const Routing* origAddOn, const Routing* destAddOn);

  static void origAdnDesfForRtg(AddonAndPublishType origin,
                                AddonAndPublishType base,
                                AddonAndPublishType dest,
                                std::string& originString,
                                std::string& destString,
                                const FareMarket& fareMarket);

  static void origAdnDesfForRtgWithoutMap(AddonAndPublishType origin,
                                          AddonAndPublishType base,
                                          AddonAndPublishType dest,
                                          std::string& originString,
                                          std::string& destString,
                                          const FareMarket& fareMarket);

  static void copyRestriction17and3(const Routing* routing,
                                    Routings& drvs,
                                    RoutingRestrictions& restsBeforeStrings,
                                    RoutingRestrictions& rests);

  static void displayMapDirectionalityInfo(FareDisplayTrx& trx, const RoutingInfo& rtgInfo);
};
} // end tse namespace
