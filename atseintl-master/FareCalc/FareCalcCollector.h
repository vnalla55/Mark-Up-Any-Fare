//----------------------------------------------------------------
//
//  File:  FareCalcCollector
//  Authors: Mike Carroll
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
#include "DataModel/DifferentialData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SurchargeData.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/LocKey.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FcMultiMessage.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <map>
#include <set>
#include <vector>

namespace tse
{
class FareCalculation;
class FarePath;
class Itin;
class PricingTrx;

namespace FareCalc
{
  class FcCollectorTest;
}

class FareCalcCollector
{
  friend class FareCalcCollectorTest;
  friend class FareCalc::FcCollectorTest;

public:
  typedef std::vector<std::pair<char, std::string> > MsgResponseVector;

  FareCalcCollector() : _validBrandsVecPtr(nullptr), _simultaneousResTkt(false), _isFlexFare(false) {}

  virtual ~FareCalcCollector() {}

  //--------------------------------------------------------------------------
  // @function FareCalcCollector::initialize
  //
  // Description: Prepare data members for both the 854 diagnostic and WP
  //              return values.
  //
  // @param pricingTrx - a valid pricing trx
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool initialize(PricingTrx& pricingTrx,
                          Itin* itin,
                          const FareCalcConfig* fcConfig,
                          bool skip = false);
  //--------------------------------------------------------------------------
  // @function FareCalcCollector::getFareCalculation
  //
  // Description: create the FareCalculation object to handle formating the
  //              data collected by this Collector
  //
  // @param  diag  - a valid DiagCollector
  // @return fareCalculation object
  //--------------------------------------------------------------------------
  virtual FareCalculation* createFareCalculation(PricingTrx* trx, const FareCalcConfig* fcConfig);

  //--------------------------------------------------------------------------
  // @function FareCalcCollector::isMixedBaseFareCurrency
  //
  // Description:   Checks if mixed currencies are involved
  //
  // @return bool
  // -------------------------------------------------------------------------
  bool isMixedBaseFareCurrency(bool forNetRemit = false);

  //--------------------------------------------------------------------------
  // @function FareCalcCollector::getBaseFareTotal
  //
  // Description:   Get the accumulated total for all base fares
  //
  // @param currencyCodeUsed
  // @param noDecUsed
  // @return MoneyAmount
  // -------------------------------------------------------------------------
  MoneyAmount getBaseFareTotal(PricingTrx& trx,
                               CurrencyCode& currencyCodeUsed,
                               CurrencyNoDec& noDecUsed,
                               bool forNetRemit,
                               const uint16_t brandIndex = INVALID_BRAND_INDEX);

  //--------------------------------------------------------------------------
  // @function FareCalcCollector::isMixedEquivFareAmountCurrency
  //
  // Description:   Checks if there are any different equivalent amounts
  //
  // @return bool
  // -------------------------------------------------------------------------
  bool isMixedEquivFareAmountCurrency();

  MoneyAmount getEquivFareAmountTotal(PricingTrx& trx,
                                      CurrencyCode& currencyCodeUsed,
                                      CurrencyNoDec& noDecUsed,
                                      bool forNetRemit,
                                      const uint16_t brandIndex = INVALID_BRAND_INDEX);

  //--------------------------------------------------------------------------
  // @function FareCalcCollector::getTaxTotal
  //
  // Description:   Get the accumulated total for all taxes
  //
  // @param currencyCodeUsed
  // @param noDecUsed
  // @return MoneyAmount
  // -------------------------------------------------------------------------
  MoneyAmount getTaxTotal(PricingTrx& trx,
                          CurrencyCode& currencyCodeUsed,
                          CurrencyNoDec& noDecUsed,
                          bool forNetRemit,
                          const uint16_t brandIndex = INVALID_BRAND_INDEX);

  //--------------------------------------------------------------------------
  // Accessors
  //--------------------------------------------------------------------------
  std::string& IATASalesCode() { return _IATASalesCode; }
  const std::string& IATASalesCode() const { return _IATASalesCode; }

  std::string& lastTicketDay() { return _lastTicketDay; }
  const std::string& lastTicketDay() const { return _lastTicketDay; }

  std::string& lastTicketTime() { return _lastTicketTime; }
  const std::string& lastTicketTime() const { return _lastTicketTime; }

  bool& simultaneousResTkt() { return _simultaneousResTkt; }
  const bool simultaneousResTkt() const { return _simultaneousResTkt; }

  std::vector<CalcTotals*>& passengerCalcTotals() { return _passengerCalcTotals; }
  const std::vector<CalcTotals*>& passengerCalcTotals() const { return _passengerCalcTotals; }

