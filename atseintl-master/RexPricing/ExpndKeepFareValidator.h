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

#include "DataModel/RexPricingTrx.h"

namespace tse
{

class Diag689Collector;
class Logger;

class ExpndKeepFareValidator
{
  friend class ExpndKeepFareValidatorTest;

public:
  ExpndKeepFareValidator(RexPricingTrx& trx) : _trx(&trx), _dc(nullptr), _IATAindicators(false) {}

  typedef std::map<const PaxTypeFare*, const PaxTypeFare*> ValidationPairMap;

  Diag689Collector*& dc() { return _dc; }
  std::vector<const PaxTypeFare*>& newExpndPtfs() { return _newExpndPtfs; }

  void setSearchScope() { _searchBegin = _newExpndPtfs.begin(); }

  const PaxTypeFare* matchTagDefinition(const PaxTypeFare& excPtf);
  void removeFromSearchScope(const PaxTypeFare& newExpndPtf);
  const PaxTypeFare* getExcPtf(const PaxTypeFare& newPtf);

  bool matchSeasonalityDOW(const ProcessTagPermutation& permutation);

protected:
  void resetIndicators(bool iata)
  {
    _seasons.clear();
    _DOWs.clear();
    _IATAindicators = iata;
  }

  template <typename Op>
  bool matchExcToNew(Op operation, const PaxTypeFare& excPtf, const PaxTypeFare*& matched)
  {
    RexPricingTrx::ExpndKeepMapI i = _trx->expndKeepMap().begin();
    for (; i != _trx->expndKeepMap().end(); ++i)
      if (i->second == &excPtf)
      {
        std::vector<const PaxTypeFare*>::iterator newPtfI =
            std::find_if(_searchBegin, _newExpndPtfs.end(), std::bind2nd(operation, i->first));

        if (newPtfI != _newExpndPtfs.end())
        {
          matched = *newPtfI;
          return true;
        }
      }
    return false;
  }

  static constexpr Indicator DOW = 'D';
  static constexpr Indicator SEASON = 'S';

  typedef std::map<const std::pair<const PaxTypeFare*, uint32_t>, bool> Cache;
  typedef std::pair<const PaxTypeFare*, const uint32_t> CacheKey;
  typedef std::pair<bool, bool> CacheResult;

  const PaxTypeFare* getNewPtf(const PaxTypeFare& excPtf);

  bool matchSeasonalityDOW(const ProcessTagInfo& pti);

  bool
  matchFareBasis(const ProcessTagInfo& pti, std::string newFareBasis, std::string excFareBasis);
  void extractIndicators(std::string& fareBasis);
  bool getSeasonalityDOWIndicators(const ProcessTagInfo& pti);
  void packIndicators(const SeasonalityDOW& sDOW);
  void addIndicator(Indicator letter, Indicator value);
  void getIATAindicators();
  void print(unsigned fcNo = 0);

private:
  RexPricingTrx* _trx;
  Diag689Collector* _dc;
  std::vector<const PaxTypeFare*> _newExpndPtfs;
  std::vector<const PaxTypeFare*>::iterator _searchBegin;
  ValidationPairMap _validationPairMap;
  std::string _diagMsg;
  std::set<Indicator> _seasons;
  std::set<Indicator> _DOWs;
  Cache _cache;
  bool _IATAindicators;
  static Logger _logger;
};
}

