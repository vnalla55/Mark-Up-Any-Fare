#pragma once

#include "DataModel/TktFeesRequest.h"
#include "Xform/CommonRequestHandler.h"

namespace tse
{

class STLTicketingFeesRequestHandler : protected CommonRequestHandler
{
  friend class STLTicketingFeesRequestHandlerTest;

public:
  STLTicketingFeesRequestHandler(Trx*& trx);
  virtual ~STLTicketingFeesRequestHandler();

  virtual bool startElement(int idx, const IAttributes& attrs) override;
  virtual bool endElement(int idx) override;

  void parse(DataHandle& dataHandle, const std::string& content) override;

private:
  virtual void createTransaction(DataHandle& dataHandle, const std::string& content) override;
  void createTicketingAgent();

  void onStartAgent(const IAttributes& attrs);
  void onStartBillingInformation(const IAttributes& attrs);
  void onStartRequestOptions(const IAttributes& attrs);
  void onStartRequestedDiagnostic(const IAttributes& attrs);
  void onStartTravelSegment(const IAttributes& attrs);
  void onStartFlight(const IAttributes& attrs);
  void onStartOBTicketingFeeRQ(const IAttributes& attrs);
  void onStartPricedSolution(const IAttributes& attrs);
  void onStartFareBreakAssociation(const IAttributes& attrs);
  void onStartFareBreakInformation(const IAttributes& attrs);
  void onStartAccountCode(const IAttributes& attrs);
  void onStartCorpID(const IAttributes& attrs);
  void onStartFormOfPayment(const IAttributes& attrs);
  void onStartDifferentialHighClass(const IAttributes& attrs);
  void onStartArunk(const IAttributes& attrs);
  void onStartPassengerIdentity(const IAttributes& attrs);
  void onStartPassengerPaymentInfo(const IAttributes& attrs);
  void onStartDiagnostic(const IAttributes& attrs);
  void onStartOption(const IAttributes& attrs);

  void onEndAgent();
  void onEndBillingInformation();
  void onEndRequestOptions();
  void onEndOBTicketingFeeRQ();
  void onEndTravelSegment();
  void onEndPricedSolution();
  void onEndFormOfPayment();
  void onEndDifferentialHighClass();
  void onEndOption();
  void onEndRequestedDiagnostic();
  void onEndFareBreakInformation();
  void onEndAccountCode();
  void onEndCorpID();
  void onEndFlight();
  void onEndFareBreakAssociation();
  void onEndArunk();
  void onEndPassengerIdentity();
  void onEndPassengerPaymentInfo();
  void onEndDiagnostic();

  void checkPassengerPaymentInfoData(const IAttributes& attrs) const;
  void checkPassengerIdentityData(const IAttributes& attrs) const;
  void checkRequiredDataFbaFbi() const;
  void checkFareBreakAssociationData(const IAttributes& attrs) const;
  void checkFlightData(const IAttributes& attrs) const;
  void checkFareBreakInformationData(const IAttributes& attrs) const;
  void checkSideTrip(Itin* itin) const;
  void addTravelSegData() const;
  void resetTravelSegData();

  void addPaxType();
  void addAccountCode(const std::string& accountCode);
  void addCorpId(const std::string& corpID);
  void addTktDesignator();
  void addFopBin(TktFeesRequest::FormOfPayment* fop);
  void addTotalAmount(TktFeesRequest::PaxTypePayment* bin);
  void populateCurrentItin();
  void fillMissingSegDataInCurrentItin();
  virtual bool serviceSpecyficRtwProperty() const override;
  void setSegmentType(const char& segType);

  TktFeesRequest* _tktFeesRequest;
  std::vector<TravelSeg*> _tSeg;
  TravelSeg* _currentTvlSegment;
  TravelSeg* _prevTvlSegment;

  int16_t _pnrSegment;
  std::string _indValueS;
  std::string _segType;
};
} // tse

