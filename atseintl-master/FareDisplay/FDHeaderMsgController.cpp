//----------------------------------------------------------------------------
//  File: FDHeaderMsgController.cpp
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

#include "FareDisplay/FDHeaderMsgController.h"

#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FDHeaderMsg.h"
#include "FareDisplay/FDHeaderMsgText.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.FDHeaderMsgController");

const Indicator FDHeaderMsgController::SINGLE_CARRIER_ENTRY = 'S';
const Indicator FDHeaderMsgController::MULTI_CARRIER_ENTRY = 'M';

FDHeaderMsgController::FDHeaderMsgController(FareDisplayTrx& trx,
                                             const std::set<CarrierCode>& preferredCarriers)
  : FDCustomerRetriever(trx), _preferredCarriers(preferredCarriers)
{
}

struct EqualByItemNumber : public std::unary_function<const uint64_t, bool>
{
public:
  EqualByItemNumber(const uint64_t itemNumber) : _itemNumber(itemNumber) {}

  bool operator()(const FDHeaderMsg*& headerMsg) const
  {
    return (headerMsg->msgItemNo() == _itemNumber);
  }

private:
  const uint64_t _itemNumber;
};

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDHeaderMsgController::isEliminateOnFareDisplayType()
//
// Verifies whether we need to eliminate a header message based on
// FareDisplayType
//
// @param	std::vector<const tse::FDHeaderMsg*>&	- HeaderMsg	list retrieved from database.
// @param	FareDisplayTrx&				- Transaction Object
//
// @return  bool			- If matching found then return false. Otherwise true.
//
// </PRE>
// -----------------------------------------------------------

bool
FDHeaderMsgController::isEliminateOnFareDisplayType(const Indicator& fdTypeEntry,
                                                    const Indicator& fdTypeData)
{
  if ((fdTypeData == NO_PARAM) || (fdTypeEntry == fdTypeData))
  {
    return false;
  }
  return true;
}

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDHeaderMsgController::isEliminateOnLocation()
//
// Finds whether any matching founds based on
// directionality
//
// @param	std::vector<const tse::FDHeaderMsg*>&	- HeaderMsg	list retrieved from database.
// @param	FareDisplayTrx&				- Transaction Object
//
// @return  bool			- If matching found then return true. Otherwise false.
//
// </PRE>
// -----------------------------------------------------------

bool
FDHeaderMsgController::isEliminateOnLocation(const tse::FDHeaderMsg* const& fdHeaderMsg,
                                             const Loc& origin,
                                             const Loc& destination)
{
  const Directionality& dir = fdHeaderMsg->directionality();

  bool ret = false;

  if (dir == WITHIN)
    ret = LocUtil::isWithin(fdHeaderMsg->loc1(), origin, destination);

  if (dir == FROM)
    ret = LocUtil::isFrom(fdHeaderMsg->loc1(), fdHeaderMsg->loc2(), origin, destination);

  if (dir == BETWEEN)
    ret = LocUtil::isBetween(fdHeaderMsg->loc1(), fdHeaderMsg->loc2(), origin, destination);

  if (dir == ORIGIN)
    ret = LocUtil::isInLoc(origin.loc(),
                           fdHeaderMsg->loc1().locType(),
                           fdHeaderMsg->loc1().loc(),
                           Vendor::SABRE,
                           MANUAL);

  if (dir == TERMINATE)
    ret = LocUtil::isInLoc(destination.loc(),
                           fdHeaderMsg->loc2().locType(),
                           fdHeaderMsg->loc2().loc(),
                           Vendor::SABRE,
                           MANUAL);

  bool isMatch = ((fdHeaderMsg->exceptLoc() == YES) ? (!ret) : ret);

  return !isMatch;
}

