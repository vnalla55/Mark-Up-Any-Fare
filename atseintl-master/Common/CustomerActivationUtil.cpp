//----------------------------------------------------------------------------
//
//  File:           CustomerActivationUtil.cpp
//  Created:        06/20/2012
//  Authors:
//
//  Description:    Utility Class for Customer activation configuration
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
#include "Common/CustomerActivationUtil.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/CustomerActivationControl.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diagnostic.h"

const std::string header = "********** CUSTOMER ACTIVATION CONTROL DIAGNOSTICS **********\n";
const std::string footer = "*********************** END DIAG 193 ************************\n";
const std::string divider = "*************************************************************\n";
const std::string mhheader = "MULTI HOST ACTIVATION APPLICATION DATA : \n";
const std::string caheader = "CARRIER ACTIVATION APPLICATION DATA : \n";
const std::string geolocheader = "GEO LOCATION APPLICATION DATA : \n";
const std::string empty_str = "";

using namespace std;

namespace tse
{

static Logger
logger("atseintl.Common.CustomerActivationUtil");

CustomerActivationUtil::CustomerActivationUtil(PricingTrx& trx, const CarrierCode* cxr)
  : _diagEnabled(false), _status(STATUS_NOT_SET), _trx(trx), _cxr(cxr)
{
}

CustomerActivationUtil::~CustomerActivationUtil() {}

//-----------------------------------------------------------------------------
// getCustomerActivationRecords
//-----------------------------------------------------------------------------
const std::vector<CustomerActivationControl*>&
CustomerActivationUtil::getCustomerActivationRecords(const std::string& projCode)
{
  return _trx.dataHandle().getCustomerActivationControl(projCode);
}

bool
CustomerActivationUtil::processCustomerActivationRecords(
    const std::vector<CustomerActivationControl*>& cacVector,
    const std::string& projCode,
    DateTime& activationDate,
    DiagCollector* diag)
{
  std::vector<CustomerActivationControl*>::const_iterator cacItrB = cacVector.begin();
  size_t i = 0;
  for (; cacItrB != cacVector.end(); ++cacItrB)
  {
    CustomerActivationControl* cacInfo = *cacItrB;
    if (UNLIKELY(_diagEnabled && diag))
    {
      if (_trx.diagnostic().diagnosticType() == Diagnostic193 && !diagSeqFilter((*cacInfo).seqNo()))
        continue;

      i++;
      printingParentTable(diag, *cacInfo, i);
    }

    if (matchCACParentInfo(*cacInfo, activationDate, diag))
    {
      if (!_diagEnabled)
      {
        ActivationResult* acResult = nullptr;
        _trx.dataHandle().get(acResult);
        if (acResult == nullptr)
          return false;

        acResult->_cacRecord = cacInfo;
        acResult->finalActvDate() = activationDate;
        acResult->isActivationFlag() = cacInfo->isActivated();

        if (!cacInfo->isActivated())
          acResult = nullptr;

        _trx.projCACMapData().insert(std::make_pair(projCode, acResult));
      }
      return cacInfo->isActivated();
    }

    if (UNLIKELY(_diagEnabled && diag))
      printFailStatus(diag, _status);
  }
  if (!_diagEnabled)
    _trx.projCACMapData().insert(std::make_pair(projCode, static_cast<ActivationResult*>(nullptr)));
  return false;
}

//-----------------------------------------------------------------------------
// matchCACParentInfo
//-----------------------------------------------------------------------------
bool
CustomerActivationUtil::matchCACParentInfo(CustomerActivationControl& cac,
                                           DateTime& activationDate,
                                           DiagCollector* diag)
{
  std::string hostName = "";
  std::string agentPCC = _trx.getRequest()->ticketingAgent()->tvlAgencyPCC();

  if (_trx.getRequest()->ticketingAgent()->agentTJR() != nullptr)
    hostName = _trx.getRequest()->ticketingAgent()->agentTJR()->hostName();

  if (UNLIKELY(_diagEnabled && diag))
  {
    std::string passMsg = "VALIDATING CUSTOMER ACTIVATION DATA \nHOSTNAME : " + hostName +
                          "   AGENT PCC : " + agentPCC + "\n";
    (*diag) << "\n" << passMsg << std::endl;
  }

  if (matchPCC(agentPCC, cac.pseudoCity(), diag) &&
      matchCRSUserAppl(hostName, cac.crsUserAppl(), diag))
  {
    if (cac.multiHostActivation().size() == 0 && cac.cxrActivation().size() == 0 &&
        cac.geoActivation().size() == 0)
    {
      activationDate = cac.effDate();
      if (_diagEnabled && diag)
      {
        _status = PASS_CAC;
        std::string passMsg = "NO DATA IN CHILD TABLES";
        (*diag) << "\n" << passMsg << std::endl;
        printFailStatus(diag, _status);
      }
      return true;
    }

    if (TrxUtil::isRequestFromAS(_trx)) // for HC
    {
      if (matchHostActivationAppl(cac.multiHostActivation(), activationDate, diag))
        return true;
    }
    else if (isRequestFromTN()) // For TN
    {
      if (matchGeoLocAppl(cac.geoActivation(), activationDate, diag))
        return true;
    }

    if (matchCarrierActivationAppl(cac.cxrActivation(), activationDate, diag))
      return true;
  }
  return false;
}

bool
CustomerActivationUtil::isRequestFromTN()
{
  if (_trx.getRequest()->ticketingAgent() && _trx.getRequest()->ticketingAgent()->agentTJR() != nullptr)
    return true;

  return false;
}

//-----------------------------------------------------------------------------
// matchHostActivationAppl
//-----------------------------------------------------------------------------
bool
CustomerActivationUtil::matchHostActivationAppl(
    std::vector<CustomerActivationControl::MultiHostActivation*>& mhActVector,
    DateTime& activationDate,
    DiagCollector* diag)
{

  typedef std::vector<CustomerActivationControl::MultiHostActivation*>::const_iterator mhActI;

  if (_diagEnabled && diag)
  {
    printMHHeader(diag);
  }

  for (mhActI mhActIb = mhActVector.begin(); mhActIb != mhActVector.end(); ++mhActIb)
  {
    CustomerActivationControl::MultiHostActivation* mhActInfo = *mhActIb;
    if (_diagEnabled && diag)
      printMHData(diag, *mhActInfo);

    if (mhActInfo->mhCxr().equalToConst("$$") || mhActInfo->mhCxr() == _trx.billing()->partitionID())
    {
      activationDate = mhActInfo->mhCxrActDate();
      if (_diagEnabled && diag)
      {
        _status = PASS_CAC;
        std::string passMsg = "PASS MULTI HOST INFO - ACTIVATION DATE SET TO : ";
        (*diag) << "\n" << passMsg << activationDate.dateToSqlString() << std::endl;
        printFailStatus(diag, _status);
      }
      return true;
    }
  }
  if (_diagEnabled)
    _status = FAIL_HOST_APPL;
  return false;
}

//-----------------------------------------------------------------------------
// matchCarrierActivationAppl
//-----------------------------------------------------------------------------
bool
CustomerActivationUtil::matchCarrierActivationAppl(
    std::vector<CustomerActivationControl::CarrierActivation*>& cxrActVector,
    DateTime& activationDate,
    DiagCollector* diag)
{
  if (!_cxr)
    return false;

  typedef std::vector<CustomerActivationControl::CarrierActivation*>::const_iterator cxrActI;

  if (_diagEnabled && diag)
  {
    printCAHeader(diag, *_cxr);
  }

  for (cxrActI cxrActIb = cxrActVector.begin(); cxrActIb != cxrActVector.end(); ++cxrActIb)
  {
    CustomerActivationControl::CarrierActivation* cxrActInfo = *cxrActIb;
    if (_diagEnabled && diag)
      printCAData(diag, *cxrActInfo);

    if (cxrActInfo->cxr().equalToConst("$$") || cxrActInfo->cxr() == *_cxr)
    {
      activationDate = cxrActInfo->cxrActDate();
      if (_diagEnabled && diag)
      {
        _status = PASS_CAC;
        std::string passMsg = "PASS CARRIER ACTIVATION INFO - ACTIVATION DATE SET TO : ";
        (*diag) << "\n" << passMsg << activationDate.dateToSqlString() << std::endl;
        printFailStatus(diag, _status);
      }
      return true;
    }
  }
  if (_diagEnabled)
    _status = FAIL_CXR_ACTV_APPL;
  return false;
}

//-----------------------------------------------------------------------------
// matchGeoLocAppl
//-----------------------------------------------------------------------------
bool
CustomerActivationUtil::matchGeoLocAppl(
    std::vector<CustomerActivationControl::GeoActivation*>& geoActVector,
    DateTime& activationDate,
    DiagCollector* diag)
{

  const Loc* agentLoc = _trx.getRequest()->ticketingAgent()->agentLocation();
  if (agentLoc == nullptr)
    return false;

  if (_diagEnabled && diag)
  {
    printGeoHeader(diag, agentLoc);
  }

  typedef std::vector<CustomerActivationControl::GeoActivation*>::const_iterator geoActI;

  for (geoActI geoActIb = geoActVector.begin(); geoActIb != geoActVector.end(); ++geoActIb)
  {
    CustomerActivationControl::GeoActivation* geoActInfo = *geoActIb;
    if (_diagEnabled && diag)
      printGeoData(diag, *geoActInfo);

    if (validateLoc(*agentLoc, *geoActInfo))
    {
      activationDate = geoActInfo->locActivationDate();
      if (_diagEnabled && diag)
      {
        _status = PASS_CAC;
        std::string passMsg = "PASS GEO LOCATION INFO - ACTIVATION DATE SET TO : ";
        (*diag) << "\n" << passMsg << activationDate.dateToSqlString() << std::endl;
        printFailStatus(diag, _status);
      }
      return true;
    }
  }
  if (_diagEnabled)
    _status = FAIL_GEO_LOC_APPL;
  return false;
}

bool
CustomerActivationUtil::validateLoc(const Loc& agentLoc,
                                    CustomerActivationControl::GeoActivation& geoCACloc)
{
  return LocUtil::isInLoc(agentLoc, geoCACloc.locType(), geoCACloc.loc());
}

bool
CustomerActivationUtil::matchPCC(std::string& reqPCC, PseudoCityCode& cacPCC, DiagCollector* diag)
{
  if (cacPCC.empty() || reqPCC.compare(cacPCC) == 0)
  {
    if (_diagEnabled && diag)
    {
      std::string passMsg = "\nPCC MATCHING DONE \n";
      printMsg(diag, passMsg);
    }
    return true;
  }
  if (UNLIKELY(_diagEnabled))
    _status = FAIL_PCC;
  return false;
}

bool
CustomerActivationUtil::matchCRSUserAppl(std::string& reqCRS,
                                         UserApplCode& cacCRS,
                                         DiagCollector* diag)
{
  if (cacCRS.empty() || reqCRS.compare(cacCRS) == 0)
  {
    if (_diagEnabled && diag)
    {
      std::string passMsg = "\nCRS USER MATCHING DONE \n";
      printMsg(diag, passMsg);
    }
    return true;
  }
  if (_diagEnabled)
    _status = FAIL_CRS_USER;
  return false;
}

void
CustomerActivationUtil::print193Diagnostic(DiagCollector* diag)
{
  if (diag == nullptr || _trx.diagnostic().diagnosticType() != Diagnostic193)
    return;

  _diagEnabled = true;

  (*diag) << divider << header << divider;
  const std::string& reqProjCode = getProjCodeDateFromTrx();

  if (!reqProjCode.empty())
  {
    processProjectCodes(diag, reqProjCode);
  }
  else if (projDateCount != 0)
  {
    for (const auto& reqProjCode : CAC_PROJ_ACTIVE_DATES)
    {
      _status = STATUS_NOT_SET;

      processProjectCodes(diag, reqProjCode);
    }
  }
  else
    (*diag) << "\nNO PROJCODES TO PROCESS" << std::endl;

  (*diag) << footer;
  return;
}

namespace
{
// this util can be used "activate by date" AND "activate by flag"
// but is seems that diagnostic creator don't give a ...damn about this fact
// so add your project below if it also want "activate by flag" behaviour
// and if you don't want lies on diag 193
bool
isActivatedByFlagRequest(const std::string& projCode)
{
  return projCode == CAC_PROJ_ACTIVE_DATES[2];
}
}

void
CustomerActivationUtil::processProjectCodes(DiagCollector* diag, const std::string& projCode)
{

  (*diag) << "\nPROJECT CODE IS : " << projCode << std::endl;
  const std::vector<CustomerActivationControl*>& cac = getCustomerActivationRecords(projCode);
  if (cac.size() != 0)
  {
    (*diag) << "NUMBER OF RECORDS FOR THIS PROJECT is : " << cac.size() << std::endl;
    DateTime activationDate;

    if (processCustomerActivationRecords(cac, projCode, activationDate, diag))
    {
      if ((_trx.getRequest() && _trx.getRequest()->ticketingDT() >= activationDate) ||
          isActivatedByFlagRequest(projCode))
      {
        _status = FINAL_PASS;
        (*diag) << divider;
        printFailStatus(diag, _status);
      }
      else
      {
        if (_trx.getRequest())
          (*diag) << "\nTICKETING DATE : " << _trx.getRequest()->ticketingDT().dateToSqlString()
                  << std::endl;
        _status = FINAL_FAIL;
        printFailStatus(diag, _status);
        (*diag) << divider;
      }
    }
    else
    {
      _status = FAIL_CAC;
      printFailStatus(diag, _status);
      _status = FINAL_FAIL;
      printFailStatus(diag, _status);
      (*diag) << divider;
    }
  }
  else
    (*diag) << "\nNO DATA IN CAC TABLE FOR THIS PROJECT CODE " << std::endl;
}

void
CustomerActivationUtil::printFailStatus(DiagCollector* diag, CACValidation& status)
{

  switch (status)
  {
  case PASS_CAC:
    (*diag) << "\nRESULT : ";
    (*diag) << "MATCHED CUSTOMER ACTIVATION RECORD";
    break;
  case FAIL_CAC:
    (*diag) << "\nRESULT : ";
    (*diag) << "NO MATCH ON CUSTOMER ACTIVATION RECORD";
    break;
  case FINAL_PASS:
    (*diag) << "\nSTATUS : ";
    (*diag) << "PROJECT IS ACTIVATED \n";
    break;
  case FINAL_FAIL:
    (*diag) << "\nSTATUS : ";
    (*diag) << "PROJECT NOT ACTIVATED \n";
    break;
  case FAIL_CRS_USER:
    (*diag) << "\nRESULT : ";
    (*diag) << "NO MATCH ON CRS";
    break;
  case FAIL_HOST_APPL:
    (*diag) << "\nRESULT : ";
    (*diag) << "NO MATCH ON HOST APPLICATION";
    break;
  case FAIL_CXR_ACTV_APPL:
    (*diag) << "\nRESULT : ";
    (*diag) << "NO MATCH ON CARRIER ACTIVATION";
    break;
  case FAIL_GEO_LOC_APPL:
    (*diag) << "\nRESULT : ";
    (*diag) << "NO MATCH ON GEO LOCATION";
    break;
  case STATUS_NOT_SET:
    (*diag) << "\nSTATUS : ";
    (*diag) << "NO STATUS SET";
    break;
  default:
    (*diag) << "\nSTATUS : ";
    (*diag) << "UNKNOWN STATUS";
    break;
  }
  (*diag) << "\n";
}

const std::string&
CustomerActivationUtil::getProjCodeDateFromTrx()
{

  std::map<std::string, std::string>& diagParamMap = _trx.diagnostic().diagParamMap();
  std::map<std::string, std::string>::const_iterator i = diagParamMap.find(PROJ_CODE_DATE);
  if (i == diagParamMap.end())
    return empty_str;
  else
    return i->second;
}

bool
CustomerActivationUtil::diagSeqFilter(int64_t projSeqNo)
{
  std::map<std::string, std::string>& diagParamMap = _trx.diagnostic().diagParamMap();
  std::map<std::string, std::string>::const_iterator i = diagParamMap.find(SEQ_NUMBER);
  if (i == diagParamMap.end())
    return true;
  else
  {
    const std::string& strSeqNo = i->second;
    int64_t inputSeqNumber = atoi(strSeqNo.c_str());
    return (projSeqNo == inputSeqNumber);
  }
}

void
CustomerActivationUtil::printingParentTable(DiagCollector* diag,
                                            CustomerActivationControl& cac,
                                            size_t rowno)
{
  if (diag)
  {
    (*diag) << "\nRECORD :" << rowno << "\n";
    (*diag) << "\n\n" << divider << "CUSTOMER ACTIVATION CONTROL TABLE DATA : " << std::endl
      << divider;

    diag->setf(std::ios::left, std::ios::adjustfield);
    (*diag) << "PROJCODE                : " << setw(20) << cac.projCode() << "\n";
    (*diag) << "SEQUENCE NUMBER         : " << setw(4) << cac.seqNo() << "\n";
    (*diag) << "EFFECTIVE DATE          : " << setw(10) << cac.effDate().dateToSqlString() << "\n";
    (*diag) << "DISCONTINUE DATE        : " << setw(10) << cac.discDate().dateToSqlString() << "\n";
    (*diag) << "EXPIRE DATE             : " << setw(10) << cac.expireDate().dateToSqlString()
      << "\n";
    (*diag) << "PSEUDO CITY             : " << setw(5) << cac.pseudoCity() << "\n";
    (*diag) << "PROJECT DESCRIPTION     : " << cac.projDesc() << "\n";
    (*diag) << "PROJECT ACTIVATION IND  : ";

    if (cac.isActivated())
      (*diag) << "TRUE";
    else
      (*diag) << "FALSE";

    (*diag) << "\nCRS USER APPL           : " << setw(4) << cac.crsUserAppl() << "\n";
    (*diag) << divider;
  }
}

void
CustomerActivationUtil::printMsg(DiagCollector* diag, const std::string& msg)
{
  (*diag) << "\n" << msg << std::endl;
}

void
CustomerActivationUtil::printMHHeader(DiagCollector* diag)
{
  if (diag)
  {
    std::string passMsg = "CHECK MULTI HOST APPLICATION DATA \nPARTITION ID : ";
    std::string partionID = _trx.billing()->partitionID().c_str();
    passMsg = passMsg + partionID + "\n";
    printMsg(diag, passMsg);
    (*diag) << divider << mhheader << divider;
    (*diag) << "\n"
            << "MULTIHOST CARRIER   "
            << "ACTIVATION DATE" << std::endl;
  }
}

void
CustomerActivationUtil::printMHData(DiagCollector* diag,
                                    CustomerActivationControl::MultiHostActivation& mhData)
{
  if (diag)
  {
    diag->setf(std::ios::left, std::ios::adjustfield);
    (*diag) << setw(3) << mhData.mhCxr() << "                 ";
    (*diag) << setw(10) << mhData.mhCxrActDate().dateToSqlString() << std::endl;
  }
}

void
CustomerActivationUtil::printCAHeader(DiagCollector* diag, const CarrierCode& reqCxr)
{
  if (diag)
  {
    std::string passMsg = "\nCHECK CARRIER ACTIVATION APPLICATION DATA \nREQUESTED CARRIER : ";
    std::string cxrCodeStr = reqCxr.c_str();
    passMsg = passMsg + cxrCodeStr + "\n";
    printMsg(diag, passMsg);
    (*diag) << divider << caheader << divider;
    (*diag) << "\n"
            << "CARRIER"
            << "   "
            << "ACTIVATION DATE" << std::endl;
  }
}

void
CustomerActivationUtil::printCAData(DiagCollector* diag,
                                    CustomerActivationControl::CarrierActivation& caData)
{
  if (diag)
  {
    diag->setf(std::ios::left, std::ios::adjustfield);
    (*diag) << setw(3) << caData.cxr() << "       ";
    (*diag) << setw(10) << caData.cxrActDate().dateToSqlString() << std::endl;
  }
}

void
CustomerActivationUtil::printGeoHeader(DiagCollector* diag, const Loc* agentLoc)
{
  if (diag)
  {
    std::string agentCity = agentLoc->city().c_str();
    std::string agentState = agentLoc->state().c_str();
    std::string agentNation = agentLoc->nation().c_str();
    std::string passMsg = "\nCHECK GEO LOCATION APPLICATION DATA \nAGENT LOC CITY   : " + agentCity;
    passMsg = passMsg + "\nAGENT LOC STATE  : " + agentState + "\nAGENT LOC NATION : " +
              agentNation + "\n";
    printMsg(diag, passMsg);
    (*diag) << divider << geolocheader << divider;
    (*diag) << "\n"
            << "LOCTYPE "
            << "  "
            << "LOC "
            << "ACTIVATION DATE" << std::endl;
  }
}

void
CustomerActivationUtil::printGeoData(DiagCollector* diag,
                                     CustomerActivationControl::GeoActivation& gaData)
{
  if (diag)
  {
    diag->setf(std::ios::left, std::ios::adjustfield);
    (*diag) << setw(2) << gaData.locType() << "        " << setw(4) << gaData.loc();
    (*diag) << setw(10) << gaData.locActivationDate().dateToSqlString() << std::endl;
  }
}

} // tse
