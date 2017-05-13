#include <vector>
#include <boost/assign/list_of.hpp>
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ExcItin.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using namespace std;
using boost::assign::list_of;

class CommonRexPricing
{
public:
  CommonRexPricing() {}

  void setUp()
  {
    _trx = _memHandle(new RexPricingTrx);
    _trx->exchangeItin().push_back(_memHandle(new ExcItin));
    _trx->exchangeItin().front()->geoTravelType() = GeoTravelType::International;

    _allRepricePTFs = _memHandle(new std::vector<const PaxTypeFare*>);

    for (uint16_t i = 1; i != 4; ++i)
    {
      _trx->exchangeItin().front()->fareComponent().push_back(makeFCI(i));
      _allRepricePTFs->push_back(makePTF());
    }
  }

  void tearDown() { _memHandle.clear(); }

  FareCompInfo* makeFCI(uint16_t no)
  {
    FareCompInfo* fci = _memHandle(new FareCompInfo);
    fci->fareCompNumber() = no;
    fci->fareMarket() = _memHandle(new FareMarket);
    fci->fareMarket()->origin() = _memHandle(new Loc);
    fci->fareMarket()->destination() = _memHandle(new Loc);
    return fci;
  }

  PaxTypeFare* makePTF()
  {
    PaxTypeFare* ptf = _memHandle(new PaxTypeFare);
    ptf->fareMarket() = _memHandle(new FareMarket);
    ptf->fareMarket()->origin() = _memHandle(new Loc);
    ptf->fareMarket()->destination() = _memHandle(new Loc);
    return ptf;
  }

  FareMarket* getFCI(int index)
  {
    return _trx->exchangeItin().front()->fareComponent().at(index)->fareMarket();
  }

  FareMarket* getPTF(int index)
  {
    return const_cast<PaxTypeFare*>(_allRepricePTFs->at(index))->fareMarket();
  }

  void setUpOrigin(const std::vector<LocCode> boardCtys,
                   const std::vector<NationCode> boardNtns,
                   const std::vector<IATASubAreaCode> boardSAs)
  {
    for (int i = 0; i != 3; ++i)
    {
      FareMarket* fm = getPTF(i);
      setUpLocation(fm->boardMultiCity(),
                    const_cast<Loc&>(*fm->origin()),
                    boardCtys.at(i),
                    boardNtns.at(i),
                    boardSAs.at(i));

      fm = getFCI(i);
      setUpLocation(fm->boardMultiCity(),
                    const_cast<Loc&>(*fm->origin()),
                    boardCtys.at(i),
                    boardNtns.at(i),
                    boardSAs.at(i));
    }
  }

  void setUpDestination(const std::vector<LocCode> destCtys,
                        const std::vector<NationCode> destNtns,
                        const std::vector<IATASubAreaCode> destSAs)
  {
    for (int i = 0; i != 3; ++i)
    {
      FareMarket* fm = getPTF(i);
      setUpLocation(fm->offMultiCity(),
                    const_cast<Loc&>(*fm->destination()),
                    destCtys.at(i),
                    destNtns.at(i),
                    destSAs.at(i));

      fm = getFCI(i);
      setUpLocation(fm->offMultiCity(),
                    const_cast<Loc&>(*fm->destination()),
                    destCtys.at(i),
                    destNtns.at(i),
                    destSAs.at(i));
    }
  }

  void setUpLocation(LocCode& targetCity,
                     Loc& targetLoc,
                     const LocCode& valueCity,
                     const NationCode& valueNation,
                     const IATASubAreaCode& valueSArea)
  {
    targetCity = valueCity;
    targetLoc.nation() = valueNation;
    targetLoc.subarea() = valueSArea;
  }

  void setUpDirectCityCityMapping()
  {
    setUpOrigin(list_of("MOV")("KRK")("DFW"), list_of("RU")("PL")("US"), list_of("32")("21")("11"));
    setUpDestination(
        list_of("KRK")("DFW")("MOV"), list_of("PL")("US")("RU"), list_of("21")("11")("32"));
  }

protected:
  TestMemHandle _memHandle;
  RexPricingTrx* _trx;
  std::vector<const PaxTypeFare*>* _allRepricePTFs;
};
}
