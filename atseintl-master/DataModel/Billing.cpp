//-------------------------------------------------------------------
//
//  File:        Billing.cpp
//  Created:     April 5, 2004
//  Authors:
//
//  Description: Billing
//
//  Updates:
//          04/05/04 - VN - file created.
//          Sep 20 2005 -Val - added 2 operators
//          June 4 2007 - Darek Kubiczek - updated transaction ids
//                               and service names
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
#include "DataModel/Billing.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"

namespace tse
{

namespace
{
const char* ServiceNames[Billing::MAX_SVC_IND] = { "V2CURR",   "V2PRIDET", "V2TAX",   "V2FARED",
                                                   "V2MILEAG", "V2SHOP",   "V2PRICE", "V2MULT",
                                                   "V2PRIDIS", "V2HIST",   "ERDDSP1", "V2TKTCXR",
                                                   "V2TKTCXR" };

const char* BusinessFunctions[Billing::MAX_SVC_IND] = {
  "CURGEN",   "ITPGENI",  "TAXGEN",  "FQOGEN",   "MILGEN",   "V2UNKNOW", "ITPGENI",
  "V2UNKNOW", "V2UNKNOW", "HISGENI", "V2UNKNOW", "V2UNKNOW", "V2UNKNOW"
};

} // namespace

void
Billing::updateTransactionIds(uint64_t transactionID)
{
  TSE_ASSERT(transactionID != 0);

  if (_transactionID == 0)
  {
    _transactionID = transactionID;
  }

  if (_clientTranID == 0)
  {
    _clientTranID = _transactionID;
  }

  if (_parentTranID == 0)
  {
    _parentTranID = _clientTranID;
  }
}

void
Billing::updateServiceNames(Service service)
{
  if (isHistoricalServer())
  {
    updateHistoricalServiceNames(service);
    return;
  }

  if (_clientServiceName.empty())
  {
    if (service >= 0 && service < MAX_SVC_IND)
      _clientServiceName = BusinessFunctions[service];
  }

  if (_parentServiceName.empty())
  {
    if (service >= 0 && service < MAX_SVC_IND)
      _parentServiceName = ServiceNames[service];
  }

  if (_serviceName.empty())
  {
    _serviceName = _parentServiceName;
  }
}

bool
Billing::isHistoricalServer()
{
  std::string serverType;
  Global::config().getValue("SERVER_TYPE", serverType, "APPLICATION_CONSOLE");
  char c = serverType[0];
  if (c == 'h' || c == 'H')
    return true;
  return false;
}

void
Billing::updateHistoricalServiceNames(Service service)
{
  if (_clientServiceName.empty())
  {
    if (service == SVC_MILEAGE)
      _clientServiceName = BusinessFunctions[SVC_MILEAGE];
    else
      _clientServiceName = BusinessFunctions[SVC_HISTORICAL];
  }

  if (_parentServiceName.empty())
    _parentServiceName = ServiceNames[SVC_HISTORICAL];

  if (_serviceName.empty())
    _serviceName = _parentServiceName;
}

uint64_t
Billing::string2transactionId(const char* cstr)
{
  TSE_ASSERT(cstr);

  try { return boost::lexical_cast<uint64_t>(cstr); }
  catch (const boost::bad_lexical_cast&) {}

  return 0;
}

std::ostream& operator<<(std::ostream& os, const Billing& bill)
{
  os << "\nuserPseudoCityCode  " << bill.userPseudoCityCode().c_str() << "\nuserStation         "
     << bill.userStation().c_str() << "\nuserBranch          " << bill.userBranch().c_str()
     << "\npartitionID         " << bill.partitionID().c_str() << "\nuserSetAddress      "
     << bill.userSetAddress().c_str() << "\nserviceName         " << bill.serviceName().c_str()
     << "\naaaCity             " << bill.aaaCity().c_str() << "\naaaSine             "
     << bill.aaaSine().c_str() << "\nactionCode          " << bill.actionCode().c_str()
     << "\ntransactionID       " << bill.transactionID() << "\nparentTransactionID "
     << bill.parentTransactionID() << "\nclientTransactionID " << bill.clientTransactionID()
     << "\nclientServiceName   " << bill.clientServiceName().c_str() << "\nparentServiceName   "
     << bill.parentServiceName().c_str() << std::endl;

  return os;
}

const char*
Billing::getServiceName(Service service)
{
  if (service >= 0 && service < MAX_SVC_IND)
    return ServiceNames[service];
  return ""; // Returning empty-string to avoid core dump.
}

const char*
Billing::getBusinessFunction(Service service)
{
  if (isHistoricalServer())
    return BusinessFunctions[SVC_HISTORICAL];
  else if (service >= 0 && service < MAX_SVC_IND)
    return BusinessFunctions[service];
  return ""; // Returning empty-string to avoid core dump.
}

} // namespace tse
