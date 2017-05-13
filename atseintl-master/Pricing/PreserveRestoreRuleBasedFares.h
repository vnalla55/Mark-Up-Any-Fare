//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include <vector>

namespace tse
{
class DiagCollector;
class FarePath;
class PaxTypeFare;
class PricingUnit;

class PreserveRestorePURuleBasedFares
{
public:
  PreserveRestorePURuleBasedFares() : _diag(nullptr) {};
  virtual ~PreserveRestorePURuleBasedFares()
  {
    Restore();
  };

  void Preserve(PricingUnit* pricingUnit, DiagCollector* diag);
  void Preserve(const std::vector<PricingUnit*> &pricingUnit, DiagCollector* diag);

protected:
  virtual PaxTypeFare* determinePaxTypeFare(PaxTypeFare* ptFare) const;
  void Preserve();

private:
  void Restore();

  DiagCollector* _diag;
  std::vector<PricingUnit*> _pricingUnits;
};

class PreserveRestoreFPRuleBasedFares
{
public:
  PreserveRestoreFPRuleBasedFares() {}
  // No need to do anything here.  The PUs are restored in the destructor of the vector elements
  ~PreserveRestoreFPRuleBasedFares() {}

  void Preserve(FarePath& farePath, DiagCollector* diag);

private:
  std::vector<PreserveRestorePURuleBasedFares> _oldPUFares;
};

} // namespace tse

