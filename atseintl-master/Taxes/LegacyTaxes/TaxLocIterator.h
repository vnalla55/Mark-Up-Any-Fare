#pragma once

#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class TaxCodeReg;
class FarePath;
class PricingTrx;
class Loc;
class TravelSeg;
class FareUsage;

class TaxLocIterator
{
  friend class TaxSP9100Test;
  friend class TaxLocIteratorMockSP9300;

private:
  uint16_t _segNo = 0;
  uint16_t _subSegNo = 0;
  uint16_t _numOfSegs = 0;
  uint16_t _numOfSubSegs = 0;

  std::vector<TravelSeg*> _travelSegs;
  std::map<uint16_t, FareUsage*> _fareBreaks;
  std::set<uint16_t> _turnAroundPoints;
  const FarePath* _farePath = nullptr;

  bool _fareBreaksFound = false;
  bool _turnAroundPointsFound = false;
  bool _skipHidden = false;
  bool _surfaceAsStop = false;
  uint16_t _stopHours = 24;
  bool _turnAroundAfterSurface = false;
  bool _treatTrainBusAsNonAir = false;

  uint16_t getNumOfHiddenStops();
  void findTurnAroundPoints();

public:
  virtual ~TaxLocIterator() = default;

  virtual void initialize(const FarePath&);

  void setSkipHidden(bool v) { _skipHidden = v; }

  void setSurfaceAsStop(bool v) { _surfaceAsStop = v; }

  void setStopHours(uint16_t v) { _stopHours = v; }

  void setTurnAroundAfterSurface(bool v) { _turnAroundAfterSurface = v; }

  void setTreatTrainBusAsNonAir(bool v) { _treatTrainBusAsNonAir = v; }

  /**
    returns true if position of iterator is after last location in itin
  */
  virtual bool isEnd() const;

  virtual bool hasNext() const;
  virtual bool hasPrevious() const;
  virtual uint16_t segNo() const { return _segNo; }

  virtual uint16_t numOfSegs() const { return _numOfSegs; }

  virtual void next();
  virtual void previous();
  virtual void toBack();
  virtual void toFront();
  virtual void toSegmentNo(uint16_t);

  virtual uint16_t prevSegNo() const;
  virtual uint16_t nextSegNo() const;

  virtual TravelSeg* prevSeg() const;
  virtual TravelSeg* nextSeg() const;

  virtual uint16_t subSegNo() const;

  virtual bool isStop();
  virtual bool isFareBreak();
  virtual bool isTurnAround();
  virtual bool isHidden();
  virtual bool isMirror();

  virtual bool isNextSegAirSeg() const;
  virtual bool isPrevSegAirSeg() const;

  virtual const Loc* loc() const;
  virtual const Loc* locDeplanement() const;
  virtual bool isInLoc1(TaxCodeReg&, PricingTrx&) const;
  virtual bool isInLoc2(TaxCodeReg&, PricingTrx&) const;
  virtual bool isInLoc(LocTypeCode, const LocCode&, Indicator, PricingTrx&, bool) const;
};
}
