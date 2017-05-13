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
class TariffCrossRefInfo;
class AddonCombFareClassInfo;

class Diag253Collector : public ACDiagCollector
{
public:
  explicit Diag253Collector(Diagnostic& root) : ACDiagCollector(root), _skipVendorCarrier(false) {}
  Diag253Collector() : _skipVendorCarrier(false) {}

  void writeTariffXRefHeader(const VendorCode& vendor);
  void writeTariffXRefRef(const TariffCrossRefInfo& tariffCrossRef, const bool isFD);
  void writeTariffXRefFooter();
  void writeCombFareClassHeader(const VendorCode& vendor);
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  void writeCombFareClass(const AddonCombFareClassInfo&, TariffNumber fareTariff);
#else
  void writeCombFareClass(const AddonCombFareClassInfo& combFareClass);
#endif
  void writeCombFareClassFooter();

private:
  bool _skipVendorCarrier;
};

} // namespace tse

