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

#include "RexPricing/GenericRexMapper.h"

#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag602Collector.h"
#include "Diagnostic/Diag689Collector.h"


#include <algorithm>

namespace tse
{

GenericRexMapper::GenericRexMapper(RexPricingTrx& trx,
                                   const std::vector<const PaxTypeFare*>* allRepricePTFs)
  : _trx(trx),
    _allRepricePTFs(allRepricePTFs),
    _dc(nullptr),
    _domesticMap(trx.exchangeItin().front()->fareComponent().size(), nullptr),
    _internationalMap(trx.exchangeItin().front()->fareComponent().size(), nullptr)
{}

namespace
{
class CityCityMapper : public std::unary_function<const PaxTypeFare&, bool>
{
public:
  const FareCompInfo& _fci;

  CityCityMapper(const FareCompInfo& fci) : _fci(fci) {}

  bool operator()(const PaxTypeFare& ptf) const
  {
    return _fci.fareMarket()->boardMultiCity() == ptf.fareMarket()->boardMultiCity() &&
           _fci.fareMarket()->offMultiCity() == ptf.fareMarket()->offMultiCity();
  }

  void print(const PaxTypeFare& ptf, std::ostringstream& diag)
  {
    diag << " CITY-CITY       MATCH: " << ptf.fareMarket()->boardMultiCity() << "-"
         << ptf.fareMarket()->offMultiCity() << "\n";
  }
};

class CityCountryMapper : public std::unary_function<const PaxTypeFare&, bool>
{
public:
  const FareCompInfo& _fci;

  CityCountryMapper(const FareCompInfo& fci) : _fci(fci) {}

  bool operator()(const PaxTypeFare& ptf) const
  {
    return _fci.fareMarket()->boardMultiCity() == ptf.fareMarket()->boardMultiCity() &&
           _fci.fareMarket()->destination()->nation() == ptf.fareMarket()->destination()->nation();
  }

  void print(const PaxTypeFare& ptf, std::ostringstream& diag)
  {
    diag << " CITY-COUNTRY    MATCH: " << ptf.fareMarket()->boardMultiCity() << "-"
         << ptf.fareMarket()->destination()->nation() << "(" << ptf.fareMarket()->offMultiCity()
         << ")\n";
  }
};

class CountryCountryMapper : public std::unary_function<const PaxTypeFare&, bool>
{
public:
  const FareCompInfo& _fci;

  CountryCountryMapper(const FareCompInfo& fci) : _fci(fci) {}

  bool operator()(const PaxTypeFare& ptf) const
  {
    return _fci.fareMarket()->origin()->nation() == ptf.fareMarket()->origin()->nation() &&
           _fci.fareMarket()->destination()->nation() == ptf.fareMarket()->destination()->nation();
  }

  void print(const PaxTypeFare& ptf, std::ostringstream& diag)
  {
    diag << " COUNTRY-COUNTRY MATCH: " << ptf.fareMarket()->origin()->nation() << "("
         << ptf.fareMarket()->boardMultiCity() << ")"
         << "-" << ptf.fareMarket()->destination()->nation() << "("
         << ptf.fareMarket()->offMultiCity() << ")"
         << "\n";
  }
};

class CountrySubAreaMapper : public std::unary_function<const PaxTypeFare&, bool>
{
public:
  const FareCompInfo& _fci;

  CountrySubAreaMapper(const FareCompInfo& fci)
    : _fci(fci)
  {}

  bool operator()(const PaxTypeFare& ptf) const
  {
    return _fci.fareMarket()->origin()->nation() == ptf.fareMarket()->origin()->nation() &&
           _fci.fareMarket()->destination()->subarea() ==
               ptf.fareMarket()->destination()->subarea();
  }

  void print(const PaxTypeFare& ptf, std::ostringstream& diag)
  {
    diag << " COUNTRY-SUBAREA MATCH: " << ptf.fareMarket()->origin()->nation() << "("
         << ptf.fareMarket()->boardMultiCity() << ")"
         << "-" << ptf.fareMarket()->destination()->subarea() << "("
         << ptf.fareMarket()->offMultiCity() << ")"
         << "\n";
  }
};

class SubAreaSubAreaMapper : public std::unary_function<const PaxTypeFare&, bool>
{
public:
  const FareCompInfo& _fci;

  SubAreaSubAreaMapper(const FareCompInfo& fci)
    : _fci(fci)
  {}

  bool operator()(const PaxTypeFare& ptf) const
  {
    return _fci.fareMarket()->origin()->subarea() == ptf.fareMarket()->origin()->subarea() &&
           _fci.fareMarket()->destination()->subarea() ==
               ptf.fareMarket()->destination()->subarea();
  }

