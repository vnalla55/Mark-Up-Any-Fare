//-------------------------------------------------------------------
//
//  File:        NegotiatedFareRuleUtil.h
//  Created:     Oct 07, 2004
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
#include "Rules/RuleConst.h"

#pragma once

namespace tse
{

class Itin;
class PricingTrx;
class FarePath;
class PaxTypeFare;
class FareUsage;
class PricingUnit;
class NegFareRest;
class CollectedNegFareData;
class NegPaxTypeFareRuleData;
class PrintOption;
class Diag535Collector;

class NegotiatedFareRuleUtil
{
  friend class NegotiatedFareRuleUtilTest;

public:
  enum WARNING_MSG
  {
    NO_WARNING = 0,
    MULTIPLE_TOUR_CODE, // wp      //w'
    TOUR_CODE_NOT_FOUND, // wp      //w'
    MULTIPLE_VALUE_CODE, // wp      //w'
    MULTIPLE_PRINT_OPTION, // wp      //w'
    NOT_ITBT, // wp      //w'
    ITBT_BSP_NOT_BLANK, // wp      //w'
    INVALID_ITBT_PSG_COUPON, // wp      //w'
    EMPTY_ITBT_PSG_COUPON, // wp      //w'
    MULTIPLE_FARE_BOX, // wp      //w'
    MULTIPLE_NET_GROSS, // wp      //w'
    MULTIPLE_BSP, // wp      //w'
    MIX_COMMISSION, // --      //w'
    MULTIPLE_COMMISSION, // --      //w'
    MIX_FARES, // wp      // w'
    NO_NET_FARE_AMOUNT, // wp throw  ---
    FARES_NOT_COMBINABLE, // wp      //w'
    FARES_NOT_COMBINABLE_NO_NET, // wp throw    ---
    NET_SELLING_CONFLICT, // wp       //w'
    ISSUE_SEPARATE_TKT, // wp       //w'
    BSP_METHOD_IS_NOT_2, // wp       //w'
    INVALID_NET_REMIT_FARE, // wp       //w'
    INVALID_NET_REMIT_COMM, // wp       //w'
    AUTO_TKT_NOT_PERMITTED, // wp       //w'
    NET_FARE_AMOUNT_EXCEEDS_FARE, // wp        --
    MIXED_FARES_COMBINATION, // wp       //w'
    CONFLICTING_TFD_BYTE101, // wp       //w'
    TFD_RETRIEVE_FAIL, // wp       //w'
    SYSTEM_ERROR, // wp       //w'
    MIXED_FARE_BOX_AMT
  };

  static constexpr Indicator BLANK = ' ';

  static constexpr int ONE_SEGMENT = 1;
  static constexpr int TWO_SEGMENTS = 2;

  static const std::string IT_TICKET;
  static const std::string BT_TICKET;
  static const std::string BLANK_CURRENCY;
  static constexpr Indicator AUDIT_COUPON = 'A'; // audit coupon
  static constexpr Indicator PSG_COUPON = 'P'; // passenger coupon

  static constexpr Indicator NET_LEVEL_NOT_ALLOWED = 'P'; // not allowed to view net level

  bool processNegFareITBT(PricingTrx& trx, FarePath& farePath);

  bool processNetRemit(PricingTrx& trx, FarePath& fPath);

  void findTktRestrictedFare(FarePath& fPath);

  void getCat18Endorsement(const PricingTrx& trx,
                           const Itin& itin,
                           const FareUsage& fu,
                           std::string& endorementTxt,
                           std::string& valueCode);

  WARNING_MSG getWarningMsg() { return _warningMsg; }

  bool
  validateTourCodeCombination(PricingTrx& trx, const std::vector<const PaxTypeFare*>& paxTypeFares);

  bool
  checkForTourCodesConflict(PricingTrx& trx, const std::vector<const PaxTypeFare*>& paxTypeFares);

