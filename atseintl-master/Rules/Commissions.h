//-------------------------------------------------------------------
//
//  File:        Commissions.h
//  Created:     Nov 05, 2004
//  Authors:     Vladimir Koliasnikov
//
//  Description:
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

#include "Common/CommissionKeys.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/CollectedNegFareData.h"
#include "Diagnostic/Diag867Collector.h"
#include "Rules/RuleConst.h"

namespace tse
{

class PricingTrx;
class FarePath;
class PaxTypeFare;
class NegFareRest;
class NegPaxTypeFareRuleData;
class Diag865Collector;
class Diag866Collector;
class CommissionCap;

static std::string EPR_COMMOVER = "COMOVR";

class Commissions
{
  friend class CommissionsTest;

public:
  static constexpr Indicator PERCENT_COMM_TYPE = 'P';
  static constexpr Indicator AMOUNT_COMM_TYPE = 'A';
  static constexpr Indicator ALL_SEG_ON_VAL_CARRIER = '1';
  static constexpr Indicator AT_LEAST_ONE_SEG_ON_VAL_CARRIER = '2';
  static constexpr Indicator NO_SEG_ON_VAL_CARRIER = '3';

  static constexpr Indicator ONE_WAY = '1';
  static constexpr Indicator ROUND_TRIP = '2';
  static constexpr Indicator BLANK = ' ';

  static constexpr Indicator FLAT_AMOUNT_TYPE = 'F'; // CommissionCap

  Commissions(PricingTrx& trx);
  virtual ~Commissions();

  void getCommissions(FarePath& farePath);
  void getCommissions(
      FarePath& farePath,
      const std::vector<CustomerSecurityHandshakeInfo*>& csHsInfoCol,
      amc::VCFMCommissionPrograms& valCxrFmCommProgs,
      amc::VCFMPTFCommissionRules& valCxrFmFareCommRules);

  MoneyAmount convertCurrency(MoneyAmount sourceAmount,
                              const CurrencyCode& sourceCurrency,
                              const CurrencyCode& targetCurrency,
                              bool doNonIataRounding = false);

  // access to members
  const bool errorCommission() const { return _errorCommission; }
  const bool ticketingEntry() const { return _ticketingEntry; }
  const MoneyAmount ruleCommAmt() const { return _ruleCommAmt; }
  const Percent rulePercent() const { return _rulePercent; }
  const MoneyAmount totalNetAmt() const { return _totalNetAmt; }
  const MoneyAmount totalSellAmt() const { return _totalSellAmt; }
  const MoneyAmount netMileageSurchargeAmt() const { return _netMileageSurchargeAmt; }
  const CurrencyCode calculationCurrency() const { return _calculationCurrency; }
  const CurrencyCode paymentCurrency() const { return _paymentCurrency; }
  const CurrencyCode baseFareCurrency() const { return _baseFareCurrency; }
  const MoneyAmount commAmount() const { return _commAmount; }
  const MoneyAmount netCommAmount() const { return _netCommAmount; }
  const Percent commPercent() const { return _commPercent; }
  const Percent netCommPercent() const { return _netCommPercent; }

  const bool allOnValidCarrier() const { return _allOnValidCarrier; }
  const bool anyOnValidCarrier() const { return _anyOnValidCarrier; }
  const bool noValidCarrier() const { return _noValidCarrier; }
  const bool isNetTktingWithItBtFare() const { return _isNetTktingWithItBtFare; }

protected:
  void calculateWPCat35Commission(CollectedNegFareData& cNegFareData);

  void getCat35Commissions(FarePath& farePath, CollectedNegFareData& cNegFareData);

  void getRegularCommissions(FarePath& farePath);

  void calculateCommissions(const FarePath& fPath, const CommissionCap& cCap);

  void calcNetRemitCommission();

  void checkAgentInputCommission(CollectedNegFareData& cNegFareData);

