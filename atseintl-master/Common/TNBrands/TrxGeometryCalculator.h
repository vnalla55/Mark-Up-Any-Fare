//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//               Andrzej Fediuk
//
//  Copyright Sabre 2014
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


#include "Common/FallbackUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TNBrands/BrandProgramRelations.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TNBrands/ItinGeometryCalculator.h"
#include "Common/TNBrands/TNBrandsInterfaces.h"
#include "Common/TNBrands/TNBrandsUtils.h"
#include "DataModel/PricingTrx.h"

#include <set>
#include <vector>

namespace tse
{

namespace skipper
{

// A "bridge" exposing brand-related data/functionality of trx object in a
// convenient way.
//
// Shall be created as a proxy to one particular Trx, where "qualifiedBrands"
// vector is passed to the constructor. Qualified brands contains all brand
// information for a trx. It is a vector of pairs (program info, brand info),
// each telling that a particular brand is available in a particular program
// for the transaction.
class TrxGeometryCalculator: public ITrxGeometryCalculator
{
public:
  TrxGeometryCalculator(PricingTrx& trx):
    _trx(trx) {}

  // see ITrxGeometryCalculator::getBrandsAndPrograms() for documentation
  void getBrandsAndPrograms(const FareMarket& fm, BrandProgramRelations& relations) const override;

  // see ITrxGeometryCalculator::getQualifiedBrands() for documentation
  const std::vector<QualifiedBrand>& getQualifiedBrands() const override
  {
    return _trx.brandProgramVec();
  }

  // see ITrxGeometryCalculator::calculateBrandParityForAllItins() for
  // documentation
  bool calculateBrandParityForAllItins(Diag892Collector* diag892 = nullptr) override;

  // see ITrxGeometryCalculator::calculateContextBrandParityForAllItins() for
  // documentation
  bool calculateContextBrandParityForAllItins(Diag892Collector* diag892 = nullptr) override;

  // see ITrxGeometryCalculator::calculateParityBrandsOverrideForAllItins() for
  // documentation
  bool calculateParityBrandsOverrideForAllItins(Diag892Collector* diag892 = nullptr) override;

  // see ITrxGeometryCalculator::removeItinsWithNoBrands() for documentation
  void removeItinsWithNoBrands(Diag892Collector* diag892 = nullptr) override;

  // see ITrxGeometryCalculator::removeBrandsNotMatchingParity() for
  // documentation
  bool removeBrandsNotMatchingParity(BrandCodesPerFareMarket& fareMarketsParityBrands,
                                     Diag892Collector* diag892 = nullptr) override;

  // see ITrxGeometryCalculator::getFareComponentShoppingContext() for
  // documentation
  const FareComponentShoppingContext*
  getFareComponentShoppingContext(size_t pnrSegmentIndex) const override
  {
    FareComponentShoppingContextsForSegments::const_iterator found =
        _trx.getFareComponentShoppingContexts().find(pnrSegmentIndex);
    if (found == _trx.getFareComponentShoppingContexts().end())
      return nullptr;
    return found->second;
  }

  // see ITrxGeometryCalculator::getFixedLegs() for documentation
  const std::vector<bool>& getFixedLegs() const override { return _trx.getFixedLegs(); }

  // see ITrxGeometryCalculator::getTrx() for documentation
  const PricingTrx& getTrx() const override { return _trx; }

private:
  void flushDiagnostics();

  PricingTrx& _trx;
};

} /* namespace skipper */

} /* namespace tse */

