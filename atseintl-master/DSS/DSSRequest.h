//----------------------------------------------------------------------------
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

#include <set>

class XMLConstruct;

namespace tse
{
class Agent;
class Billing;
class DateTime;
class FareDisplayOptions;
class FareDisplayRequest;
class FareDisplayTrx;
class TravelSeg;

/**
 * class DSSRequest : creates XML request for the DSS service to get schedule count.
 */

class DSSRequest
{
  friend class DSSRequestTest;

public:
  DSSRequest(FareDisplayTrx&);

  /**
   * only public interface that builds the DSS request.
   * @param a reference to the FareDisplayTrx.
   * @takes and empty request and fills up with an XML request after build operation.
   */

  void build(FareDisplayTrx& trx, std::set<CarrierCode>& carrierList, std::string& request) const;

private:
  FareDisplayTrx& _trx;
  const GeoTravelType& _geoTravelType;

  void addXrayRecord(XMLConstruct& construct) const;

  /**
   *Prepares the Billing Informantion for the request.
   *@param Billing a reference to the billing object
   *@param construct a reference to the XMLConstuct.
   **/
  void addBillingRecord(const Billing&, XMLConstruct& consturct) const;

  /**
   *Prepares the Agent Informantion for the request.
   *@param Agent a reference to the agent object
   *@param construct a reference to the XMLConstuct.
   **/
  void addAgentRecord(const Agent& agent, bool, XMLConstruct& construct) const;

  /**
   *Prepares the Travel Seg Information.
   *it sets the board and off city and also identifies
   *them as Airport/City Code.
   *@param construct a reference to the XMLConstuct.
   **/
  void addTravelSegRecord(const TravelSeg& tvlSeg, XMLConstruct& construct) const;

  /**
    * Prepares the Date Time Recored
    *@param date a reference to the travel date.
    *@param construct a reference to the XMLConstuct.
    **/

  void addDateRecord(const DateTime& date,
                     FareDisplayRequest& request,
                     FareDisplayOptions& options,
                     bool,
                     XMLConstruct& construct) const;

  /**
    * Prepares the Date Time Recored
    *@param date a reference to the travel date.
    *@param construct a reference to the XMLConstuct.
    **/
  void addTimeRecord(XMLConstruct& construct) const;

  /**
   * Prepares the Flight Informantion for the request.
   *@param construct a reference to the XMLConstuct.
   **/

  void addFlightRecord(XMLConstruct& construct) const;

  /**
   * Prepares the Entry Type  Informantion for the request.
   * @param construct a reference to the XMLConstuct.
   **/
  void addEntryTypeRecord(XMLConstruct& construct) const;

  /**
   * Prepares the Segment record.
   * This sets the defaul values for DSS request.
   * It doenst have any influence on FlightCount.
   * @param construct a reference to the XMLConstuct.
   **/

  void addConnectTimeRecord(XMLConstruct& consturct) const;

  /**
   * Prepares the Segment record.
   * This sets the defaul values for DSS request.
   * It doenst have any influence on FlightCount.
   * @param construct a reference to the XMLConstuct.
   **/

  void addSegmentRecord(XMLConstruct& consturct, bool isWebUser) const;

  /**
    * Prepares the Billing Informantion for the request.
    *@param Billing a reference to the billing object
    *@param construct a reference to the XMLConstuct.
    **/
  void addStopRecord(XMLConstruct& consturct) const;

  /**
    * Prepares the Flight Count Record.
    * If it is a travelocity entry the interlince connection indicator
    * is set to true.
    * if it is a travelocity entry and also the market is international
    * then, online double connection indicator is set to true.
    * @param isTravelociy a boolean value
    * @param construct a reference to the XMLConstuct.
    **/
  void addFlightCountRecord(bool isTravelocity, XMLConstruct& construct) const;

  /**
   * Determines if the Request is a WEB request or not
   *@param psuedocity
   *@param travelDate
   *@retunr true/false
   **/
  /**
   * explicitly list the carrier for which we need to generate schedule count.
   * this is an enhancement to the existion request.
   */
  void addCarrierListRecord(XMLConstruct& construct, std::set<CarrierCode>&) const;
  std::string getCarrierString(std::set<CarrierCode>& cxrs) const;

  bool isTravelocity() const;

  std::string setIIC(bool) const;

  std::string setIOD(bool) const;

  std::string setWEB(bool) const;

  const static std::string INDICATOR_TRUE;
  const static std::string INDICATOR_FALSE;
  const static std::string NOT_APPLICABLE;
  const static std::string DEFAULT_MAX_STOP;
  const static std::string MAX_SOLUTION;

  const static std::string TWO_SEG;
  const static std::string THREE_SEG;
  const static std::string MIN_FLT;
  const static std::string MAX_FLT;
  const static std::string DEFAULT_ZERO;
  const static std::string DEFAULT_USER;
  const static std::string ANY_NUMBER;
  const static std::string DSS_REQUEST;
  const static std::string CITY_CODE;
  const static std::string AREA_CODE;
};
} // End namespace tse
