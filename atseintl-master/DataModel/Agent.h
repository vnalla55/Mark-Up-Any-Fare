//-------------------------------------------------------------------
//
//  File:        Agent.h
//  Created:     March 10, 2004
//  Authors:
//
//  Description: Ticketing agent
//
//  Updates:
//          03/10/04 - VN - file created.
//			03/31/04 - Mike Carroll - Added new members
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
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TypeConvert.h"

namespace tse
{
class Loc;
class Customer;

class Agent
{
  const Loc* _agentLocation;
  Customer* _agentTJR;
  LocCode _agentCity; // ACD
  PseudoCityCode _tvlAgencyPCC; // PCC
  PseudoCityCode _mainTvlAgencyPCC; // HTA
  CarrierCode _cxrCode; // ALC
  std::string _tvlAgencyIATA; // ITA
  std::string _airlineIATA; // AB2
  std::string _homeAgencyIATA; // HIT
  std::string _agentFunctions; // AGT
  std::string _agentDuty; // DUT
  std::string _airlineDept; // SET
  std::string _currencyCodeAgent; // ACC
  std::string _agentCommissionType; // CMT P, A or ' '
  MoneyAmount _commissionAmount;
  Percent _commissionPercent;

  // Abacus indicator and partition
  std::string _vendorCrsCode; // AE0

  // PCC Host Carrier
  std::string _hostCarrier; // AE0 under PRO

  // Office Designator
  std::string _officeDesignator; // AE1 under AGI

  // Office/Station code
  std::string _officeStationCode; // AE2 under AGI

  // Default ticketing carrier
  std::string _defaultTicketingCarrier; // AE3 under AGI

  // Airline Channel Code
  ChannelCode _airlineChannelCode; //AE4 under AGI

  uint32_t _agentCommissionAmount; // CMV

  int16_t _coHostID; // CHT

public:
  static constexpr int CWT_GROUP_NUMBER = 52;

  Agent();

  //-------------------------------------------------------------------------
  // Accessors
  //-------------------------------------------------------------------------
  const Loc*& agentLocation() { return _agentLocation; }
  const Loc* agentLocation() const { return _agentLocation; }

  Customer*& agentTJR() { return _agentTJR; }
  const Customer* agentTJR() const { return _agentTJR; }
  TJRGroup tjrGroup();

  LocCode& agentCity() { return _agentCity; }
  const LocCode& agentCity() const { return _agentCity; }

  PseudoCityCode& tvlAgencyPCC() { return _tvlAgencyPCC; }
  const PseudoCityCode& tvlAgencyPCC() const { return _tvlAgencyPCC; }

  PseudoCityCode& mainTvlAgencyPCC() { return _mainTvlAgencyPCC; }
  const PseudoCityCode& mainTvlAgencyPCC() const { return _mainTvlAgencyPCC; }

  std::string& tvlAgencyIATA() { return _tvlAgencyIATA; }
  const std::string& tvlAgencyIATA() const { return _tvlAgencyIATA; }

  std::string& airlineIATA() { return _airlineIATA; }
  const std::string& airlineIATA() const { return _airlineIATA; }

  std::string& homeAgencyIATA() { return _homeAgencyIATA; }
  const std::string& homeAgencyIATA() const { return _homeAgencyIATA; }

  std::string& agentFunctions() { return _agentFunctions; }
  const std::string& agentFunctions() const { return _agentFunctions; }

  std::string& agentDuty() { return _agentDuty; }
  const std::string& agentDuty() const { return _agentDuty; }

  std::string& airlineDept() { return _airlineDept; }
  const std::string& airlineDept() const { return _airlineDept; }

  CarrierCode& cxrCode() { return _cxrCode; }
  const CarrierCode& cxrCode() const { return _cxrCode; }

  std::string& currencyCodeAgent() { return _currencyCodeAgent; }
  const std::string& currencyCodeAgent() const { return _currencyCodeAgent; }

  int16_t& coHostID() { return _coHostID; }
  const int16_t& coHostID() const { return _coHostID; }

  std::string& agentCommissionType() { return _agentCommissionType; }
  const std::string& agentCommissionType() const { return _agentCommissionType; }

  uint32_t& agentCommissionAmount() { return _agentCommissionAmount; }
  const uint32_t& agentCommissionAmount() const { return _agentCommissionAmount; }

  MoneyAmount& commissionAmount() { return _commissionAmount; }
  const MoneyAmount& commissionAmount() const { return _commissionAmount; }

  Percent& commissionPercent() { return _commissionPercent; }
  const Percent& commissionPercent() const { return _commissionPercent; }

  std::string& vendorCrsCode() { return _vendorCrsCode; }
  const std::string& vendorCrsCode() const { return _vendorCrsCode; }

  const bool abacusUser() const;
  const bool axessUser() const;
  const bool cwtUser() const;
  const bool sabre1SUser() const;
  const bool infiniUser() const;

  std::string& hostCarrier() { return _hostCarrier; }
  const std::string& hostCarrier() const { return _hostCarrier; }

  std::string& officeDesignator() { return _officeDesignator; }
  const std::string& officeDesignator() const { return _officeDesignator; }

  std::string& officeStationCode() { return _officeStationCode; }
  const std::string& officeStationCode() const { return _officeStationCode; }

  std::string& defaultTicketingCarrier() { return _defaultTicketingCarrier; }
  const std::string& defaultTicketingCarrier() const { return _defaultTicketingCarrier; }

  ChannelCode& airlineChannelCode() { return _airlineChannelCode; }
  const ChannelCode& airlineChannelCode() const { return _airlineChannelCode; }

  bool isArcUser() const;
  bool isMultiSettlementPlanUser() const;
  void getMultiSettlementPlanTypes(std::vector<SettlementPlanType>& settlementPlanTypes);
};

inline Agent::Agent()
  : _agentLocation(nullptr),
    _agentTJR(nullptr),
    _commissionAmount(0),
    _commissionPercent(0),
    _agentCommissionAmount(0),
    _coHostID(0)
{
}

} // tse namespace

