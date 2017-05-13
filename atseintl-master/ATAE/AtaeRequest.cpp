//----------------------------------------------------------------------------
//
//  File:     AtaeRequest.cpp
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2005
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have
//          been supplied.
//
//----------------------------------------------------------------------------

#include "ATAE/AtaeRequest.h"

#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/DateTime.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/XMLConstruct.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FrequentFlyerAccount.h"
#include "DataModel/Itin.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ReservationData.h"
#include "DataModel/Traveler.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"

#include <string>

namespace tse
{
namespace
{
ConfigurableValue<std::string>
asV2ShoppingServiceName("ITIN_SVC", "ASV2_SHOPPING_SERVICE");
ConfigurableValue<std::string>
asV2PricingServiceName("ITIN_SVC", "ASV2_PRICING_SERVICE", "INTLWPI1");
}
Logger
AtaeRequest::_logger("atseintl.ATAE.AtaeRequest");

const std::string AtaeRequest::ATAE_REQUEST = "ATQ";
const std::string AtaeRequest::DEFAULT_TRX_ID = "12345";
const std::string AtaeRequest::DEFAULT_ACTION_CODE = "**";
const std::string AtaeRequest::INTERFACE_VERSION = "0";

AtaeRequest::AtaeRequest(PricingTrx& trx, bool useAVS)
  : _trx(trx), _fareMarketsSentToAtae(0), _useAVS(useAVS)
{
}

void
AtaeRequest::build(std::string& request)
{
  LOG4CXX_INFO(_logger, " Entered AtaeRequest::build()");
  XMLConstruct construct;

  construct.openElement(ATAE_REQUEST);
  construct.addAttribute("Q3B", INTERFACE_VERSION.c_str());

  addXray(construct);
  addFlights(construct);
  addAsl(construct);
  addBilling(construct);
  addAgent(construct);

  if (sendResData())
    addRes(construct);

  addReq(construct);
  addDia(construct);

  construct.closeElement();

  request = construct.getXMLData();
  LOG4CXX_INFO(_logger, " Returning AtaeRequest::build()");

  return;
}

void
AtaeRequest::buildShopping(std::string& request)
{
  LOG4CXX_INFO(_logger, " Entered AtaeRequest::buildShopping()");

  std::vector<TravelSeg*> uniqueTvlSegs;
  buildUniqueTvlSegs(uniqueTvlSegs);

  if (uniqueTvlSegs.empty())
  {
    LOG4CXX_INFO(_logger, " No flights found Returning AtaeRequest::buildShopping()");
    return;
  }

  std::vector<FareMarket*> uniqueFareMkts;
  buildUniqueFareMkts(uniqueFareMkts);

  if (uniqueFareMkts.empty())
  {
    LOG4CXX_INFO(_logger, " No FareMarkets found Returning AtaeRequest::buildShopping()");
    return;
  }

  XMLConstruct construct;

  construct.openElement(ATAE_REQUEST);
  construct.addAttribute("Q3B", INTERFACE_VERSION.c_str());

  addXray(construct);
  addAsgShopping(construct, uniqueTvlSegs);
  addAslShopping(construct, uniqueTvlSegs, uniqueFareMkts);
  addBilling(construct);
  addAgent(construct);
  addDia(construct);

  construct.closeElement();

  request = construct.getXMLData();
  LOG4CXX_INFO(_logger, " Returning AtaeRequest::buildShopping()");
}

void
AtaeRequest::addXray(XMLConstruct& construct)
{
  xray::JsonMessage* jsonMessage = _trx.getXrayJsonMessage();
  if (jsonMessage == nullptr)
    return;

  construct.openElement("XRA");
  construct.addAttribute("MID", jsonMessage->generateMessageId("ASV2").c_str());
  construct.addAttribute("CID", jsonMessage->generateConversationId().c_str());
  construct.closeElement();
}

void
AtaeRequest::addFlights(XMLConstruct& construct) const
{
  if (_trx.itin().size() < 1)
    return;

  construct.openElement("ASG");

  Itin& itin = *(_trx.itin()[0]);
  const AirSeg* airSeg = nullptr;
  std::vector<TravelSeg*>::const_iterator tvlI = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = itin.travelSeg().end();

  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlI);
    if (airSeg == nullptr || (*tvlI)->segmentType() == Open || (*tvlI)->segmentType() == Arunk)
    {
      continue;
    }

    if (airSeg->flightNumber() == 0)
      continue;

    addFlt((*tvlI), construct);
  }

  construct.closeElement();
}

void
AtaeRequest::addFlt(const TravelSeg* tvlSeg, XMLConstruct& construct) const
{
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg);
  char tmpBuf[128];
  const DateTime& depart = airSeg->departureDT();
  const DateTime& arrive = airSeg->arrivalDT();

  std::string offeredBookingCodes;
  int iCos = 0;
  int sizeCos = airSeg->classOfService().size();
  if (airSeg->classOfService().empty())
  {
    offeredBookingCodes.push_back(airSeg->getBookingCode()[0]);
  }
  for (; iCos < sizeCos; iCos++)
  {
    ClassOfService& cos = *(airSeg->classOfService()[iCos]);
    offeredBookingCodes.push_back(cos.bookingCode()[0]);
  }

  construct.openElement("SEG");

  sprintf(tmpBuf, "%d", airSeg->pnrSegment());
  construct.addAttribute("Q2X", tmpBuf);
  construct.addAttribute("A01", airSeg->origAirport().c_str());
  construct.addAttribute("A02", airSeg->destAirport().c_str());

  construct.addAttribute("B00", MCPCarrierUtil::swapToPseudo(&_trx, airSeg->carrier()).c_str());

  sprintf(tmpBuf, "%d", airSeg->flightNumber());
  construct.addAttribute("Q0B", tmpBuf);
  sprintf(tmpBuf, "%lu", depart.totalMinutes());
  construct.addAttribute("D31", tmpBuf);
  sprintf(tmpBuf, "%lu", arrive.totalMinutes());
  construct.addAttribute("D32", tmpBuf);
  if (TrxUtil::isDateAdjustmentIndicatorActive(_trx) && airSeg->arrivalDayAdjust())
  {
    sprintf(tmpBuf, "%d", airSeg->arrivalDayAdjust());
    construct.addAttribute("DD2", tmpBuf);
  }
  construct.addAttribute("S1E", offeredBookingCodes.c_str());
  if (airSeg->bbrCarrier())
    construct.addAttribute("P0T", "T");
  else
    construct.addAttribute("P0T", "F");
  construct.closeElement();
}

