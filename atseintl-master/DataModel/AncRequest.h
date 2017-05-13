//-------------------------------------------------------------------
//
//  File:         AncRequest.h
//  Created:
//  Authors:
//
//  Description:
//
//  Updates:
//          10/28/10 - Jayanthi Shyam Mohan
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

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PricingRequest.h"

#include <string>
#include <vector>

namespace tse
{
class PaxType;
class Itin;

class AncRequest : public PricingRequest
{
public:
  enum AncRequestType
  {
    M70Request,
    WPAERequest,
    PostTktRequest,
    BaggageRequest,
    WPBGRequest
  };

  enum BagReqAgent
  { CheckInAgent,
    TicketingAgent };

  class AncFareBreakInfo
  {
  public:
    SequenceNumber& fareComponentID() { return _fareComponentID; }
    const SequenceNumber& fareComponentID() const { return _fareComponentID; }

    CarrierCode& governingCarrier() { return _airlineCode; }
    const CarrierCode& governingCarrier() const { return _airlineCode; }

    MoneyAmount& fareAmount() { return _fareAmount; }
    const MoneyAmount& fareAmount() const { return _fareAmount; }

    FareClassCode& fareBasis() { return _fareBasis; }
    const FareClassCode& fareBasis() const { return _fareBasis; }

    FareType& fareType() { return _fareType; }
    const FareType& fareType() const { return _fareType; }

    VendorCode& vendorCode() { return _vendorCode; }
    const VendorCode& vendorCode() const { return _vendorCode; }

    TariffNumber& fareTariff() { return _fareTariff; }
    const TariffNumber& fareTariff() const { return _fareTariff; }

    RuleNumber& fareRule() { return _fareRule; }
    const RuleNumber& fareRule() const { return _fareRule; }

    uint16_t& fareIndicator() { return _fareIndicator; }
    const uint16_t& fareIndicator() const { return _fareIndicator; }

    bool& privateIndicator() { return _privateIndicator; }
    const bool& privateIndicator() const { return _privateIndicator; }

    OCFareTypeCode& fareStat() { return _fareStat; }
    const OCFareTypeCode& fareStat() const { return _fareStat; }

  private:
    SequenceNumber _fareComponentID = -1;
    CarrierCode _airlineCode;
    MoneyAmount _fareAmount = 0;
    FareClassCode _fareBasis;
    FareType _fareType;
    VendorCode _vendorCode = "ATP";
    TariffNumber _fareTariff = -1;
    RuleNumber _fareRule;
    uint16_t _fareIndicator = 0;
    bool _privateIndicator = false;
    OCFareTypeCode _fareStat;
  };

  class AncFareBreakAssociation
  {
  public:
    SequenceNumber& segmentID() { return _segmentID; }
    const SequenceNumber& segmentID() const { return _segmentID; }

    SequenceNumber& fareComponentID() { return _fareComponentID; }
    const SequenceNumber& fareComponentID() const { return _fareComponentID; }

    SequenceNumber& sideTripID() { return _sideTripID; }
    const SequenceNumber& sideTripID() const { return _sideTripID; }

    bool& sideTripStart() { return _sideTripStart; }
    const bool& sideTripStart() const { return _sideTripStart; }

    bool& sideTripEnd() { return _sideTripEnd; }
    const bool& sideTripEnd() const { return _sideTripEnd; }

  private:
    SequenceNumber _segmentID = 0;
    SequenceNumber _fareComponentID = 0;
    SequenceNumber _sideTripID = 0;
    bool _sideTripStart = false;
    bool _sideTripEnd = false;
  };

  class AncAttrMapTree
  {
    std::map<std::string, std::pair<std::string, int>> _attrs;
    std::map<std::string, AncAttrMapTree> _childs;

  public:
    AncAttrMapTree() = default;
    AncAttrMapTree(const AncAttrMapTree& r);
    const std::string& getAttributeValue(const std::string& name);
    void addAttribute(const std::string& name, const std::string& value, int hashIndex = 0);
    size_t getHash(int index) const;
    AncAttrMapTree& operator[](const std::string& name);
    AncAttrMapTree& operator[](int name);
  };

  using FareBreakAssocPerItinMap = std::map<const Itin*, std::vector<AncFareBreakAssociation*>>;

  AncRequest();
  AncRequest(const AncRequest&) = delete;
  AncRequest& operator=(const AncRequest&) = delete;

  std::map<const Itin*, std::vector<PaxType*> >& paxTypesPerItin() { return _paxTypesPerItin; }
  const std::map<const Itin*, std::vector<PaxType*> >& paxTypesPerItin() const
  {
    return _paxTypesPerItin;
  }

