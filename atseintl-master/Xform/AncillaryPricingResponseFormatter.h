//----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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

#include "Common/Money.h"
#include "Common/XMLConstruct.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "ServiceFees/OCFees.h"
#include "Xform/PricingResponseFormatter.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class OCFees;
class ServiceFeesGroup;
struct PaxOCFees;
struct PaxOCFeesUsages;

class FindPoint
{
  const TravelSeg* _tvl;

public:
  FindPoint(const TravelSeg* tvl) : _tvl(tvl) {}
  bool operator()(const TravelSeg* seg) const { return _tvl->pnrSegment() == seg->pnrSegment(); }
};

class AncillaryPricingResponseFormatter : public PricingResponseFormatter
{
  friend class AncillaryPricingResponseFormatterTest;

public:

  using GroupFeesVector = std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFees>>>;
  using GroupFeesUsagesVector = std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages>>>;

  AncillaryPricingResponseFormatter();

  // Prepares an Ancillary Fee detail response XML for client
  virtual std::string formatResponse(
      const std::string& responseString,
      AncillaryPricingTrx& ancTrx,
      ErrorResponseException::ErrorResponseCode errCode = ErrorResponseException::NO_ERROR);

  virtual void formatAEFeesResponse(XMLConstruct& construct, AncillaryPricingTrx& ancTrx);

  void setAeFeesResponseLimits(AncillaryPricingTrx& ancTrx);

  virtual void prepareResponseText(const std::string& responseString,
                                   XMLConstruct& construct,
                                   bool noSizeLImit = false) const;

  void formatResponse(const ErrorResponseException& ere, std::string& response);

  void buildDisclosureText(AncillaryPricingTrx& ancTrx, Itin* itin, XMLConstruct& construct);

  virtual void buildOCFeesResponse(AncillaryPricingTrx& ancTrx, Itin* itin, XMLConstruct& constructA);

  virtual void buildOcFeesSecondResponseForMonetaryDiscount(AncillaryPricingTrx& ancTrx, Itin* itin, XMLConstruct& construct);

  void buildOperatingCarrierInfo(AncillaryPricingTrx& ancTrx, Itin* itin, XMLConstruct& constructA);

  bool isTimeOutBeforeStartOCFees(AncillaryPricingTrx& ancTrx, XMLConstruct& construct);

  bool anyTimeOutMaxCharCountIssue(AncillaryPricingTrx& ancTrx, XMLConstruct& construct);

  void timeOutMaxCharCountNoOCFeesReturned(AncillaryPricingTrx& ancTrx, XMLConstruct& construct);

  void
  timeOutMaxCharCountRequestedOCFeesReturned(AncillaryPricingTrx& ancTrx, XMLConstruct& constructA);

  bool isGenericTrailer(AncillaryPricingTrx& ancTrx, XMLConstruct& construct);

  void createOCGSection(PricingTrx& pricingTrx, XMLConstruct& construct) override;

  bool checkIfAnyGroupValid(AncillaryPricingTrx& ancTrx);

  virtual void
  formatOCFeesForR7(AncillaryPricingTrx& ancTrx, XMLConstruct& construct, const bool a = false);

  void processOCFeesInServiceGroup(const ServiceFeesGroup* sfg,
                                   AncillaryPricingTrx& ancTrx,
                                   const std::vector<ServiceGroup>& groupCodes,
                                   bool timeOutMax,
                                   uint16_t& feesCount,
                                   uint16_t& dispOnlyFeesCount,
                                   std::vector<PaxOCFees>& paxOcFees);

  virtual void formatOCFees(AncillaryPricingTrx& ancTrx, XMLConstruct& construct, const bool a = false);

  // start - Methods for PaxOCFeesUsages
  virtual void processServiceGroupForR7U(ServiceFeesGroup* sfg,
                                         AncillaryPricingTrx& ancTrx,
                                         bool timeOutMax,
                                         const std::vector<ServiceGroup>& groupCodes,
                                         uint16_t& maxNumberOfOCFees,
                                         bool& maxNumOfFeesReached,
                                         uint16_t& feesCount,
                                         uint16_t& dispOnlyFeesCount,
                                         std::vector<PaxOCFeesUsages>& paxOcFees);
  virtual void
  formatOCFeesForR7U(AncillaryPricingTrx& ancTrx, XMLConstruct& construct, const bool a = false);

