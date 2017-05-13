#pragma once

#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/CustomXMLParser/IBaseHandler.h"

#include <vector>

#include <tr1/array>

namespace tse
{
class AirSeg;
class Itin;
class PaxType;
class PricingTrx;

class CommonRequestHandler : public IBaseHandler
{
  friend class CommonRequestHandlerTest;

protected:
  enum DateTimeInRequest
  { FIRST_MATCH,
    DATE_IN_FRIST_SEGMENT,
    DATE_PRESENT,
    DATE_NOT_PRESENT };

  enum RequestProcessLogic
  { M70,
    WPAE,
    PostTkt,
    WPBG,
    PostTktWPBGWPAE,
    PreTktWPBGWPAE };

  enum FareCabinChars
  { PREMIUM_FIRST_CLASS = 'R',
    FIRST_CLASS = 'F',
    PREMIUM_BUSINESS_CLASS = 'J',
    BUSINESS_CLASS = 'C',
    PREMIUM_ECONOMY_CLASS = 'P',
    ECONOMY_CLASS = 'Y',
    UNDEFINED_CLASS = ' ' };

public:
  CommonRequestHandler(Trx*& trx);
  virtual ~CommonRequestHandler() = default;

protected:
  virtual void createTransaction(DataHandle& dataHandle, const std::string& content) = 0;
  virtual void parse(DataHandle& dataHandle, const std::string& content) = 0;
  void parse(DataHandle& dataHandle, const std::string& content, IBaseHandler& handler);
  virtual bool startElement(const IKeyString&, const IAttributes&) override;
  virtual bool endElement(const IKeyString&) override;
  virtual void characters(const char* value, size_t length) override;

  void onStartAGI(const IAttributes& attrs);
  virtual void onStartBIL(const IAttributes& attrs);
  void onStartValCxrBIL(const IAttributes& attrs, Billing& billing);
  void onStartDIG(const IAttributes& attrs);
  virtual void onStartITN(const IAttributes& attrs);
  void onStartIRO(const IAttributes& attrs);
  void onStartSGI(const IAttributes& attrs);
  void onStartFBI(const IAttributes& attrs);
  void onStartPXI(const IAttributes& attrs);
  void onStartPNM(const IAttributes& attrs);
  void onStartACI(const IAttributes& attrs);
  void onStartCII(const IAttributes& attrs);
  void onStartFBA(const IAttributes& attrs);
  void onStartDynamicConfig(const IAttributes& attrs);

  void onEndAGI();
  void onEndFFY();
  void onEndDIG();
  void onEndIRO();
  void onEndSGI();
  void onEndFBI();
  void onEndPNM();
  void onEndPXI();
  void onEndACI();
  void onEndCII();
  void onEndFLI();
  void onEndFBA();
  void onEndValCxrBIL(Billing& billing, Billing::Service serviceId, uint64_t transactionId);

  void createITN(const IAttributes& attrs);

  template <size_t n>
  void getAttr(const IAttributes& attrs, int idx, Code<n>& str) const;
  void getAttr(const IAttributes& attrs, int idx, std::string& str) const;
  void getAttr(const IAttributes& attrs, int idx, BoostString& str) const;
  template <typename C, size_t n>
  void inline getAttr(const IAttributes& attrs, int idx, std::tr1::array<C, n>& strBuf) const
  {
    const IValueString& attr = attrs.get(idx);
    const size_t charsToCopy = std::min(attr.length(), n - 1);
    std::memcpy(strBuf.data(), attr.c_str(), charsToCopy);
    strBuf[charsToCopy] = '\0';
  }
  template <size_t n, typename T>
  void getValue(Code<n, T>& str) const
  {
    if (!_value.empty())
      str.assign(_value.c_str(), _value.length());
  }
  void getValue(std::string& str) const;

  uint16_t& maxItinNo() { return _maxITNNo; }
  uint16_t maxItinNo() const { return _maxITNNo; }