bool
FDHeaderMsgController::isEliminateOnCarrier(const tse::FDHeaderMsg* const& fdHeaderMsg)
{
  if (_preferredCarriers.empty())
    return false;

  if (fdHeaderMsg->carrier().empty())
    return false;

  // ------------------
  // If Single carrier entry then look at requested carrier
  // ------------------
  if (_trx.isShopperRequest() == false)
  {
    if (_trx.requestedCarrier() == fdHeaderMsg->carrier())
      return false;
    return true;
  }

  // ------------------------------
  // Otherwise iterate through each carrier in preferred list
  // ------------------------------
  std::set<CarrierCode>::const_iterator cB = _preferredCarriers.begin();
  std::set<CarrierCode>::const_iterator cE = _preferredCarriers.end();

  for (; cB != cE; cB++)
  {
    if (fdHeaderMsg->carrier() == (*cB))
      return false;
  }
  return true;
}

bool
FDHeaderMsgController::isEliminateOnRouting(const tse::FDHeaderMsg* const& fdHeaderMsg)
{
  if (fdHeaderMsg->routing1().empty())
    return false;

  std::vector<PaxTypeFare*>::iterator itr = _trx.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator itrEnd = _trx.allPaxTypeFare().end();

  // If routing1 = not empty , routing2 = empty
  if (fdHeaderMsg->routing2().empty())
  {
    for (; itr != itrEnd; itr++)
    {
      if ((*itr)->isValid() && fdHeaderMsg->routing1() == (*itr)->routingNumber())
        return false;
    }
    return true;
  }

  // If routing1 = not empty , routing2 = not empty
  int16_t routing1, routing2;
  routing1 = atoi(fdHeaderMsg->routing1().c_str());
  routing2 = atoi(fdHeaderMsg->routing2().c_str());

  for (; itr != itrEnd; itr++)
  {
    if (!((*itr)->isValid()))
      continue;

    int16_t routing = atoi((*itr)->routingNumber().c_str());

    if (routing1 <= routing && routing <= routing2)
      return false;
  }
  return true;
}

bool
FDHeaderMsgController::isEliminateOnGlobalDirection(const tse::FDHeaderMsg* const& fdHeaderMsg)
{
  std::string gDir;
  globalDirectionToStr(gDir, fdHeaderMsg->globalDir());

  if (gDir.empty())
    return false;

  std::vector<PaxTypeFare*>::iterator itr = _trx.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator itrEnd = _trx.allPaxTypeFare().end();

  for (; itr != itrEnd; itr++)
  {
    if (!(*itr)->isValid() || !(*itr)->isGlobalDirectionValid())
      continue;
    if ((*itr)->globalDirection() == fdHeaderMsg->globalDir())
      return false;
  }
  return true;
}

bool
FDHeaderMsgController::isEliminateOnPosLoc(const tse::FDHeaderMsg* const& fdHeaderMsg)
{
  if (!_trx.getRequest()->ticketingAgent())
  {
    LOG4CXX_DEBUG(logger, "ERROR: No ticketingAgent Found in request object ");
    return false;
  }
  Loc agentLocation;

  if (_trx.getRequest()->salePointOverride().empty())
  {
    const Loc* loc = _trx.getRequest()->ticketingAgent()->agentLocation();

    if (!loc)
    {
      LOG4CXX_FATAL(logger, "ERROR: No ticketingAgent location Found in request object ");
      return false;
    }

    agentLocation = *loc;
  }
  else
  {
    const Loc* loc =
        _trx.dataHandle().getLoc(_trx.getRequest()->salePointOverride(), _trx.travelDate());

    if (!loc)
    {
      LOG4CXX_FATAL(logger, "ERROR: No SalesPoint overried Agent location Found ");
      return false;
    }

    agentLocation = *loc;
  }

  const Indicator& exceptPosLoc = fdHeaderMsg->exceptPosLoc();
  LocKey locKey;
  locKey.loc() = fdHeaderMsg->posLoc();
  locKey.locType() = fdHeaderMsg->posLocType();
  if (exceptPosLoc == YES)
    return LocUtil::isWithin(locKey, agentLocation, agentLocation);

  if (exceptPosLoc == NO)
    return !LocUtil::isWithin(locKey, agentLocation, agentLocation);

  return false;
}