void
AtaeRequest::addAsl(XMLConstruct& construct)
{
  if (_trx.itin().size() < 1)
    return;

  Itin& itin = *(_trx.itin()[0]);

  construct.openElement("ASL");
  FareMarket* fm = nullptr;
  std::vector<FareMarket*>::iterator fmIt = itin.fareMarket().begin();
  std::vector<FareMarket*>::iterator fmItEnd = itin.fareMarket().end();
  int asoSolId = 0;
  for (; fmIt != fmItEnd; ++fmIt)
  {
    fm = (*fmIt);
    if (stopOversArunkIncluded(*fm))
      continue;
    if (partOfFlowJourney(*fm))
      continue;
    if (duplicateFareMarket(fm))
      continue;
    if (journeyCOIncluded(itin, *fm))
      continue;
    ++asoSolId;
    addAso(*fm, construct, asoSolId);
    _fareMarketsSentToAtae.push_back(fm);
  }

  construct.closeElement();
}

void
AtaeRequest::addAso(const FareMarket& fm, XMLConstruct& construct, int asoSolId) const
{
  std::vector<TravelSeg*>::const_iterator tvlI = fm.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = fm.travelSeg().end();
  const AirSeg* airSeg = nullptr;
  // advance to the first non-ARUNK segment
  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlI);
    if (airSeg != nullptr)
      break;
  }

  const TravelSeg* firstTravelSeg = (*tvlI);
  char tmpBuf[128];

  construct.openElement("ASO");

  sprintf(tmpBuf, "%d", asoSolId);
  construct.addAttribute("Q3A", tmpBuf); // solution id
  construct.addAttribute("Q2Z", "1"); // solution validity indicator
  construct.addAttribute("D86", firstTravelSeg->departureDT().dateToString(YYYYMMDD, "").c_str());
  if (_useAVS)
  {
    construct.addAttribute("Q3D", "1"); // for soak test etc, use AVS
  }
  else
  {
    construct.addAttribute("Q3D", "0"); // use DCA
  }
  construct.addAttribute("P50", "F"); // SOLO indicator
  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlI);
    if (airSeg == nullptr || (*tvlI)->segmentType() == Open || (*tvlI)->segmentType() == Arunk)
    {
      continue;
    }
    addSgr((*tvlI), construct, firstTravelSeg);
  }

  construct.closeElement();
}

void
AtaeRequest::addSgr(const TravelSeg* tvlSeg,
                    XMLConstruct& construct,
                    const TravelSeg* firstTravelSeg) const
{
  construct.openElement("SGR");
  char tmpBuf[128];
  sprintf(tmpBuf, "%d", tvlSeg->pnrSegment());
  construct.addAttribute("Q2X", tmpBuf); // seg reference
  construct.addAttributeInteger("Q2Y",
                                daysDiff(tvlSeg->departureDT(), firstTravelSeg->departureDT()));
  construct.addAttribute("Q2Z", "1"); // validity indicator
  construct.closeElement();
}

int64_t
AtaeRequest::daysDiff(DateTime currSegDate, DateTime firstSegDate) const
{
  currSegDate = DateTime(currSegDate.year(), currSegDate.month(), currSegDate.day());
  firstSegDate = DateTime(firstSegDate.year(), firstSegDate.month(), firstSegDate.day());
  return DateTime::diffTime(currSegDate, firstSegDate) / SECONDS_PER_DAY;
}

const std::string&
AtaeRequest::getActionCode(const Billing& billing) const
{
  if (!getOverrideActionCode().empty())
    return getOverrideActionCode();

  if (!billing.actionCode().empty())
    return billing.actionCode();

  return DEFAULT_ACTION_CODE;
}

void
AtaeRequest::addBilling(XMLConstruct& construct) const
{
  const Billing& billing = *(_trx.billing());

  construct.openElement("BIL");

  construct.addAttribute("A20", billing.userPseudoCityCode().c_str());
  construct.addAttribute("Q03", billing.userStation().c_str());
  construct.addAttribute("Q02", billing.userBranch().c_str());
  construct.addAttribute("AE0", billing.partitionID().c_str());
  construct.addAttribute("AD0", billing.userSetAddress().c_str());
  construct.addAttribute("A22", billing.aaaCity().c_str());
  construct.addAttribute("AA0", billing.aaaSine().c_str());
  if (_trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (asV2ShoppingServiceName.getValue().empty())
      construct.addAttribute("C20", billing.parentServiceName().c_str());
    else
      construct.addAttribute("C20", asV2ShoppingServiceName.getValue().c_str());
  }
  else
  {
    construct.addAttribute("C20", asV2PricingServiceName.getValue().c_str());
  }
  construct.addAttribute("A70", getActionCode(billing).c_str());
  construct.addAttribute("C21", billing.clientServiceName());
  construct.addAttributeLong("C00", billing.transactionID());
  construct.addAttributeLong("C01", billing.clientTransactionID());
  construct.addAttributeLong("L00", billing.clientTransactionID()); // obsolete

  construct.closeElement();
}

