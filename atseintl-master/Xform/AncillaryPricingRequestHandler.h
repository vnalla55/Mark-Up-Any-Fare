#pragma once

#include "Xform/CommonRequestHandler.h"

namespace tse
{

class AncillaryPricingRequestHandler : protected CommonRequestHandler
{
  friend class AncillaryPricingRequestHandlerTest;

public:
  AncillaryPricingRequestHandler(Trx*& trx);
  virtual ~AncillaryPricingRequestHandler();

  virtual bool startElement(int idx, const IAttributes& attrs) override;
  virtual bool endElement(int idx) override;

  void parse(DataHandle& dataHandle, const std::string& content) override;

protected:
  virtual void createTransaction(DataHandle& dataHandle, const std::string& content) override;
  void createTicketingAgent(uint8_t ticketNumber = 0);

  virtual void onStartBIL(const IAttributes& attrs) override;
  void onStartPRO(const IAttributes& attrs);
  virtual void onStartITN(const IAttributes& attrs) override;
  void onStartFLI(const IAttributes& attrs);
  void onStartBTS(const IAttributes& attrs);
  void onStartFFY(const IAttributes& attrs);
  void onStartAncillaryPricingRequest(const IAttributes& attrs);
  void onStartRFG(const IAttributes& attrs);
  void onStartAST(const IAttributes& attrs);
  void onStartTAG(const IAttributes& attrs);
  void onStartIRO(const IAttributes& attrs);
  void onStartOSC(const IAttributes& attrs);
  void onEndBIL();
  void onEndRFG();
  void onEndITN();
  void onEndPRO();
  void onEndAncillaryPricingRequest();
  void onEndTAG();
  void onEndS01();

  void checkCurrencyAndSaleLoc();
  void checkFBAandFBIdata() const;
  void checkFLIDataForWPAE(const IAttributes& attrs) const;
  void checkFlights(const Itin* pnrItin, const Itin* itin) const;
  virtual bool serviceSpecyficRtwProperty() const override;
  void defineOcFeeGroup();
  void insertDisclosure();
  void insertCatalog();
  void insertAncillary();
  void insertDefaultOCFeeGroups();
  void insertRequestedOcFeeGroup(RequestedOcFeeGroup::RequestedInformation ri,
                                const ServiceGroup& groupCode, const Indicator& serviceType);
  void insertRequestedOcFeeGroups(RequestedOcFeeGroup::RequestedInformation ri,
                                const ServiceGroup& groupCode, const Indicator* serviceTypes);
  void insertRequestedOcFeeGroups(RequestedOcFeeGroup::RequestedInformation ri,
                                  const std::vector<OptionalServicesActivationInfo*>& activeGroups,
                                  const Indicator* serviceTypes);
  void insertRequestedOcFeeGroup(RequestedOcFeeGroup::RequestedInformation ri, const ServiceGroup& groupCode);
  RequestedOcFeeGroup& findOrCreateRequestedOcFeeGroup(RequestedOcFeeGroup::RequestedInformation ri, const ServiceGroup& groupCode);

  void parseResOrCheckinPath(const IAttributes& attrs);

  void setNewRBDforM70ActivationFlag();
  UserApplCode getUserApplCode();

  AncillaryIdentifier getAidAttributeValueFromOsc(const IAttributes& attrs);
  std::string  getPmiAttributeValueFromOsc(const IAttributes& attrs);
  unsigned int getQtyAttributeValueFromOsc(const IAttributes& attrs);
  unsigned int getDrpAttributeValueFromOsc(const IAttributes& attrs, bool clampTo100Percent = false);
  MoneyAmount getDrmAttributeValueFromOsc(const IAttributes& attrs);

  void checkValueWithRegex(const std::string& path, const std::string& name, const std::string& value, const std::string& regex);
  unsigned int convertAttributeValueToUInt(const std::string& path, const std::string& name, const std::string& value);
  MoneyAmount convertAttributeValueToMoneyAmount(const std::string& path, const std::string& name, const std::string& value);
  MoneyAmount convertValueToMoneyAmount(const std::string& path, const std::string& name, const std::string& value);

  RequestedOcFeeGroup _currentRequestedOcFeeGroup;
  std::vector<ServiceGroup> _groupCodes;
  std::set<std::string> _priceModificationIdentifiers;
  bool _processAllGroupsForAncillary;
  bool _isRfgElementOpen;
  bool _isItnElementOpen;
  bool _isPnmFound;
  bool _isOCFeeGroupAdded;

private:
  static const std::string RCP_CHECKIN_PATH;
  static const std::string RCP_RESERVATION_PATH;

};
} // tse

