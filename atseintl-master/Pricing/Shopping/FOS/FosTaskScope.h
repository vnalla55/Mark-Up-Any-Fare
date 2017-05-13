// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"

namespace tse
{
namespace fos
{

class FosTaskScope
{
public:
  FosTaskScope()
    : _numOnlineFos(0),
      _numSnowmanFos(0),
      _numDiamondFos(0),
      _numTriangleFos(0),
      _numDirectFos(0),
      _numCustomFos(-1),
      _numLongConxFos(-1),
      _checkConnectingFlights(false),
      _checkConnectingCities(false),
      _pqConditionOverride(false),
      _deferredAdditionalNSProcessing(false)
  {
  }

  // Number of both online and interline solutions required
  uint32_t getNumFos() const
  {
    return getNumOnlineFos() + getNumSnowmanFos() + getNumDiamondFos() + getNumTriangleFos();
  }

  uint32_t getNumOnlineFos() const { return _numOnlineFos; }
  uint32_t getNumSnowmanFos() const { return _numSnowmanFos; }
  uint32_t getNumDiamondFos() const { return _numDiamondFos; }
  uint32_t getNumTriangleFos() const { return _numTriangleFos; }
  uint32_t getNumDirectFos() const { return _numDirectFos; }
  int32_t getNumCustomFos() const { return _numCustomFos; }
  int32_t getNumLongConxFos() const { return _numLongConxFos; }

  bool checkConnectingFlights() const { return _checkConnectingFlights; }
  bool checkConnectingCities() const { return _checkConnectingCities; }
  bool pqConditionOverride() const { return _pqConditionOverride; }

  const std::map<CarrierCode, uint32_t>& getNumFosPerCarrier() const { return _numFosPerCarrier; }

  const std::map<CarrierCode, uint32_t>& getNumDirectFosPerCarrier() const
  {
    return _numDirectFosPerCarrier;
  }

  const bool isDeferredAdditionaNSProcessingEnabled() const
  {
    return _deferredAdditionalNSProcessing;
  }

protected:
  uint32_t _numOnlineFos;
  uint32_t _numSnowmanFos;
  uint32_t _numDiamondFos;
  uint32_t _numTriangleFos;
  uint32_t _numDirectFos;
  int32_t _numCustomFos;
  int32_t _numLongConxFos;

  std::map<CarrierCode, uint32_t> _numFosPerCarrier;
  std::map<CarrierCode, uint32_t> _numDirectFosPerCarrier;

  bool _checkConnectingFlights;
  bool _checkConnectingCities;
  bool _pqConditionOverride;
  bool _deferredAdditionalNSProcessing;
};

} // fos
} // tse

