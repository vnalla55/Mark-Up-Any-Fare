//-------------------------------------------------------------------
//  Copyright Sabre 2014
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//-------------------------------------------------------------------
#pragma once

#include "Xform/CommonRequestHandler.h"

namespace tse
{
class TicketingCxrRequest;
class STLTicketingCxrRequestHandler : protected CommonRequestHandler
{
public:
  friend class STLTicketingCxrRequestHandlerTest;
  STLTicketingCxrRequestHandler(Trx*& trx);

  virtual ~STLTicketingCxrRequestHandler() = default;

  virtual bool startElement(int idx, const IAttributes& attrs) override;
  virtual bool endElement(int idx) override;
  void parse(DataHandle& dataHandle, const std::string& content) override;

private:
  virtual void createTransaction(DataHandle& dataHandle, const std::string& content) override;

  void onStartValidatingCxrCheck(const IAttributes& attrs);
  void onStartSettlementPlanCheck(const IAttributes& attrs);
  void onStartPOS(const IAttributes& attrs);
  void onStartActual(const IAttributes& attrs);
  void onStartHome(const IAttributes& attrs);
  void onStartSettlementPlan(const IAttributes& attrs);
  void onStartValidatingCxr(const IAttributes& attrs);
  void onStartParticipatingCxr(const IAttributes& attrs);
  void onStartTicketType(const IAttributes& attrs);
  void onStartTicketDate(const IAttributes& attrs);
  void onStartRequestedDiagnostic(const IAttributes& attrs);

  void onEndValidatingCxrCheck();
  void onEndSettlementPlanCheck();
  void onEndPOS();
  void onEndActual();
  void onEndHome();
  void onEndPcc();
  void onEndSettlementPlan();
  void onEndValidatingCxr();
  void onEndParticipatingCxr();
  void onEndTicketType();
  void onEndTicketDate();
  void onEndRequestedDiagnostic();

  TicketingCxrRequest* _tcsReq = nullptr;
};
}