bool
FDHeaderMsgController::isEliminateOnCurrency(const tse::FDHeaderMsg* const& fdHeaderMsg)
{
  if (!_trx.getRequest()->ticketingAgent())
  {
    LOG4CXX_DEBUG(logger, "ERROR: No ticketingAgent Found in request object ");
    return false;
  }

  CurrencyCode displayCurrency = _trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  // Check Override Currency for Target

  if (!_trx.getOptions()->currencyOverride().empty())
    displayCurrency = _trx.getOptions()->currencyOverride();

  if (fdHeaderMsg->exceptCur() == YES)
  {
    if (displayCurrency == fdHeaderMsg->cur())
      return true;
  }

  if (fdHeaderMsg->exceptCur() == NO)
  {
    if (!fdHeaderMsg->cur().empty() && displayCurrency != fdHeaderMsg->cur())
      return true;
  }
  return false;
}

bool
FDHeaderMsgController::isEliminateOnInclusionCode(const tse::FDHeaderMsg* const& fdHeaderMsg,
                                                  const InclusionCode& inclusionCode)
{
  if (fdHeaderMsg->inclusionCode().empty())
    return false;
  if (inclusionCode == fdHeaderMsg->inclusionCode())
    return false;

  return true;
}

bool
FDHeaderMsgController::isEliminateOnBufferZone(const tse::FDHeaderMsg* const& fdHeaderMsg,
                                               const Loc& origin,
                                               const Loc& destination)
{
  // -----------------------------------
  // Both Inside and Outside = NO -> Doesn't apply
  // -----------------------------------
  if (fdHeaderMsg->insideBufferZone() == NO && fdHeaderMsg->outsideBufferZone() == NO)
    return false;

  // ----------------------------------------------------
  // Is origin / destination is in US ( Pueto Rico, Vigin Islands not included )
  // ----------------------------------------------------
  bool isOriginInUS = LocUtil::isUS(origin) && !LocUtil::isUSTerritoryOnly(origin);
  bool isDestinationInUS = LocUtil::isUS(destination) && !LocUtil::isUSTerritoryOnly(destination);

  // ------------------------------------
  // Is Origin / Destination inside Canada Buffer Zone
  // ------------------------------------
  bool isOriginInsideCABufferZone = (origin.bufferZoneInd() && LocUtil::isCanada(origin));
  bool isDestinationInsideCABufferZone =
      (destination.bufferZoneInd() && LocUtil::isCanada(destination));

  // ------------------------------------
  // Is Origin / Destination inside Mexico Buffer Zone
  // ------------------------------------
  bool isOriginInsideMXBufferZone = (origin.bufferZoneInd() && LocUtil::isMexico(origin));
  bool isDestinationInsideMXBufferZone =
      (destination.bufferZoneInd() && LocUtil::isMexico(destination));

  if (fdHeaderMsg->insideBufferZone() == YES)
  {
    switch (fdHeaderMsg->directionality())
    {
    default:
    {
      // US to/from CA Buffer cities
      if (isOriginInUS && isDestinationInsideCABufferZone)
        return false;
      if (isOriginInsideCABufferZone && isDestinationInUS)
        return false;

      // CA Buffer cities to/from CA Buffer cities
      if (isOriginInsideCABufferZone && isDestinationInsideCABufferZone)
        return false;

      // US to/from MX Buffer cities
      if (isOriginInUS && isDestinationInsideMXBufferZone)
        return false;
      if (isOriginInsideMXBufferZone && isDestinationInUS)
        return false;

      // MX Buffer cities to/from MX Buffer cities
      if (isOriginInsideMXBufferZone && isDestinationInsideMXBufferZone)
        return false;

      // CA Buffer cities to/from MX Buffer cities
      if (isOriginInsideCABufferZone && isDestinationInsideMXBufferZone)
        return false;
      if (isOriginInsideMXBufferZone && isDestinationInsideCABufferZone)
        return false;

      return true;
    }
    } // End of switch
  }

  if (fdHeaderMsg->outsideBufferZone() == YES)
  {
    bool isOriginOutsideCABufferZone = !isOriginInsideCABufferZone;
    bool isDestinationOutsideCABufferZone = !isDestinationInsideCABufferZone;
    bool isOriginOutsideMXBufferZone = !isOriginInsideMXBufferZone;
    bool isDestinationOutsideMXBufferZone = !isDestinationInsideMXBufferZone;

    switch (fdHeaderMsg->directionality())
    {
    default:
    {
      // US to/from CA Non-Buffer cities
      if (isOriginInUS && isDestinationOutsideCABufferZone)
        return false;
      if (isOriginOutsideCABufferZone && isDestinationInUS)
        return false;

      // CA Non-Buffer cities to/from CA Non-Buffer cities
      if (isOriginOutsideCABufferZone && isDestinationOutsideCABufferZone)
        return false;

      // CA Buffer cities to/from CA Non-Buffer cities
      if (isOriginInsideCABufferZone && isDestinationOutsideCABufferZone)
        return false;
      if (isOriginOutsideCABufferZone && isDestinationInsideCABufferZone)
        return false;

      // US to/from MX Non-Buffer cities
      if (isOriginInUS && isDestinationOutsideMXBufferZone)
        return false;
      if (isOriginOutsideMXBufferZone && isDestinationInUS)
        return false;

      // MX Non-Buffer cities to/from MX Non-Buffer cities
      if (isOriginOutsideMXBufferZone && isDestinationOutsideMXBufferZone)
        return false;

      // MX Buffer cities to/from MX Non-Buffer cities
      if (isOriginInsideMXBufferZone && isDestinationOutsideMXBufferZone)
        return false;
      if (isOriginOutsideMXBufferZone && isDestinationInsideMXBufferZone)
        return false;

      // CA Non-Buffer cities to/from MX Non-Buffer cities
      if (isOriginOutsideCABufferZone && isDestinationOutsideMXBufferZone)
        return false;
      if (isOriginOutsideMXBufferZone && isDestinationOutsideCABufferZone)
        return false;

      // CA Buffer cities to/from MX Non-Buffer cities
      if (isOriginInsideCABufferZone && isDestinationOutsideMXBufferZone)
        return false;
      if (isOriginOutsideMXBufferZone && isDestinationInsideCABufferZone)
        return false;

      // CA Non-Buffer cities to MX Buffer cities
      if (isOriginOutsideCABufferZone && isDestinationInsideMXBufferZone)
        return false;
      if (isOriginInsideMXBufferZone && isDestinationOutsideCABufferZone)
        return false;

      return true;
    }
    } // End of switch
  }

  return false;
}

