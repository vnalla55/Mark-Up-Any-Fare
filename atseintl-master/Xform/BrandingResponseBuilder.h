//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "DataModel/BrandingTrx.h"

#include <iostream>
#include <map>
#include <set>
#include <string>

namespace tse
{

// Build a response structure for branding request
// The structure has the following layout:
// itin
//        |-> brandId
//        |           |-> programId
// itin ----> brandId ----> programId
//        |           |-> programId...
//        |-> brandId...
//
// itin...
// Itins are represented as integers (denoting the
// index in the itin vector) and all other data
// as strings
class BrandingResponseBuilder
{
public:
  void addItin(const Itin* itin);
  // Adds a triple (itin, program, brand) to the response structure
  // Result data set is extended by the new information,
  // unless the whole triple is a duplicate. Then no update
  // occurs and also no error is raised.
  void addBrandForItin(const Itin* itin,
                       const std::string& programId,
                       const std::string& brandId);

  // Get the response structure.
  const BrandingResponseType& getBrandingResponse() const { return _returnMap; }

private:
  // Here we accumulate input data.
  BrandingResponseType _returnMap;
};

class BrandingRequestUtils
{
public:
  // Produces output data for branding request N06=N
  //
  // Inspects Itin objects in the transaction and builds a map
  // containing the response for a branding request.
  // (for map format, see BrandingResponseBuilder)
  static BrandingResponseType trxToResponse(const BrandingTrx& trx);
};

class FareCalcCollector;
class PaxType;
class CalcTotals;

class BrandingResponseUtils
{
public:
  // Produces output data for branding request N06=N
  //
  // Inspects Itin objects in the transaction and builds a map
  // containing the response for a branding request.
  // (for map format, see BrandingResponseBuilder)

  static
  std::pair<const FarePath*, CalcTotals*>
  findFarePathAndCalcTotalsForPaxTypeAndBrand(const Itin& itin,
                                              const FareCalcCollector& calc,
                                              const PaxType* paxType,
                                              const size_t brandIndex,
                                              const PricingTrx& pricingTrx);

  static const FarePath*
  findFarePathForPaxTypeAndBrand(const Itin& itin,
                                 const PaxType* paxType,
                                 const size_t brandIndex,
                                 const PricingTrx& pricingTrx);

  static void addItinSpecificBrandsToResponse(const Itin* itin, BrandingResponseBuilder& brb);
  static void addAnyBrandToBrandedItins(const Itin* itin, BrandingResponseBuilder& brb);
};

} // namespace tse

