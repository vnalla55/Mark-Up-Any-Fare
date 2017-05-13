#pragma once

#include "DataModel/TktFeesRequest.h"
#include "Xform/CommonRequestHandler.h"

namespace tse
{
class TicketingFeesRequestHandler : protected CommonRequestHandler
{
public:
  TicketingFeesRequestHandler(Trx*& trx);
  virtual ~TicketingFeesRequestHandler();

  virtual bool startElement(int idx, const IAttributes& attrs) override;
  virtual bool endElement(int idx) override;

  virtual void parse(DataHandle& dataHandle, const std::string& content) override;

protected:
  virtual void createTransaction(DataHandle& dataHandle, const std::string& content) override;
  void setTransactionParameters();
  void createTicketingAgent();

  void onStartPRO(const IAttributes& attrs);
  virtual void onStartFLI(const IAttributes& attrs);
  void onStartTicketingFeesRequest(const IAttributes& attrs);
  virtual void onStartSGI(const IAttributes& attrs);
  void onStartIRO(const IAttributes& attrs);
  virtual void onStartPXI(const IAttributes& attrs);
  virtual void onStartFBA(const IAttributes& attrs);
  virtual void onStartFBI(const IAttributes& attrs);
  void onStartACI(const IAttributes& attrs);
  void onStartCII(const IAttributes& attrs);
  void onStartBIN(const IAttributes& attrs);
  void onStartDIF(const IAttributes& attrs);
  void onEndBIL();
  virtual void onEndITN();
  void onEndPRO();
  void onEndTicketingFeesRequest();
  void onEndSGI();
  void onEndPXI();
  void onEndIRO();
  void onEndBIN();
  void onEndDIF();

  void processOnEndITN();
  virtual void setCarrierAndSegmentStatusFLI(const IAttributes& attrs);
  virtual void setSequenceNumberAndSideTripIdFBA(const IAttributes& attrs,
                                                 TktFeesRequest::TktFeesFareBreakAssociation& fba);
  void checkRequiredDataFbaFbi() const;
  virtual void checkFBAData(const IAttributes& attrs) const;
  virtual void checkFLIData(const IAttributes& attrs) const;
  virtual void checkFBIData(const IAttributes& attrs) const;
  void checkIROData(const IAttributes& attrs) const;
  void checkSideTrip(Itin* itin) const;

  void initPaxType(const PaxTypeCode& ptc, uint16_t number);
  void addPaxType();
  void addAccountCode(const std::string& accountCode);
  void addCorpId(const std::string& corpID);
  void addTktDesignator(const std::string tktDesig, const std::vector<int>& segIds);
  void addFopBin(const FopBinNumber& bin);
  void addTotalAmount(const TktFeesRequest::PaxTypePayment* bin);

  void addArunk();
  void fillArunkData();

  TktFeesRequest::TktFeesFareBreakInfo* _tktFbi;
  TktFeesRequest* _tktFeesRequest;
};

} // tse

