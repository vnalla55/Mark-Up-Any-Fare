//----------------------------------------------------------------------------
//
//  File   :  PricingDssRequest.h
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

#pragma once

#include "ATAE/PricingDssFlightKey.h"
#include "ATAE/PricingDssFlightMapBuilder.h"
#include "Common/XMLConstruct.h"

namespace tse
{
class Agent;
class Billing;
class Logger;

// class PricingDssRequest : creates XML request for the DSS service to get schedule.

class PricingDssRequest final
{
public:
  PricingDssRequest(PricingTrx& trx) : _trx(trx) {}

  // only public interface that builds the DSS request.
  // @param a reference to the PricingTrx.
  // @takes and empty request and fills up with an XML request after build operation.

  template <class FlightContainer>
  void build(const FlightContainer& flights, std::string& request) const;

private:
  static Logger _logger;
  PricingTrx& _trx;

  void addXray(XMLConstruct& construct) const;

  // Prepares the Billing Informantion for the request.
  // BIL tag.
  void addBilling(XMLConstruct& construct) const;

  // Prepares the Agent Informantion for the request.
  // UID tag.
  void addUserId(XMLConstruct& construct) const;

  /**
   *Prepares the Agent Informantion for the request.
   *@param Agent a reference to the agent object
   *@param construct a reference to the XMLConstuct.
   **/
  void addAgent(XMLConstruct& construct) const;

  // Prepares all the flights.
  // FLS tag(s).
  template <class FlightContainer>
  void addFlights(const FlightContainer& flights, XMLConstruct& construct) const;

  // Determines if the Request is a WEB request or not
  bool isTravelocity() const;

  std::string setWEB(bool) const;

  const static std::string DSS_REQUEST;
  const static std::string INDICATOR_TRUE;
  const static std::string INDICATOR_FALSE;
  const static std::string DEFAULT_TRX_ID;
  const static std::string DEFAULT_USER;
  const static std::string DEFAULT_ACTION_CODE;
};

template <class FlightContainer>
void
PricingDssRequest::build(const FlightContainer& flights, std::string& request) const
{
  LOG4CXX_INFO(_logger, " Entered PricingDssRequest::build()");
  if (boost::empty(flights))
    return;

  XMLConstruct construct;
  construct.openElement(DSS_REQUEST);

  construct.addAttribute("SHS", "true"); // hidden stop details, HSG
  addXray(construct);
  addBilling(construct);
  addUserId(construct);
  addAgent(construct);

  addFlights(flights, construct);

  construct.closeElement();
  request = construct.getXMLData();

  LOG4CXX_INFO(_logger, " Returning PricingDssRequest::build()");
}

template <class FlightContainer>
void
PricingDssRequest::addFlights(const FlightContainer& flights, XMLConstruct& construct) const
{
  construct.openElement("FLL");
  for (const PricingDssFlightKey& key : flights)
  {
    construct.openElement("FLS");
    construct.addAttribute("BRD", key._origin.c_str());
    construct.addAttribute("OFF", key._destination.c_str());
    construct.addAttribute("CXR", key._carrier.c_str());
    construct.addAttributeInteger("FLN", key._flightNumber);
    construct.addAttribute("DAT", key._dssFlightDate.dateToString(YYYYMMDD, "-"));
    construct.closeElement();
  }
  construct.closeElement();
}
} // End namespace tse
