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
#ifndef BAGGAGETRAVELBUILDER_H
#define BAGGAGETRAVELBUILDER_H
#include <boost/assign/std/vector.hpp>
#include "test/include/TestMemHandle.h"

#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "ServiceFees/OCFees.h"

namespace tse
{
using boost::assign::operator+=;

class BaggageTravelBuilder
{
  BaggageTravel* _bgTravel;
  TestMemHandle* _memHandle;
  std::vector<TravelSeg*>* _travelSegs;

public:
  BaggageTravelBuilder(TestMemHandle* memHandle) : _memHandle(memHandle), _travelSegs(0)
  {
    _bgTravel = _memHandle->create<BaggageTravel>();
  }

  BaggageTravelBuilder& withTrx(PricingTrx* trx)
  {
    _bgTravel->_trx = trx;
    return *this;
  }

  BaggageTravelBuilder& withMSSTravelSeg(std::vector<TravelSeg*>::const_iterator travelSeg);

  BaggageTravelBuilder& withMSS(std::vector<TravelSeg*>::const_iterator travelSeg)
  {
    _bgTravel->_MSS = travelSeg;
    return *this;
  }

  BaggageTravelBuilder& withMSSJourney(std::vector<TravelSeg*>::const_iterator travelSeg)
  {
    _bgTravel->_MSSJourney = travelSeg;
    return *this;
  }

  BaggageTravelBuilder& withTravelSeg(std::vector<TravelSeg*>::const_iterator begin,
                                      std::vector<TravelSeg*>::const_iterator end);

  BaggageTravelBuilder& withTravelSeg(const std::vector<TravelSeg*>& segments);

  BaggageTravelBuilder& withTravelSegMore(const std::vector<TravelSeg*>& segments);

  BaggageTravelBuilder& addEmbargoOCFees(SubCodeInfo* s5, OptionalServicesInfo* s7);

  BaggageTravelBuilder& withNoAllowance()
  {
    _bgTravel->_allowance = 0;
    return *this;
  }

  BaggageTravelBuilder& withAllowance(OCFees* ocfees)
  {
    _bgTravel->_allowance = ocfees;
    return *this;
  }

  BaggageTravelBuilder& withAllowance(const SubCodeInfo* s5, const OptionalServicesInfo* s7);

  BaggageTravelBuilder& withAllowanceS5(const SubCodeInfo* s5);

  BaggageTravelBuilder& withAllowanceS5()
  {
    _bgTravel->_allowance = _memHandle->create<OCFees>();
    return *this;
  }

  BaggageTravelBuilder& withAllowanceS7(const OptionalServicesInfo* s7);

  BaggageTravelBuilder& withOrigin(const LocCode& locCode);

  BaggageTravelBuilder& withDestination(const LocCode& locCode);

  BaggageTravelBuilder& withOperatingCarrier(const CarrierCode& carrier)
  {
    createTravelSegs();
    static_cast<AirSeg*>(_travelSegs->front())->setOperatingCarrierCode(carrier);
    return *this;
  }

  BaggageTravelBuilder& withBaggageCharge(BaggageCharge* charge)
  {
    _bgTravel->_chargeVector += charge;
    return *this;
  }

  BaggageTravelBuilder& with1stBagS7(const OptionalServicesInfo* s7, FarePath* fp = 0)
  {
    BaggageCharge* charge = _memHandle->create<BaggageCharge>();
    charge->optFee() = s7;
    charge->farePath() = fp;
    _bgTravel->_charges[0] = charge;
    return *this;
  }

  BaggageTravelBuilder& with2ndBagS7(const OptionalServicesInfo* s7, FarePath* fp = 0)
  {
    BaggageCharge* charge = _memHandle->create<BaggageCharge>();
    charge->optFee() = s7;
    charge->farePath() = fp;
    _bgTravel->_charges[1] = charge;
    return *this;
  }

  BaggageTravelBuilder& withFarePath(FarePath* farePath)
  {
    _bgTravel->setupTravelData(*farePath);
    return *this;
  }

  BaggageTravelBuilder& withCarrier(const CarrierCode& code)
  {
    _bgTravel->_allowanceCxr = code;
    return *this;
  }

  BaggageTravelBuilder& withCarrierTravelSeg(const TravelSeg* seg)
  {
    _bgTravel->_carrierTravelSeg = seg;
    return *this;
  }

  BaggageTravelBuilder& withDefer(const bool defer = true)
  {
    _bgTravel->_defer = defer;
    return *this;
  }

  BaggageTravel* build() const { return _bgTravel; }

private:
  void createTravelSegs();
};
}
#endif // BAGGAGETRAVELBUILDER_H