bool
FDHeaderMsgController::eliminateRows(
    const std::vector<const tse::FDHeaderMsg*>& fdHeaderMsgDataList,
    std::vector<const tse::FDHeaderMsg*>& fdFilteredHdrMsg)
{
  if (fdHeaderMsgDataList.empty())
  {
    LOG4CXX_INFO(logger, "Empty list. No rows elimated. Leaving eliminateRows with FALSE");
    return false;
  }

  LOG4CXX_DEBUG(logger, "Total No of Rows:" << fdHeaderMsgDataList.size());

  std::vector<const tse::FDHeaderMsg*>::const_iterator iB = fdHeaderMsgDataList.begin();

  for (; iB != fdHeaderMsgDataList.end(); iB++)
  {
    LOG4CXX_DEBUG(logger,
                  "Verifying itemNo:" << (*iB)->msgItemNo() << ", seqNo:" << (*iB)->seqNo() << ", "
                                      << (*iB)->text());

    // Check for duplicate
    if (std::find_if(fdFilteredHdrMsg.begin(),
                     fdFilteredHdrMsg.end(),
                     EqualByItemNumber((*iB)->msgItemNo())) != fdFilteredHdrMsg.end())
    {
      LOG4CXX_DEBUG(logger, "Duplicate entry");
      continue;
    }
    if (isEliminateOnFareDisplayType(_fareDisplayType, (*iB)->fareDisplayType()))
    {
      LOG4CXX_DEBUG(logger, "Eliminated on FareDisplay Type");
      continue;
    }
    if (isEliminateOnLocation(*iB, *_trx.origin(), *_trx.destination()))
    {
      LOG4CXX_DEBUG(logger, "Eliminate on Match Location ");
      continue;
    }
    if (isEliminateOnCarrier(*iB))
    {
      LOG4CXX_DEBUG(logger, "Eliminated on Carrier ");
      continue;
    }
    if (isEliminateOnRouting(*iB))
    {
      LOG4CXX_DEBUG(logger, "Eliminated on Routing");
      continue;
    }
    if (isEliminateOnGlobalDirection(*iB))
    {
      LOG4CXX_DEBUG(logger, "Eliminated on Global Direction");
      continue;
    }
    if (isEliminateOnPosLoc(*iB))
    {
      LOG4CXX_DEBUG(logger, "Eliminated on Pos Loc ");
      continue;
    }
    if (isEliminateOnCurrency(*iB))
    {
      LOG4CXX_DEBUG(logger, "Eliminated on Currency ");
      continue;
    }
    if (isEliminateOnInclusionCode(*iB, _trx.getRequest()->inclusionCode()))
    {
      LOG4CXX_DEBUG(logger, "Eliminated on Inclusion code");
      continue;
    }
    if (isEliminateOnBufferZone(*iB, *_trx.origin(), *_trx.destination()))
    {
      LOG4CXX_DEBUG(logger, "Eliminated on Buffer Zone ");
      continue;
    }

    fdFilteredHdrMsg.push_back(*iB);
  }

  LOG4CXX_INFO(logger,
               "Total " << fdFilteredHdrMsg.size()
                        << " header msgs picked up..Leaving eliminateRows with TRUE.");
  return true;
}