  DateTime convertDate(const std::string& date) const;
  void checkForDateConsistency(const std::string& depatureDate);
  void checkForTimeConsistency(const std::string& departureTime, const std::string& arriveTime);
  void addPaxType();
  short getTimeDiff();
  void setDate(const DateTime& date);
  void setTime(DateTime& date, std::string time);
  std::string pssTime(std::string time);
  void setCabin(Indicator cabin);
  virtual const Cabin* getCabin();
  void checkSideTrip(Itin* itin);
  void checkCurrency(const CurrencyCode& cur);
  void checkData(const IAttributes& attrs,
                 const std::vector<int>& requredAttrNames,
                 const char* attributesNames,
                 const std::string& msg) const;
  void checkFBADataForWPAE(const IAttributes& attrs);
  void checkFBIDataForWPAE(const IAttributes& attrs);
  void parseOriginalPricingCommandPostTkt();
  std::string cleanOriginalPricingCommandPostTkt();

  void addAccountCode(const std::string& accountCode);
  void addCorpId(const std::string& corpID);
  void addTktDesignator(const std::string tktDesig, const std::vector<int>& segIds);
  void addAttributeToHashMap(AncRequest::AncAttrMapTree& map,
                             int name,
                             const IAttributes& attt,
                             int hashIndex);
  void mergeSameFBAAndFBI(Itin* it);
  void parseOriginalPricingCommandWPAE();
  void addFrequentFlyerStatus(PaxType::FreqFlyerTierWithCarrier* ffd, const PaxTypeCode& ptc);
  void setFrequentFlyerStatus();
  void setSegmentDates(const std::string& departureDT,
                       const std::string& depTime,
                       const std::string& arrivalDT,
                       const std::string& arrTime);
  void adjustOpenSegmentDates();
  void checkFlight(const Itin& itin, bool checkSequence, bool checkHistorical) const;
  void detectRtw();
  virtual bool serviceSpecyficRtwProperty() const { return false; }
  bool isWPAEReqType() const;
  bool isPostTktReqType() const;
  bool isWPBGRequest() const;
  void readAgentAttrs(const IAttributes& attrs, Agent* agent);
  void finalizeAgent(Agent* agent);
  bool isNewRBDforAncillaryActive() const;
  bool isBaggageTransaction() const;
  bool isBaggageRequest() const;
  bool isNewRBDforWPAEActive() const;
  bool isNewRBDforAB240Active() const;

  IValueString _value;
  Trx*& _trx;
  PricingTrx* _pricingTrx;
  AncRequest* _request;
  std::multimap<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*> _ffData;
  std::vector<PaxType::FreqFlyerTierWithCarrier*> _ffDataForItin; // AncillaryPricingRequest v3
  PaxType* _paxType;
  uint16_t _paxInputOrder;
  Itin* _itin;
  AirSeg* _currentTvlSeg;
  AirSeg* _prevTvlSeg;
  DateTime _curDate;
  uint16_t _segmentOrder;
  DateTimeInRequest _datesInRequest;
  DateTimeInRequest _timesInRequest;
  uint16_t _itinOrderNum;
  uint16_t _itinGroupNum;
  std::string _actionCode;
  RequestProcessLogic _reqType;
  bool _acsBaggageRequest;
  bool _MISC6BaggageRequest;
  std::string _originalPricingCommand;
  uint16_t _maxITNNo;
  std::map<const Itin*, std::vector<const Itin*> > _mergedPaxItins;
  bool _ignoreBE0;
  bool _unFlownSegMatch;
  int _tktRefNumber;
  std::map<const Itin*, std::vector<int> > _removedFBA;
  DateTime _itinTicketIssueDate;
};

template <size_t n>
void
CommonRequestHandler::getAttr(const IAttributes& attrs, int idx, Code<n>& str) const
{
  if (attrs.has(idx))
  {
    const IValueString& attr = attrs.get(idx);
    str.assign(attr.c_str(), attr.length());
  }
}

} // tse

