// ---------------------------------------------------------------------------
// (C) 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <vector>

namespace tse
{
class CarrierApplicationInfo;
class RexBaseRequest;
class VoluntaryRefundsInfo;
class DataHandle;
class DiagCollector;

class CarrierApplicationValidator
{
  friend class CarrierApplicationValidatorTest;

public:
  typedef std::vector<CarrierApplicationInfo*> CxrTbl;
  static const Indicator ALLOW;

  CarrierApplicationValidator(const RexBaseRequest& request,
                              const VoluntaryRefundsInfo& record3,
                              DiagCollector* dc,
                              DataHandle& dh);
  virtual ~CarrierApplicationValidator() {}
  bool validate();

protected:
  bool determineValidity();
  void composeDiagnostic(bool result);
  virtual const CxrTbl& getCarrierApplTbl();

  bool needToCheckTable();
  bool dataError();

private:
  const CarrierCode& _cxr;
  const CarrierCode& _excCxr;
  const VoluntaryRefundsInfo& _record3;
  DiagCollector* _dc;
  DataHandle& _dh;
};
}

