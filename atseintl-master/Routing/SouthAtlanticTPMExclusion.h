//----------------------------------------------------------------------------
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/Singleton.h"
#include "Routing/MileageExclusion.h"
#include "Routing/MileageRoute.h"

namespace tse
{
/**
 * @class SouthAtlanticTPMExclusionOld.
 * A singleton - stateless class to apply MileageExclusion logi. .
 * Applies IATA exceptions and carrier exception for mileage when
 * travel is southAmerica/SouthAtlnatic to area3 or middle east.
 * Please refer to RoutingRequirement section 3.3.6.6.2 for further
 * details.
 */

class MileageRouteItem;
class Loc;
class TPMExclusion;
class Diag452Collector;
class MileageRouteItem;
class MultiTransportRetriever;

class SouthAtlanticTPMExclusion : public MileageExclusion
{
  friend class SouthAtlanticTPMExclusionTest;
  friend class SouthAtlanticTPMExclusionMock;

public:
  // Concrete implementation of apply() for MileageExclusion.
  // @param MileageRoute& A coolection of mileage route items.
  // @returns true if exclusion is applicable. False otherwise.

  virtual bool apply(MileageRoute&) const override;

  enum TPMExclusionFailCode
  {
    MATCHED,
    FAILED_CRS,
    FAILED_GOV_CXR,
    FAILED_SEQ_NO,
    FAILED_NOT_APPL_TO_YY,
    FAILED_ONLINE_SERVICE_ONLY,
    FAILED_DIR_FARE_COMPONENT,
    FAILED_FC_LOC_1,
    FAILED_FC_LOC_2,
    FAILED_GI,
    FAILED_SR_1_APPL,
    FAILED_SR_1_LOC_1,
    FAILED_SR_1_LOC_2,
    FAILED_SR_2_APPL,
    FAILED_SR_2_LOC_1,
    FAILED_SR_2_LOC_2,
    FAILED_VIA_POINT_RES,
    FAILED_CSR_ONLINE_ON_GOV,
    FAILED_SURFACE_PER,
    FAILED_CREATE_DATE,
    FAILED_FIRST_SALE_DATE,
    FAILED_LAST_SALE_DATE,
    FAILED_EXPIRE_DATE,
    // extra error codes
    FAILED_SR_1,
    FAILED_SR_2,
    FAILED_MULTIHOST,
  };

  static const LocCode& multiCityOrig(const MileageRouteItem& mi);
  static const LocCode& multiCityDest(const MileageRouteItem& mi);

private:
  void cloneMileageRoute(const MileageRoute& src,
                         MileageRoute& dst,
                         std::vector<std::pair<uint8_t, uint8_t> >& idxMap,
                         DataHandle& dataHandle) const;
  void copyResponseMileageRoute(MileageRoute& src,
                                MileageRoute& dst,
                                const std::vector<std::pair<uint8_t, uint8_t> >& idxMap) const;
  virtual const MultiTransportRetriever& getMultTransretriever() const;

  bool process(MileageRoute& mRoute, Diag452Collector* diag) const;
  TPMExclusionFailCode matchTPMExclusion(TPMExclusion& tpm, MileageRoute& mRoute) const;
  bool matchGlobalDirection(MileageRoute& mRoute, const TPMExclusion& tpm) const;
  bool matchYY(MileageRoute& mRoute, const TPMExclusion& tpm) const;
  bool matchOlineSvcOnly(MileageRoute& mRoute, const TPMExclusion& tpm) const;
  bool matchCRS(MileageRoute& mRoute, const TPMExclusion& tpm) const;
  bool matchMultiHost(MileageRoute& mRoute, const TPMExclusion& tpm) const;
  bool matchLocation1(MileageRoute& mRoute, const TPMExclusion& tpm) const;
  bool matchLocation2(MileageRoute& mRoute, const TPMExclusion& tpm) const;
  bool matchSector1(MileageRoute& mRoute,
                    const TPMExclusion& tpm,
                    MileageRouteItems::iterator& sec1) const;
  bool matchSector2(MileageRoute& mRoute,
                    const TPMExclusion& tpm,
                    MileageRouteItems::iterator& sec2) const;
  bool matchConsecutive(MileageRoute& mRoute,
                        bool asc,
                        const MileageRouteItems::iterator& sec1,
                        const MileageRouteItems::iterator& sec2) const;
  bool matchSectorAppl(MileageRoute& mRoute,
                       const TPMExclusion& tpm,
                       const MileageRouteItems::iterator& sec,
                       bool) const;
  bool matchViaPointOfIntrestr(MileageRoute& mRoute,
                               bool asc,
                               const TPMExclusion& tpm,
                               const MileageRouteItems::iterator& sec1,
                               const MileageRouteItems::iterator& sec2) const;
  bool matchConsecMustBeOnGov(MileageRoute& mRoute,
                              const TPMExclusion& tpm,
                              bool asc,
                              const MileageRouteItems::iterator& sec1,
                              const MileageRouteItems::iterator& sec2) const;
  bool matchSurfacePermitted(MileageRoute& mRoute,
                             const TPMExclusion& tpm,
                             bool asc,
                             const MileageRouteItems::iterator& sec1,
                             const MileageRouteItems::iterator& sec2) const;
  void markSectors(MileageRoute& mRoute,
                   bool asc,
                   const MileageRouteItems::iterator& sec1,
                   const MileageRouteItems::iterator& sec2) const;

  bool checkCodeAndDoDiag(Diag452Collector* diag,
                          MileageRoute& mRoute,
                          TPMExclusion* tpm,
                          const SouthAtlanticTPMExclusion::TPMExclusionFailCode failCode,
                          bool& returnCode) const;

  virtual bool matchLocation(const Loc* l,
                             const LocTypeCode& locType,
                             const LocCode& loc,
                             MileageRoute& mRoute,
                             CarrierCode cxr = EMPTY_STRING()) const;
  virtual const std::vector<TPMExclusion*>& getTPMExclus(MileageRoute& mRoute) const;

  SouthAtlanticTPMExclusion() {}
  SouthAtlanticTPMExclusion(const SouthAtlanticTPMExclusion&);
  SouthAtlanticTPMExclusion& operator=(const SouthAtlanticTPMExclusion&);
  friend class tse::Singleton<SouthAtlanticTPMExclusion>;
};

} // namespace tse

