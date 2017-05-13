//-------------------------------------------------------------------
//
//  File:        FareByRuleProcessingInfo.h
//  Created:     May 26, 2004
//  Design:
//  Authors:     Doug Boeving
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

namespace tse
{

class PricingTrx;
class Itin;
class FareMarket;
class FareByRuleApp;
class TariffCrossRefInfo;
class DiagManager;

class FareByRuleProcessingInfo
{
public:
  FareByRuleProcessingInfo(PricingTrx* trx = nullptr,
                           Itin* itin = nullptr,
                           FareMarket* fareMarket = nullptr,
                           FareByRuleApp* fbrApp = nullptr,
                           TariffCrossRefInfo* tcrInfo = nullptr,
                           FareByRuleItemInfo* fbrItemInfo = nullptr,
                           DiagManager* diagManager = nullptr,
                           bool isR8LocationSwapped = false,
                           bool isFdTrx = false,
                           bool isUsCaRuleTariff = false,
                           bool failCat35MSF = false,
                           bool isSpanishDiscountApplied = false,
                           bool isCat35If1PreValidationProcessed = false,
                           bool isCat35If1PreValidationPassed = false)
    : _trx(trx),
      _itin(itin),
      _fareMarket(fareMarket),
      _fbrApp(fbrApp),
      _tcrInfo(tcrInfo),
      _fbrItemInfo(fbrItemInfo),
      _diagManager(diagManager),
      _isR8LocationSwapped(isR8LocationSwapped),
      _isFdTrx(isFdTrx),
      _isUsCaRuleTariff(isUsCaRuleTariff),
      _failCat35MSF(failCat35MSF),
      _isSpanishDiscountApplied(isSpanishDiscountApplied),
      _isCat35If1PreValidationProcessed(isCat35If1PreValidationProcessed),
      _isCat35If1PreValidationPassed(isCat35If1PreValidationPassed)
  {
  }

  void initialize(PricingTrx* trx,
                  Itin* itin,
                  FareMarket* fareMarket,
                  FareByRuleApp* fbrApp,
                  TariffCrossRefInfo* tcrInfo = nullptr,
                  FareByRuleItemInfo* fbrItemInfo = nullptr,
                  DiagManager* diagManager = nullptr,
                  bool isR8LocationSwapped = false,
                  bool isFdTrx = false,
                  bool isUsCaRuleTariff = false,
                  bool failCat35MSF = false)
  {
    _trx = trx;
    _itin = itin;
    _fareMarket = fareMarket;
    _fbrApp = fbrApp;
    _tcrInfo = tcrInfo;
    _fbrItemInfo = fbrItemInfo;
    _diagManager = diagManager;
    _isR8LocationSwapped = false;
    _isFdTrx = false;
    _isUsCaRuleTariff = false;
    _failCat35MSF = false;
    _fbrAppVendorPCC = false;
    _isSpanishDiscountApplied = false;
    _isSpanishPassengerType = false;
    _isResidenceFiledInR8 = false;
    _isResidenceFiledInR3Cat25 = false;
    _isCat35If1PreValidationProcessed = false;
    _isCat35If1PreValidationPassed = false;
  }

  PricingTrx*& trx() { return _trx; }
  const PricingTrx* trx() const { return _trx; }

  Itin*& itin() { return _itin; }
  const Itin* itin() const { return _itin; }

  FareMarket*& fareMarket() { return _fareMarket; }
  const FareMarket* fareMarket() const { return _fareMarket; }

  FareByRuleApp*& fbrApp() { return _fbrApp; }
  const FareByRuleApp* fbrApp() const { return _fbrApp; }

  TariffCrossRefInfo*& tcrInfo() { return _tcrInfo; }
  const TariffCrossRefInfo* tcrInfo() const { return _tcrInfo; }

  const FareByRuleItemInfo*& fbrItemInfo() { return _fbrItemInfo; }
  const FareByRuleItemInfo* fbrItemInfo() const { return _fbrItemInfo; }

