//-------------------------------------------------------------------
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

#include "Diagnostic/DiagCollector.h"

namespace tse
{

class TravelSeg;

class InternalDiagUtil
{
public:
  explicit InternalDiagUtil(const Trx& trx, DiagCollector& dc) : _dc(dc), _trx(trx) {}

  void printSegmentInfo(const TravelSeg& ts);
  void addIataArea(const TravelSeg& ts);
  void addTripCharacteristic(const Itin& itin);

private:
  void addCarrierFlightCode(const TravelSeg& ts);
  void addDate(const TravelSeg& ts);
  void addCityPair(const TravelSeg& ts);

  DiagCollector& _dc;
  const Trx& _trx;
};
}