  bool validateTFDCombination(PricingTrx& trx, const std::vector<const PaxTypeFare*>& paxTypeFares);

  bool checkFareBox(const std::string& fareBoxText);

  static const NegFareRest*
  getCat35Record3(const PaxTypeFare* paxTypeFare, NegPaxTypeFareRuleData*& negPaxTypeFare);
  static bool isNetRemitFare(const Indicator& displayType, const NegFareRest* negFareRest);

  static bool validatePrintOptionCombination(const std::vector<const PaxTypeFare*>& paxTypeFares);

  bool validateFareBoxCombination(const std::vector<const PaxTypeFare*>& paxTypeFares);

  static bool hasFareBoxAmount(const NegFareRest* negFareRest);
  static bool hasFareBoxNetRemitMethod(const NegFareRest* negFareRest);
  static bool validateNetRemitMethod1(std::vector<const PaxTypeFare*>& paxTypeFares);
  static bool validateNetRemitMethod1(const NegFareRest* negFareRest);
  static Indicator getFareAmountIndicator(const PricingTrx& trx, const FareUsage* fu);
  static bool isTFDPSC(const std::vector<FareUsage*>& fuCol);
  bool isItBtTicketingData(FarePath& fp);
  bool isNetTicketingWithItBtData(const PricingTrx& trx, FarePath& fp);
  bool isRegularNet(FarePath& fp);
  static bool shouldUseRedistributedWholeSale(PricingTrx* trx, const PaxTypeFare& ptFare);

  // access to members
private:
  bool isNegotiatedFareCombo(PricingTrx& trx,
                             FarePath& farePath,
                             CollectedNegFareData* collectedNegFareData);

  bool checkTktDataCat35(PricingTrx& trx,
                         FarePath& farePath,
                         FareUsage& fu,
                         CollectedNegFareData* cNegFareData,
                         bool& firstCat35Fare);

  bool checkTktDataCat35Fare(PricingTrx& trx,
                             FarePath& fPath,
                             FareUsage& fu,
                             PaxTypeFare* ptf,
                             const NegFareRest* negFareRest,
                             CollectedNegFareData* cNegFareData,
                             NegPaxTypeFareRuleData* negPaxTypeFare);

  void processFirstCat35Fare(PricingTrx& trx,
                             const Itin& itin,
                             FareUsage& fu,
                             PaxTypeFare* ptf,
                             const NegFareRest* negFareRest,
                             CollectedNegFareData* cNegFareData,
                             NegPaxTypeFareRuleData* negPaxTypeFare);

  void processCat35NetTicketingFare(CollectedNegFareData* cNegFareData);

  void processCat35NetRemitFare(PricingTrx& trx,
                                const Itin& itin,
                                FareUsage& fu,
                                const NegFareRest* negFareRest,
                                CollectedNegFareData* cNegFareData);

  bool validateEndorsements(PricingTrx& trx, const std::string& endorsementTxt);

  bool saveCommissions(PricingTrx& trx,
                       const NegFareRest* negFareRest,
                       const NegPaxTypeFareRuleData* negPaxTypeFare,
                       const PaxTypeFare* ptf);

  bool validateTourCode(PricingTrx& trx,
                        const NegFareRest* negFareRest,
                        CollectedNegFareData* cNegFareData);

  bool validateBSPMethod(PricingTrx& trx, const NegFareRest* negFareRest);

  bool validateFareBoxText(const NegFareRest* negFareRest);

  bool checkFareBoxTextITBT(const std::string& firstFBT, const std::string& secondFBT);

  void processAxessCat35(PricingTrx& trx,
                         FarePath& farePath,
                         CollectedNegFareData* collectedNegFareData);

  bool checkTktDataAxessCat35(PricingTrx& trx,
                              FareUsage& fu,
                              CollectedNegFareData* cNegFareData,
                              bool& firstCat35Fare);

