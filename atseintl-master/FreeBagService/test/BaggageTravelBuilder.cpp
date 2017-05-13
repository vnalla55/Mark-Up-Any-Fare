#include "FreeBagService/test/BaggageTravelBuilder.h"
namespace tse
{
BaggageTravelBuilder&
BaggageTravelBuilder::withMSSTravelSeg(std::vector<TravelSeg*>::const_iterator travelSeg)
{
  _bgTravel->updateSegmentsRange(travelSeg, travelSeg + 1);
  _bgTravel->_MSS = travelSeg;
  return *this;
}

BaggageTravelBuilder&
BaggageTravelBuilder::withTravelSeg(std::vector<TravelSeg*>::const_iterator begin,
                                    std::vector<TravelSeg*>::const_iterator end)
{
  _bgTravel->updateSegmentsRange(begin, end);
  return *this;
}

BaggageTravelBuilder&
BaggageTravelBuilder::withTravelSeg(const std::vector<TravelSeg*>& segments)
{
  _bgTravel->updateSegmentsRange(segments.begin(), segments.end());
  _bgTravel->_MSS = segments.end() - 1;
  _bgTravel->_MSSJourney = segments.end() - 1;

  return *this;
}

BaggageTravelBuilder&
BaggageTravelBuilder::withTravelSegMore(const std::vector<TravelSeg*>& segments)
{
  _bgTravel->updateSegmentsRange(segments.begin(), segments.end());
  _bgTravel->_MSS = segments.begin();
  _bgTravel->_MSSJourney = segments.begin();

  return *this;
}

BaggageTravelBuilder&
BaggageTravelBuilder::addEmbargoOCFees(SubCodeInfo* s5, OptionalServicesInfo* s7)
{
  OCFees* ocFees = _memHandle->create<OCFees>();
  ocFees->subCodeInfo() = s5;
  ocFees->optFee() = s7;
  _bgTravel->_embargoVector.push_back(ocFees);
  return *this;
}

BaggageTravelBuilder&
BaggageTravelBuilder::withAllowance(const SubCodeInfo* s5, const OptionalServicesInfo* s7)
{
  OCFees* ocFees = _memHandle->create<OCFees>();
  ocFees->subCodeInfo() = s5;
  ocFees->optFee() = s7;
  _bgTravel->_allowance = ocFees;

  return *this;
}
BaggageTravelBuilder&
BaggageTravelBuilder::withAllowanceS5(const SubCodeInfo* s5)
{
  OCFees* ocFees = _memHandle->create<OCFees>();
  ocFees->subCodeInfo() = s5;
  _bgTravel->_allowance = ocFees;
  return *this;
}

BaggageTravelBuilder&
BaggageTravelBuilder::withAllowanceS7(const OptionalServicesInfo* s7)
{
  OCFees* ocFees = _memHandle->create<OCFees>();
  ocFees->optFee() = s7;
  _bgTravel->_allowance = ocFees;
  return *this;
}

BaggageTravelBuilder&
BaggageTravelBuilder::withOrigin(const LocCode& locCode)
{
  createTravelSegs();
  Loc* origin = _memHandle->create<Loc>();
  origin->loc() = locCode;
  _travelSegs->front()->origin() = origin;
  _bgTravel->updateSegmentsRange(_travelSegs->begin(), _travelSegs->end());

  return *this;
}

BaggageTravelBuilder&
BaggageTravelBuilder::withDestination(const LocCode& locCode)
{
  createTravelSegs();
  Loc* destination = _memHandle->create<Loc>();
  destination->loc() = locCode;
  _travelSegs->back()->destination() = destination;
  _bgTravel->updateSegmentsRange(_travelSegs->begin(), _travelSegs->end());

  return *this;
}

void
BaggageTravelBuilder::createTravelSegs()
{
  if (!_travelSegs)
  {
    _travelSegs = _memHandle->create<std::vector<TravelSeg*> >();
    AirSeg* airSeg = _memHandle->create<AirSeg>();
    _travelSegs->push_back(airSeg);
    _bgTravel->_MSS = _travelSegs->begin();
  }
}
}
