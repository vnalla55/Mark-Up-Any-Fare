//-------------------------------------------------------------------
//
//  File:        CurrencyTrx.h
//  Created:     June 2, 2004
//  Design:      Mark Kasprowicz
//  Authors:
//
//  Description: Currency Transaction object
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Common/TypeConvert.h"
#include "DataModel/Trx.h"
#include "Service/Service.h"

namespace tse
{
class PricingRequest;
class Billing;

class CurrencyTrx : public Trx
{
public:
  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  void setRequest(PricingRequest* request) { _request = request; }
  const PricingRequest* getRequest() const { return _request; }
  PricingRequest* getRequest() { return _request; }

  Billing*& billing() { return _billing; }
  const Billing* billing() const override { return _billing; }

  double& amount() { return _amount; }
  const double& amount() const { return _amount; }

  std::string& amountStr() { return _amountStr; }
  const std::string& amountStr() const { return _amountStr; }

  NationCode& baseCountry() { return _baseCountry; }
  const NationCode& baseCountry() const { return _baseCountry; }

  CurrencyCode& sourceCurrency() { return _sourceCurrency; }
  const CurrencyCode& sourceCurrency() const { return _sourceCurrency; }

  CurrencyCode& targetCurrency() { return _targetCurrency; }
  const CurrencyCode& targetCurrency() const { return _targetCurrency; }

  DateTime& baseDT() { return _baseDT; }
  const DateTime& baseDT() const { return _baseDT; }

  char& reciprocal() { return _reciprocal; }
  const char& reciprocal() const { return _reciprocal; }
  const bool isReciprocal() const { return TypeConvert::pssCharToBool(_reciprocal); }

  char& commandType() { return _commandType; }
  const char& commandType() const { return _commandType; }

  char& eprBDK() { return _eprBDK; }
  const char& eprBDK() const { return _eprBDK; }

  DateTime& pssLocalDate() { return _pssLocalDate; }
  const DateTime& pssLocalDate() const { return _pssLocalDate; }

  Indicator& dcAlphaChar() { return _dcAlphaChar; }
  const Indicator& dcAlphaChar() const { return _dcAlphaChar; }

  char& requestType() { return _requestType; }
  const char& requestType() const { return _requestType; }

  Indicator& futureDateInd() { return _futureDateInd; }
  const Indicator& futureDateInd() const { return _futureDateInd; }

  std::ostringstream& response() { return _response; }

  bool isErrorResponse() { return _isErrorResponse; }
  const bool isErrorResponse() const { return _isErrorResponse; }

  void setErrorResponse() { _isErrorResponse = true; }

private:
  PricingRequest* _request = nullptr;
  Billing* _billing = nullptr;

  // DC options are living here for the time being
  double _amount = 0; // AMT
  std::string _amountStr; // AMT as a string
  NationCode _baseCountry; // CCB
  CurrencyCode _sourceCurrency; // CUR
  CurrencyCode _targetCurrency; // CUT
  DateTime _baseDT{tse::pos_infin}; // CDT
  char _reciprocal = 'F'; // RCP
  char _commandType = 0; // CMD
  char _eprBDK = 'F'; // BDK
  DateTime _pssLocalDate{tse::pos_infin}; // D01
  Indicator _dcAlphaChar = 0; // N1Q
  char _requestType = 0; // N1V
  Indicator _futureDateInd = ' '; // N1W
  bool _isErrorResponse = false;

  std::ostringstream _response;

}; // class CurrencyTrx
} // tse namespace
