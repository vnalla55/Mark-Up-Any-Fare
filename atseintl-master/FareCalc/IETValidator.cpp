//-------------------------------------------------------------------
//
//  File:       IETValidator.cpp
//  Created:    March 01, 2012
//
//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
//

#include "FareCalc/IETValidator.h"

#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/InterlineTicketCarrierInfo.h"
#include "DBAccess/InterlineTicketCarrierStatus.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag851Collector.h"
#include "Diagnostic/DiagManager.h"
#include "FareCalc/CalcTotals.h"

namespace tse
{

static Logger
logger("atseintl.FareCalc.IETValidator");

IETValidator::IETValidator(PricingTrx& trx) : _trx(trx), _diag(nullptr), _diagHeaderAdded(false)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic851)
  {
    DCFactory* factory = DCFactory::instance();
    _diag = dynamic_cast<Diag851Collector*>(factory->create(_trx));
    if (_diag != nullptr)
      _diag->enable(Diagnostic851);
  }
}

IETValidator::~IETValidator()
{
  if (_diag)
  {
    _diag->flushMsg();
    _diag = nullptr;
  }
}

bool
IETValidator::validate(const PricingTrx& trx, const Itin& itin, std::string& message)
{

  CarrierCode validatingCarrier = itin.validatingCarrier();

  if (_diag && !_diagHeaderAdded)
  {
    _diag->printIETHeader(validatingCarrier);
    _diagHeaderAdded = true;
  }

  // Check IET status Active
  CrsCode crs = trx.getRequest()->ticketingAgent()->cxrCode();

  const InterlineTicketCarrierStatus* interlineTicketCarrierStatus =
      trx.dataHandle().getInterlineTicketCarrierStatus(validatingCarrier, crs);

  if (interlineTicketCarrierStatus == nullptr || interlineTicketCarrierStatus->status() != 'A')
  {

    message = validatingCarrier + " HAS NO TICKETING AGREEMENT-CHANGE VALIDATING CARRIER";
    if (_diag)
    {
      if (interlineTicketCarrierStatus == nullptr)
      {
        *_diag << " IET PROFILE NOT AVAILABLE \n";
        *_diag << message << std::endl;
      }
      else if (interlineTicketCarrierStatus->status() == 'D')
      {
        *_diag << "CRS " << crs << " - DEACTIVATED \n ";
        *_diag << message << std::endl;
      }
      else if (interlineTicketCarrierStatus->status() == 'N')
      {
        *_diag << "CRS " << crs << " - NOT INITIALIZED \n ";
        *_diag << message << std::endl;
      }
      else if (interlineTicketCarrierStatus->status() == 'B')
      {
        *_diag << "CRS " << crs << " - BETA \n ";
        *_diag << message << std::endl;
      }
    }
    return false;
  }
  else if (_diag)
  {
    *_diag << "CRS " << crs << " - ACTIVE \n ";
  }

  std::vector<CarrierCode> crxVec;
  std::vector<TravelSeg*>::const_iterator iter = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = itin.travelSeg().end();
  std::vector<CarrierCode>::iterator it;
  bool participating = false;
  it = crxVec.begin();
  // Loop through travel segments of Itin and save unique marketing carriers in vector

  for (; iter != iterEnd; ++iter)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iter);

    if (airSeg != nullptr)
    {
      CarrierCode& crx = airSeg->carrier();
      if (crx == validatingCarrier)
        participating = true; // set validating carrier as participating carrier
      it = find(crxVec.begin(), crxVec.end(), crx);
      if (it == crxVec.end())
        crxVec.push_back(crx);

      if (!airSeg->operatingCarrierCode().empty() &&
          airSeg->operatingCarrierCode() != airSeg->carrier())
      {
        it = find(crxVec.begin(), crxVec.end(), airSeg->operatingCarrierCode());
        if (it == crxVec.end())
          crxVec.push_back(airSeg->operatingCarrierCode());
      }
    }
  }
  if (_diag)
  {
    _diag->printItinCrxLine(crxVec);
  }

  // Retrieve Interline Ticketing Carriers agreement with the Validating carrier
  const std::vector<InterlineTicketCarrierInfo*>& interlineInfoVec =
      trx.dataHandle().getInterlineTicketCarrier(validatingCarrier, DateTime::localTime());

  if (!validateInterlineTicketCarrierAgreement(
          trx, crxVec, validatingCarrier, interlineInfoVec, participating, message))
  {
    return false;
  }

  // end of process
  return true;
}

bool
IETValidator::validateInterlineTicketCarrierAgreement(
    const PricingTrx& trx,
    std::vector<CarrierCode>& cxrVec,
    const CarrierCode& validatingCarrier,
    const std::vector<InterlineTicketCarrierInfo*>& interlineInfoVec,
    bool participating,
    std::string& message)
{
  bool rcCode = true;
  bool normalFlag = false;
  bool superPseudoFlag = false;

  // Looping through unique marketing carriers
  // check if validating carrier is participating carrier
  // match each marketing carrier against IET interline carriers to find
  // first carrier matched with Normal relation
  // first carrier matched with Hosted relation
  // first carrier matched with Pseudo relation
  // first carrier matched with SuperPseudo relation
  // first carrier not interline carrier

  std::vector<CarrierCode>::const_iterator it = cxrVec.begin();
  std::vector<CarrierCode>::const_iterator itEnd = cxrVec.end();
  for (; it != itEnd; ++it)
  {
    if (*it == validatingCarrier) // Is validating carrier participating carrier?
    {
      continue;
    }

    std::vector<InterlineTicketCarrierInfo*>::const_iterator iter = interlineInfoVec.begin();
    std::vector<InterlineTicketCarrierInfo*>::const_iterator iterEnd = interlineInfoVec.end();

    for (; iter != iterEnd; ++iter)
    {
      if ((*iter)->interlineCarrier() == *it)
        break;
    }

    if (iter == iterEnd) // Is there any carrier not interline carrier?
    {
      message = validatingCarrier + " HAS NO INTERLINE TICKETING AGREEMENT WITH " + *it;
      if (_diag)
      {
        *_diag << "\n" << message << std::endl;
      }
      rcCode = false;
      break;
    }

    if (_diag)
    {
      _diag->printIETLine((*iter));
    }

    if ((*iter)->superPseudoInterline() == 'Y') //  If any carrier with  SuperPseudo relation?
    {
      superPseudoFlag = true;
      continue;
    }
    else if ((*iter)->pseudoInterline() == 'Y') //  If any carrier with Pseudo relation?
    {
      continue;
    }
    else if ((*iter)->hostInterline() == 'Y')
    {
      continue;
    }
    else
    {
      normalFlag = true;
    }
  }
  if (rcCode)
  {
    if (participating || superPseudoFlag)
    {
      rcCode = true;
    }
    else if (normalFlag)
    {
      message = validatingCarrier + "-NO VALID TICKETING AGREEMENT-CHANGE VALIDATING CARRIER";
      if (_diag)
      {
        *_diag << "\n" << message << std::endl;
      }
      rcCode = false;
    }
  }
  if (_diag && trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL")
  {
    _diag->printAllCarriers(interlineInfoVec);
  }

  return rcCode;
}
} // namespace
