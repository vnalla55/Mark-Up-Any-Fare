#pragma once

#include "DataModel/BaggageTrx.h"
#include "Xform/CommonRequestHandler.h"
#include "Xform/CustomXMLParser/IBaseHandler.h"

namespace tse
{
class BaggageRequestHandler : private CommonRequestHandler
{
  friend class BaggageRequestHandlerTest;
public:
  BaggageRequestHandler(Trx*& trx) : CommonRequestHandler(trx) {}

  virtual bool startElement(int idx, const IAttributes& attrs) override;
  virtual bool endElement(int idx) override;

  void parse(DataHandle& dataHandle, const std::string& content) override;

private:
  std::vector<PaxType::FreqFlyerTierWithCarrier*> _ffd;

  virtual void createTransaction(DataHandle& dataHandle, const std::string& content) override;
  virtual bool serviceSpecyficRtwProperty() const override;

  void onStartPRO(const IAttributes& attrs);
  void onStartFLI(const IAttributes& attrs);
  void onStartFFY(const IAttributes& attrs);
  void onStartHSP(const IAttributes& attrs);
  void onStartEQP(const IAttributes& attrs);
  void onStartBaggageRequest(const IAttributes& attrs);
  void onEndBaggageRequest();
  void onEndBIL();
  void onEndITN();
};
}

