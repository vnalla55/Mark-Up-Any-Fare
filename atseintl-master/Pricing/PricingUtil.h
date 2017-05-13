//-------------------------------------------------------------------
// File:    PricingUtil.h
// Created: April 2005
// Authors: Andrew Ahmad
//
//  Copyright Sabre 2005
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

#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/MergedFareMarket.h"
#include "Rules/RuleControllerWithChancelor.h"

namespace tse
{
class PricingTrx;
class FarePath;
class FareUsage;
class GroupFarePath;
class MergedFareMarket;
class PaxTypeFare;
class TaxItem;

struct FarePathDownToFareUsage
{
  FarePathDownToFareUsage(PricingTrx& trx, FarePath& fPath) : _farePath(fPath), _trx(trx)
  {
    _categories.push_back(18);
    RuleControllerWithChancelor<PricingUnitRuleController> rc(NormalValidation, _categories, &trx);
    _rc = rc;
  }

  void operator()(PricingUnit* pu) { _rc.validate(_trx, _farePath, *pu); }

private:
  std::vector<uint16_t> _categories;
  RuleControllerWithChancelor<PricingUnitRuleController> _rc; //(RuleController::NormalValidation,
                                                              //categories);
  FarePath& _farePath;
  PricingTrx& _trx;
};

class AltDateCellCompare
{
public:
  AltDateCellCompare(DatePair& datePair) : _datePair(datePair) {}

  bool operator()(const std::pair<const DatePair, PricingTrx::AltDateInfo*>& p)
  {
    if (_datePair.first.date() == p.first.first.date() &&
        _datePair.second.date() == p.first.second.date())
    {
      return true;
    }
    return false;
  }

private:
  const DatePair& _datePair;
};

class PricingUtil
{
  friend class PricingUtilTest;

public:
  // Calculate and update FarePath's ISI code
  static Itin::ISICode calculateFarePathISICode(const PricingTrx& trx, FarePath& farePath);

  // Calculate ISI code
  static Itin::ISICode calculateISICode(const PricingTrx& trx, const FarePath& farePath);
  /**
  *
  *   @method determineBaseFare
  *
  *   Description: Determines the base fare currency on a per fare path basis.
  *                For Foreign domestic travel it will roll back the NUC amounts
  *                to the original currency if they are all in one currency.
  *
  *   @param  FarePath      - farePath
  *   @param  PricingTrx    - trx
  *   @param  Itin          - itinerary
  *
  *   @return void
  */
  static void determineBaseFare(FarePath* farePath, PricingTrx& trx, Itin* itin);
  /**
  *
  *   @method checkOriginationCurrency
  *
  *   Description: Checks to see if the origination currency needs to be changed
  *                for an international itinerary. this is due to
  *                the final fare path containing multiple currency codes
  *                or an arunk segment in the itinerary which might
  *                mean international travel originates in a different
  *                nation.
  *
  *   @param  PricingTrx      -  pricing transaction
  *   @param  Itin            - itinerary.
  *
  *   @return void
  *
  */
  static void checkOriginationCurrency(PricingTrx& trx,
                                       const Itin* itin,
                                       FarePath* farePath,
                                       CurrencyCode& originationCurrency);

  /**
  *
  *   @method farePathCurrencyCodesAreSame
  *
  *   Description: Checks to see if all currency codes for one passenger type
  *                for an international itinerary the same.
  *
  *   @param  FarePath    - farePath
  *
  *   @return unsigned int        - 1 - they are all the same, else 0.
  *
  */
  static bool farePathCurrencyCodesAreSame(PricingTrx& trx,
                                           const FarePath* farePath,
                                           MoneyAmount& cat12SurchargeAmount,
                                           MoneyAmount& stopOverSurchAmount,
                                           MoneyAmount& transfersSurchAmount,
                                           bool multiCurrencyPricing,
                                           bool& fareChgsAreSame);

  static void fareRelatedCurrencyChgsAreSame(const std::vector<FareUsage*>& fareUsageVector,
                                             MoneyAmount& cat12SurchargeAmount,
                                             MoneyAmount& stopOverSurchAmount,
                                             MoneyAmount& transfersSurchAmount,
                                             unsigned int& multCurrencies,
                                             bool& fareChgsSame);

  /**
  *
  *   @method setFarePathBaseFareCurrency
  *
  *   Description: Determines the base fare on a per fare path basis
  *
  *   @param  FarePath      - farePath
  *   @param  PricingTrx    - trx
  *   @param  CurrencyCode  - originationCurrency - where international travel
  *                           originates
  *
  *   @param  bool          - sameCurrencyCodes, true - all currency codes for this
  *                           fare path are the same, else false
  *
  *   @return void          -
  */
  static void setFarePathBaseFareCurrency(FarePath* farePath,
                                          PricingTrx& trx,
                                          CurrencyCode& originationCurrency,
                                          bool sameCurrencyCodes);

