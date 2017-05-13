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
#include "RexPricing/ExpndKeepFareValidator.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/Logger.h"
#include "DataModel/ExcItin.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DBAccess/SeasonalityDOW.h"
#include "Diagnostic/Diag689Collector.h"

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

namespace tse
{
namespace
{
ConfigurableValue<ConfigSet<Indicator>>
seasonIndicators("PRICING_SVC", "REX_IATA_SEASONALITY_IND");
ConfigurableValue<ConfigSet<Indicator>>
dowIndicators("PRICING_SVC", "REX_IATA_DOW_IND");
}

namespace
{
class CityCityFinder : public std::binary_function<const PaxTypeFare*, const FareMarket*, bool>
{
public:
  bool operator()(const PaxTypeFare* newPtf, const FareMarket* newFm) const
  {
    return newPtf->fareMarket()->boardMultiCity() == newFm->boardMultiCity() &&
           newPtf->fareMarket()->offMultiCity() == newFm->offMultiCity();
  }
};

class CityNationFinder : public std::binary_function<const PaxTypeFare*, const FareMarket*, bool>
{
public:
  bool operator()(const PaxTypeFare* newPtf, const FareMarket* newFm) const
  {
    return newPtf->fareMarket()->boardMultiCity() == newFm->boardMultiCity() &&
           RexPricingTrx::expndKeepSameNation(*newPtf->fareMarket()->destination(),
                                              *newFm->destination());
  }
};

class NationCityFinder : public std::binary_function<const PaxTypeFare*, const FareMarket*, bool>
{
public:
  bool operator()(const PaxTypeFare* newPtf, const FareMarket* newFm) const
  {
    return RexPricingTrx::expndKeepSameNation(*newPtf->fareMarket()->origin(), *newFm->origin()) &&
           newPtf->fareMarket()->offMultiCity() == newFm->offMultiCity();
  }
};

class NationNationFinder : public std::binary_function<const PaxTypeFare*, const FareMarket*, bool>
{
public:
  bool operator()(const PaxTypeFare* newPtf, const FareMarket* newFm) const
  {
    return RexPricingTrx::expndKeepSameNation(*newPtf->fareMarket()->origin(), *newFm->origin()) &&
           RexPricingTrx::expndKeepSameNation(*newPtf->fareMarket()->destination(),
                                              *newFm->destination());
  }
};

class NewPtf
    : public std::unary_function<const ExpndKeepFareValidator::ValidationPairMap::value_type&, bool>
{
  const PaxTypeFare& _newPtf;

public:
  NewPtf(const PaxTypeFare& newPtf) : _newPtf(newPtf) {}

  bool operator()(const ExpndKeepFareValidator::ValidationPairMap::value_type& mapItem)
  {
    return mapItem.second == &_newPtf;
  }
};

} // namespace

Logger
ExpndKeepFareValidator::_logger("atseintl.RexPricing");

const PaxTypeFare*
ExpndKeepFareValidator::matchTagDefinition(const PaxTypeFare& excPtf)
{
  const PaxTypeFare* newPtf = nullptr;
  if ((_trx->exchangeItin().front()->geoTravelType() != GeoTravelType::International &&
       !matchExcToNew(CityCityFinder(), excPtf, newPtf)) ||
      (!matchExcToNew(CityCityFinder(), excPtf, newPtf) &&
       !matchExcToNew(CityNationFinder(), excPtf, newPtf) &&
       !matchExcToNew(NationCityFinder(), excPtf, newPtf) &&
       !matchExcToNew(NationNationFinder(), excPtf, newPtf)))
    return nullptr;

  removeFromSearchScope(*newPtf);
  _validationPairMap.insert(std::make_pair(&excPtf, newPtf));
  return newPtf;
}

void
ExpndKeepFareValidator::removeFromSearchScope(const PaxTypeFare& newExpndPtf)
{
  std::vector<const PaxTypeFare*>::iterator found =
      std::find(_searchBegin, _newExpndPtfs.end(), &newExpndPtf);

  if (found != _newExpndPtfs.end())
  {
    std::iter_swap(found, _searchBegin);
    ++_searchBegin;
  }
}

const PaxTypeFare*
ExpndKeepFareValidator::getNewPtf(const PaxTypeFare& excPtf)
{
  ValidationPairMap::const_iterator eit = _validationPairMap.find(&excPtf);

  if (eit != _validationPairMap.end())
    return eit->second;

  return nullptr;
}

const PaxTypeFare*
ExpndKeepFareValidator::getExcPtf(const PaxTypeFare& newPtf)
{
  ValidationPairMap::const_iterator eit =
      std::find_if(_validationPairMap.begin(), _validationPairMap.end(), NewPtf(newPtf));

  if (eit != _validationPairMap.end())
    return eit->first;

  return nullptr;
}

bool
ExpndKeepFareValidator::matchSeasonalityDOW(const ProcessTagPermutation& permutation)
{
  for (const ProcessTagInfo* pt : permutation.processTags())
  {
    if (permutation.fareApplMap().at(pt->paxTypeFare()) == KEEP && !matchSeasonalityDOW(*pt))
    {
      if (_dc)
        print(pt->fareCompNumber());

      return false;
    }
  }

  if (_dc)
    print();

  return true;
}

bool
ExpndKeepFareValidator::matchSeasonalityDOW(const ProcessTagInfo& pti)
{
  if (!pti.reissueSequence()->orig() ||
      pti.reissueSequence()->expndKeep() != ProcessTagInfo::EXPND_A)
    return true;

  const PaxTypeFare* newPtf = getNewPtf(*pti.paxTypeFare());
  if (!newPtf)
    return true;

  CacheKey cacheKey = std::make_pair(newPtf, pti.reissueSequence()->seasonalityDOWTblItemNo());
  Cache::const_iterator cacheItem = _cache.find(cacheKey);
  if (cacheItem != _cache.end() && !_dc)
    return cacheItem->second;

  bool result =
      matchFareBasis(pti, newPtf->createFareBasis(_trx), pti.paxTypeFare()->createFareBasis(_trx));

  _cache.insert(std::make_pair(cacheKey, result));
  return result;
}

void
ExpndKeepFareValidator::print(unsigned fcNo)
{
  *_dc << "SEASONALITY DOW VALIDATION: ";

  if (fcNo)
    *_dc << "FAILED FC: " << fcNo << "\n" << _diagMsg << "\n";

  else
    *_dc << "PASSED\n";
}

bool
ExpndKeepFareValidator::matchFareBasis(const ProcessTagInfo& pti,
                                       std::string newFareBasis,
                                       std::string excFareBasis)
{
  if (newFareBasis == excFareBasis)
    return true;

  if (_dc)
    _diagMsg = "EXC: " + excFareBasis + " NEW: " + newFareBasis;

  if (newFareBasis[0] != excFareBasis[0] || !getSeasonalityDOWIndicators(pti))
    return false;

  extractIndicators(excFareBasis);
  extractIndicators(newFareBasis);

  return newFareBasis == excFareBasis;
}

void
ExpndKeepFareValidator::extractIndicators(std::string& fareBasis)
{
  fareBasis = fareBasis.substr(1);

  if (_seasons.find(fareBasis[0]) != _seasons.end())
    fareBasis = fareBasis.substr(1);

  if (_DOWs.find(fareBasis[0]) != _DOWs.end())
    fareBasis = fareBasis.substr(1);
}

bool
ExpndKeepFareValidator::getSeasonalityDOWIndicators(const ProcessTagInfo& pti)
{
  if (pti.reissueSequence()->seasonalityDOWTblItemNo())
  {
    const SeasonalityDOW* dow =
        _trx->getSeasonalityDOW(pti.reissueSequence()->vendor(),
                                pti.reissueSequence()->seasonalityDOWTblItemNo(),
                                pti.fareMarket()->ruleApplicationDate());
    if (!dow)
    {
      _diagMsg = "DB ERROR: NO TABLE " +
                 boost::lexical_cast<std::string>(pti.reissueSequence()->seasonalityDOWTblItemNo());
      return false;
    }

    resetIndicators(false);
    packIndicators(*dow);
    return true;
  }

  if (!_IATAindicators)
  {
    resetIndicators(true);
    getIATAindicators();
  }

  return true;
}

void
ExpndKeepFareValidator::packIndicators(const SeasonalityDOW& sDOW)
{
  addIndicator('A', sDOW.indA());
  addIndicator('B', sDOW.indB());
  addIndicator('C', sDOW.indC());
  addIndicator('D', sDOW.indD());
  addIndicator('E', sDOW.indE());
  addIndicator('F', sDOW.indF());
  addIndicator('G', sDOW.indG());
  addIndicator('H', sDOW.indH());
  addIndicator('I', sDOW.indI());
  addIndicator('J', sDOW.indJ());
  addIndicator('K', sDOW.indK());
  addIndicator('L', sDOW.indL());
  addIndicator('M', sDOW.indM());
  addIndicator('N', sDOW.indN());
  addIndicator('O', sDOW.indO());
  addIndicator('P', sDOW.indP());
  addIndicator('Q', sDOW.indQ());
  addIndicator('R', sDOW.indR());
  addIndicator('S', sDOW.indS());
  addIndicator('T', sDOW.indT());
  addIndicator('U', sDOW.indU());
  addIndicator('V', sDOW.indV());
  addIndicator('W', sDOW.indW());
  addIndicator('X', sDOW.indX());
  addIndicator('Y', sDOW.indY());
  addIndicator('Z', sDOW.indZ());
}

void
ExpndKeepFareValidator::addIndicator(Indicator letter, Indicator value)
{
  switch (value)
  {
  case DOW:
    _DOWs.insert(letter);
    return;
  case SEASON:
    _seasons.insert(letter);
  }
}

void
ExpndKeepFareValidator::getIATAindicators()
{
  _seasons.insert(seasonIndicators.getValue().begin(), seasonIndicators.getValue().end());
  _DOWs.insert(dowIndicators.getValue().begin(), dowIndicators.getValue().end());
}

} // tse
