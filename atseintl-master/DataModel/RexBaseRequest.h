//-------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "DataModel/PricingRequest.h"

namespace tse
{
class RexBaseTrx;
class Loc;

class RexBaseRequest : public PricingRequest
{
public:
  friend class RexPricingTrxTest;

  RexBaseRequest() : _trx(nullptr), _prevTicketIssueAgent(nullptr), _originalTicketAgentLocation(nullptr) {}

  void setTrx(RexBaseTrx* trx) { _trx = trx; }

  Agent*& currentTicketingAgent() { return _ticketingAgents[0]; }
  const Agent* currentTicketingAgent() const
  {
    std::map<uint8_t, Agent*>::const_iterator agent = _ticketingAgents.cbegin();
    if (agent == _ticketingAgents.end())
      return nullptr;

    return agent->second;
  }

  Agent*& prevTicketIssueAgent() { return _prevTicketIssueAgent; }
  const Agent* prevTicketIssueAgent() const { return _prevTicketIssueAgent; }

  std::string& excCorporateID() { return _excCorporateID; }
  const std::string& excCorporateID() const { return _excCorporateID; }

  std::string& excAccountCode() { return _excAccountCode; }
  const std::string& excAccountCode() const { return _excAccountCode; }

  std::string& newCorporateID() { return _corporateID; }
  const std::string& newCorporateID() const { return _corporateID; }

  std::string& newAccountCode() { return _accountCode; }
  const std::string& newAccountCode() const { return _accountCode; }

  CarrierCode& validatingCarrier() override;
  const CarrierCode& validatingCarrier() const override;

  CarrierCode& excValidatingCarrier() { return _excValidatingCarrier; }
  const CarrierCode& excValidatingCarrier() const { return _excValidatingCarrier; }

  CarrierCode& newValidatingCarrier() { return _validatingCarrier; }
  const CarrierCode& newValidatingCarrier() const { return _validatingCarrier; }

  virtual Agent*& ticketingAgent() override;
  virtual const Agent* ticketingAgent() const override;

  virtual std::string& corporateID() override;
  virtual const std::string& corporateID() const override;

  virtual std::string& accountCode() override;
  virtual const std::string& accountCode() const override;

  virtual LocCode& ticketPointOverride() override;
  virtual const LocCode& ticketPointOverride() const override;

  virtual LocCode& salePointOverride() override;
  virtual const LocCode& salePointOverride() const override;

  void setTicketingDT(const DateTime& ticketingDT) { _ticketingDT = ticketingDT; }

  DateTime& getTicketingDT() { return (_ticketingDT); }

  const DateTime& getTicketingDT() const { return (_ticketingDT); }

  virtual DateTime& ticketingDT() override;
  virtual const DateTime& ticketingDT() const override;
  virtual std::map<int16_t, TktDesignator>& tktDesignator() override;
  virtual const std::map<int16_t, TktDesignator>& tktDesignator() const override;
  virtual const TktDesignator tktDesignator(const int16_t segmentOrder) const override;
  virtual bool isTktDesignatorEntry() override;

  void setOriginalTicketAgentLocation(const LocCode& loc);
  const Loc* getOriginalTicketAgentLocation() const { return _originalTicketAgentLocation; }

  bool isAirlineRequest() const;

  Discounts& excDiscounts() { return _excDiscounts; }
  const Discounts& excDiscounts() const { return _excDiscounts; }

protected:
  RexBaseTrx* _trx;
  Agent* _prevTicketIssueAgent;
  CarrierCode _excValidatingCarrier;
  std::string _excCorporateID;
  std::string _excAccountCode;
  LocCode _excTicketPointOverride;
  LocCode _excSalePointOverride;
  const Loc* _originalTicketAgentLocation;

  std::map<int16_t, TktDesignator> _excTktDesignator; // *Q/TKT DESIGNATOR with Segment Select

  Discounts _excDiscounts;
};

} // tse namespace