  std::map<const Itin*, std::vector<std::string> >& corpIdPerItin() { return _corpIdPerItin; }
  const std::map<const Itin*, std::vector<std::string> >& corpIdPerItin() const
  {
    return _corpIdPerItin;
  }

  std::map<const Itin*, std::vector<std::string> >& invalidCorpIdPerItin()
  {
    return _invaliCorpIdPerItin;
  }
  const std::map<const Itin*, std::vector<std::string> >& invalidCorpIdPerItin() const
  {
    return _invaliCorpIdPerItin;
  }

  std::map<const Itin*, std::vector<std::string> >& accountCodeIdPerItin()
  {
    return _accountCodeIdPerItin;
  }
  const std::map<const Itin*, std::vector<std::string> >& accountCodeIdPerItin() const
  {
    return _accountCodeIdPerItin;
  }

  std::map<const Itin*, std::string>& tourCodePerItin() { return _tourCodePerItin; }
  const std::map<const Itin*, std::string>& tourCodePerItin() const { return _tourCodePerItin; }

  std::map<const Itin*, std::vector<AncFareBreakInfo*> >& fareBreakPerItin()
  {
    return _fareBreakPerItin;
  }
  const std::map<const Itin*, std::vector<AncFareBreakInfo*> >& fareBreakPerItin() const
  {
    return _fareBreakPerItin;
  }

  FareBreakAssocPerItinMap& fareBreakAssociationPerItin() { return _fareBreakAssociationPerItin; }
  const FareBreakAssocPerItinMap& fareBreakAssociationPerItin() const
  {
    return _fareBreakAssociationPerItin;
  }

  std::map<const Itin*, std::map<int16_t, TktDesignator> >& tktDesignatorPerItin()
  {
    return _tktDesignatorPerItin;
  }
  const std::map<const Itin*, std::map<int16_t, TktDesignator> >& tktDesignatorPerItin() const
  {
    return _tktDesignatorPerItin;
  }

  std::map<const Itin*, std::vector<Itin*> >& pricingItins() { return _pricingItins; }
  const std::map<const Itin*, std::vector<Itin*> >& pricingItins() const { return _pricingItins; }

  std::map<const Itin*, bool>& ancillNonGuaranteePerItin() { return _ancillNonGuaranteePerItin; }
  const std::map<const Itin*, bool>& ancillNonGuaranteePerItin() const
  {
    return _ancillNonGuaranteePerItin;
  }

  AncRequestType& ancRequestType() { return _ancRequestType; }
  const AncRequestType& ancRequestType() const { return _ancRequestType; }

  bool isPostTktRequest() const;
  bool isWPBGRequest() const;
  bool isWPAERequest() const;

  Itin*& masterItin() { return _masterItin; }
  const Itin* masterItin() const { return _masterItin; }

  std::map<uint16_t, Itin*>& subItins() { return _subItins; }
  const std::map<uint16_t, Itin*> subItins() const { return _subItins; }
  Itin*& subItin(uint16_t groupNum);
  const Itin* subItin(uint16_t groupNum) const;

  std::map<const Itin*, AncAttrMapTree>& itinAttrMap() { return _itinAttrMap; }
  const std::map<const Itin*, AncAttrMapTree>& itinAttrMap() const { return _itinAttrMap; }

  uint32_t& noHoursBeforeDeparture() { return _noHoursBeforeDeparture; }
  uint32_t noHoursBeforeDeparture() const { return _noHoursBeforeDeparture; }

  bool& hardMatchIndicator() { return _hardMatchIndicator; }
  const bool& hardMatchIndicator() const { return _hardMatchIndicator; }
  std::map<const PaxType*, Itin*>& paxToOriginalItinMap() { return _paxToOriginalItin; }
  const std::map<const PaxType*, Itin*>& paxToOriginalItinMap() const { return _paxToOriginalItin; }

  bool& noTktRefNumberInR7() { return _noTktRefNumberInR7; }
  const bool& noTktRefNumberInR7() const { return _noTktRefNumberInR7; }

  std::vector<PaxType*> paxType(const Itin* it) const;

  Agent*& ticketingAgent() override; // WARNING: this function changes behavior according to setActiveAgent
  				     // FIXME Refactor ASAP
  const Agent* ticketingAgent() const override; // WARNING: this function changes behavior according to
					        // setActiveAgent FIXME Refactor ASAP

  bool isTicketNumberValid(uint8_t ticketNumber) const
  {
    return (_ticketingAgents.find(ticketNumber) != _ticketingAgents.end());
  }