void
FDHeaderMsgController::addMsgText(std::vector<const tse::FDHeaderMsg*>& fdHeaderMsgDataList,
                                  std::vector<tse::FDHeaderMsgText*>& headerMsgs)
{
  LOG4CXX_INFO(logger, "Entering addMsgText");

  if (headerMsgs.size() == _maxAllowedMsg)
  {
    LOG4CXX_WARN(logger,
                 "Warning: Unable to add header msg texts.Maximum allowed no of msg "
                     << _maxAllowedMsg << " reached");
    return;
  }

  if (TrxUtil::isSupplementChargeEnabled(_trx))
    addSupplementHdrMessage(headerMsgs);

  std::vector<const tse::FDHeaderMsg*>::const_iterator iter = fdHeaderMsgDataList.begin();
  std::vector<const tse::FDHeaderMsg*>::const_iterator iterEnd = fdHeaderMsgDataList.end();

  for (; iter != iterEnd; iter++)
  {
    FDHeaderMsgText* hmt;
    // Allocate memory for FDHeaderMsg object
    _trx.dataHandle().get(hmt);

    // Assign text and startPoint values
    hmt->text() = (*iter)->text();
    hmt->startPoint() = (*iter)->startPoint();

    // Add to container
    headerMsgs.push_back(hmt);

    // Check for max allowed messages constrain
    if (headerMsgs.size() == _maxAllowedMsg)
      break;
  }
  LOG4CXX_INFO(logger, "Leaving addMsgText");
}

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDHeaderMsgController::addSupplementHdrMessage()
//
// Verifies whether we need to eliminate a header message based on
// Supplement Carrier Charge
//
// @param	std::vector<tse::FDHeaderMsgText*>&	- HeaderMsg list to be displayed
//
//
// </PRE>
// -----------------------------------------------------------

