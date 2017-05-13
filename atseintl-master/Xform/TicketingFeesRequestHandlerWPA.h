#pragma once

#include "Xform/TicketingFeesRequestHandler.h"

namespace tse
{
class ArunkSeg;

class TicketingFeesRequestHandlerWPA : public TicketingFeesRequestHandler
{
public:
  TicketingFeesRequestHandlerWPA(Trx*& trx)
    : TicketingFeesRequestHandler(trx)
  {
  }

  bool startElement(int idx, const IAttributes& attrs) override;
  bool endElement(int idx) override;

  void parse(DataHandle& dataHandle, const std::string& content) override;

protected:
  void createTransaction(DataHandle& dataHandle, const std::string& content) override;

  void onStartCAL(const IAttributes& attrs);
  void onStartREQ(const IAttributes& attrs);
  void onStartDIG(const IAttributes& attrs);
  void onStartPXI(const IAttributes& attrs) override;
  void onStartSEG(const IAttributes& attrs);
  void onStartERD(const IAttributes& attrs);
  void onStartITN(const IAttributes& attrs) override;
  void onStartOBF(const IAttributes& attrs);
  void onStartDCX(const IAttributes& attrs);
  void onStartSUM(const IAttributes& attrs);

  void onEndPXI();
  void onEndITN() override;
  void onEndSEG();

  void checkFBIData(const IAttributes& attrs) const override;
  void checkFLIData(const IAttributes& attrs) const override;

private:
  typedef TktFeesRequest::TktFeesFareBreakInfo Fbi;
  typedef TktFeesRequest::TktFeesFareBreakAssociation Fba;

  void addPaxTypePayment();
  void addFopBin(const std::string& bin);
  void setMonayAmountAttr(const IAttributes& attrs, const int idx, MoneyAmount& monayAmount);
  void setIntAttr(const IAttributes& attrs, const int idx, uint16_t& val);
  void setCharAttr(const IAttributes& attrs, const int idx, char& val);
  void initTicketDate(const IAttributes& sumAttrs);

  void parseSegmentOrgDst(const IAttributes& attrs, TravelSeg& ts);
  void parseCarriersAndStatus(const IAttributes& attrs, AirSeg& as);
  void determineStopConx(const IAttributes& seg, TravelSeg& ts);
  void updateFareData(const IAttributes& seg, TravelSeg& ts);
  void parseArunk(const IAttributes& attrs);
  void doItinPostProcessing();

  void parseAccCorpId(const IAttributes& attrs, const int idx, std::vector<std::string>& out);
  void propagateAccCorpIdToAllItins();
  void propagateAllOpenSegToAllItins();
  void propageteUnconfirmedSeg();
  void parseFormsOfPayment(const IAttributes& attrs);

  ArunkSeg* _arunk = nullptr;
  bool _isTicketingDtInitialized = false;
  std::string _chargeAmount;
  bool _isInSum = false;

  Fbi* _fbiWithSideTrip = nullptr;
  SequenceNumber _sideTripId = 0;
  bool _isInSideTrip = false;
  bool _forbidCreditCard = false;
};

} // tse