  /**
  *
  *   @method currencyRollBack
  *
  *   Description:  For Foreign Domestic itinerary( in future also might include
  *                 Domestic) and if the AAA location allows multi-currency
  *                 pricing we calculate everything in NUCs. Then if the final
  *                 fare path is priced in only one currency we roll back the
  *                 total nuc amounts on the PaxType fare to the original
  *                 fare amounts.
  *
  *   @param  FarePath      - farePath
  *   @param  PricingTrx    - trx
  *   @param  MoneyAmount   - accumulated CAT 12 surcharge amount
  *   @param  MoneyAmount   - accumulated stop overs surcharge amount
  *   @param  MoneyAmount   - accumulated transfers surcharge amount
  *   @param  bool          - true/false multi-currency pricing
  *
  *   @return void
  */
  static void currencyRollBack(FarePath& farePath,
                               PricingTrx& trx,
                               MoneyAmount& totalCat12SurchAmt,
                               MoneyAmount& totalStopOversSurchAmt,
                               MoneyAmount& totalTransfersSurchAmt);

  /**
  *
  *   @method rollBackFareRelatedCharges
  *
  *   Description:  For Foreign Domestic itinerary we should only roll back
  *                 fare related charges for multi-currency pricing.
  *
  *   @param  FareUsage     - fareUsage
  *
  *   @return void
  */
  static void rollBackFareRelatedCharges(const PricingTrx& trx, FareUsage* fareUsage);

  /**
  *
  *   @method setFarePathCalculationCurrency
  *
  *   Description: Determines the calculation currency for a given fare path
  *
  *   @param  FarePath      - farePath
  *   @param  PricingTrx    - trx
  *   @param  CurrencyCode  - originationCurrency - where international travel
  *                           originates
  *   @param  bool          - whether or not this is multi-currency pricing
  *   @param  bool          - sameCurrencyCodes, true - all currency codes for this
  *                           fare path are the same, else false
  *
  *   @return void
  */
  static void setFarePathCalculationCurrency(FarePath* farePath,
                                             PricingTrx& trx,
                                             CurrencyCode& originationCurrency,
                                             bool isMultiCurrencyPricing,
                                             bool sameCurrencyCodes);

  static bool hasAnyCAT25SpecialFares(FarePath* farePath);

  //
  // Description: Check if there is a Carrier Fare corresponding to this
  // YY-Fare
  //
  //
  static bool cxrFareTypeExists(const MergedFareMarket& mfm,
                                const PaxTypeFare& paxTypeFare,
                                const PricingUnit::Type puType,
                                const Directionality puDir,
                                const PaxType* paxType,
                                const CarrierCode&  valCxr);

  static bool cxrFareTypeExists(const MergedFareMarket& mfm,
                                const PaxTypeFare& paxTypeFare,
                                const PricingUnit::Type puType,
                                const Directionality puDir,
                                const PaxType* paxType,
                                const std::vector<CarrierCode>&  valCxrList);

  static bool cxrFareTypeExists_old(const MergedFareMarket& mfm,
                                const PaxTypeFare& paxTypeFare,
                                const PricingUnit::Type puType,
                                const Directionality puDir,
                                const PaxType* paxType);

  static MergedFareMarket::CxrFareTypeBitSet
  getMergedFareMarketPuBitSet(const PricingUnit::Type puType, const Directionality puDir);

  /**
  *
  *   @method processCat35Combination
  *
  *   Description: Process IT/BT, Net Remit cat35 fare combination.
  *
  *   @param  FarePath      - farePath
  *   @param  PricingTrx    - trx
  *
  *   @return bool          - true for WP... , false - for failed farePath on WPA entry
  */
  static bool processCat35Combination(PricingTrx& trx, FarePath& farePath);

  /**
   *
   *   @method hasNoFareConstructionCharges
   *
   *   Description: Checks to see if there are any differential, class of service,
   *                or min fare construction charges.
   *
   *   @param  FarePath      - farePath
   *   @param  PricingTrx    - trx
   *
   *   @return bool          - true - there are no fare construction charges, else false
   */
  static bool hasNoFareConstructionCharges(PricingTrx& trx, FarePath& farePath);

  // To collect endorsements for Cat18
  static void collectEndorsements(PricingTrx& trx, FarePath& farePath);

  // If all segments are requiring command pricing
  static bool isFullCPTrx(const PricingTrx& trx);

  static void checkTrxAborted(PricingTrx& trx,
                              const uint32_t& threshold,
                              const uint32_t& curCombCount,
                              bool& maxNbrCombMsgSet);

  static void discountOrPlusUpPricing(PricingTrx& trx, FarePath& farePath);

  static bool finalPricingProcess(PricingTrx& trx, FarePath& farePath);

  static bool finalFPathCreationForAxess(PricingTrx& trx, FarePath& farePath, FarePath& fPathAxess);

  static bool roundPeruTotalAmount(PricingTrx& trx, FarePath& farePath);

  static const PaxTypeFare* determinePaxTypeFare(const PaxTypeFare* ptFare);