  void print(const PaxTypeFare& ptf, std::ostringstream& diag)
  {
    diag << " SUBAREA-SUBAREA MATCH: " << ptf.fareMarket()->origin()->subarea() << "("
         << ptf.fareMarket()->boardMultiCity() << ")"
         << "-" << ptf.fareMarket()->destination()->subarea() << "("
         << ptf.fareMarket()->offMultiCity() << ")"
         << "\n";
  }
};

bool
directionalityMatch(const FareMarket& exc, const FareMarket& reprice)
{
  return exc.direction() == FMDirection::UNKNOWN || reprice.direction() == FMDirection::UNKNOWN ||
         (exc.direction() == FMDirection::INBOUND && reprice.direction() == FMDirection::INBOUND) ||
         (exc.direction() == FMDirection::OUTBOUND && reprice.direction() == FMDirection::OUTBOUND);

  //... and we have a problem here, since pricing FareUsage structures access is far far away...
}
}

template <typename Mapper>
bool
GenericRexMapper::mappingStep(std::list<const PaxTypeFare*>& notMappedRepricePtf,
                              std::vector<const PaxTypeFare*>& map,
                              std::ostringstream& diag,
                              Mapper mapper)
{
  std::list<const PaxTypeFare*>::iterator currentRepricePtf = notMappedRepricePtf.begin();
  for (; currentRepricePtf != notMappedRepricePtf.end(); ++currentRepricePtf)
    if (directionalityMatch(*mapper._fci.fareMarket(), *(**currentRepricePtf).fareMarket()) &&
        mapper(**currentRepricePtf))
    {
      if (_dc)
        mapper.print(**currentRepricePtf, diag);

      map[mapper._fci.fareCompNumber() - 1] = *currentRepricePtf;
      notMappedRepricePtf.erase(currentRepricePtf);
      return true;
    }

  return false;
}

void
GenericRexMapper::map()
{
  std::list<const PaxTypeFare*> ptfForStrictMapping(_allRepricePTFs->begin(),
                                                    _allRepricePTFs->end());

    if (_dc)
      _domesticDiag << "STRICT CITY-CITY MAPPING\n";

    for (const FareCompInfo* fci : _trx.exchangeItin().front()->fareComponent())
    {
      if (_dc)
        _domesticDiag << "  EXC FC " << fci->fareCompNumber();

      if (mappingStep(ptfForStrictMapping, _domesticMap, _domesticDiag, CityCityMapper(*fci)))
        ;
      else if (_dc)
        _domesticDiag << " DO NOT MATCH\n";
    }

    createQuickAccessMap();

    if (_trx.exchangeItin().front()->geoTravelType() == GeoTravelType::International)
  {
    std::list<const PaxTypeFare*> ptfForInternationalMapping(_allRepricePTFs->begin(),
                                                             _allRepricePTFs->end());

    if (_dc)
      _internationalDiag << "MAPPING FOR INTERNATIONAL TICKET\n";

    for (const FareCompInfo* fci : _trx.exchangeItin().front()->fareComponent())
    {
      if (_dc)
        _internationalDiag << "  EXC FC " << fci->fareCompNumber();

      if (mappingStep(ptfForInternationalMapping,
                      _internationalMap,
                      _internationalDiag,
                      CityCityMapper(*fci)) ||
          mappingStep(ptfForInternationalMapping,
                      _internationalMap,
                      _internationalDiag,
                      CityCountryMapper(*fci)) ||
          mappingStep(ptfForInternationalMapping,
                      _internationalMap,
                      _internationalDiag,
                      CountryCountryMapper(*fci)) ||
          mappingStep(ptfForInternationalMapping,
                      _internationalMap,
                      _internationalDiag,
                      CountrySubAreaMapper(*fci)) ||
          mappingStep(ptfForInternationalMapping,
                      _internationalMap,
                      _internationalDiag,
                      SubAreaSubAreaMapper(*fci)))
        ;
      else if (_dc)
        _internationalDiag << " DO NOT MATCH\n";
    }
  }
}

void
GenericRexMapper::createQuickAccessMap()
{
  for (const PaxTypeFare* ptf : *_allRepricePTFs)
  {
    std::vector<const PaxTypeFare*>::const_iterator fcNumberIterator =
        std::find(_domesticMap.begin(), _domesticMap.end(), ptf);

    _quickAccessMap.push_back(
        fcNumberIterator == _domesticMap.end()
            ? 0
            : static_cast<uint16_t>(fcNumberIterator - _domesticMap.begin() + 1));
  }
}

} // tse
