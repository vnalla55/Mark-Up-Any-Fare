//-------------------------------------------------------------------
//
//  File:        ConstructionJob.h
//  Created:     May 3, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description:
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

#include "AddonConstruction/ConstructedFareInfoResponse.h"
#include "AddonConstruction/ConstructionDefs.h"
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
#include "AddonConstruction/SpecifiedFareCache.h"
#endif
#include "Common/TseConsts.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/Diagnostic.h"
#include <type_traits>

namespace tse
{
class ACDiagCollector;
class ConstructionVendor;
class Diag251Collector;
class Diag252Collector;
class Diag253Collector;
class Diag254Collector;
class Diag255Collector;
class Diag257Collector;
class Diag259Collector;
class DiagRequest;
class Loc;
#ifdef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
class PricingTrx;
#endif

namespace dispatching
{
// TODO: I wish we had a library for template metaprogramming like
// boost::hana. We could avoid all these template specializations.
// However our current compiler doesn't support it.
template <typename T>
struct ACDiagCollDispatcher;
template <>
struct ACDiagCollDispatcher<Diag251Collector>
{
  constexpr static const DiagnosticTypes type = Diagnostic251;
};
template <>
struct ACDiagCollDispatcher<Diag252Collector>
{
  constexpr static const DiagnosticTypes type = Diagnostic252;
};
template <>
struct ACDiagCollDispatcher<Diag253Collector>
{
  constexpr static const DiagnosticTypes type = Diagnostic253;
};
template <>
struct ACDiagCollDispatcher<Diag254Collector>
{
  constexpr static const DiagnosticTypes type = Diagnostic254;
};
template <>
struct ACDiagCollDispatcher<Diag255Collector>
{
  constexpr static const DiagnosticTypes type = Diagnostic255;
};
template <>
struct ACDiagCollDispatcher<Diag257Collector>
{
  constexpr static const DiagnosticTypes type = Diagnostic257;
};
template <>
struct ACDiagCollDispatcher<Diag259Collector>
{
  constexpr static const DiagnosticTypes type = Diagnostic259;
};
}

class ConstructionJob
{
public:
  friend class ConstructionJobMock;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  ConstructionJob(PricingTrx& trx,
                  const DateTime& travelDate,
                  const DateTime& ticketingDate,
                  const LocCode& orig,
                  const LocCode& boardCity,
                  const LocCode& dest,
                  const LocCode& offCity,
                  const GlobalDirection globalDir,
                  const bool singleOverDouble,
                  SpecifiedFareCache*);
#else
  ConstructionJob(PricingTrx& trx,
                  const DateTime& travelDate,
                  const DateTime& ticketingDate,
                  const LocCode& orig,
                  const LocCode& boardCity,
                  const LocCode& dest,
                  const LocCode& offCity,
                  const bool singleOverDouble);
#endif

  ConstructionJob(const ConstructionJob& rhs);

  ConstructionJob& operator=(const ConstructionJob&) = delete;

  ~ConstructionJob() { _response.responseHashSet().clear(); }

  void createDiagCollector();
  void reclaimDiagCollector();

  // accessors to add-on construction request parameters
  // ========= == ====== ============ ======= ==========

  PricingTrx& trx() { return _trx; }

  const bool& isHistorical() const { return _isHistorical; }

  void setVendorCode(const VendorCode& v)
  {
    _vendorCode = v;
    _smfProcess = (_vendorCode != ATPCO_VENDOR_CODE && _vendorCode != SITA_VENDOR_CODE);
  }
  const VendorCode& vendorCode() const { return _vendorCode; }

