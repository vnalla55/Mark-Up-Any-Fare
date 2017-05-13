#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/TpdPsr.h"
#include "Routing/RoutingConsts.h"
#include "Routing/RtgKey.h"
#include "Routing/ValidationInfo.h"

#include <map>
#include <vector>

namespace tse
{
using Market = std::pair<std::string, std::string>;
class TPMExclusion;

class MileageInfo : public ValidationInfo
{
public:
  MileageInfo() = default;
  MileageInfo(bool processed, bool valid) : ValidationInfo(processed, valid) {}

  uint16_t surchargeAmt() const { return _surchargeAmt; }
  uint16_t& surchargeAmt() { return _surchargeAmt; }

  uint16_t surchargeAmtSAException() const { return _southAtlanticExceptionSurchargeAmount; }
  uint16_t& surchargeAmtSAException() { return _southAtlanticExceptionSurchargeAmount; }

  uint16_t totalApplicableTPM() const { return _totalApplicableTPM; }
  uint16_t& totalApplicableTPM() { return _totalApplicableTPM; }

  uint16_t totalApplicableTPMSAException() const
  {
    return _southAtlanticExceptionTotalApplicableTPM;
  }
  uint16_t& totalApplicableTPMSAException() { return _southAtlanticExceptionTotalApplicableTPM; }

  uint16_t tpd() const { return _tpd; }
  uint16_t& tpd() { return _tpd; }

  uint16_t totalApplicableMPM() const { return _totalApplicableMPM; }
  uint16_t& totalApplicableMPM() { return _totalApplicableMPM; }

  const std::vector<Market>& surfaceSectorExemptCities() const
  {
    return _surfaceSectorExemptCities;
  }
  std::vector<Market>& surfaceSectorExemptCities() { return _surfaceSectorExemptCities; }
  const std::vector<Market>& southAtlanticTPDCities() const { return _southAtlanticTPDCities; }
  std::vector<Market>& southAtlanticTPDCities() { return _southAtlanticTPDCities; }

  const std::pair<uint16_t, uint16_t>& equalizationSurcharges() const
  {
    return _equalizationSurcharges;
  }
  std::pair<uint16_t, uint16_t>& equalizationSurcharges() { return _equalizationSurcharges; }

  bool psrApplies() const { return _psrApplies; }
  bool& psrApplies() { return _psrApplies; }
  bool psrHipExempt() const { return _psrHipExempt; }
  bool& psrHipExempt() { return _psrHipExempt; }

  bool mileageEqualizationApplies() const { return _mileageEqualizationApplies; }
  bool& mileageEqualizationApplies() { return _mileageEqualizationApplies; }

  const CarrierCode& psrGovCxr() const { return _psrGovCxr; }
  CarrierCode& psrGovCxr() { return _psrGovCxr; }

  bool southAtlanticTPMExclusion() const { return _southAtlanticTPMExclusion; }
  bool& southAtlanticTPMExclusion() { return _southAtlanticTPMExclusion; }

  const std::vector<TpdPsrViaGeoLoc>& psrGeoLocs() const { return _psrGeoLocs; }
  std::vector<TpdPsrViaGeoLoc>& psrGeoLocs() { return _psrGeoLocs; }

  const std::map<LocCode, TpdViaGeoLocMatching>& tpdViaGeoLocs() const { return _tpdViaGeoLocs; }
  std::map<LocCode, TpdViaGeoLocMatching>& tpdViaGeoLocs() { return _tpdViaGeoLocs; }

  const std::vector<int16_t>& tpdMatchedViaLocs() const { return _tpdMatchedViaLocs; }
  std::vector<int16_t>& tpdMatchedViaLocs() { return _tpdMatchedViaLocs; }

  const TPMExclusion* tpmExclusion() const { return _tpmExclusion; }
  const TPMExclusion*& tpmExclusion() { return _tpmExclusion; }

private:
  uint16_t _surchargeAmt = 0;
  uint16_t _southAtlanticExceptionSurchargeAmount = 0;
  uint16_t _totalApplicableTPM = 0;
  uint16_t _southAtlanticExceptionTotalApplicableTPM = 0;
  uint16_t _totalApplicableMPM = 0;
  uint16_t _tpd = 0;
  bool _southAtlanticTPMExclusion = false;
  bool _psrApplies = false;
  bool _psrHipExempt = false;
  bool _mileageEqualizationApplies = false;

  CarrierCode _psrGovCxr;
  std::pair<uint16_t, uint16_t> _equalizationSurcharges;
  std::vector<Market> _surfaceSectorExemptCities;
  std::vector<Market> _southAtlanticTPDCities;
  std::vector<TpdPsrViaGeoLoc> _psrGeoLocs;
  std::map<LocCode, TpdViaGeoLocMatching> _tpdViaGeoLocs;
  std::vector<int16_t> _tpdMatchedViaLocs;
  const TPMExclusion* _tpmExclusion = nullptr;
};
}
