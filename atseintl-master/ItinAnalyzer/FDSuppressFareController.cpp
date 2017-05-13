//----------------------------------------------------------------------------
//  File: FDSuppressFareController.cpp
//
//  Author: Partha Kumar Chakraborti
//  Created:      04/12/2005
//  Description:  This takes cares of suppression of a fare based on suppression table.cpp
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

#include "ItinAnalyzer/FDSuppressFareController.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FDSuppressFare.h"
#include "DBAccess/FDSuppressFareDAO.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "Diagnostic/DCFactoryFareDisplay.h"
#include "Diagnostic/Diag212CollectorFD.h"

namespace tse
{
static Logger
logger("atseintl.DBAccess.FDSuppressFareController");

const Indicator FDSuppressFareController::TYPE_NONE = ' ';
const PseudoCityCode FDSuppressFareController::PCC_NONE = "";
const TJRGroup FDSuppressFareController::TJR_NONE = 0;
const CarrierCode FDSuppressFareController::CARRIER_NONE = "";

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDSuppressFareController::find()
//
// Finds whether any matching founds based on
// directionality
//
// @param std::vector<const tse::FDSuppressFare*>& - Suppress Fare
// list retrieved from database.
// @param const Loc& origin& - Origin
//
// const Loc& destination - destination
// @return  bool - If matching found then return true. Otherwise false.
//
// </PRE>
// -----------------------------------------------------------

bool
FDSuppressFareController::find(const std::vector<const tse::FDSuppressFare*>& fdSuppressFareList,
                               const Loc& origin,
                               const Loc& destination,
                               std::set<CarrierCode>& matchedCarriers)
{
  LOG4CXX_INFO(logger, "Entering Find");

  std::vector<const FDSuppressFare*>::const_iterator i = fdSuppressFareList.begin();

  for (; i != fdSuppressFareList.end(); i++)
  {
    const Directionality& dir = (*i)->directionality();
    LOG4CXX_INFO(logger, "directionality:" << dir);
    if (dir == WITHIN)
    {
      if (!LocUtil::isWithin((*i)->loc1(), origin, destination))
        continue;
    }
    else if (dir == FROM)
    {
      if (!LocUtil::isFrom((*i)->loc1(), (*i)->loc2(), origin, destination))
        continue;
    }
    else if (dir == BETWEEN)
    {
      if (!LocUtil::isBetween((*i)->loc1(), (*i)->loc2(), origin, destination))
        continue;
    }
    else if (dir == ORIGIN)
    {
      // NOT IMPLEMENTED
    }
    else if (dir == TERMINATE)
    {
      // NOT IMPLEMENTED
    }
    // Eliminate based on Single Carrier or Multi Carrier Entry
    if ((*i)->fareDisplayType() != NO_PARAM && fareDisplayType() != (*i)->fareDisplayType())
      continue;
    // found match
    matchedCarriers.insert((*i)->carrier());
  }
  return true;
}

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDSuppressFareController::useTjrGroupNo()
//
// Check if we should serch suppression records using TJR group
//
// @param tjrGroup    - tjr group number which is checked
//
// @return  true if should proceed with search
//
// </PRE>
// -----------------------------------------------------------

bool
FDSuppressFareController::useTjrGroupNo(TJRGroup tjrGroup)
{
  if (tjrGroup == TJR_NONE)
    return false;
  std::string configVal("N");
  if (!(Global::config()).getValue("TJR_GROUP", configVal, "FAREDISPLAY_SVC"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(logger, "TJR_GROUP", "FAREDISPLAY_SVC");
  }

  if (configVal == "N")
    return false;
  return true;
}

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDSuppressFareController::SuppressFare()
//
// This is the entry point of the class. Call processCarrier
// and very much depends on the return value. This function
// iterates through FareDisplayTrx::_preferredCarrier and
// removes those carries for which processCarrier() returns true.
// sends to
//
// @param FareDisplayTrx& - FareDisplay Trasaction object
//
// @return  void
//
// </PRE>
// -----------------------------------------------------------

void
FDSuppressFareController::suppressFare(FareDisplayTrx& trx)
{
  if (trx.diagnostic().isActive() && trx.diagnostic().diagnosticType() == Diagnostic212)
  {
    _diag = dynamic_cast<Diag212CollectorFD*>(DCFactoryFareDisplay::instance()->create(trx));
    if (_diag)
    {
      _diagActive = true;
      _diag->enable(Diagnostic212);
      _diag->init(trx);
      _diag->isMainPcc(!getSuppressFareList(trx, SFS_MAINPCC).empty());
      _diag->isAgentPcc(!getSuppressFareList(trx, SFS_AGENTPCC).empty());
      if (useTjrGroupNo(trx.getRequest()->ticketingAgent()->tjrGroup()))
        _diag->isTjrGrNo(!getSuppressFareList(trx, SFS_TJRGROUPNO).empty());
      _diag->printHeader();
    }
  }
  LOG4CXX_INFO(logger, "Entering SuppressFare");

  if (trx.preferredCarriers().empty())
  {
    LOG4CXX_DEBUG(logger, "No Carrier Preference Found");
    return;
  }

  std::set<CarrierCode> matchedCarriers;

  // Get suppressed carriers trying all levels
  if (!processSupressRecords(trx, SFS_AGENTPCC, matchedCarriers))
    if (!processSupressRecords(trx, SFS_MAINPCC, matchedCarriers))
      processSupressRecords(trx, SFS_TJRGROUPNO, matchedCarriers);

  std::stringstream retStr;
  if (matchedCarriers.size() > 0)
  {
    std::set<CarrierCode>::const_iterator itb = matchedCarriers.begin();
    std::set<CarrierCode>::const_iterator ite = matchedCarriers.end();
    for (; itb != ite; itb++)
      retStr << " " << *itb;
  }
  else
    retStr << "(NONE)";
  LOG4CXX_DEBUG(logger, "Suppressed carriers: " << retStr.str());

  eliminateCarriers(trx.preferredCarriers(), matchedCarriers);

  LOG4CXX_INFO(logger, "Leaving SuppressFare");

  if (_diag)
  {
    _diag->printFooter();
    _diag->flushMsg();
  }
}

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDSuppressFareController::processSupressRecords()
//
// Get Suppression fare records for current search level, and validate them to
// get list of suppressed carriers
//
// @param   level   - define level on which we try to obtain suppression
// records
//
// @param   matchedCarriers  - set of matched carriers
//
// @return  true if any matching record was found
//
// </PRE>
// -----------------------------------------------------------

bool
FDSuppressFareController::processSupressRecords(FareDisplayTrx& trx,
                                                SuppressFareMatchLevel level,
                                                std::set<CarrierCode>& matchedCarriers)
{
  const Indicator SINGLE_CARRIER_ENTRY = 'S';
  const Indicator MULTI_CARRIER_ENTRY = 'M';
  matchedCarriers.clear();
  if (level == SFS_TJRGROUPNO && !useTjrGroupNo(trx.getRequest()->ticketingAgent()->tjrGroup()))
  {
    LOG4CXX_DEBUG(logger,
                  "Skip searching Suppression records with TJR group: "
                      << trx.getRequest()->ticketingAgent()->tjrGroup());
    return false;
  }
  _fareDisplayType = ((trx.isShopperRequest()) ? MULTI_CARRIER_ENTRY : SINGLE_CARRIER_ENTRY);

  std::vector<const tse::FDSuppressFare*> fdSupp = getSuppressFareList(trx, level);

  // No matching record found, return false and proceed with next level
  if (fdSupp.empty())
    return false;

  if (_diag)
  {
    find(fdSupp, *trx.origin(), *trx.destination(), matchedCarriers);
    bool fMatch = matchedCarriers.size() > 0;
    switch (level)
    {
    case SFS_AGENTPCC:
      _diag->printAgentPccSuppFares(fdSupp, fMatch);
      break;
    case SFS_MAINPCC:
      _diag->printMainPccSuppFares(fdSupp, fMatch);
      break;
    case SFS_TJRGROUPNO:
      _diag->printTjrGroupNoSuppFares(fdSupp, fMatch);
      break;
    }
    return false; // in diagnostic return false to process all levels
  }
  return find(fdSupp, *trx.origin(), *trx.destination(), matchedCarriers);
}

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDSuppressFareController::getSuppressFareList()
//
// Get Suppression fare records for current search level
//
// @param   level   - define level on which we try to obtain suppression
// records
//
// @param   matchedCarriers  - set of matched carriers
//
// @return  vector of suppression records
//
// </PRE>
// -----------------------------------------------------------

std::vector<const tse::FDSuppressFare*>&
FDSuppressFareController::getSuppressFareList(FareDisplayTrx& trx, SuppressFareMatchLevel level)
{
  const PseudoCityCode pCC = (level == SFS_AGENTPCC)
                                 ? trx.getRequest()->ticketingAgent()->tvlAgencyPCC()
                                 : (level == SFS_MAINPCC)
                                       ? trx.getRequest()->ticketingAgent()->mainTvlAgencyPCC()
                                       : PCC_NONE;
  const Indicator pCCType =
      (level == SFS_AGENTPCC) ? PCCTYPE_BRANCH : (level == SFS_MAINPCC) ? PCCTYPE_HOME : TYPE_NONE;
  const TJRGroup tjrGroup =
      (level == SFS_TJRGROUPNO) ? trx.getRequest()->ticketingAgent()->tjrGroup() : TJR_NONE;
  const CarrierCode carrier = CARRIER_NONE;

  LOG4CXX_DEBUG(logger,
                "Get records using PCC: " << pCC << " PCC type: " << pCCType
                                          << " TJR No.: " << tjrGroup);
  return getSuppressFareList(trx, pCC, pCCType, tjrGroup, carrier, trx.travelDate());
}

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDSuppressFareController::eliminateCarriers()
//
// Remove carriers which exist in one set from another set
//
// @param   from    - set from which carriers will be removed.
//
// @param   which   - set of carriers to remove
//
// @return  true if any carrier was removed
//
// </PRE>
// -----------------------------------------------------------

bool
FDSuppressFareController::eliminateCarriers(std::set<CarrierCode>& from,
                                            const std::set<CarrierCode>& which)
{
  bool bFound = false;
  std::set<CarrierCode>::const_iterator itb = which.begin();
  std::set<CarrierCode>::const_iterator ite = which.end();
  for (; itb != ite; itb++)
  {
    if (from.find(*itb) != from.end())
    {
      bFound = true;
      from.erase(*itb);
    }
  }
  return bFound;
}
}
