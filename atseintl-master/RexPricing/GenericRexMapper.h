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

#include "DataModel/ExcItin.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RexPricingTrx.h"


namespace tse
{

class Diag689Collector;

class InternationalComponent : public std::unary_function<const PaxTypeFare*, bool>
{
public:
  bool operator()(const PaxTypeFare* ptf) const
  {
    return ptf && ptf->fareMarket()->geoTravelType() == GeoTravelType::International;
  }
};

class GenericRexMapper
{
  friend class GenericRexMapperTest;
  friend class RexAdvResTktValidatorAdvPurTest;

public:
  GenericRexMapper(RexPricingTrx& trx, const std::vector<const PaxTypeFare*>* allRepricePTFs);
  void setDiag(Diag689Collector* dc) { _dc = dc; }
  void map();

  const PaxTypeFare* getInternationalyMappedPtf(const ProcessTagInfo& pti) const
  {
    return _trx.exchangeItin().front()->geoTravelType() == GeoTravelType::International
               ? _internationalMap[pti.fareCompNumber() - 1]
               : _domesticMap[pti.fareCompNumber() - 1];
  }

  const PaxTypeFare* getCityMappedPtf(const ProcessTagInfo& pti) const
  {
    return _domesticMap[pti.fareCompNumber() - 1];
  }

  const std::vector<uint16_t>& getQuickAccessMap() const { return _quickAccessMap; }

  const std::ostringstream& domesticDiag() const { return _domesticDiag; }
  const std::ostringstream& internationalDiag() const
  {
    return _trx.exchangeItin().front()->geoTravelType() == GeoTravelType::International ? _internationalDiag
                                                                         : _domesticDiag;
  }
  inline bool hasInternationalComponent() const
  {
    return std::find_if(_internationalMap.begin(),
                        _internationalMap.end(),
                        InternationalComponent()) != _internationalMap.end();
  }

private:
  template <typename Mapper>
  bool mappingStep(std::list<const PaxTypeFare*>& notMappedRepricePtf,
                   std::vector<const PaxTypeFare*>& map,
                   std::ostringstream& diag,
                   Mapper mapper);
  void createQuickAccessMap();

  RexPricingTrx& _trx;
  const std::vector<const PaxTypeFare*>* _allRepricePTFs;
  Diag689Collector* _dc;

  std::vector<const PaxTypeFare*> _domesticMap;
  std::vector<const PaxTypeFare*> _internationalMap;
  std::vector<uint16_t> _quickAccessMap;

  std::ostringstream _domesticDiag;
  std::ostringstream _internationalDiag;
};

}