  bool checkWarningMsgForTicketing(FarePath& farePath, const PricingTrx& trx);

  bool
  handleCat35Processing(CollectedNegFareData* cNegFareData, PricingTrx& trx, FarePath& farePath);

  bool checkWarningMsgForPricing(CollectedNegFareData* cNegFareData,
                                 PricingTrx& trx,
                                 FarePath& farePath);

  bool areTourCodesInConflict(PricingTrx& trx, const PaxTypeFare& paxTypeFare) const;

  bool checkCmdPricingFare(PricingTrx& trx, FarePath& farePath);

  virtual bool processNetRemitFareSelection(PricingTrx& trx,
                                            const FarePath& farePath,
                                            PricingUnit& pricingUnit,
                                            FareUsage& fareUsage,
                                            const NegFareRest& negFareRest) const;

  bool validateCxrTourCodeMap(const std::multimap<CarrierCode, std::string>& cxrTourMap) const;

  static Indicator getPrintOption(const PaxTypeFare& ptf);

  virtual bool checkVendor(PricingTrx& trx, const VendorCode& vendor) const;

  const PrintOption* getPrintOptionInfo(PricingTrx& trx, FarePath& farePath) const;

  void prepareTourCodeValueCodeByPO(PricingTrx& trx, FarePath& farePath) const;

  bool validateCat35Combination(PricingTrx& trx,
                                FarePath& fPath,
                                const std::vector<const PaxTypeFare*>& paxTypeFares,
                                bool cat35NetRemitFound);

  void processGlobalNetRemit(PricingTrx& trx,
                             FarePath& fPath,
                             CollectedNegFareData* cNegFareData,
                             bool dg,
                             Diag535Collector* diag);

  void setNetRemitTicketInd(PricingTrx& trx, FarePath& farePath) const;
  void saveTourCodeForPQ(PricingTrx& trx, FarePath& farePath) const;
  void accumulateCommissionAmt(PricingTrx& trx, CollectedNegFareData* cNegFareData);
  bool isTktRestrictedForGnrUsers(const FarePath& farePath);
  bool isTfrRestrictedForGnrUsers(const FarePath& farePath);

  // data members

  bool _indicatorTkt = false;
  bool _abacusUser = false;
  bool _infiniUser = false;
  bool _isGnrUser = false;
  bool _axessUser = false;

  bool _cat35NetRemit = false; // Cat 35 Net Remit
  bool _cat35NetTicketing = false; // Cat 35 Net Ticketing

  bool _processFareBox = false;

  MoneyAmount _netTotalAmt = 0;
  Indicator _indNetGross = ' '; // N/G/B/blank
  Indicator _bspMethod = ' '; // 1/2/3/4/5/blank

  Indicator _indTypeTour = ' '; // B/C/V/T/blank
  std::string _tourCode;

  std::string _fareBox;

  Percent _comPercent = RuleConst::PERCENT_NO_APPL;
  int _noComPerDec = 0;

  CurrencyCode _currency;
  int _noDec = 0;
  MoneyAmount _comAmount = 0;

  TktDesignator _tDesignator;

  std::string _bagNo; // 0-99/blank

  WARNING_MSG _warningMsg = WARNING_MSG::NO_WARNING;

  Indicator _tktAppl = ' ';
  Indicator _tktFareDataInd = ' '; // Indicator F/A/N
  CarrierCode _tktCarrier;

  Indicator _owrt = ' ';
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  TariffNumber _ruleTariff = 0;
  CarrierCode _carrier;
  RuleNumber _rule;

  CurrencyCode _baseFareCurrency;
  CurrencyCode _paymentCurrency;

  bool _invalidNetRemComm = false;
  bool _netAmtWasFlipped = false;
  bool _isCmdPricing = false;

  PaxTypeFare* _firstCat35Fare = nullptr;
  std::string _endorsementTxt; // Cat 18 endorsement
};

} // namespace tse

