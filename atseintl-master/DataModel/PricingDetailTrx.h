//-------------------------------------------------------------------
//
//  File:        PricingDetailTrx.h
//  Created:     November 2, 2004
//  Design:      Mike Carroll
//  Authors:
//
//  Description: Pricing Detail Transaction's root object.
//
//  Updates:
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "Service/Service.h"

#include <string>
#include <vector>

namespace tse
{
class PaxDetail;

class PricingDetailTrx : public TktFeesPricingTrx
{
public:
  PricingDetailTrx()
  {
    if(!_billing)
      dataHandle().get(_billing);
  }

  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  // Accessors
  bool& wpnTrx() { return _wpnTrx; }
  const bool& wpnTrx() const { return _wpnTrx; }

  uint16_t& selectionChoice() { return _selectionChoice; }
  const uint16_t& selectionChoice() const { return _selectionChoice; }

  MoneyAmount& totalPriceAll() { return _totalPriceAll; }
  const MoneyAmount& totalPriceAll() const { return _totalPriceAll; }

  uint16_t& totalNumberDecimals() { return _totalNumberDecimals; }
  const uint16_t& totalNumberDecimals() const { return _totalNumberDecimals; }

  CurrencyCode& totalCurrencyCode() { return _totalCurrencyCode; }
  const CurrencyCode& totalCurrencyCode() const { return _totalCurrencyCode; }

  CarrierCode& validatingCarrier() { return _validatingCarrier; }
  const CarrierCode& validatingCarrier() const { return _validatingCarrier; }

  DateTime& ticketingDate() override { return _ticketingDT; }
  const DateTime& ticketingDate() const override { return _ticketingDT; }

  DateTime& lastTicketDate() { return _lastTicketDT; }
  const DateTime& lastTicketDate() const { return _lastTicketDT; }

  DateTime& advancePurchaseDate() { return _advancePurchaseDate; }
  const DateTime& advancePurchaseDate() const { return _advancePurchaseDate; }

  std::string& iataSalesCode() { return _iataSalesCode; }
  const std::string& iataSalesCode() const { return _iataSalesCode; }

  LocCode& salesLocation() { return _salesLocation; }
  const LocCode& salesLocation() const { return _salesLocation; }

  Agent& ticketingAgent() { return _ticketingAgent; }
  const Agent& ticketingAgent() const { return _ticketingAgent; }

  CurrencyCode& consolidatorPlusUpCurrencyCode() { return _consolidatorPlusUpCurrencyCode; }
  const CurrencyCode& consolidatorPlusUpCurrencyCode() const
  {
    return _consolidatorPlusUpCurrencyCode;
  }

  MoneyAmount& consolidatorPlusUpFareCalcAmount() { return _consolidatorPlusUpFareCalcAmount; }
  const MoneyAmount& consolidatorPlusUpFareCalcAmount() const
  {
    return _consolidatorPlusUpFareCalcAmount;
  }

  std::vector<PaxDetail*>& paxDetails() { return _paxDetails; }
  const std::vector<PaxDetail*>& paxDetails() const { return _paxDetails; }

  std::ostringstream& response() { return _response; }

private:
  uint16_t _selectionChoice = 0;
  bool _wpnTrx = false;
  // Summary Information // SUM
  MoneyAmount _totalPriceAll = 0; // TPA
  uint16_t _totalNumberDecimals = 0; // TND
  CurrencyCode _totalCurrencyCode; // TCC
  CarrierCode _validatingCarrier; // VCR
  DateTime _ticketingDT; // D07
  DateTime _lastTicketDT; // LDT LTT
  DateTime _advancePurchaseDate; // APD
  std::string _iataSalesCode; // ISC
  LocCode _salesLocation; // LSC
  Agent _ticketingAgent; // AGI
  CurrencyCode _consolidatorPlusUpCurrencyCode;
  MoneyAmount _consolidatorPlusUpFareCalcAmount = 0;

  // Detail Pax records
  std::vector<PaxDetail*> _paxDetails;

  std::ostringstream _response;
};
} // tse namespace