  static bool isDivideParty(const PricingTrx& trx, std::vector<FarePath*>& groupFP);

  static MoneyAmount getRollBackedNucAmount(PricingTrx& trx,
                                            FareUsage* fareUsage,
                                            PricingUnit& pricingUnit,
                                            bool useIntlRounding);

  static void determineCat27TourCode(FarePath& farePath);

  static bool isJLExempt(PricingTrx& trx, const FarePath& farePath);
  static bool isJLExemptAccntCode(PricingTrx& trx);
  static bool isJLExemptTktDesig(PricingTrx& trx, const FarePath& farePath);

  /**
   * @return true for vendor that support Highest RT check
   */
  static bool allowHRTCForVendor(const PricingTrx& trx, const PaxTypeFare* fare);

  /**
   * Calls allowHRTFareForVendor on each FC
   * @return true only when vendor for all FC is allowed for HRTC
   */
  static bool allowHRTCForVendor(const PricingTrx& trx, const PricingUnit& pu);

  static bool isMirrorImage(const PricingUnit& pu);

  static MoneyAmount
  readSurchargeAndConvertCurrency(PricingTrx& trx, const Itin& curItin, const FareUsage& fareUsage);

  static MoneyAmount readTaxAndConvertCurrency(PricingTrx& trx,
                                               const TaxResponse *taxResponse,
                                               Itin& curItin,
                                               const PaxTypeFare& ptf,
                                               PricingUnit& prU,
                                               bool& noBookingCodeTax,
                                               BookingCode& bkCode1,
                                               BookingCode& bkCode2,
                                               BookingCode& bkCode3,
                                               DiagCollector& diag);

  static MoneyAmount convertCurrency(const PricingTrx& trx,
                                     const MoneyAmount& amount,
                                     const CurrencyCode& targetCurrency,
                                     const CurrencyCode& sourceCurrency);

  static bool validateSpanishDiscountInFP(FarePath* farePath);
  static BrandCodeSet getCommonBrandsFromPU(PricingTrx& trx, const PricingUnit* pricingUnit);
  static void intersectCarrierList(std::vector<CarrierCode>& list1, const std::vector<CarrierCode>& list2);
  static NetFarePath* createNetFarePath(PricingTrx& trx, FarePath& farePath, bool isRegularNet = false);
  static bool adjustedSellingCalcDataExists(const FarePath& farePath);

  static CurrencyCode getPaymentCurrency(PricingTrx& trx);

protected:
  static void processAdjustedSellingFarePath(PricingTrx& trx,
                                             FarePath& farePath,
                                             bool isMslAdjusted);

  static FarePath* cloneFarePathForAdjusted(PricingTrx& trx, FarePath& farePath);

  static void displayAdjustedSellingFarePath(PricingTrx& trx, const FarePath& fp);

  static MoneyAmount convertCurrencyForMsl(PricingTrx& trx,
                                           MoneyAmount amount,
                                           const CurrencyCode& targetCurrency,
                                           const CurrencyCode& sourceCurrency);

  static MoneyAmount getFuAmountInCalculationCurrency(PricingTrx& trx,
                                                      const FarePath& farePath,
                                                      const PaxTypeFare& ptf,
                                                      MoneyAmount amountInPtfCurrency);

  static MoneyAmount getAdjustmentAmountInFareCurrency(PricingTrx& trx,
                                                       const FarePath& farePath,
                                                       const PaxTypeFare& ptf,
                                                       MoneyAmount amountInCalculationCurrency);

  static MoneyAmount getManualAdjustmentAmount(PricingTrx& trx, FarePath& farePath);

  static MoneyAmount getAslMileageDiff(bool isMslAdjusted, PaxTypeFare& newPtf, FarePath& newFp);

  static bool getManualAdjustmentAmountsPerFUHelper(const std::vector<MoneyAmount>& perFuAmount,
                                                   MoneyAmount adjustmentAmount,
                                                   std::vector<MoneyAmount>& perFuAdjustmentAmts);

  static void getManualAdjustmentAmountsPerFU(PricingTrx& trx,
                                              FarePath& farePath,
                                              std::vector<MoneyAmount>& perFuAdjustmentAmts);

  static void updateAslMslDifference(PricingTrx& trx, FarePath& farePath);

private:
  // Private constructor
  PricingUtil();
  ~PricingUtil();

  static void processConsolidatorPlusUp(PricingTrx& trx, FarePath& farePath);
  static void setBookingCodeFromTax(const TaxItem& taxItem,
                                    bool& noBookingCodeTax,
                                    BookingCode& bkCode1,
                                    BookingCode& bkCode2,
                                    BookingCode& bkCode3,
                                    DiagCollector& diag);

  static void printTaxItem(const TaxItem& taxItem, DiagCollector& diag);

  static void processNetFarePath(PricingTrx& trx, FarePath& farePath, bool isRegularNet = false);

  static const char SMF_VENDOR;
};

}; // end of namespace tse

