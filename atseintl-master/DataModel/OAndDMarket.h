#pragma once


#include <map>
#include <vector>

namespace tse
{

class TravelSeg;
class ClassOfService;
class Itin;
class FareMarket;

class OAndDMarket
{
  friend class OAndDMarketTest;

  typedef std::map<const TravelSeg*, std::vector<ClassOfService*>*> SegToCosMap;
  enum JourneyCarrierType
  {
    CT_local,
    CT_flow,
    CT_marriage
  };

public:
  void initializeOD(const FareMarket& fm,
                    const bool isLowFareRequested,
                    const bool isLocalJourneyCarrier,
                    const bool isItinAnalyzerPhase,
                    const bool allowLATAMDualRBD,
                    const std::map<const TravelSeg*, bool>* marriedSegs = nullptr);
  void initializeCOS();
  const bool isJourneyByMarriage() const { return (_carrierType == CT_marriage); }
  const bool isFlowCarrierJourney() const { return (_carrierType == CT_flow); }
  const bool maySegUseODCos(TravelSeg* tSeg);
  const bool hasTravelSeg(const TravelSeg* tSeg) const;
  std::vector<ClassOfService*>* getCosVector(const TravelSeg* tSeg) const;
  const FareMarket* fareMarket() const { return _fareMarket; }
  bool& validAfterRebook() { return _validAfterRebook; }
  const bool validAfterRebook() const { return _validAfterRebook; }

private:
  JourneyCarrierType _carrierType;
  const FareMarket* _fareMarket;
  std::map<const TravelSeg*, bool> _fmTvlSegMayUseCosFromThis;
  SegToCosMap _seg2CosVectorMap;
  bool _validAfterRebook;
};

typedef std::vector<uint16_t /*segOrder*/> MarriedGroup;

} // namespace tse