  void formatOCFeesGroupsForR7(AncillaryPricingTrx& ancTrx,
                               XMLConstruct& construct,
                               const GroupFeesUsagesVector& groupFeesUsagesVector,
                               const bool allFeesDisplayOnly,
                               bool timeOutMax);

  virtual void formatOCFeesLineForR7(AncillaryPricingTrx& ancTrx,
                                     XMLConstruct& construct,
                                     const PaxOCFeesUsages& paxOcFees,
                                     const uint16_t index,
                                     const char& indicator);

  void buildOSCData(AncillaryPricingTrx& ancTrx,
                    XMLConstruct& construct,
                    const PaxOCFeesUsages& paxOcFeesUsages,
                    const uint16_t index = 0);

  void buildOSCOptionalData(AncillaryPricingTrx& ancTrx,
                            XMLConstruct& construct,
                            const PaxOCFeesUsages& paxOcFees,
                            const uint16_t index = 0);
  void buildS7OOSData(AncillaryPricingTrx& ancTrx,
                      XMLConstruct& construct,
                      const PaxOCFeesUsages& paxOcFees,
                      const uint16_t index = 0);

  virtual void buildSegmentsTickeCoupons(AncillaryPricingTrx& ancTrx,
                                         XMLConstruct& construct,
                                         const PaxOCFeesUsages& paxOcFees);

  virtual std::string getPaxType(const PaxOCFeesUsages& paxOcFees);
  void buildOSCCommonData(XMLConstruct& construct, const OCFeesUsage& ocFee);

  // End

  void formatOCFeesGroups(AncillaryPricingTrx& ancTrx,
                          XMLConstruct& construct,
                          const GroupFeesVector& groupFeesVector,
                          bool timeOutMax);

  void logGroupFeesVector(const GroupFeesVector& groupFeesVector);

  virtual bool shouldSkipCreatingOcgElement(AncillaryPricingTrx& ancTrx,
                                            const std::vector<PaxOCFees>& paxOcFeesVector) const;

  void formatOCFeesGroupsForR7(AncillaryPricingTrx& ancTrx,
                               XMLConstruct& construct,
                               const GroupFeesVector& groupFeesVector,
                               const bool allFeesDisplayOnly,
                               bool timeOutMax);

  virtual void formatOCFeesLineForR7(AncillaryPricingTrx& ancTrx,
                                     XMLConstruct& construct,
                                     const PaxOCFees& paxOcFees,
                                     const uint16_t index,
                                     const char& indicator);

  bool isOCSData(const PaxOCFees& paxOCFees) const;

  bool isOCSData(const std::vector<PaxOCFees>& paxOCFeesVector) const;

  int getOCSNumber(const std::vector<PaxOCFees>& paxOCFeesVector) const;

  void buildOSCData(AncillaryPricingTrx& ancTrx,
                    XMLConstruct& construct,
                    const PaxOCFees& paxOcFees,
                    const uint16_t index = 0);

  virtual bool shouldSkipCreatingOscElement(AncillaryPricingTrx& ancTrx, const PaxOCFees& paxOcFees) const;

  virtual void buildOscContent(AncillaryPricingTrx& ancTrx,
                               XMLConstruct& construct,
                               const PaxOCFees& paxOcFees,
                               const uint16_t index);

  std::map<AncillaryIdentifier, std::vector<OCFeesUsage*>>
  getAncillaryIdentifierToOcFeesUsageMap(const PaxOCFees& paxOcFees);

  unsigned getOcFeesUsageCountInOsc(const PaxOCFees& paxOcFees) const;

  void buildOccData(AncillaryPricingTrx& ancTrx,
                    XMLConstruct& construct,
                    const PaxOCFees& paxOcFees,
                    const uint16_t index,
                    std::map<AncillaryIdentifier, std::vector<OCFeesUsage*>>::value_type ocFeesUsageGroup);

  bool buildOosElement(AncillaryPricingTrx& ancTrx,
                       XMLConstruct& construct,
                       const PaxOCFees& paxOcFees,
                       const uint16_t index,
                       OCFeesUsage& ocFeeUsage);

  void buildOSCOptionalData(AncillaryPricingTrx& ancTrx,
                            XMLConstruct& construct,
                            const PaxOCFees& paxOcFees,
                            const uint16_t index = 0);