  const bool isAtpco() { return _vendorCode == ATPCO_VENDOR_CODE; }
  const bool isSita() { return _vendorCode == SITA_VENDOR_CODE; }
  const bool isSMF() const { return _smfProcess; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  const DateTime& travelDate() const { return _travelDate; }
  const DateTime& ticketingDate() const { return _ticketingDate; }

  const LocCode& origin() const { return _origin; }
  const LocCode& boardMultiCity() const { return _boardMultiCity; }

  const LocCode& destination() const { return _destination; }
  const LocCode& offMultiCity() const { return _offMultiCity; }

  const LocCode& od(ConstructionPoint cp) const
  {
    return (cp == CP_ORIGIN ? _origin : _destination);
  }

  const LocCode& odCity(ConstructionPoint cp) const
  {
    return (cp == CP_ORIGIN ? _boardMultiCity : _offMultiCity);
  }

  const bool reversedFareMarket() const { return _reversedFareMarket; }

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  GlobalDirection getGlobalDirection() const { return _globalDir; }
#endif

  const bool singleOverDouble() const { return _singleOverDouble; }

  const bool forceAddonConstruction() const { return _forceAddonConstruction; }

  // accessors to diagnostic collectors
  // ========= == ========== ==========
  template <typename DiagCollType>
  auto diagnostic()
  {
    using RetType = std::decay_t<DiagCollType>;
    constexpr bool isAllowedDiagnosticType =
        !std::is_pointer<RetType>::value && std::is_base_of<ACDiagCollector, RetType>::value;

    static_assert(isAllowedDiagnosticType, "Error, unsupported DiagCollector type");
    constexpr auto allowedDiagnosticTag = dispatching::ACDiagCollDispatcher<RetType>::type;

    if (LIKELY(!_diagCollector))
      return static_cast<RetType*>(nullptr);

    DiagnosticTypes diagnosticType = _trx.diagnostic().diagnosticType();
    return diagnosticType == allowedDiagnosticTag ? static_cast<RetType*>(_diagCollector) : nullptr;
  }

  Diag251Collector* diag251();
  Diag252Collector* diag252();
  Diag253Collector* diag253();
  Diag254Collector* diag254();
  Diag255Collector* diag255();
  Diag257Collector* diag257();
  Diag259Collector* diag259();

  bool& constructionCacheFlush() { return _constructionCacheFlush; }
  const bool constructionCacheFlush() const { return _constructionCacheFlush; }

  bool& showDateIntervalDetails() { return _showDateIntervalDetails; }
  const bool showDateIntervalDetails() const { return _showDateIntervalDetails; }

  // accessors to add-on construction internal objects
  // ========= == ====== ============ ======== =======

  DataHandle& dataHandle() { return _dataHandle; }
  const DataHandle& dataHandle() const { return _dataHandle; }

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  DataHandle& trxDataHandle() { return _trx.dataHandle(); }
  const DataHandle& trxDataHandle() const { return _trx.dataHandle(); }
#endif

  DiagRequest* diagRequest() { return _diagRequest; }
  const DiagRequest* diagRequest() const { return _diagRequest; }

  const Loc* loc(ConstructionPoint cp) const
  {
    return (cp == CP_ORIGIN ? _originLoc : _destinationLoc);
  }

  ConstructionVendor*& constructionVendor() { return _constructionVendor; }
  const ConstructionVendor* constructionVendor() const { return _constructionVendor; }

  ConstructedFareInfoResponse& response() { return _response; }
  const ConstructedFareInfoResponse& response() const { return _response; }

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  SpecifiedFareCache* specifiedCache() const { return _specifiedCache; }

  bool fallbackConstructedFareEffectiveDate() const {
    return _fallbackConstructedFareEffectiveDate;
  }
#endif

private:
  void
  init(const LocCode& orig, const LocCode& boardCity, const LocCode& dest, const LocCode& offCity);

  ConstructionJob(PricingTrx& trx) : _trx(trx), _response(trx) {} // for unit tests

  // add-on construction request
  // ====== ============ =======

  PricingTrx& _trx;

  bool _isHistorical = false;

  VendorCode _vendorCode = EMPTY_VENDOR;
  CarrierCode _carrier = INVALID_CARRIERCODE;

  DateTime _travelDate;
  DateTime _ticketingDate;

  LocCode _origin;
  LocCode _boardMultiCity;

  LocCode _destination;
  LocCode _offMultiCity;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  GlobalDirection _globalDir;
#endif

  bool _smfProcess = false;

  bool _reversedFareMarket = false;
  bool _singleOverDouble = false;

  bool _forceAddonConstruction = false;
  bool _constructionCacheFlush = false;
  bool _showDateIntervalDetails = false;

  // add-on construction internal objects
  // ====== ============ ======== =======

  DataHandle _dataHandle;
  DiagRequest* _diagRequest = nullptr;
  ACDiagCollector* _diagCollector = nullptr;

  const Loc* _originLoc = nullptr;
  const Loc* _destinationLoc = nullptr;

  ConstructionVendor* _constructionVendor = nullptr;
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  SpecifiedFareCache* _specifiedCache;
  bool _fallbackConstructedFareEffectiveDate;
#endif

  ConstructedFareInfoResponse _response;
}; // End of class ConstructionJob

} // End of namespace tse