void
AtaeRequest::addAgent(XMLConstruct& construct) const
{
  if (_trx.getRequest()->ticketingAgent() == nullptr)
  {
    return;
  }

  const Agent& agent = *(_trx.getRequest()->ticketingAgent());

  construct.openElement("AGI");

  construct.addAttribute("A10", agent.agentCity().c_str());

  if (!(agent.tvlAgencyPCC().empty() || agent.tvlAgencyPCC() == "HDQ"))
  {
    construct.addAttribute("A20", agent.tvlAgencyPCC().c_str());
  }

  construct.addAttribute("A21", agent.mainTvlAgencyPCC().c_str());

  std::string st;

  if (agent.tvlAgencyIATA().empty())
    construct.addAttribute("AB0", "4553728"); //@TODO why empty??
  else
  {
    st = agent.tvlAgencyIATA() + calculateIATACheckDigit(agent.tvlAgencyIATA().c_str());
    construct.addAttribute("AB0", st.c_str());
  }

  if (agent.homeAgencyIATA().empty())
    construct.addAttribute("AB1", "4553728"); //@TODO why empty??
  else
  {
    st = agent.homeAgencyIATA() + calculateIATACheckDigit(agent.homeAgencyIATA().c_str());
    construct.addAttribute("AB1", st.c_str());
  }

  if (entryFromAirlinePartition())
    construct.addAttribute("B00", _trx.billing()->partitionID().c_str());
  else
    construct.addAttribute("B00", agent.cxrCode().c_str());

  construct.addAttribute("A80", agent.airlineDept().c_str());
  construct.addAttribute("N0G", agent.agentDuty().c_str());
  construct.addAttribute("A90", agent.agentFunctions().c_str());
  construct.addAttribute("C40", agent.currencyCodeAgent().c_str());

  char tmpBuf[128];
  sprintf(tmpBuf, "%d", agent.coHostID());
  construct.addAttribute("Q01", tmpBuf);

  if (agent.officeDesignator().size() >= 5)
  {
    construct.addAttribute("A11", agent.officeDesignator().substr(0, 3));
    construct.addAttribute("B11", agent.officeDesignator().substr(3, 2));
  }

  if ((!agent.officeStationCode().empty()))
  {
    construct.addAttribute("C11", agent.officeStationCode());
  }

  if ((!agent.defaultTicketingCarrier().empty()))
  {
    construct.addAttribute("D11", agent.defaultTicketingCarrier());
  }

  construct.closeElement();
}

void
AtaeRequest::addRes(XMLConstruct& construct) const
{
  construct.openElement("RES");
  addRli(construct);
  addPax(construct);
  addPii(construct);
  addCfi(construct);
  addOci(construct);
  construct.closeElement();
}

void
AtaeRequest::addRli(XMLConstruct& construct) const
{
  const ReservationData* resData = _trx.getRequest()->reservationData();
  if (resData == nullptr)
    return;

  const uint16_t numRLS = resData->recordLocators().size();
  if (numRLS == 0)
    return;

  const RecordLocatorInfo* rl = nullptr;
  for (uint16_t i = 0; i < numRLS; ++i)
  {
    rl = resData->recordLocators()[i];
    if (rl == nullptr)
      continue;

    construct.openElement("RLI");
    construct.addAttribute("B00", rl->originatingCxr().c_str());
    construct.addAttribute("S15", rl->recordLocator().c_str());
    construct.closeElement();
  }
}

void
AtaeRequest::addDia(XMLConstruct& construct) const
{
  const DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
  if (!_trx.diagnostic().isActive())
    return;
  if (!(diagType == Diagnostic998 || diagType == Diagnostic195))
    return;

  std::map<std::string, std::string>::iterator endI = _trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::iterator beginI =
      _trx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);

  if (beginI == endI)
    return;

  const std::string& diagParam = (*beginI).second;
  if (diagParam.empty())
    return;
  if (isalpha(diagParam[0]))
    return;

  construct.openElement("DIA");
  construct.addAttribute("Q0A", diagParam);
  construct.closeElement();
}

