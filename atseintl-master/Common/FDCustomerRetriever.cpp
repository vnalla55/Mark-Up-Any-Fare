//----------------------------------------------------------------------------
//  File: FDCustomerRetriever.cpp
//
//  Author: Partha Kumar Chakraborti
//  Created:      04/12/2005
//  Description:  This takes cares of getting header message text based on header msg table
//  Copyright Sabre 2005
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/FDCustomerRetriever.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{
const Indicator FDCustomerRetriever::TYPE_NONE = ' ';
const UserApplCode FDCustomerRetriever::USER_NONE = "";
const PseudoCityCode FDCustomerRetriever::PCC_NONE = "";
const TJRGroup FDCustomerRetriever::TJR_NONE = 0;

FDCustomerRetriever::FDCustomerRetriever(FareDisplayTrx& trx)
  : _trx(trx),
    _userMulti(trx.billing()->partitionID()),
    _userCrs(trx.getRequest()->ticketingAgent()->cxrCode()),
    _pccAgent(trx.getRequest()->ticketingAgent()->tvlAgencyPCC()),
    _pccHome(trx.getRequest()->ticketingAgent()->mainTvlAgencyPCC()),
    _tjrGroup(trx.getRequest()->ticketingAgent()->tjrGroup())
{
}

FDCustomerRetriever::~FDCustomerRetriever() {}

bool
FDCustomerRetriever::retrieveData(const TJRGroup& tjrGroup)
{
  return retrieveData(TYPE_NONE, USER_NONE, TYPE_NONE, PCC_NONE, tjrGroup);
}

bool
FDCustomerRetriever::retrieve(bool allRecs)
{
  bool found = false;
  if (_trx.isTravelAgent())
  {
    found |= retrieveData(PCCTYPE_BRANCH, _pccAgent);
    if (found && !allRecs)
      return true;

    found |= retrieveData(PCCTYPE_HOME, _pccHome);
    if (found && !allRecs)
      return true;

    // tjrGroup doesn't have a PCCTYPE, so need extra check
    // to prevent getting default record (all keys empty)
    if (_tjrGroup != TJR_NONE)
    {
      found |= retrieveData(_tjrGroup);
      if (found && !allRecs)
        return true;
    }
  }
  else
  {
    found |= retrieveData(MULTIHOST_USER_APPL, _userMulti);
    if (found && !allRecs)
      return true;
  }

  UserApplCode userAppl;
  if (_userCrs == INFINI_MULTIHOST_ID)
    userAppl = INFINI_USER;
  else if (_userCrs == AXESS_MULTIHOST_ID)
    userAppl = AXESS_USER;
  else if (_userCrs == ABACUS_MULTIHOST_ID)
    userAppl = ABACUS_USER;
  else if (_userCrs == SABRE_MULTIHOST_ID)
    userAppl = SABRE_USER;
  else
    userAppl = "";

  found |= retrieveData(CRS_USER_APPL, userAppl);
  if (found && !allRecs)
    return true;

  found |= retrieveDefault();
  return found;
}
}
