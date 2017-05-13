// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"

#include <memory>
#include <vector>

namespace tse
{
class Loc;
class PricingTrx;
class TaxResponse;
class TaxNation;
class CountrySettlementPlanInfo;

class AbstractGetTaxNation
{
public:
  virtual const std::vector<const TaxNation*>& get() const = 0;
};


class GetTaxNation : public AbstractGetTaxNation
{
public:
  GetTaxNation(PricingTrx& trx, TaxResponse& taxResponse,
      const CountrySettlementPlanInfo* cspi);
  const std::vector<const TaxNation*>& get() const;

private:
  bool addNation(PricingTrx& trx, const NationCode& inCountry);
  bool findNation(PricingTrx& trx, const NationCode& inCountry) const;
  bool findNationCollect(const NationCode& inCountry, const TaxNation& taxNation) const;
  const Loc* getPointOfSaleLocation(PricingTrx& trx) const;
  const TaxNation* getTaxNation(PricingTrx& trx, const NationCode& nation);

  std::vector<const TaxNation*> _taxNationVector;
  const Loc* _pointOfSaleLocation;

  static constexpr char NONE = 'N';
  static constexpr char SALE_COUNTRY = 'S';
  static constexpr char ALL = 'A';
  static constexpr char SELECTED = 'I';
  static constexpr char EXCLUDED = 'E';
};

} // end of tse namespace
