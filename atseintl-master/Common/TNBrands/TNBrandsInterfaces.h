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

#include "Common/TNBrands/BrandProgramRelations.h"
#include "Common/TseStlTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareComponentShoppingContext.h"
#include "DataModel/TNBrandsTypes.h"

namespace tse
{
class Diag892Collector;
class PricingTrx;

namespace skipper
{

// Defines interface to a "bridge" exposing brand-related data/functionality
// of a trx object in a convenient way.
class ITrxGeometryCalculator
{
public:
  virtual ~ITrxGeometryCalculator() {}

  // Returns brands and programs available for given faremarket.
  // Brands and programs are placed in the response object.
  virtual void getBrandsAndPrograms(const FareMarket& fm,
                                    BrandProgramRelations& relations) const = 0;

  // Return a reference to qualified brands vector defined
  // for the transaction
  virtual const std::vector<QualifiedBrand>& getQualifiedBrands() const = 0;

  // Calculates brands matching parity condition for all itins in this trx.
  // For each itin its properties brandCodes() and getProgramsForBrandMap()
  // are updated.
  // For all fare markets across all itin's brandProgramIndexVec() property
  // is updated (only brands fulfilling the parity requirement are left).
  // If trx isCatchAllBucketRequest() property is false all itins with no
  // brands are removed from trx.
  // Returns false only if no brand matching parity requirements found across
  // all itins and trx's property isCatchAllBucketRequest() is false.
  virtual bool calculateBrandParityForAllItins(Diag892Collector* diag892) = 0;

  // Calculates brands matching parity condition for all itins in this trx with
  // respect to fixed parts of travel.
  // For each itin its properties brandCodes() and getProgramsForBrandMap()
  // are updated.
  // For all fare markets across all itin's brandProgramIndexVec() property
  // is updated (only brands fulfilling the parity requirement are left).
  // If trx isCatchAllBucketRequest() property is false all itins with no
  // brands are removed from trx.
  // Returns false only if no brand matching parity requirements found across
  // all itins and trx's property isCatchAllBucketRequest() is false.
  virtual bool calculateContextBrandParityForAllItins(Diag892Collector* diag892) = 0;

  // Calculates brands matching parity condition for all itins in this trx with
  // respect to fixed parts of travel, and allow all brands on currently shopped leg.
  // For each itin its properties brandCodes() and getProgramsForBrandMap()
  // are updated.
  // For all fare markets across all itin's brandProgramIndexVec() property
  // is updated (only brands fulfilling the parity requirement are left).
  // If trx isCatchAllBucketRequest() property is false all itins with no
  // brands are removed from trx.
  // Returns false only if no brand matching parity requirements found across
  // all itins and trx's property isCatchAllBucketRequest() is false.
  virtual bool calculateParityBrandsOverrideForAllItins(Diag892Collector* diag892) = 0;

  // From all itins in trx removes itins for which set of brands is empty
  virtual void removeItinsWithNoBrands(Diag892Collector* diag892) = 0;

  // Modifies all fare markets removing brands not fulfilling the parity
  // requirements (valid brands given in fareMarketsParityBrands).
  // As fare markets are shared across all itins in trx, we pass trx instead
  // of itins list.
  virtual bool removeBrandsNotMatchingParity(
      BrandCodesPerFareMarket& fareMarketsParityBrands,
      Diag892Collector* diag892) = 0;

  // Returns a pointer to Context Shopping data for specified pnr segment. If
  // this data is not defined for given segment a null pointer is returned.
  virtual const FareComponentShoppingContext* getFareComponentShoppingContext(
    size_t pnrSegmentIndex) const = 0 ;

  // Returns a reference to a vector with information about leg fix status
  virtual const std::vector<bool>& getFixedLegs() const = 0;

  // Returns a reference to Trx
  virtual const PricingTrx& getTrx() const = 0;
};


// Defined interface to parity calculator algorithm. Used to be able to mock
// recursive calls.
class IFareMarketsParityCalculator
{
public:
  virtual ~IFareMarketsParityCalculator() {}

  virtual void addFareMarket(const FareMarket& fareMarket) = 0;
  virtual FareMarketsPerBrandCode possibleBrands(
    size_t startSegmentIndex, size_t endSegmentIndex) = 0;
  virtual FareMarketsPerBrandCode possibleBrands(size_t startSegmentIndex) = 0;
};


} /* namespace skipper */

} /* namespace tse */