  void setActiveAgent(BagReqAgent agentType, uint8_t ticketNumber = 0) const
  {
    // This function changes the behavior of ticketingAgent() function: selects,
    // which agent should be returned - one of the ticketing agents or the checkin agent
    // FIXME this hack is required to preserve the old functionality. Refactor ASAP
    if (agentType == BagReqAgent::CheckInAgent)
      _activeAgent = const_cast<Agent**>(&_checkInAgent);
    else
      _activeAgent = &_ticketingAgents[ticketNumber];
  }

  void setBaggageTicketingAgent(Agent* agent, uint8_t ticketNumber = 0)
  {
    _ticketingAgents[ticketNumber] = agent;
  }

  bool& wpbgDisplayAllowance() { return _wpbgDisplayAllowance; }
  const bool wpbgDisplayAllowance() const { return _wpbgDisplayAllowance; }

  bool& wpbgDisplayCharges() { return _wpbgDisplayCharges; }
  const bool wpbgDisplayCharges() const { return _wpbgDisplayCharges; }

  const bool wpbgDisplayItinAnalysis() const
  {
    return !_wpbgDisplayAllowance && !_wpbgDisplayCharges;
  }

  bool& wpbgPostTicket() { return _wpbgPostTicket; }
  const bool wpbgPostTicket() const { return _wpbgPostTicket; }

  bool& selectFirstChargeForOccurrence() { return _selectFirstChargeForOccurrence; }
  const bool selectFirstChargeForOccurrence() const { return _selectFirstChargeForOccurrence; }

  std::map<const Itin*, DateTime>& ticketingDatesPerItin() { return _ticketingDatesPerItin; }
  const std::map<const Itin*, DateTime>& ticketingDatesPerItin() const
  {
    return _ticketingDatesPerItin;
  }

  std::set<uint32_t>& displayBaggageTravelIndices() { return _displayBaggageTravelIndices; }
  const std::set<uint32_t>& displayBaggageTravelIndices() const
  {
    return _displayBaggageTravelIndices;
  }

  CarrierCode& carrierOverriddenForBaggageAllowance()
  {
    return _carrierOverriddenForBaggageAllowance;
  }
  const CarrierCode& carrierOverriddenForBaggageAllowance() const
  {
    return _carrierOverriddenForBaggageAllowance;
  }

  CarrierCode& carrierOverriddenForBaggageCharges() { return _carrierOverriddenForBaggageCharges; }
  const CarrierCode& carrierOverriddenForBaggageCharges() const
  {
    return _carrierOverriddenForBaggageCharges;
  }

  std::string getDefaultTicketingCarrierFromTicketingAgent(uint8_t ticketNumber) const;
  std::string getAirlineCarrierCodeFromTicketingAgent(uint8_t ticketNumber) const;

private:
  std::map<const Itin*, std::vector<PaxType*>> _paxTypesPerItin;
  std::map<const Itin*, std::vector<std::string>> _corpIdPerItin;
  std::map<const Itin*, std::vector<std::string>> _invaliCorpIdPerItin;
  std::map<const Itin*, std::vector<std::string>> _accountCodeIdPerItin;
  std::map<const Itin*, std::string> _tourCodePerItin;
  std::map<const Itin*, std::vector<AncFareBreakInfo*>> _fareBreakPerItin;
  FareBreakAssocPerItinMap _fareBreakAssociationPerItin;
  std::map<const Itin*, std::map<int16_t, TktDesignator>> _tktDesignatorPerItin;
  std::map<const Itin*, std::vector<Itin*>> _pricingItins;
  AncRequestType _ancRequestType = AncRequestType::M70Request;
  Itin* _masterItin = nullptr;
  std::map<uint16_t, Itin*> _subItins;
  std::map<const Itin*, AncAttrMapTree> _itinAttrMap;
  std::map<const Itin*, bool> _ancillNonGuaranteePerItin;
  uint32_t _noHoursBeforeDeparture = 0;
  bool _hardMatchIndicator = false;
  std::map<const PaxType*, Itin*> _paxToOriginalItin;
  bool _noTktRefNumberInR7 = false;
  Agent* _checkInAgent = nullptr;
  mutable Agent** _activeAgent = nullptr;
  bool _wpbgDisplayAllowance = false;
  bool _wpbgDisplayCharges = false;
  bool _wpbgPostTicket = false;
  bool _selectFirstChargeForOccurrence = true;
  std::map<const Itin*, DateTime> _ticketingDatesPerItin;
  std::set<uint32_t> _displayBaggageTravelIndices;
  CarrierCode _carrierOverriddenForBaggageAllowance;
  CarrierCode _carrierOverriddenForBaggageCharges;
};

} // tse namespace

