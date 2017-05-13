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
class ConstructionVendor;
class GatewayPair;

class Diag255Collector : public ACDiagCollector
{
public:
  explicit Diag255Collector(Diagnostic& root) : ACDiagCollector(root), _skipGW(false) {}
  Diag255Collector() : _skipGW(false) {}

  void writeGWHeader(const ConstructionVendor& vendor, const GatewayPair& gw);
  void writeFarePair(const AddonFareInfo& af, const FareInfo& sf, const FareMatchCode matchResult);
  void writeBadFare(const AddonFareInfo& origAddon,
                    const FareInfo& specFare,
                    const AddonFareInfo& destAddon,
                    const FareMatchCode matchResult);
  void writeGWFooter();

private:
  bool _skipGW;

  void formatMatchResult(FareMatchCode matchResult,
                         const AddonFareInfo* origAddon,
                         const FareInfo* specFare,
                         const AddonFareInfo* destAddon,
                         std::string& failCode);
};

} // namespace tse

