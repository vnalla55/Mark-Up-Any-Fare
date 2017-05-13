//-----------------------------------------------------------------------------
//
//  File:     Diag291CollectorFD.h
//
//  Author :  Slawek Machowicz
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FareDisplayOptions;
class FareDisplayRequest;

// Diag291Collector is the diagnostic for FareSelectorRD
class Diag291CollectorFD : public DiagCollector
{
public:
  enum D291FailCode
  {
    // general fields
    FAIL_RD_FARECLASS = 0,
    FAIL_RD_LINK,
    FAIL_RD_SEQUENCE,
    FAIL_RD_DATETIME,
    FAIL_RD_FAREBASIS,
    FAIL_RD_FAREAMOUNT,
    FAIL_RD_FARETYPE,
    FAIL_RD_DISCOUNTED,
    FAIL_RD_FBRULE,
    FAIL_RD_NEGOTIATED,
    FAIL_RD_SOURCE_PCC,
    FAIL_RD_CONSTRUCTED,
    FAIL_RD_CAT35,
    FAIL_RD_CARRIER,
    FAIL_RD_CURRENCY,
    FAIL_RD_VENDOR,
    FAIL_RD_TARIFF,
    FAIL_RD_RULENUMBER,
    FAIL_RD_BOOKINGCODE,
    FAIL_RD_ORIGIN,
    FAIL_RD_DESTINATION,
    // construction fields
    FAIL_RD_GATEWAY1,
    FAIL_RD_GATEWAY2,
    FAIL_RD_ORIGAO,
    FAIL_RD_DESTAO,
    FAIL_RD_CONSTRUCTIONTYPE,
    FAIL_RD_SPECIFIED_AMOUNT,
    FAIL_RD_CONSTR_NUC_AMOUNT,
    // qualify
    FAIL_RD_QUALIFY,
    FAIL_RD_PAXTYPE,
    // positive
    FAIL_RD_MATCHED,
    // number of enums
    FAIL_RD_LAST,
    FAIL_RD_FIRST = 0,
    FAIL_RD_LAST_GENERAL = FAIL_RD_DESTINATION,
    FAIL_RD_LAST_CONSTRUCTED = FAIL_RD_CONSTR_NUC_AMOUNT
  };

  explicit Diag291CollectorFD(Diagnostic& root)
    : DiagCollector(root),
      _fdTrx(nullptr),
      _fdo(nullptr),
      _request(nullptr),
      _wantDisc(false),
      _wantFbr(false),
      _wantNeg(false),
      _wantConstr(false)
  {
  }

  Diag291CollectorFD()
    : _fdTrx(nullptr),
      _fdo(nullptr),
      _request(nullptr),
      _wantDisc(false),
      _wantFbr(false),
      _wantNeg(false),
      _wantConstr(false)
  {
  }

  virtual void initParam(Diagnostic& root) override;

  void init(FareDisplayTrx& trx);
  void printHeader() override;
  void printFooter(size_t faresFound, int faresProcessed);
  void printFare(PaxTypeFare& ptFare, D291FailCode failCode);

private:
  FareDisplayTrx* _fdTrx;
  FareDisplayOptions* _fdo;
  FareDisplayRequest* _request;

  bool _wantDisc;
  bool _wantFbr;
  bool _wantNeg;
  bool _wantConstr;
};

} // namespace tse