  void
  getNetRemitCommission(FarePath& fPath, CollectedNegFareData& cNegFareData, bool calculate = true);

  void getCat35TFSFCommission(FarePath& fPath,
                              CollectedNegFareData& cNegFareData,
                              bool calculate = true);

  void getNetTicketingCommission(FarePath& fPath,
                                 bool calculate = true);

  void getRuleCommission(const PaxTypeFare* ptf,
                         const NegFareRest* negFareRest,
                         const NegPaxTypeFareRuleData* negPaxTypeFare);

  MoneyAmount calculateNetTicketingCommission(PaxTypeFare* ptf, const NegFareRest* negFareRest);

  void calculateNetTktCommissions(PaxTypeFare* ptf,
                                  const NegFareRest* negFareRest,
                                  MoneyAmount& sellingComm,
                                  MoneyAmount& netComm,
                                  const NegPaxTypeFareRuleData& negPaxTypeFare);

  bool convertToNUC(MoneyAmount amount, const CurrencyCode& currency, MoneyAmount& result);

  bool isITBTFare(const NegFareRest* negFareRest) const;
  bool isNetRemitAllowed(const CollectedNegFareData& cNegFareData) const;
  bool
  isCalculateCommFromTFD(const FarePath& fPath, const CollectedNegFareData& cNegFareData) const;

  inline bool isTicketingAgentSpecifiedCommission() const;

  bool agencyCommissionsApplicable(
      FarePath& fpath,
      const std::vector<CustomerSecurityHandshakeInfo*>& csHsInfoCol) const;
  bool securityHandShakeValid(FarePath& fpath) const;
  bool isTrxApplicableForAgencyCommission(FarePath& fpath) const;
  bool isRequestFromTravelAgent(FarePath& fpath) const;
  bool isCat35Solution(const FarePath& fp) const;
  bool isTicketingAgentSpecCommissionWithEPR(FarePath& fpath) const;

  // data members
  PricingTrx& _trx;
  bool _errorCommission = false;
  bool _ticketingEntry = false;
  std::string _commType;
  MoneyAmount _inputCommAmt = 0.0;
  Percent _inputCommPrc = 0.0;
  MoneyAmount _ruleCommAmt = 0.0;
  Percent _rulePercent = 0.0;
  MoneyAmount _totalNetAmt = 0.0;
  MoneyAmount _totalSellAmt = 0.0;
  MoneyAmount _markUpCommission = 0.0;
  MoneyAmount _amtForCalcComm = 0.0;
  MoneyAmount _netMileageSurchargeAmt = 0.0;
  CurrencyCode _calculationCurrency;
  CurrencyCode _paymentCurrency;
  MoneyAmount _commAmount = 0.0;
  MoneyAmount _netCommAmount = 0.0;
  Percent _commPercent = 0.0;
  Percent _netCommPercent = 0.0;
  CurrencyCode _baseFareCurrency;
  CurrencyConversionFacade _ccFacade;
  Diag865Collector* _diag865 = nullptr;
  Diag866Collector* _diag866 = nullptr;
  Diag867Collector* _diag867 = nullptr;
  MoneyAmount _commBaseAmount = 0.0;
  MoneyAmount _markUpAmount = 0.0;

  bool _allOnValidCarrier = true; // all itinerary on validating carrier
  bool _anyOnValidCarrier = false; // at least one segment on Validating carrier
  bool _noValidCarrier = false; // there are no segments on Validating crx
  bool _useInternationalRounding = false;
  bool _applyNonIATARounding = false;
  Indicator _indNetGross = ' ';
  bool _isNetTktingWithItBtFare = false;
};

bool Commissions::isTicketingAgentSpecifiedCommission() const
{
  return (_trx.getRequest() &&
          _trx.getRequest()->ticketingAgent() &&
          !_trx.getRequest()->ticketingAgent()->agentCommissionType().empty());
}
} // namespace tse