void
AtaeRequest::addPax(XMLConstruct& construct) const
{
  const ReservationData* resData = _trx.getRequest()->reservationData();
  if (resData == nullptr)
    return;

  std::vector<std::string> lastNames;
  lastNames.resize(0);
  uint16_t numLastNames = 0;
  uint16_t iLastNames = 0;
  bool lastNameFound = false;
  char tmpBuf[128];
  std::string tmpString;
  uint16_t numPartnerCxrs = 0;
  uint16_t iPartnerCxrs = 0;

  const uint16_t numPax = resData->passengers().size();
  if (numPax == 0)
    return;

  const Traveler* pax = nullptr;
  const FrequentFlyerAccount* freqFlyerAcct = nullptr;

  for (uint16_t i = 0; i < numPax; ++i)
  {
    pax = resData->passengers()[i];
    if (pax == nullptr)
      continue;
    if (i == 0)
    {
      lastNames.push_back(pax->lastName());
      continue;
    }

    // check if we already have this last Name
    lastNameFound = false;
    numLastNames = lastNames.size();
    for (iLastNames = 0; iLastNames < numLastNames; iLastNames++)
    {
      if (pax->lastName() == lastNames[iLastNames])
      {
        lastNameFound = true;
        break;
      }
    }
    if (!lastNameFound)
      lastNames.push_back(pax->lastName());
  } // end for

  numLastNames = lastNames.size();
  for (iLastNames = 0; iLastNames < numLastNames; iLastNames++)
  {
    construct.openElement("PAX");
    construct.addAttribute("S16", "CP");
    construct.addAttribute("S17", lastNames[iLastNames].c_str());

    // first create all the TVL tags
    for (uint16_t i = 0; i < numPax; ++i)
    {
      pax = resData->passengers()[i];
      if (pax == nullptr)
        continue;
      if (pax->lastName() != lastNames[iLastNames])
        continue;

      construct.openElement("TVL");
      sprintf(tmpBuf, "%d", pax->referenceNumber());
      construct.addAttribute("Q2S", tmpBuf);
      construct.addAttribute("S16", pax->firstNameQualifier().c_str());
      if (pax->travelWithInfant())
        construct.addAttribute("N0T", "Y");
      else
        construct.addAttribute("N0T", "N");
      construct.addAttribute("S18", pax->firstName().c_str());
      construct.addAttribute("S19", pax->otherName().c_str());
      construct.closeElement(); // TVL
    }

    // now create all the FQF tags
    for (uint16_t i = 0; i < numPax; ++i)
    {
      pax = resData->passengers()[i];
      if (pax == nullptr)
        continue;
      if (pax->lastName() != lastNames[iLastNames])
        continue;

      freqFlyerAcct = pax->freqFlyerAccount();
      if (freqFlyerAcct == nullptr)
        continue;

      construct.openElement("FQF");

      construct.openElement("PFF");

      sprintf(tmpBuf, "%d", pax->referenceNumber());
      construct.addAttribute("Q2S", tmpBuf);
      tmpString.resize(0);
      tmpString.push_back(freqFlyerAcct->vipType());
      construct.addAttribute("N0U", tmpString.c_str());

      construct.addAttribute(
          "B00",
          MCPCarrierUtil::swapToPseudo(&_trx, freqFlyerAcct->carrier()).c_str()); // MCP project
      // construct.addAttribute("B00", freqFlyerAcct->carrier().c_str());

      construct.addAttribute("S1A", freqFlyerAcct->accountNumber().c_str());
      construct.addAttribute("FTL", freqFlyerAcct->tierLevel().c_str());

      construct.closeElement(); // PFF

      numPartnerCxrs = freqFlyerAcct->partner().size();
      for (iPartnerCxrs = 0; iPartnerCxrs < numPartnerCxrs; iPartnerCxrs++)
      {
        construct.openElement("PCC");
        construct.addElementData(freqFlyerAcct->partner()[iPartnerCxrs].c_str(),
                                 freqFlyerAcct->partner()[iPartnerCxrs].size());
        construct.closeElement(); // PCC
      }
      construct.closeElement(); // FQF
    }

    construct.closeElement(); // PAX
  }
}

void
AtaeRequest::addPii(XMLConstruct& construct) const
{
  const ReservationData* resData = _trx.getRequest()->reservationData();
  if (resData == nullptr)
    return;
  const uint16_t flts = resData->reservationSegs().size();
  if (flts == 0)
    return;

  const ReservationSeg* resSeg = nullptr;
  char tmpBuf[128];
  std::string tmpString;

  for (uint16_t i = 0; i < flts; ++i)
  {
    resSeg = resData->reservationSegs()[i];
    if (resSeg == nullptr)
      continue;

    construct.openElement("PII");
    tmpString.resize(0);
    if (resSeg->carrier().size() >= 2)
    {
      tmpString.push_back(resSeg->carrier()[0]);
      tmpString.push_back(resSeg->carrier()[1]);
    }

    construct.addAttribute("B00",
                           MCPCarrierUtil::swapToPseudo(&_trx, tmpString).c_str()); // MCP project
    // construct.addAttribute("B00", tmpString.c_str());

    sprintf(tmpBuf, "%d", resSeg->flightNumber());
    construct.addAttribute("Q0B", tmpBuf);
    tmpString.resize(0);
    if (!(resSeg->bookingCode().empty()))
    {
      tmpString.push_back(resSeg->bookingCode()[0]);
      if (resSeg->bookingCode().size() >= 2)
      {
        if (resSeg->bookingCode()[1] == 'N')
          tmpString.push_back('N');
      }
    }
    construct.addAttribute("B30", tmpString.c_str());
    construct.addAttribute("D01", resSeg->pssDepartureDate().c_str());
    construct.addAttribute("D31", resSeg->pssDepartureTime().c_str());
    construct.addAttribute("A01", resSeg->origAirport().c_str());
    construct.addAttribute("D02", resSeg->pssArrivalDate().c_str());
    construct.addAttribute("D32", resSeg->pssArrivalTime().c_str());
    construct.addAttribute("A02", resSeg->destAirport().c_str());
    construct.addAttribute("S1B", resSeg->actionCode().c_str());
    sprintf(tmpBuf, "%d", resSeg->numInParty());
    construct.addAttribute("Q2T", tmpBuf);
    // construct.addAttribute("N0W", .c_str());
    tmpString.resize(0);
    tmpString.push_back(resSeg->marriedSegCtrlId());
    construct.addAttribute("N0X", tmpString.c_str());
    sprintf(tmpBuf, "%d", resSeg->marriedSeqNo());
    construct.addAttribute("Q2U", tmpBuf);
    sprintf(tmpBuf, "%d", resSeg->marriedGrpNo());
    construct.addAttribute("Q2W", tmpBuf);
    tmpString.resize(0);
    tmpString.push_back(resSeg->pollingInd());
    construct.addAttribute("N0Y", tmpString.c_str());
    construct.addAttribute("S1C", resSeg->eticket().c_str());
    construct.closeElement();
  }
}

void
AtaeRequest::addCfi(XMLConstruct& construct) const
{
  const ReservationData* resData = _trx.getRequest()->reservationData();
  if (resData == nullptr)
    return;

  const uint16_t numCorpAcct = resData->corpFreqFlyerAccounts().size();
  if (numCorpAcct == 0)
    return;
  const FrequentFlyerAccount* freqFlyerAcct = nullptr;
  for (uint16_t i = 0; i < numCorpAcct; ++i)
  {
    freqFlyerAcct = resData->corpFreqFlyerAccounts()[i];
    if (freqFlyerAcct == nullptr)
      continue;

    if (freqFlyerAcct->accountNumber().empty())
      continue;

    construct.openElement("CFI");
    if (!freqFlyerAcct->carrier().empty())
    {
      construct.addAttribute(
          "B00",
          MCPCarrierUtil::swapToPseudo(&_trx, freqFlyerAcct->carrier()).c_str()); // MCP project
      //   construct.addAttribute("B00", freqFlyerAcct->carrier().c_str());
    }
    else
    {
      construct.addAttribute("B00", "   ");
    }
    construct.addAttribute("S1A", freqFlyerAcct->accountNumber().c_str());
    construct.closeElement();
  }
}

