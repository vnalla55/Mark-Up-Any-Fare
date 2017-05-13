//----------------------------------------------------------------------------
//
//  File:     AtaeRequest.h
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

#include "Common/TseCodeTypes.h"

class XMLConstruct;

namespace tse
{
class PricingTrx;
class ReservationData;
class TravelSeg;
class DateTime;
class Billing;
class Agent;
class FareMarket;
class Itin;
class Logger;

// class AtaeRequest : creates XML request for the AS V2 service to get
// availability.

class AtaeRequest
{
  friend class AtaeRequestTest;

public:
  AtaeRequest(PricingTrx&, bool);

  /**
   * only public interface that builds the ATAE request.
   * @param a reference to the PricingTrx.
   * @takes an empty request and fills up with an XML request after
   * build operation.
   */

  void build(std::string& request);

  void buildShopping(std::string& request);

  /**
   * vector of fare markets sent to Atae
   */
  std::vector<FareMarket*>& fareMarketsSentToAtae() { return _fareMarketsSentToAtae; }
  const std::vector<FareMarket*>& fareMarketsSentToAtae() const { return _fareMarketsSentToAtae; }

  const std::string& getOverrideActionCode() const { return _overrideActionCode; }
  void setOverrideActionCode(const std::string& ac) { _overrideActionCode = ac; }

  static bool sameFlight(const TravelSeg* tvlSeg1, const TravelSeg* tvlSeg2);

private:
  static Logger _logger;
  PricingTrx& _trx;
  std::vector<FareMarket*> _fareMarketsSentToAtae;
  std::string _overrideActionCode;
  bool _useAVS;

  void addXray(XMLConstruct& construct);

  void addFlights(XMLConstruct& construct) const;

  void addFlt(const TravelSeg* tvlSeg, XMLConstruct& construct) const;

  void addAsl(XMLConstruct& construct);

  void addAso(const FareMarket& fm, XMLConstruct& construct, int asoSolId) const;

  void
  addSgr(const TravelSeg* tvlSeg, XMLConstruct& construct, const TravelSeg* firstTravelSeg) const;

  int64_t daysDiff(DateTime currSegDate, DateTime firstSegDate) const;

  void addRes(XMLConstruct& construct) const;

  void addRli(XMLConstruct& construct) const;

  void addDia(XMLConstruct& construct) const;

  void addPax(XMLConstruct& construct) const;

  void addPii(XMLConstruct& construct) const;

  void addCfi(XMLConstruct& construct) const;

  void addOci(XMLConstruct& construct) const;

  void addReq(XMLConstruct& construct) const;

  void addPoc(XMLConstruct& construct, ReservationData* resData) const;

  bool stopOversArunkIncluded(FareMarket& fm) const;

  const std::string& getActionCode(const Billing& billing) const;

  /**
   *Prepares the Billing Informantion for the request.
   *@param Billing a reference to the billing object
   *@param construct a reference to the XMLConstuct.
   **/
  void addBilling(XMLConstruct& construct) const;

  /**
   *Prepares the Agent Informantion for the request.
   *@param Agent a reference to the agent object
   *@param construct a reference to the XMLConstuct.
   **/
  void addAgent(XMLConstruct& construct) const;

  bool sendResData() const;

  void buildUniqueTvlSegs(std::vector<TravelSeg*>& uniqueTvlSegs) const;

  void buildUniqueFareMkts(std::vector<FareMarket*>& uniqueFareMkts) const;

  void addAsgShopping(XMLConstruct& construct, std::vector<TravelSeg*>& uniqueTvlSegs) const;

  void addAslShopping(XMLConstruct& construct,
                      std::vector<TravelSeg*>& uniqueTvlSegs,
                      std::vector<FareMarket*>& uniqueFareMkts);

  void addAsoShopping(FareMarket& fm,
                      XMLConstruct& construct,
                      uint16_t asoSolId,
                      std::vector<TravelSeg*>& uniqueTvlSegs) const;

  void addSgrShopping(TravelSeg* tvlSeg,
                      XMLConstruct& construct,
                      TravelSeg* firstTravelSeg,
                      std::vector<TravelSeg*>& uniqueTvlSegs) const;

  uint16_t indexTvlSeg(TravelSeg* tvlSeg, std::vector<TravelSeg*>& uniqueTvlSegs) const;

  bool partOfFlowJourney(const FareMarket& fm) const;

  bool
  partOfFlowJourneyShopping(const FareMarket& fm, std::vector<FareMarket*>& uniqueFareMkts) const;

  bool allFlowJourneyCarriers(const FareMarket& fm) const;

  bool
  segFoundInOtherJourney(const FareMarket& fm, const std::vector<FareMarket*>& otherFareMkts) const;

  std::string calculateIATACheckDigit(const char* iataNumber) const;

  bool duplicateFareMarket(FareMarket* fm);

  bool journeyCOIncluded(Itin& itin, FareMarket& fm) const;

  bool entryFromAirlinePartition() const;

  const static std::string ATAE_REQUEST;
  const static std::string DEFAULT_TRX_ID;
  const static std::string DEFAULT_ACTION_CODE;
  const static std::string INTERFACE_VERSION;
};
} // End namespace tse
