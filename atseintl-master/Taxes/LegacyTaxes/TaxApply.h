// ---------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------
#ifndef TAX_APPLY_H
#define TAX_APPLY_H

#include <string>
#include "Common/TseCodeTypes.h"

#include <log4cxx/helpers/objectptr.h>
namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class CountrySettlementPlanInfo;
class PricingTrx;
class TaxResponse;
class TaxCodeReg;
class TaxMap;
class Tax;

// ----------------------------------------------------------------------------
// TaxApply will call all the TaxValidator functions for all special taxes
// ----------------------------------------------------------------------------

class TaxApplicator
{
public:
  TaxApplicator() {}
  virtual ~TaxApplicator() {}

  virtual Tax* findSpecialTax(TaxMap& taxMap, TaxCodeReg& taxCodeReg);

  virtual void applyTax(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxMap& taxMap,
                        TaxCodeReg& taxCodeReg,
                        const CountrySettlementPlanInfo* cspi)
  {
  }

protected:
  virtual void validateTaxSeq(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              Tax& tax,
                              TaxCodeReg& taxCodeReg,
                              const CountrySettlementPlanInfo* cspi)
  {
  }

  virtual void
  initializeTaxItem(PricingTrx& trx, Tax& tax, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
  {
  }
};

class TaxApply : public TaxApplicator
{
public:
  static constexpr char PERCENTAGE = 'P';
  static constexpr char APPLY_TAX_ONCE_PER_BOARD_OFF = 'O';
  static const uint16_t MULTIPLE_PERCENTAGE_TAX31 = 31;
  static const uint16_t MULTIPLE_PERCENTAGE_TAXKH1 = 65; // dvb
  static const uint16_t MULTIPLE_PERCENTAGE_TAX_JP1_00 = 3200;
  static const std::string HIDDEN_POINT_PARAM_NAME;
  static const std::string HIDDEN_POINT_LOC1_VALUE;
  static const std::string HIDDEN_POINT_LOC2_VALUE;
  static const std::string HIDDEN_POINT_BOTH_LOCS_VALUE;

  TaxApply();
  virtual ~TaxApply();

  virtual void applyTax(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxMap& taxMap,
                        TaxCodeReg& taxCodeReg,
                        const CountrySettlementPlanInfo* cspi) override;

  virtual void validateTaxSeq(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              Tax& tax,
                              TaxCodeReg& taxCodeReg,
                              const CountrySettlementPlanInfo* cspi) override;

  virtual void initializeTaxItem(PricingTrx& trx,
                                 Tax& tax,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg) override;

protected:
  void setHiddenStopInfo(PricingTrx& trx, Tax& tax, TaxCodeReg& taxCodeReg);

  bool applyOccurenceValidation(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                const Tax& tax,
                                TaxCodeReg& taxCodeReg);

  bool doInitialTravelSegValidations(PricingTrx& trx,
                                     TaxResponse& taxResponse,
                                     Tax& tax,
                                     TaxCodeReg& taxCodeReg);

  bool doFinalTravelSegValidations(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   Tax& tax,
                                   TaxCodeReg& taxCodeReg);

  void setMixedTaxType(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  bool isReissueOrRefund(PricingTrx& trx) const;

  // private:
  uint16_t _travelSegStartIndex;
  uint16_t _travelSegEndIndex;
  static log4cxx::LoggerPtr _logger;

private:
  TaxApply(const TaxApply& apply);
  TaxApply& operator=(const TaxApply& apply);
};
}
#endif