void
AtaeRequest::addOci(XMLConstruct& construct) const
{
  const ReservationData* resData = _trx.getRequest()->reservationData();
  if (resData == nullptr)
    return;

  //<< "     OWNER: " << reservData->agent() << "\n"
  std::string agentIndicator;
  agentIndicator.resize(0);
  agentIndicator.push_back(resData->agentInd());

  construct.openElement("OCI");
  construct.addAttribute("N0Z", agentIndicator.c_str());
  construct.addAttribute("S1D", resData->agentIATA().c_str());
  construct.addAttribute("A20", resData->agentPCC().c_str());
  construct.addAttribute("A10", resData->agentCity().c_str());
  construct.addAttribute("A40", resData->agentNation().c_str());
  construct.addAttribute("AR0", resData->agentCRS().c_str());
  construct.closeElement();
}

bool
AtaeRequest::stopOversArunkIncluded(FareMarket& fm) const
{
  if (fm.travelSeg().size() < 1)
    return true;

  OAndDMarket* od = JourneyUtil::getOAndDMarketFromFM(*(_trx.itin()[0]), &fm);
  if (od && od->isJourneyByMarriage())
    return false;

  std::vector<TravelSeg*>::const_iterator tvlSegIter = fm.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEnd = fm.travelSeg().end();
  std::vector<TravelSeg*>::const_iterator tvlSegLast = tvlSegEnd - 1;
  const AirSeg* airSeg = nullptr;
  const AirSeg* firstAirSeg = nullptr;
  const AirSeg* prevAirSeg = nullptr;
  uint16_t nonArunks = 0;

  for (; tvlSegIter != tvlSegEnd; tvlSegIter++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlSegIter);
    if (airSeg == nullptr)
    {
      if ((*tvlSegIter) != nullptr)
      {
        // only multiAirport ARUNKS are considered connections
        if (!(*tvlSegIter)->arunkMultiAirportForAvailability())
          return true;
        if (tvlSegIter == fm.travelSeg().begin() || tvlSegIter == tvlSegLast)
          return true;
      }
      continue;
    }
    else
      ++nonArunks;
    if (firstAirSeg == nullptr)
      firstAirSeg = airSeg;

    if (airSeg->segmentType() == Open)
      return true;

    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
        _trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
    {
      if (!airSeg->unflown())
        return true;
    }

    if (_trx.billing()->requestPath() == SWS_PO_ATSE_PATH &&
        _trx.excTrxType() == PricingTrx::NOT_EXC_TRX && !airSeg->unflown())
    {
      return true;
    }

    if ((airSeg->carrier() != firstAirSeg->carrier()) && !fm.flowMarket())
      return true;

    if (airSeg->flightNumber() == 0)
      return true;

    if (airSeg != firstAirSeg && prevAirSeg != nullptr && !fm.flowMarket())
    {
      if ((airSeg->geoTravelType() == GeoTravelType::Domestic ||
           airSeg->geoTravelType() == GeoTravelType::Transborder) &&
          (prevAirSeg->geoTravelType() == GeoTravelType::Domestic ||
           prevAirSeg->geoTravelType() == GeoTravelType::Transborder) &&
          (firstAirSeg->geoTravelType() == GeoTravelType::Domestic ||
           firstAirSeg->geoTravelType() == GeoTravelType::Transborder))
      {
        int64_t domCnx = 14400; // domestic connection is 4 hr by default

        //!!! EXCEPTION CODE FOR NW FOR THE JOURNEYS THAT INCLUDE HAWAII/ALASKA !!!!
        // THE CONNECTION TIME FOR SUCH JOURNEYS IS 24 HRS INSTEAD OF 4 HR
        if (airSeg->carrier().equalToConst("NW"))
        {
          if (airSeg->origin()->state() == HAWAII || airSeg->origin()->state() == ALASKA ||
              airSeg->destination()->state() == HAWAII ||
              airSeg->destination()->state() == ALASKA || prevAirSeg->origin()->state() == HAWAII ||
              prevAirSeg->origin()->state() == ALASKA || firstAirSeg->origin()->state() == HAWAII ||
              firstAirSeg->origin()->state() == ALASKA ||
              firstAirSeg->destination()->state() == HAWAII ||
              firstAirSeg->destination()->state() == ALASKA)
            domCnx = 86400;
        }
        if (airSeg->isStopOverWithOutForceCnx(prevAirSeg, domCnx))
          return true;
      }
      else
      {
        if (airSeg->carrier() == SPECIAL_CARRIER_AA)
        {
          if (airSeg->isStopOverWithOutForceCnx(prevAirSeg, 46800))
            return true;
        }
        else
        {
          if (airSeg->isStopOverWithOutForceCnx(prevAirSeg, 86400))
            return true;
        }
      }
    }
    prevAirSeg = airSeg;
  }
  // maximum 3 segment connection only
  if (nonArunks > 3)
    return true;

  return false;
}

bool
AtaeRequest::sendResData() const
{
  return _trx.getRequest()->reservationData() != nullptr;
}