  void buildS7OOSData(AncillaryPricingTrx& ancTrx,
                      XMLConstruct& construct,
                      const PaxOCFees& paxOcFees,
                      OCFeesUsage& ocFees,
                      const uint16_t index = 0);

  void
  buildSegments(AncillaryPricingTrx& ancTrx, XMLConstruct& construct, const OCFeesUsage& ocFees);
  virtual void buildSegmentsTickeCoupons(AncillaryPricingTrx& ancTrx,
                                         XMLConstruct& construct,
                                         const PaxOCFees& paxOcFees);

  void addDisclosureMarkers(AncillaryPricingTrx& ancTrx,
                           XMLConstruct& construct,
                           const OCFeesUsage& ocFees);

  void buildSegmentElement(XMLConstruct& construct, const int16_t& pnrSegment);

  void buildQBT(XMLConstruct& construct, const int16_t& pnrSegment);

  void buildS7OOSOptionalData(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void
  buildPadisData(AncillaryPricingTrx& ancTrx, XMLConstruct& construct, const OCFeesUsage& ocFees);

  void buildSUMData(AncillaryPricingTrx& ancTrx,
                    XMLConstruct& construct,
                    const PaxOCFees& paxOcFees,
                    OCFeesUsage& ocFees);

  void buildSUMData(AncillaryPricingTrx& ancTrx,
                    XMLConstruct& construct,
                    const PaxOCFeesUsages& paxOcFees);

  void insertPriceDataToSum(AncillaryPricingTrx& ancTrx,
                            XMLConstruct& construct,
                            const PaxOCFees& paxOcFees,
                            OCFeesUsage& ocFees);

  void insertPriceDataToSum(AncillaryPricingTrx& ancTrx,
                            XMLConstruct& construct,
                            const PaxOCFeesUsages& paxOcFees);

  void buildSUMData_old_implementation(AncillaryPricingTrx& ancTrx,
                                       XMLConstruct& construct,
                                       const PaxOCFees& paxOcFees,
                                       OCFeesUsage& ocFees);

  void buildSUMData_old_implementation(AncillaryPricingTrx& ancTrx,
                                       XMLConstruct& construct,
                                       const PaxOCFeesUsages& paxOcFees);

  void buildSFQData(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void buildSFQDataR7(XMLConstruct& construct, const OCFeesUsage& ocFees, const uint16_t index = 0);

  void buildSTOData(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void buildDTEData(AncillaryPricingTrx& ancTrx,
                    XMLConstruct& construct,
                    const OCFeesUsage& ocFees,
                    const uint16_t index = 0);

  void buildFATData(AncillaryPricingTrx& ancTrx,
                    XMLConstruct& construct,
                    const OCFeesUsage& ocFees,
                    const uint16_t index = 0);

  void buildRBDData(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void buildBKCData(XMLConstruct& construct, const BookingCode& bkCode);

  void buildFCLData(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void buildCFTData(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void buildBGD(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void
  buildBGAData(AncillaryPricingTrx& ancTrx, XMLConstruct& construct, const OCFeesUsage& ocFees);

  void buildITRData(AncillaryPricingTrx& ancTrx,
                    XMLConstruct& construct,
                    const OCFees::BaggageItemProperty& baggageItemProperty);

  void buildOSCCommonData(XMLConstruct& construct, const SubCodeInfo* subCodeInfo);

  void buildITTData(AncillaryPricingTrx& ancTrx,
                    XMLConstruct& construct,
                    const SubCodeInfo* subCodeInfo);

  void buildOSCCommonOptionalData(AncillaryPricingTrx& ancTrx,
                                  XMLConstruct& construct,
                                  const SubCodeInfo* subCodeInfo,
                                  bool addSchema2Elements = true);

  void buildOSCEmdSoftPassIndicator(const AncillaryPricingTrx& ancTrx, XMLConstruct& construct, const OCFees* ocfees) const;

  void buildBIRData(AncillaryPricingTrx&, XMLConstruct&, const SubCodeInfo*);

  void buildBIRData(XMLConstruct& construct, const BaggageSizeWeightDescription&);

  void buildBIRData(XMLConstruct&, const ServiceGroupDescription&);

  void buildSDCData(AncillaryPricingTrx&, XMLConstruct&, const SubCodeInfo*);

  void buildMSGData(AncillaryPricingTrx&, XMLConstruct&, const SubCodeInfo*);

  void processTicketEffDate(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void processTicketEffDateForR7(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void processTicketDiscDate(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void processTicketDiscDateForR7(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void processPurchaseByDate(XMLConstruct& construct, const OCFeesUsage& ocFees);

  void processPurchaseByDateWPDispAE(AncillaryPricingTrx& ancTrx,
                                     XMLConstruct& construct,
                                     const OCFeesUsage& ocFees);
  virtual void builPNMData(XMLConstruct& construct);
  virtual std::string getPaxType(const PaxOCFees& paxOcFees);
  virtual void buildGroupHeader(const ServiceFeesGroup* sfg, XMLConstruct& construct);
  virtual bool ancillariesNonGuarantee() { return _ancillariesNonGuarantee; }

  virtual bool preformattedMsgEmpty() { return true; }

  bool samePaxType(AncillaryPricingTrx& ancTrx);

  void emptyOCFeeGroups(XMLConstruct& construct);

  bool checkFFStatus(const OCFeesUsage& ocFees);

  bool isOriginalOrderNeeded(const ServiceFeesGroup* sfg, const AncillaryPricingTrx& ancTrx) const;
  bool isApplyAMTaxLogic(AncillaryPricingTrx& ancTrx,
                         const OCFeesUsage& ocFees) const;
  void getAmount(bool isC52avaliable,
                 MoneyAmount basePrice,
                 MoneyAmount equivalentBasePrice,
                 CurrencyNoDec currencyNoDecForC51,
                 CurrencyNoDec currencyNoDecForC52,
                 MoneyAmount& actualBasePrice,
                 uint16_t& numberOfDec);

  void applyAMTaxLogic(AncillaryPricingTrx& ancTrx,
                       OCFeesUsage& ocFees,
                       bool isC52avaliable,
                       MoneyAmount basePrice,
                       MoneyAmount equivalentBasePrice,
                       CurrencyNoDec currencyNoDecForC51,
                       CurrencyNoDec currencyNoDecForC52,
                       const CurrencyCode& currency);

protected:
  bool maxResponseReached() { return _curOCInResponse >= _maxOCInResponse; }
  Itin*& currentItin() { return _itin; }
  const Itin* currentItin() const { return _itin; }

  static const std::string readConfigXMLNamespace(const std::string& configName);

  void buildErrorAndDiagnosticElements(PricingTrx& trx,
                                       XMLConstruct& construct,
                                       const std::string& errorString,
                                       ErrorResponseException::ErrorResponseCode errorCode);

  void prepareHostPortInfo(PricingTrx& ancTrx, XMLConstruct& construct);

  bool allowanceServiceType(const SubCodeInfo* codeInfo) const;
  Indicator convertWeightUnit(const Indicator& unit) const;

  void buildUpc(AncillaryPricingTrx& ancTrx,
                XMLConstruct& construct,
                const SvcFeesResBkgDesigInfo& padis,
                const OCFeesUsage& ocFeesUsage);

  void buildPds(XMLConstruct& construct,
                const BookingCode& padisCode,
                std::map<BookingCode, std::string>& padisCodeAbbreviationMap,
                std::map<BookingCode, std::string>& padisCodeDescriptionMap);

  std::string getPadisCodeString(const BookingCode& padisCode,
                                 std::map<BookingCode, std::string>& padisCodeMap);

  bool isC52ActiveForAB240(AncillaryPricingTrx& ancTrx) const;

  static log4cxx::LoggerPtr _logger;
  Itin* _itin = nullptr;
  bool _doNotProcess = false;
  int32_t _maxOCInResponse = 0;
  const std::string XML_NAMESPACE_TEXT = readConfigXMLNamespace("ANC_PRC_XML_NAMESPACE");
  int32_t _curOCInResponse = 0;
  bool _isWpAePostTicket = false;
  bool _isWPDispAE = false;
  int32_t _maxTotalBuffSizeForWPDispAE = 0;
  bool _ancillariesNonGuarantee = false;
  bool _isHeader = true;
  uint16_t _currentIndex = 1;
  uint16_t _currentIndexForMT_AE = 1;
  bool _maxSizeAllowed = false;
  NationCode _nation;

  // OCF xml construct for derived
  XMLConstruct _ocfXmlConstruct{0};
};

} // namespace tse

