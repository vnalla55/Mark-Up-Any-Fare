#pragma once

#include "Xform/CommonRequestHandler.h"

namespace tse
{
class TicketingCxrDisplayRequest;

class STLTicketingCxrDisplayRequestHandler : protected CommonRequestHandler
{
public:
  friend class STLTicketingCxrDispReqHandlerTest;
  STLTicketingCxrDisplayRequestHandler(Trx*& trx);
  virtual ~STLTicketingCxrDisplayRequestHandler() = default;

  virtual bool startElement(int idx, const IAttributes& attrs) override;
  virtual bool endElement(int idx) override;
  void parse(DataHandle& dataHandle, const std::string& content) override;

private:
  virtual void createTransaction(DataHandle& dataHandle, const std::string& content) override;
  void onStartDisplayValidatingCxr(const IAttributes& attrs);
  void onStartDisplayInterlineAgreement(const IAttributes& attrs);
  void onEndDisplayValidatingCxr();
  void onEndDisplayInterlineAgreement();
  void onStartPOS(const IAttributes& attrs);
  void onEndPOS();
  void onEndPcc();
  void onStartActual(const IAttributes& attrs);
  void onEndActual();
  void onStartRequestedDiagnostic(const IAttributes& attrs);
  void onEndRequestedDiagnostic();

  TicketingCxrDisplayRequest* _tcdReq = nullptr;
};
}
