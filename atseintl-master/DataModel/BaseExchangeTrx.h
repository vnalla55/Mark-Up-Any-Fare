//-------------------------------------------------------------------
//  Created:     March 25, 2008
//  Design:
//  Authors:
//
//  Description: Exchange types of transactions' root object.
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "DataModel/ExchangeOverrides.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
class PaxType;
class ExcItin;
class Service;

class BaseExchangeTrx : public PricingTrx
{
  friend class BaseExchangeTrxTest;

public:
  virtual bool process(Service& srv) override = 0;

  std::string& reqType() { return _reqType; }
  const std::string& reqType() const { return _reqType; }

  DateTime& setOriginalTktIssueDT() { return _originalTktIssueDT; }
  virtual const DateTime& originalTktIssueDT() const;

  DateTime& purchaseDT() { return _purchaseDT; }
  const DateTime& purchaseDT() const { return _purchaseDT; }

  DateTime& lastTktReIssueDT() { return _lastTktReIssueDT; }
  const DateTime& lastTktReIssueDT() const { return _lastTktReIssueDT; }

  DateTime& currentTicketingDT() { return _currentTicketingDT; }
  const DateTime& currentTicketingDT() const { return _currentTicketingDT; }

  std::vector<ExcItin*>& exchangeItin() { return _exchangeItin; }
  const std::vector<ExcItin*>& exchangeItin() const { return _exchangeItin; }

  std::vector<Itin*>& newItin() { return _itin; }
  const std::vector<Itin*>& newItin() const { return _itin; }

  std::vector<PaxType*>& accompanyPaxType() { return _accompanyPaxType; }
  const std::vector<PaxType*>& accompanyPaxType() const { return _accompanyPaxType; }

  LocCode& reissueLocation() { return _reissueLocation; }
  const LocCode& reissueLocation() const { return _reissueLocation; }

  ExchangeOverrides& exchangeOverrides() { return _exchangeOverrides; }
  const ExchangeOverrides& exchangeOverrides() const { return _exchangeOverrides; }

  uint16_t& itinIndex() { return _itinIndex; }
  const uint16_t& itinIndex() const { return _itinIndex; }

  void setItinIndex(uint16_t itinIndex);
  Itin* curNewItin() { return _itin[_itinIndex]; }
  const Itin* curNewItin() const { return _itin[_itinIndex]; }

  uint16_t getItinPos(const Itin* itin) const;

  void setActionCode() override;

  DateTime& previousExchangeDT() { return _previousExchangeDT; }
  const DateTime& previousExchangeDT() const { return _previousExchangeDT; }

  void setRexPrimaryProcessType(const Indicator& rexPrimaryProcessType)
  {
    _rexPrimaryProcessType = rexPrimaryProcessType;
  }
  void setRexSecondaryProcessType(const Indicator& rexSecondaryProcessType)
  {
    _rexSecondaryProcessType = rexSecondaryProcessType;
  }
  const Indicator rexSecondaryProcessType() const { return _rexSecondaryProcessType; }

  const bool applyReissueExcPrimary() const
  {
    return _rexPrimaryProcessType == PROCESS_TYPE_REISSUE_EXCHANGE;
  }

  virtual bool applyReissueExchange() const { return applyReissueExcPrimary(); }

  void setHistoricalBsrRoeDate();
  void setBsrRoeDate(const Indicator& ind);

  const DateTime& getHistoricalBsrRoeDate() const { return _historicalBsrRoeDate; }

  virtual void applyCurrentBsrRoeDate() {}
  virtual void applyHistoricalBsrRoeDate() {}

  bool isExcRtw() const;
  void setupFootNotePrevalidation() override { _footNotePrevalidationAllowed = false; }

protected:
  BaseExchangeTrx();

  std::string _reqType;
  std::vector<ExcItin*> _exchangeItin;
  DateTime _originalTktIssueDT; // D92
  DateTime _purchaseDT; // D93
  DateTime _lastTktReIssueDT; // D94
  DateTime _currentTicketingDT; // D07
  LocCode _reissueLocation; // AP1 - used only to match tax reissue location
  ExchangeOverrides _exchangeOverrides{*this};
  uint16_t _itinIndex = 0; // Exchange schopping - to get current new itin index
  std::map<const Itin*, uint16_t> _motherItinIndex; // Exchange schopping - to get current new itin
  // mother index for child itin

  std::vector<PaxType*> _accompanyPaxType;

  Indicator _rexPrimaryProcessType = RuleConst::BLANK;
  Indicator _rexSecondaryProcessType = RuleConst::BLANK;
  DateTime _previousExchangeDT; // D95 - Previous Exchange Date
  DateTime _historicalBsrRoeDate;

private:
  static constexpr Indicator PROCESS_TYPE_REISSUE_EXCHANGE = 'A';
};
} // tse namespace