void
FDHeaderMsgController::addSupplementHdrMessage(std::vector<tse::FDHeaderMsgText*>& headerMsgs)
{
  // ------------------------------
  // iterate through each carrier in preferred list to add header for Supplement Charges
  // ------------------------------

  if (_preferredCarriers.empty() && !_supplementalCarrierHdrMsg)
    return;

  std::string carrierList = TrxUtil::supplementChargeCarrierListData(_trx);
  if (carrierList.empty())
    return;

  std::vector<CarrierCode> supplementChargeCarriersVec;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator("|");
  tokenizer tokens(carrierList, separator);
  tokenizer::iterator tokenI = tokens.begin();

  for (; tokenI != tokens.end(); ++tokenI)
    supplementChargeCarriersVec.push_back(tokenI->data());

  std::vector<PaxTypeFare*>::const_iterator ptfItr = _trx.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::const_iterator ptfEnd = _trx.allPaxTypeFare().end();
  for (; ptfItr != ptfEnd; ptfItr++)
  {
    PaxTypeFare* paxTypeFare = (*ptfItr);

    if (paxTypeFare == nullptr)
    {
      continue;
    }
    else
    {
      Indicator owrt = paxTypeFare->owrt();
      const std::vector<CarrierCode>::const_iterator itor =
          std::find(supplementChargeCarriersVec.begin(),
                    supplementChargeCarriersVec.end(),
                    paxTypeFare->carrier());
      if (itor == supplementChargeCarriersVec.end())
        continue;

      FareDisplayInfo* fareDisplayInfo = paxTypeFare->fareDisplayInfo();
      if (!fareDisplayInfo)
      {
        LOG4CXX_ERROR(logger, "No FareDisplayInfo for paxTypeFare");
        continue;
      }

      const std::vector<TaxRecord*>& taxRecordVector =
          (owrt == ONE_WAY_MAYNOT_BE_DOUBLED) ? fareDisplayInfo->taxRecordVector()
                                              : fareDisplayInfo->taxRecordOWRTFareTaxVector();

      std::vector<TaxRecord*>::const_iterator iB = taxRecordVector.begin();
      std::vector<TaxRecord*>::const_iterator iE = taxRecordVector.end();

      for (; iB != iE; ++iB)
      {
        if ((*iB)->specialProcessNo() == 64)
          return setHdrMsg(headerMsgs);
      }
    }
  }

  return;
}

void
FDHeaderMsgController::setHdrMsg(std::vector<tse::FDHeaderMsgText*>& headerMsgs)
{
  FDHeaderMsgText* hmt;
  // Allocate memory for FDHeaderMsg object
  _trx.dataHandle().get(hmt);

  // Assign text and startPoint values
  std::string fdHeaderMsg =
      TrxUtil::supplementHdrMsgFareDisplayData(_trx); // Header message read from config

  if (fdHeaderMsg.empty())
    return;

  replace(fdHeaderMsg.begin(), fdHeaderMsg.end(), '_', ' ');

  hmt->text() = fdHeaderMsg; //"CARRIER IMPOSED SURCHARGES MAY NOT BE INCLUDED UNTIL PRICED";
  hmt->startPoint() = "1";
  // Add to container
  headerMsgs.push_back(hmt);
  _supplementalCarrierHdrMsg = false;
}

bool
FDHeaderMsgController::retrieve()
{
  _fareDisplayType = (_trx.isShopperRequest() ? MULTI_CARRIER_ENTRY : SINGLE_CARRIER_ENTRY);

  _fdFilteredHdrMsg.clear();
  retrieveAll();
  return true;
}

bool
FDHeaderMsgController::retrieveData(const Indicator& userApplType,
                                    const UserApplCode& userAppl,
                                    const Indicator& pseudoCityType,
                                    const PseudoCityCode& pseudoCity,
                                    const TJRGroup& tjrGroup)
{
  const std::vector<const tse::FDHeaderMsg*>& tempMsgList = _trx.dataHandle().getHeaderMsgDataList(
      pseudoCity, pseudoCityType, userApplType, userAppl, tjrGroup, _trx.travelDate());
  if (tempMsgList.empty())
    return false;
  eliminateRows(tempMsgList, _fdFilteredHdrMsg);
  return true;
}

void
FDHeaderMsgController::getHeaderMsgs(std::vector<tse::FDHeaderMsgText*>& headerMsgs)
{
  _headerMsgs = &headerMsgs;
  _headerMsgs->clear();

  retrieve();
  addMsgText(_fdFilteredHdrMsg, *_headerMsgs);
}

std::vector<const FDHeaderMsg*>
FDHeaderMsgController::getFilteredHeaderMsgs()
{
  retrieve();

  return _fdFilteredHdrMsg;
}
}