  DiagManager*& diagManager() { return _diagManager; }
  const DiagManager* diagManager() const { return _diagManager; }

  bool isR8LocationSwapped() const { return _isR8LocationSwapped; }
  bool& isR8LocationSwapped() { return _isR8LocationSwapped; }

  bool isFdTrx() const { return _isFdTrx; }
  bool& isFdTrx() { return _isFdTrx; }

  bool isUsCaRuleTariff() const { return _isUsCaRuleTariff; }
  bool& isUsCaRuleTariff() { return _isUsCaRuleTariff; }

  bool failCat35MSF() const { return _failCat35MSF; }
  bool& failCat35MSF() { return _failCat35MSF; }

  const FareByRuleCtrlInfo* fbrCtrlInfo() const { return _fbrCtrlInfo; }
  const FareByRuleCtrlInfo*& fbrCtrlInfo() { return _fbrCtrlInfo; }

  bool isSoftPassFor25() const { return _isSoftPassFor25; }
  bool& isSoftPassFor25() { return _isSoftPassFor25; }

  bool isFbrAppVendorPCC() const { return _fbrAppVendorPCC; }
  bool& isFbrAppVendorPCC() { return _fbrAppVendorPCC; }

  void stateCodeFromFirstRequested(StateCode& s)
  {
    s = (_fareMarket == nullptr) ? ""
                           : _fareMarket->paxTypeCortege().front().requestedPaxType()->stateCode();
  }

  MoneyAmount& combinedPercent() { return _combinedPercent; }
  MoneyAmount combinedPercent() const { return _combinedPercent; }

  std::vector<CarrierCode>& validatingCarriers() { return _validatingCarriers;}
  const std::vector<CarrierCode>& validatingCarriers() const { return _validatingCarriers;}

  bool isSpanishDiscountApplied() const { return _isSpanishDiscountApplied; }
  bool& isSpanishDiscountApplied() { return _isSpanishDiscountApplied; }

  bool isSpanishPassengerType() const { return _isSpanishPassengerType; }
  bool& isSpanishPassengerType() { return _isSpanishPassengerType; }

  bool isResidenceFiledInR8() const { return _isResidenceFiledInR8; }
  bool& isResidenceFiledInR8() { return _isResidenceFiledInR8; }

  bool isResidenceFiledInR3Cat25() const { return _isResidenceFiledInR3Cat25; }
  bool& isResidenceFiledInR3Cat25() { return _isResidenceFiledInR3Cat25; }

  bool isCat35If1PreValidationProcessed() const { return _isCat35If1PreValidationProcessed; }
  bool& isCat35If1PreValidationProcessed() { return _isCat35If1PreValidationProcessed; }

  bool isCat35If1PreValidationPassed() const { return _isCat35If1PreValidationPassed; }
  bool& isCat35If1PreValidationPassed() { return _isCat35If1PreValidationPassed; }

private:
  PricingTrx* _trx = nullptr;
  Itin* _itin = nullptr;
  FareMarket* _fareMarket = nullptr;
  FareByRuleApp* _fbrApp = nullptr;
  TariffCrossRefInfo* _tcrInfo = nullptr;
  const FareByRuleItemInfo* _fbrItemInfo = nullptr;
  DiagManager* _diagManager = nullptr;
  bool _isR8LocationSwapped = false;
  bool _isFdTrx = false;
  bool _isUsCaRuleTariff = false;
  bool _failCat35MSF = false;
  const FareByRuleCtrlInfo* _fbrCtrlInfo = nullptr;
  bool _isSoftPassFor25 = false;
  bool _fbrAppVendorPCC = false;
  MoneyAmount _combinedPercent = 0;
  std::vector<CarrierCode> _validatingCarriers;
  bool _isSpanishDiscountApplied = false;
  bool _isSpanishPassengerType = false;
  bool _isResidenceFiledInR8 = false;
  bool _isResidenceFiledInR3Cat25 = false;
  bool _isCat35If1PreValidationProcessed = false;
  bool _isCat35If1PreValidationPassed = false;
};

} // tse

