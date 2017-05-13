//----------------------------------------------------------------------------
//
//      File: FareDisplayResponseFormatter.h
//      Description: Class to format Fare Display responses back to sending
//                   client
//      Created: February 17, 2005
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

#include "Common/XMLConstruct.h"
#include "DataModel/FareDisplayTrx.h"
#include "Xform/ResponseFormatter.h"

namespace tse
{
class FareDisplayResponseXMLTags;
class FareDisplaySDSTaxInfo;

class FareDisplayResponseFormatter : public ResponseFormatter
{
  friend class FareDisplayResponseFormatterTest;

public:
  FareDisplayResponseFormatter(FareDisplayResponseXMLTags& xmlTags) : _xmlTags(xmlTags) {}

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::formatResponse
  //
  // Description: Prepare a FareDisplayRequest response tagged
  //              suitably for client
  //
  // @param currencyTrx - a valid CurrencyTrx
  //--------------------------------------------------------------------------
  std::string formatResponse(FareDisplayTrx& fareDisplayTrx);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::formatResponse
  //
  // Description: Prepare an error response
  //
  // @param ere - error type
  // @param response - formatted response
  //--------------------------------------------------------------------------
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

protected:
  char _tmpBuf[256];
  int _recNum = 2;
  void addMessageLine(const std::string& line, XMLConstruct& construct, const std::string& msgType);

private:
  FareDisplayResponseXMLTags& _xmlTags;

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::mainDiagResponse
  //
  // Description: Add detail display to FQ Master diagnostics
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @return void
  //--------------------------------------------------------------------------
  void mainDiagResponse(FareDisplayTrx& fareDisplayTrx);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addDetailResponse
  //
  // Description: Add detail portion of a client FareDisplayResponse
  //              suitably for client
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addDetailResponse(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addODCType
  //
  // Description: Add origin/destination and currency information
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLContruct
  // @return void
  //--------------------------------------------------------------------------
  void addODCType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addOCMType
  //
  // Description: Add Other Carriers in Market section to a working response
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addOCMType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addMACType
  //
  // Description: Add marketed airline code section to a working response
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addMACType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addNSFType
  //
  // Description: Add non-stop flights section to a working response
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addNSFType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addDFCType
  //
  // Description: Add direct flights count section to a working response
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addDFCType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addCFCType
  //
  // Description: Add connecting flights count section to a working response
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addCFCType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addYYMType
  //
  // Description: Add YY fare message section to a working response
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addYYMType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addHDMType
  //
  // Description: Add header message section to a working response
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addHDMType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addRTGType
  //
  // Description: Add routing message section to a working response
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addRTGType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addFQDType
  //
  // Description: Add fare quote data section to a working response
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addFQDType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addFQDTypeForAddon
  //
  // Description: Add Addon fare quote data section to a working response
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addFQDTypeForAddon(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addFTDTypeForAddon
  //
  // Description: Add Addon fare tax data section to a working SDS response
  //              message
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addFTDType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addRULType
  //
  // Description: Add rules section to a working response
  //              message
  //   @param fareDisplayTrx - a valid FareDisplayTrx
  // @param fareDisplayInfo - a valid FareDisplayInfo
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addRULType(FareDisplayTrx& fareDisplayTrx,
                  FareDisplayInfo*& fareDisplayInfo,
                  XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addC25Type
  //
  // Description: Add CAT 25 section to a working response
  //
  // @param paxTypeFare - a valid PaxTypeFare
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addC25Type(PaxTypeFare& paxTypeFare, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addC35Type
  //
  // Description: Add CAT 35 section to a working response
  //
  // @param paxTypeFare - a valid PaxTypeFare
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addC35Type(PaxTypeFare& paxTypeFare, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addDFIType
  //
  // Description: Add Discounted fare section to a working response
  //
  // @param paxTypeFare - a valid PaxTypeFare
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addDFIType(PaxTypeFare& paxTypeFare, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addDAOType
  //
  // Description: Add Destination Addon section to a working response
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param paxTypeFare - a valid PaxTypeFare
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void
  addDAOType(FareDisplayTrx& fareDisplayTrx, PaxTypeFare& paxTypeFare, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addOAOType
  //
  // Description: Add Origin Addon section to a working response
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param paxTypeFare - a valid PaxTypeFare
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void
  addOAOType(FareDisplayTrx& fareDisplayTrx, PaxTypeFare& paxTypeFare, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addPTCType
  //
  // Description: Add one way and round trip fare amounts to the response
  //
  // @param fareDisplayInfo - a valid FareDisplayInfo
  // @param paxTypeFare - a valid PaxTypeFare
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void
  addPTCType(FareDisplayInfo*& fareDisplayInfo, PaxTypeFare& paxTypeFare, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addFAMType
  //
  // Description: Add one way and round trip fare amounts to the response
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param paxTypeFare - a valid PaxTypeFare
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void
  addFAMType(FareDisplayTrx& fareDisplayTrx, PaxTypeFare& paxTypeFare, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addSEAType
  //
  // Description: Add seasons info to a working response
  //
  // @param fareDisplayInfo - a valid FareDisplayInfo
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addSEAType(FareDisplayInfo*& fareDisplayInfo, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addFTCType
  //
  // Description: Add currency conversion info in SDS Tax response
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addFTCType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addFTIType
  //
  // Description: Add Tax info in SDS Tax response
  //
  // @param taxInfo - a valid FareDisplaySDSTaxInfo
  // @param ccode - a valid CurrencyCode
  // @param construct - a valid XMLConstruct
  // @param ticketingDate - transaction ticketing date
  // @return void
  //--------------------------------------------------------------------------
  void addFTIType(const std::vector<FareDisplaySDSTaxInfo*>& taxInfo,
                  CurrencyCode& ccode,
                  XMLConstruct& construct,
                  const DateTime& ticketingDate);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::addFTDTypeForFQDType
  //
  // Description: Determine Tax info type to be put into FQD response for SDS on FQ with FL
  //
  // @param taxInfo - a valid FareDisplaySDSTaxInfo
  // @param ccode - a valid CurrencyCode
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void addFTDTypeForFQDType(FareDisplayTrx& fareDisplayTrx,
                            XMLConstruct& construct,
                            PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::buildFTDTypeForFQDType
  //
  // Description: Add Tax info in FQD response for SDS on FQ with FL
  //
  // @param taxInfo - a valid FareDisplaySDSTaxInfo
  // @param ccode - a valid CurrencyCode
  // @param construct - a valid XMLConstruct
  // @return void
  //--------------------------------------------------------------------------
  void buildFTDTypeForFQDType(FareDisplayTrx& fareDisplayTrx,
                              XMLConstruct& construct,
                              PaxTypeFare& paxTypeFare,
                              std::vector<TaxRecord*> taxRecordVec,
                              std::vector<TaxItem*> taxItemVec,
                              std::string owrt,
                              MoneyAmount fareAmount);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseFormatter::isErrorCondition
  //
  // Description: Determine whether the response is an error or not.
  //
  // @param fareDisplayTrx - a valid FareDisplayTrx
  // @return bool
  //--------------------------------------------------------------------------
  bool isErrorCondition(FareDisplayTrx& fareDisplayTrx);

  void buildRuleCategoryInfoTypesForSWS(FareDisplayTrx& fareDisplayTrx,
                                        FareDisplayInfo* fareDisplayInfo,
                                        PaxTypeFare& paxTypeFare,
                                        XMLConstruct& construct);
  void addMNUType(FareDisplayTrx& fareDisplayTrx,
                  FareDisplayInfo* fareDisplayInfo,
                  XMLConstruct& construct);
  void addRSDType(FareDisplayTrx& fareDisplayTrx, XMLConstruct& construct);
  void addCATType(FareDisplayTrx& fareDisplayTrx,
                  FareDisplayInfo* fareDisplayInfo,
                  XMLConstruct& construct);
  void addMXTextForCATType(FareDisplayTrx& fareDisplayTrx,
                           FareDisplayInfo* fareDisplayInfo,
                           XMLConstruct& construct,
                           CatNumber& catNumber);
  void addCatTextForCATType(FareDisplayTrx& fareDisplayTrx,
                            FareDisplayInfo* fareDisplayInfo,
                            XMLConstruct& construct,
                            CatNumber& catNumber);
  void addMXAndCatTextForCATType(FareDisplayTrx& fareDisplayTrx,
                                 FareDisplayInfo* fareDisplayInfo,
                                 XMLConstruct& construct,
                                 CatNumber& catNumber);
  void addCATTypeForFB(FareDisplayTrx& fareDisplayTrx,
                       FareDisplayInfo* fareDisplayInfo,
                       PaxTypeFare& paxTypeFare,
                       XMLConstruct& construct);

  void buildAdditionalRuleInfoForSWS(FareDisplayTrx& fareDisplayTrx,
                                     FareDisplayInfo* fareDisplayInfo,
                                     PaxTypeFare& paxTypeFare,
                                     XMLConstruct& construct);

  void addAddonAttributes(FareDisplayTrx& fareDisplayTrx,
                          PaxTypeFare& paxTypeFare,
                          const Footnote& addonFootnote1,
                          const Footnote& addonFootnote2,
                          const FareClassCode& addonFareClass,
                          const TariffNumber& addonTariff,
                          const RoutingNumber& addonRouting,
                          const MoneyAmount& addonAmount,
                          const CurrencyCode& addonCurrency,
                          const Indicator& addonOWRT,
                          const LocCode& gateWay,
                          const LocCode& market,
                          XMLConstruct& construct);

  void addAddonAttributesForERDFromSWS(FareDisplayTrx& fareDisplayTrx,
                                       PaxTypeFare& paxTypeFare,
                                       const TariffNumber& addonTariff,
                                       const MoneyAmount& addonAmount,
                                       const CurrencyCode& addonCurrency,
                                       const LocCode& gateWay,
                                       const LocCode& market,
                                       XMLConstruct& construct);

  void addResponseLine(std::string& line,
                       XMLConstruct& construct,
                       const Indicator& responseType,
                       const std::string& msgType = " ");

  void addAttributesForSDSInFQD(FareDisplayTrx& fareDisplayTrx,
                                FareDisplayInfo* fareDisplayInfo,
                                PaxTypeFare& paxTypeFare,
                                const PaxTypeFare* baseFare,
                                const Fare* fare,
                                XMLConstruct& construct);

  void addAttributesForBrandInFQD(const FareDisplayInfo* fareDisplayInfo,
                                  XMLConstruct& construct);

  void addAttribute(const FareDisplayTrx& fareDisplayTrx,
                          int index,
                          XMLConstruct& construct,
                          bool addJourneyInd = true);

  void getFareDataMapValue(const std::map<FieldColumnElement, std::string>* fareDataMap,
                           FieldColumnElement key,
                           std::string& value,
                           bool format = true);

  void formatValue(std::string& value);

  bool reqHasAltCxrs = false;
  bool reqHasYYCxr = false;

}; // End class FareDisplayResponseFormatter

} // End namespace tse

