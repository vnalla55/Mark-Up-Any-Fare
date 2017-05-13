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
class FareInfo;
class AddonFareInfo;
class GatewayPair;

class Diag252Collector : public ACDiagCollector
{
public:
  enum SpecFareStatus
  {
    SP_OK,
    CP_SITA_NOT_FOR_CONSTRUCTION
  };

  explicit Diag252Collector(Diagnostic& root)
    : ACDiagCollector(root), _processedGWFaresCnt(0), _processedFaresCnt(0)
  {
  }
  Diag252Collector() : _processedGWFaresCnt(0), _processedFaresCnt(0) {}

  void writeAddonFareDiagHeader(ConstructionPoint cp);
  void writeAddonFare(const AddonFareInfo& af, AddonZoneStatus zs);
  void writeAddonFareDiagFooter();
  void writeSpecFareDiagHeader(const GatewayPair& gw);
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  void writeSpecifiedFares(const GatewayPair& gw, const SpecifiedFareList& specFares);
#else
  void writeSpecifiedFares(const GatewayPair& gw, std::vector<const FareInfo*>& specFares);
#endif
  void writeSpecifiedFare(const GatewayPair& gw, const FareInfo& spFare, SpecFareStatus spStatus);
  void writeSpecFareDiagFooter(const GatewayPair& gw);

private:
  unsigned int _processedGWFaresCnt;
  unsigned int _processedFaresCnt;
};

} // namespace tse