void
AtaeRequest::buildUniqueTvlSegs(std::vector<TravelSeg*>& uniqueTvlSegs) const
{
  std::vector<TravelSeg*>::iterator uTvlI = uniqueTvlSegs.begin();
  std::vector<TravelSeg*>::iterator uTvlE = uniqueTvlSegs.end();
  std::vector<TravelSeg*>::iterator tvlI;
  std::vector<TravelSeg*>::iterator tvlE;
  AirSeg* airSeg = nullptr;
  bool dupeFlight = false;
  std::vector<Itin*>::iterator iI = _trx.itin().begin();
  std::vector<Itin*>::iterator iE = _trx.itin().end();
  for (; iI != iE; iI++)
  {
    Itin& itin = *(*iI);

    if ((_trx.getOptions()->callToAvailability() == 'T') || (itin.dcaSecondCall()))
    {
      tvlI = itin.travelSeg().begin();
      tvlE = itin.travelSeg().end();

      for (; tvlI != tvlE; tvlI++)
      {
        airSeg = dynamic_cast<AirSeg*>(*tvlI);
        if (airSeg == nullptr || (*tvlI)->segmentType() == Open || (*tvlI)->segmentType() == Arunk)
        {
          continue;
        }

        uTvlI = uniqueTvlSegs.begin();
        uTvlE = uniqueTvlSegs.end();
        dupeFlight = false;

        for (; uTvlI != uTvlE; uTvlI++)
        {
          if (sameFlight(*uTvlI, *tvlI))
          {
            dupeFlight = true;
            break;
          }
        }
        if (!dupeFlight)
          uniqueTvlSegs.push_back(*tvlI);
      }
    }
  }
}

namespace
{
struct UniqueFareMarket
{
  UniqueFareMarket(const AtaeRequest& ar, const FareMarket* fm) : _ar(ar), _fm(fm) {}

  bool operator()(const FareMarket* fm) const
  {
    if (fm->travelSeg().size() != _fm->travelSeg().size())
      return false;

    for (size_t index = 0; index < fm->travelSeg().size(); index++)
    {
      const AirSeg* airSeg1 = dynamic_cast<const AirSeg*>(_fm->travelSeg()[index]);
      const AirSeg* airSeg2 = dynamic_cast<const AirSeg*>(fm->travelSeg()[index]);

      if (airSeg1 == nullptr || airSeg2 == nullptr)
      {
        if (airSeg1 != airSeg2)
          return false;
        else
          continue;
      }

      if (airSeg1->segmentType() != airSeg2->segmentType())
        return false;
      else if (airSeg1->segmentType() == Open || airSeg1->segmentType() == Arunk)
        continue;

      if (!_ar.sameFlight(_fm->travelSeg()[index], fm->travelSeg()[index]))
        return false;
    }

    return true;
  }

  const AtaeRequest& _ar;
  const FareMarket* _fm;
};

} // tse

void
AtaeRequest::buildUniqueFareMkts(std::vector<FareMarket*>& uniqueFareMkts) const
{
  std::vector<FareMarket*>::iterator uFmI = uniqueFareMkts.begin();
  std::vector<FareMarket*>::iterator uFmE = uniqueFareMkts.end();
  std::vector<FareMarket*>::iterator fmI;
  std::vector<FareMarket*>::iterator fmE;
  std::vector<Itin*>::iterator iI = _trx.itin().begin();
  std::vector<Itin*>::iterator iE = _trx.itin().end();
  for (; iI != iE; iI++)
  {
    Itin& itin = *(*iI);
    if ((_trx.getOptions()->callToAvailability() == 'T') || (itin.dcaSecondCall()))
    {
      fmI = itin.fareMarket().begin();
      fmE = itin.fareMarket().end();

      for (; fmI != fmE; fmI++)
      {
        FareMarket& fm = *(*fmI);
        if (fm.travelSeg().empty())
          continue;
        uFmE = uniqueFareMkts.end();
        uFmI = find_if(uniqueFareMkts.begin(), uFmE, UniqueFareMarket(*this, *fmI));

        if (uFmI != uFmE)
          continue;

        uniqueFareMkts.push_back(*fmI);
      }
    }
  }
}

void
AtaeRequest::addAsgShopping(XMLConstruct& construct, std::vector<TravelSeg*>& uniqueTvlSegs) const
{
  construct.openElement("ASG");

  std::vector<TravelSeg*>::iterator uTvlI = uniqueTvlSegs.begin();
  std::vector<TravelSeg*>::iterator uTvlE = uniqueTvlSegs.end();
  uint16_t iTvlSeg = 1;
  AirSeg* airSeg = nullptr;
  char tmpBuf[128];
  std::string offeredBookingCodes;
  int iCos = 0;
  int sizeCos = 0;
  for (; uTvlI != uTvlE; uTvlI++, iTvlSeg++)
  {
    airSeg = dynamic_cast<AirSeg*>(*uTvlI);

    if (airSeg == nullptr)
      continue;

    if (airSeg->flightNumber() == 0)
      continue;

    DateTime& depart = airSeg->departureDT();
    DateTime& arrive = airSeg->arrivalDT();

    offeredBookingCodes.resize(0);
    if (airSeg->classOfService().empty())
    {
      if (!(airSeg->getBookingCode().empty()))
        offeredBookingCodes.push_back(airSeg->getBookingCode()[0]);
    }

    sizeCos = airSeg->classOfService().size();
    for (iCos = 0; iCos < sizeCos; iCos++)
    {
      ClassOfService& cos = *(airSeg->classOfService()[iCos]);
      if (!cos.bookingCode().empty())
        offeredBookingCodes.push_back(cos.bookingCode()[0]);
    }

    construct.openElement("SEG");

    sprintf(tmpBuf, "%d", iTvlSeg);
    construct.addAttribute("Q2X", tmpBuf);
    construct.addAttribute("A01", airSeg->origAirport().c_str());
    construct.addAttribute("A02", airSeg->destAirport().c_str());
    construct.addAttribute("B00", airSeg->carrier().c_str());
    sprintf(tmpBuf, "%d", airSeg->flightNumber());
    construct.addAttribute("Q0B", tmpBuf);
    sprintf(tmpBuf, "%lu", depart.totalMinutes());
    construct.addAttribute("D31", tmpBuf);
    sprintf(tmpBuf, "%lu", arrive.totalMinutes());
    construct.addAttribute("D32", tmpBuf);
    construct.addAttribute("S1E", offeredBookingCodes.c_str());
    if (airSeg->bbrCarrier())
      construct.addAttribute("P0T", "T");
    else
      construct.addAttribute("P0T", "F");
    construct.closeElement();
  }

  construct.closeElement();
}

