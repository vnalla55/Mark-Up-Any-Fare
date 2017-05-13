//----------------------------------------------------------------------------
//
//  File:        CustomerActivationUtil.h
//  Created:     06/20/2012
//  Authors:     Jayanthi Shyam Mohan
//
//  Description: Utility Class for Customer activation configuration.
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
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CustomerActivationControl.h"


#include <string>

namespace tse
{

class DiagCollector;
class PricingTrx;
class Loc;

// Start - For Projects activated by Date, enter project code in CAC_PROJ_ACTIVE_FLAGS and increase
// the projDateCount value
static const int16_t projDateCount = 4;
static const std::string CAC_PROJ_ACTIVE_DATES[projDateCount] = {
    std::string("GNR"),
    std::string("GSA"),
    std::string("RTW"),
    std::string("TCH") };
// End
// Start - For Projects activated by flag, enter project code in CAC_PROJ_ACTIVE_FLAGS and increase
// the projFlagCount value
static const int16_t projFlagCount = 0;
static const std::string CAC_PROJ_ACTIVE_FLAGS[projFlagCount] = {};
// End
static const std::string PROJ_CODE_DATE = "PD";
static const std::string PROJ_CODE_FLAG = "PF";
static const std::string SEQ_NUMBER = "SQ";

struct ActivationResult
{
  ActivationResult() : _isActivationFlag(false), _finalActvDate(time(nullptr)), _cacRecord(nullptr) {}

  bool _isActivationFlag;
  DateTime _finalActvDate;
  CustomerActivationControl* _cacRecord;

  virtual ~ActivationResult() {}

  bool& isActivationFlag() { return _isActivationFlag; }
  const bool& isActivationFlag() const { return _isActivationFlag; }

  DateTime& finalActvDate() { return _finalActvDate; }
  const DateTime& finalActvDate() const { return _finalActvDate; }
};

enum CACValidation
{ PASS_CAC = 0,
  FAIL_CAC,
  FINAL_PASS,
  FINAL_FAIL,
  FAIL_PCC,
  FAIL_CRS_USER,
  FAIL_HOST_APPL,
  FAIL_CXR_ACTV_APPL,
  FAIL_GEO_LOC_APPL,
  STATUS_NOT_SET };

class CustomerActivationUtil
{
public:
  CustomerActivationUtil(PricingTrx& trx, const CarrierCode* cxr = nullptr);
  virtual ~CustomerActivationUtil();

  const std::vector<CustomerActivationControl*>&
  getCustomerActivationRecords(const std::string& projCode);

  bool processCustomerActivationRecords(const std::vector<CustomerActivationControl*>& cacVector,
                                        const std::string& projCode,
                                        DateTime& activationDate,
                                        DiagCollector* diag = nullptr);

  void print193Diagnostic(DiagCollector* diag);

protected:
  bool matchCACParentInfo(CustomerActivationControl& cac,
                          DateTime& activationDate,
                          DiagCollector* diag = nullptr);

  bool isRequestFromTN();

  bool
  matchHostActivationAppl(std::vector<CustomerActivationControl::MultiHostActivation*>& mhActVector,
                          DateTime& activationDate,
                          DiagCollector* diag = nullptr);

  bool matchCarrierActivationAppl(
      std::vector<CustomerActivationControl::CarrierActivation*>& cxrActVector,
      DateTime& activationDate,
      DiagCollector* diag = nullptr);

  bool matchGeoLocAppl(std::vector<CustomerActivationControl::GeoActivation*>& geoActVector,
                       DateTime& activationDate,
                       DiagCollector* diag = nullptr);

  bool validateLoc(const Loc& agentLoc, CustomerActivationControl::GeoActivation& geoCACloc);

  bool matchPCC(std::string& reqPCC, PseudoCityCode& cacPCC, DiagCollector* diag = nullptr);
  bool matchCRSUserAppl(std::string& reqCRS, UserApplCode& cacCRS, DiagCollector* diag = nullptr);
  void processProjectCodes(DiagCollector* diag, const std::string& projCode);
  void printFailStatus(DiagCollector* diag, CACValidation& status);
  const std::string& getProjCodeDateFromTrx();
  bool diagSeqFilter(int64_t projSeqNo);
  void printingParentTable(DiagCollector* diag, CustomerActivationControl& cac, size_t rowno);
  void printMsg(DiagCollector* diag, const std::string& msg);
  void printMHHeader(DiagCollector* diag);
  void printMHData(DiagCollector* diag, CustomerActivationControl::MultiHostActivation& mhData);
  void printCAHeader(DiagCollector* diag, const CarrierCode& reqCxr);
  void printCAData(DiagCollector* diag, CustomerActivationControl::CarrierActivation& caData);
  void printGeoHeader(DiagCollector* diag, const Loc* agentLoc);
  void printGeoData(DiagCollector* diag, CustomerActivationControl::GeoActivation& gaData);

  bool _diagEnabled;
  CACValidation _status;

private:
  PricingTrx& _trx;
  const CarrierCode* _cxr;
};

} // end tse namespace