  struct FarePathCompare : public std::binary_function<const FarePath*, const FarePath*, bool>
  {
    bool operator()(const FarePath* lhs, const FarePath* rhs) const
    {
      return lhs < rhs;
    }
  };

  typedef std::map<const FarePath*, CalcTotals*, FarePathCompare> CalcTotalsMap;
  typedef std::map<BaggageFcTagType, CalcTotals*> BaggageTagMap;

  CalcTotals* getCalcTotals(PricingTrx* trx, const FarePath* fp, const FareCalcConfig* fcConfig);

  CalcTotals* findCalcTotals(const FarePath* fp) const;
  CalcTotals*
  findCalcTotals(const PaxType* paxType, const uint16_t brandIndex = INVALID_BRAND_INDEX) const;

  const CalcTotalsMap& calcTotalsMap() const { return _calcTotalsMap; }

  BaggageTagMap& baggageTagMap() { return _baggageTagMap; }
  const BaggageTagMap& baggageTagMap() const { return _baggageTagMap; }

  void collectBkgMessage(PricingTrx& trx, Itin* itin);
  void collectBkgRebookMessage(PricingTrx& trx, Itin* itin, CalcTotals* calcTotals);
  void collectPriceByCabinMessage(PricingTrx& trx, Itin* itin, CalcTotals* calcTotals);
  void collectWPNCBPriceByCabinTrailerMessage(PricingTrx& trx, Itin* itin);

  void collectCmdPrcMessage(PricingTrx& trx, const FareCalcConfig* fcConfig);

  void collectCmdPrcMessage(PricingTrx& trx,
                            const FareCalcConfig* fcConfig,
                            const PaxTypeFare::PaxTypeFareCPFailedStatus cpFStatus,
                            CalcTotals* calcTotals);

  //--------------------------------------------------------------------------
  // Convenience routines
  //--------------------------------------------------------------------------
  CurrencyCode getTotalPriceCurrency();

  bool isFareCalcTooLong();

  bool hasNoMatch() const;

  void addMultiMessage(PricingTrx& trx, const FarePath* farePath, const FcMessage& msg);

  const std::map<std::string, FcMultiMessage>& multiMessage() const { return _multiMessage; }

protected:
  virtual CalcTotals*
  createCalcTotals(PricingTrx& pricingTrx, const FareCalcConfig* fcConfig, const FarePath* fp);

  MoneyAmount getTotalInternal(PricingTrx& pricingTrx,
                               uint16_t actionCode,
                               CurrencyCode& currencyCodeUsed,
                               CurrencyNoDec& noDecUsed,
                               bool forNetRemit,
                               const uint16_t brandIndex);

private:
  std::vector<BrandCode>* _validBrandsVecPtr;

  std::string _lastTicketDay;
  std::string _lastTicketTime;
  std::string _IATASalesCode;

  bool _simultaneousResTkt;

  // TODO: deprecated, see _calcTotalsMap
  std::vector<CalcTotals*> _passengerCalcTotals;

  CalcTotalsMap _calcTotalsMap;
  BaggageTagMap _baggageTagMap;
  DataHandle _dataHandle;

  CalcTotals* createCalcTotals(PricingTrx& pricingTrx,
                               const FareCalcConfig* fcConfig,
                               const Itin* itin,
                               const PaxType* paxType);

  void addCalcTotals(const FarePath* fp, CalcTotals* ct);

  const std::string& getIataSalesCode(const FarePath* fp);

  void refreshTaxResponse(PricingTrx& pricingTrx, Itin& itin, const FareCalcConfig& fcConfig);

  void processConsolidatorPlusUp(PricingTrx& pricingTrx, Itin*& itin, FarePath*& fp);

  bool createNetRemitCalcTotal(PricingTrx& pricingTrx,
                               const FareCalcConfig* fcConfig,
                               FarePath* fp,
                               CalcTotals* totals);
  bool createNetCalcTotal(PricingTrx& pricingTrx,
                          const FareCalcConfig* fcConfig,
                          FarePath* fp,
                          CalcTotals* totals);

  bool createAdjustedCalcTotal(PricingTrx& pricingTrx,
                               const FareCalcConfig* fcConfig,
                               const FarePath& adjustedSellingFarePath,
                               CalcTotals* totals);

  void copyItemsToAdjustedFP(PricingTrx& pricingTrx, FarePath* fp);
  void copyFcCommInfoColToAdjustedFP(FarePath& fp) const;

  void processAdjustedSellingDiffInfo(PricingTrx& pricingTrx, CalcTotals& calcTotals);

  MoneyAmount getAdjustedSellingDifference(PricingTrx& trx, CalcTotals& calcTotals);

  void reorderPaxTypes(PricingTrx& pricingTrx) const;

  std::map<std::string, FcMultiMessage> _multiMessage;

  bool _isFlexFare;
};

} /* end namespace tse */