void
AtaeRequest::addAslShopping(XMLConstruct& construct,
                            std::vector<TravelSeg*>& uniqueTvlSegs,
                            std::vector<FareMarket*>& uniqueFareMkts)
{
  construct.openElement("ASL");
  std::vector<FareMarket*>::iterator uFmI = uniqueFareMkts.begin();
  std::vector<FareMarket*>::iterator uFmE = uniqueFareMkts.end();
  uint16_t iFm = 0;
  FareMarket* fm = nullptr;
  for (; uFmI != uFmE; uFmI++)
  {
    fm = *uFmI;
    if (stopOversArunkIncluded(*fm))
      continue;
    if (partOfFlowJourneyShopping(*fm, uniqueFareMkts))
      continue;
    if (duplicateFareMarket(fm))
      continue;
    ++iFm;

    addAsoShopping(*fm, construct, iFm, uniqueTvlSegs);
    _fareMarketsSentToAtae.push_back(fm);
  }
  construct.closeElement();
}

void
AtaeRequest::addAsoShopping(FareMarket& fm,
                            XMLConstruct& construct,
                            uint16_t asoSolId,
                            std::vector<TravelSeg*>& uniqueTvlSegs) const
{
  std::vector<TravelSeg*>::iterator tvlI = fm.travelSeg().begin();
  std::vector<TravelSeg*>::iterator tvlE = fm.travelSeg().end();
  AirSeg* airSeg = nullptr;
  // advance to the first non-ARUNK segment
  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<AirSeg*>(*tvlI);
    if (airSeg != nullptr)
      break;
  }

  TravelSeg* firstTravelSeg = (*tvlI);
  char tmpBuf[128];

  construct.openElement("ASO");

  sprintf(tmpBuf, "%d", asoSolId);
  construct.addAttribute("Q3A", tmpBuf); // solution id
  construct.addAttribute("Q2Z", "1"); // solution validity indicator
  construct.addAttribute("D86", firstTravelSeg->departureDT().dateToString(YYYYMMDD, "").c_str());
  construct.addAttribute("Q3D", "0"); // avail type
  construct.addAttribute("P50", "F"); // SOLO indicator
  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<AirSeg*>(*tvlI);
    if (airSeg == nullptr || (*tvlI)->segmentType() == Open || (*tvlI)->segmentType() == Arunk)
    {
      continue;
    }
    addSgrShopping((*tvlI), construct, firstTravelSeg, uniqueTvlSegs);
  }

  construct.closeElement();
}

void
AtaeRequest::addSgrShopping(TravelSeg* tvlSeg,
                            XMLConstruct& construct,
                            TravelSeg* firstTravelSeg,
                            std::vector<TravelSeg*>& uniqueTvlSegs) const
{
  uint16_t iTvlSeg = indexTvlSeg(tvlSeg, uniqueTvlSegs);
  if (iTvlSeg == 0)
    return;

  construct.openElement("SGR");
  char tmpBuf[128];

  sprintf(tmpBuf, "%d", iTvlSeg);
  construct.addAttribute("Q2X", tmpBuf); // seg reference
  construct.addAttributeInteger("Q2Y",
                                daysDiff(tvlSeg->departureDT(), firstTravelSeg->departureDT()));
  construct.addAttribute("Q2Z", "1"); // validity indicator
  construct.closeElement();
  return;
}

uint16_t
AtaeRequest::indexTvlSeg(TravelSeg* tvlSeg, std::vector<TravelSeg*>& uniqueTvlSegs) const
{
  uint16_t iTvl = 0;
  uint16_t nTvl = uniqueTvlSegs.size();
  bool tvlSegFound = false;
  for (; iTvl < nTvl; iTvl++)
  {
    if (sameFlight(uniqueTvlSegs[iTvl], tvlSeg))
    {
      tvlSegFound = true;
      break;
    }
  }

  if (!tvlSegFound)
    return 0;

  return iTvl + 1;
}

bool
AtaeRequest::partOfFlowJourney(const FareMarket& fm) const
{
  if (!(_trx.getOptions()->journeyActivatedForPricing()))
    return false;

  if (!(_trx.getOptions()->applyJourneyLogic()))
    return false;

  const Itin& itin = *_trx.itin().front();

  if (fm.flowMarket())
    return false;

  return JourneyUtil::checkIfFmSegInFlowOd(&fm, itin.segmentOAndDMarket());
}

bool
AtaeRequest::partOfFlowJourneyShopping(const FareMarket& fm,
                                       std::vector<FareMarket*>& uniqueFareMkts) const
{
  if (!(_trx.getOptions()->journeyActivatedForShopping()))
    return false;

  if (!(_trx.getOptions()->applyJourneyLogic()))
    return false;

  if (!allFlowJourneyCarriers(fm))
    return false;

  if (!segFoundInOtherJourney(fm, uniqueFareMkts))
    return false;

  return true;
}

bool
AtaeRequest::allFlowJourneyCarriers(const FareMarket& fm) const
{
  if (fm.flowMarket())
    return false;

  std::vector<TravelSeg*>::const_iterator tvlI = fm.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = fm.travelSeg().end();

  for (; tvlI != tvlE; tvlI++)
  {
    if (*tvlI == nullptr || dynamic_cast<AirSeg*>(*tvlI) == nullptr)
      continue;
    const AirSeg& airSeg = *(dynamic_cast<AirSeg*>(*tvlI));
    if (!airSeg.flowJourneyCarrier())
      return false;
  }
  return true;
}

