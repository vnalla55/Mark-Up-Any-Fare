//-------------------------------------------------------------------
//
//  File:        AddonConstruction.h
//  Created:     May 14, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Carrier Addon Construction
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

#include "AddonConstruction/ConstructionDefs.h"


namespace tse
{

class ConstructionJob;
class ConstructionVendor;
class ConstructedCacheDataWrapper;

class AddonConstruction
{

  friend class ConstructionVendorTest;

public:
  typedef std::pair<VendorCode, CarrierCode> VCPair;
  typedef std::set<VCPair> VCList;
  typedef VCList::iterator VCListI;

public:
  // Main interface
  // ==== =========

  static bool defineVendorCarrierPairs(DataHandle& dataHandle,
                                       const CarrierCode& carrier,
                                       const LocCode& origin,
                                       const LocCode& boardCity,
                                       const LocCode& destination,
                                       const LocCode& offCity,
                                       VCList& vcPairs,
                                       bool disableYY = false);

  static bool runConstructionProcess(ConstructionJob& cj, ConstructedCacheDataWrapper& dw);

  static bool runReconstructionProcess(ConstructionJob& cj, ConstructedCacheDataWrapper& dw);

  static void createResponse(ConstructionJob& cj, CacheConstructedFareInfoVec& cachedFares);

private:
  static bool defineVendorCarrierPairs(DataHandle& dataHandle,
                                       const CarrierCode& carrier,
                                       const LocCode& interiorMarket,
                                       VCList& vcPairs);

  static void processConstructionPoint(ConstructionJob& cj, ConstructionPoint cp);

  static unsigned int
  processAddonFares(ConstructionJob& cj, ConstructionPoint cp, const LocCode location);

  static void getVendor(ConstructionJob& cj);

  static AddonZoneStatus isApplicableForRw(ConstructionJob& cj, const AddonFareInfo& addonFare);

}; // End of class AddonConstruction

} // End namespace tse

