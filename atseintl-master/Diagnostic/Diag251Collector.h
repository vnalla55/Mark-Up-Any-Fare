//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/ACDiagCollector.h"

namespace tse
{
class ConstructionVendor;

class Diag251Collector : public ACDiagCollector
{
public:
  explicit Diag251Collector(Diagnostic& root)
    : ACDiagCollector(root), _gwPairPerRow(0), _gwPairCount(0), _zoneCount(0)
  {
  }
  Diag251Collector() : _gwPairPerRow(0), _gwPairCount(0), _zoneCount(0) {}

  void writeArbZoneHeader(const LocCode& location);
  void writeArbZoneFooter();
  void writeArbZone(const LocCode& location,
                    const AddonZone zone,
                    const TariffNumber tariff,
                    const TSEDateInterval& effInterval);
  void writeVendor(const ConstructionVendor& vendor);

private:
  struct GwDescriptor
  {
    GwDescriptor() : _isDEGateway(false) {};
    GwDescriptor(const LocCode& gw, bool isDE) : _gateway(gw), _isDEGateway(isDE) {};
    LocCode _gateway;
    bool _isDEGateway;
  };

  typedef std::vector<GwDescriptor> GwDescriptorVec;

  int _gwPairPerRow;
  int _gwPairCount;

  int _zoneCount;

  void writeGateways(const ConstructionVendor& vendor);
  void writeGWPair(const LocCode& gw1, const LocCode& gw2);
  void writeTotalGWPair();
};

} // namespace tse