bool
AtaeRequest::segFoundInOtherJourney(const FareMarket& fm,
                                    const std::vector<FareMarket*>& otherFareMkts) const
{
  std::vector<TravelSeg*>::const_iterator tvlI = fm.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = fm.travelSeg().end();

  std::vector<FareMarket*>::const_iterator fmI = otherFareMkts.begin();
  const std::vector<FareMarket*>::const_iterator fmE = otherFareMkts.end();

  std::vector<TravelSeg*>::const_iterator tvlISearch;
  std::vector<TravelSeg*>::const_iterator tvlESearch;

  for (; fmI != fmE; ++fmI)
  {
    const FareMarket& fmSearch = *(*fmI);
    if (!fmSearch.flowMarket())
      continue;

    tvlI = fm.travelSeg().begin();
    tvlESearch = fmSearch.travelSeg().end();
    for (; tvlI != tvlE; tvlI++)
    {
      tvlISearch = fmSearch.travelSeg().begin();
      for (; tvlISearch != tvlESearch; tvlISearch++)
      {
        if ((*tvlI) == (*tvlISearch))
          return true;
      }
    }
  }
  return false;
}

std::string
AtaeRequest::calculateIATACheckDigit(const char* iataNumber) const
{
  uint64_t checkdigit;
  uint8_t mod = 7;
  uint64_t iatanum = atoi(iataNumber);

  if (iatanum == 9999999)
  {
    checkdigit = 9;
  }
  else
  {
    div_t divresult;
    divresult = div(iatanum, mod);
    checkdigit = divresult.rem;
  }

  char buf[10];
  sprintf(buf, "%lu", checkdigit);
  std::string str(buf);
  return str;
}

bool
AtaeRequest::duplicateFareMarket(FareMarket* fm)
{
  if (_fareMarketsSentToAtae.empty())
    return false;

  std::vector<FareMarket*>::iterator fmB = _fareMarketsSentToAtae.begin();
  std::vector<FareMarket*>::iterator fmE = _fareMarketsSentToAtae.end();
  std::vector<FareMarket*>::iterator fmI = find(fmB, fmE, fm);

  if (fmI != fmE)
    return true;

  for (fmI = fmB; fmI != fmE; fmI++)
  {
    const FareMarket& fmInVec = *(*fmI);
    if (fmInVec.travelSeg() == fm->travelSeg())
      return true;
  }

  return false;
}

bool
AtaeRequest::sameFlight(const TravelSeg* tvlSeg1, const TravelSeg* tvlSeg2)
{
  const AirSeg* airSeg1 = dynamic_cast<const AirSeg*>(tvlSeg1);
  const AirSeg* airSeg2 = dynamic_cast<const AirSeg*>(tvlSeg2);

  if (airSeg1->flightNumber() != airSeg2->flightNumber())
    return false;

  if (airSeg1->carrier() != airSeg2->carrier())
    return false;

  if (airSeg1->origAirport() != airSeg2->origAirport())
    return false;

  if (airSeg1->destAirport() != airSeg2->destAirport())
    return false;

  if (airSeg1->operatingCarrierCode() != airSeg2->operatingCarrierCode())
    return false;

  if (airSeg1->operatingFlightNumber() != airSeg2->operatingFlightNumber())
    return false;

  if (airSeg1->departureDT() != airSeg2->departureDT())
    return false;

  if (airSeg1->arrivalDT() != airSeg2->arrivalDT())
    return false;

  return true;
}

bool
AtaeRequest::journeyCOIncluded(Itin& itin, FareMarket& fm) const
{
  if (fm.travelSeg().size() != 3)
    return false;

  if (fm.flowMarket())
    return false;

  if (!(_trx.getOptions()->journeyActivatedForPricing()))
    return false;

  if (!(_trx.getOptions()->applyJourneyLogic()))
    return false;

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(fm.travelSeg().front());
  if (airSeg == nullptr || !airSeg->carrier().equalToConst("CO"))
    return false;

  std::vector<TravelSeg*> journeySegs;
  journeySegs.push_back(fm.travelSeg()[0]);
  journeySegs.push_back(fm.travelSeg()[1]);
  FareMarket* journeyFm = ItinUtil::findMarket(itin, journeySegs);
  if (journeyFm && journeyFm->flowMarket())
    return true;

  journeySegs.clear();
  journeySegs.push_back(fm.travelSeg()[1]);
  journeySegs.push_back(fm.travelSeg()[2]);
  journeyFm = ItinUtil::findMarket(itin, journeySegs);
  if (journeyFm && journeyFm->flowMarket())
    return true;
  return false;
}

bool
AtaeRequest::entryFromAirlinePartition() const
{
  if (_trx.getRequest()->ticketingAgent() == nullptr ||
      _trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
  {
    if (!(_trx.billing()->partitionID().empty()) && _trx.billing()->aaaCity().size() < 4)
      return true;
  }
  return false;
}

void
AtaeRequest::addReq(XMLConstruct& construct) const
{
  if (!_trx.getRequest())
    return;
  ReservationData* resData = _trx.getRequest()->reservationData();
  if (resData && !(resData->pocAirport().empty()))
  {
    construct.openElement("REQ");
    addPoc(construct, resData);
    construct.closeElement();
  }
}

void
AtaeRequest::addPoc(XMLConstruct& construct, ReservationData* resData) const
{
  construct.openElement("POC");
  construct.addAttribute("A03", resData->pocAirport().c_str());
  if (resData->pocDepartureDate().isValid())
  {
    construct.addAttribute("D01", resData->pocDepartureDate().dateToString(YYYYMMDD, "").c_str());
    if (!resData->pocDepartureTime().empty())
    {
      construct.addAttribute("D02", resData->pocDepartureTime().c_str());
    }
  }
  construct.closeElement();
}
} // tse
