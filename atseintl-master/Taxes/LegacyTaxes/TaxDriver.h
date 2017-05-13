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
#ifndef TAX_DRIVER_H
#define TAX_DRIVER_H

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include "Taxes/LegacyTaxes/TaxApply.h"
#include "Taxes/LegacyTaxes/TaxApplyOnOC.h"
#include "Taxes/LegacyTaxes/TaxApplyOnChangeFee.h"
#include "Taxes/LegacyTaxes/TaxMap.h"

#include <log4cxx/helpers/objectptr.h>

#include <vector>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class CountrySettlementPlanInfo;
class FareDisplayTrx;
class PricingTrx;
class TaxNation;
class TaxResponse;
class TaxCodeReg;

// ----------------------------------------------------------------------------
// <PRE>
//
// @class Tax Orchestrator
// Description:
//
// </PRE>
// ----------------------------------------------------------------------------

class TaxApplyFactory
{
public:
  TaxApplicator& getTaxApplicator(PricingTrx& trx, const std::vector<TaxCodeReg*>& taxCodeReg,
      const DateTime& ticketDate);

private:
  TaxApplicator _taxApplicator;
  TaxApply _taxApply;
  TaxApplyOnOC _taxApplyOnOC;
  TaxApplyOnChangeFee _taxApplyOnChangeFee;
};

class TaxDriver
{
  friend class TaxDriverTest;

public:
  static constexpr char NONE = 'N';
  static constexpr char SALE_COUNTRY = 'S';
  static constexpr char ALL = 'A';
  static constexpr char SELECTED = 'I';
  static constexpr char EXCLUDED = 'E';

  static const std::string TAX_DATA_ERROR;
  static const std::string TAX_US1;
  static const std::string TAX_YZ1;

  using TaxCodes = std::vector<TaxCode>;

  void ProcessTaxesAndFees(PricingTrx& trx, TaxResponse& taxResponse,
      TaxMap::TaxFactoryMap& taxFactoryMap,
      const CountrySettlementPlanInfo* cspi);

  void ProcessTaxesAndFees_OLD(PricingTrx& trx, TaxResponse& taxResponse,
      TaxMap::TaxFactoryMap& taxFactoryMap,
      const CountrySettlementPlanInfo* cspi);

  void ProcessTaxesAndFees(FareDisplayTrx& trx, TaxResponse& taxResponse);

  void processDefaultTaxAndFees(PricingTrx& trx, TaxResponse& taxResponse,
      TaxMap& taxMap, const CountrySettlementPlanInfo* cspi);

  void processTaxesOnChangeFeeForCat33(PricingTrx& trx,
      TaxResponse& taxResponse, TaxMap& taxMap,
      const CountrySettlementPlanInfo* cspi);

  static void validateAndApplyServicesFees(PricingTrx& trx, TaxResponse& taxResponse);

  TaxDriver() = default;

private:
  TaxDriver(const TaxDriver&) = delete;
  TaxDriver& operator=(const TaxDriver&) = delete;

  void buildTaxNationVector(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            const CountrySettlementPlanInfo* cspi);

  bool addNation(PricingTrx& trx, const NationCode& inCountry);

  bool findNation(PricingTrx& trx, const NationCode& inCountry) const;

  bool findNationCollect(const NationCode& inCountry, const TaxNation& taxNation);

  void validateAndApplyTax(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxMap& taxMap,
                           const TaxCode& taxCode,
                           const CountrySettlementPlanInfo* cspi,
                           std::list<TaxCode>* pChangeFeeTaxes = nullptr);
  void validateAndApplyTaxSeq(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxMap& taxMap,
                              const std::vector<TaxCodeReg*>& taxCodeReg,
                              const DateTime& ticketDate,
                              const CountrySettlementPlanInfo* cspi);

  bool checkTicketingAgentInformation(const PricingTrx& trx) const;

  TaxApplyFactory _taxApplyFactory;
  std::vector<const TaxNation*> _taxNationVector;

  static log4cxx::LoggerPtr _logger;
};
}
#endif
