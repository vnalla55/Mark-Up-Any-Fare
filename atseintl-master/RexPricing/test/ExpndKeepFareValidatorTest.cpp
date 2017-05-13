#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/list_of.hpp>
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "test/include/TestMemHandle.h"
#include "test/include/PrintCollection.h"
#include "DataModel/ExcItin.h"
#include "DBAccess/Loc.h"
#include "RexPricing/ExpndKeepFareValidator.h"

namespace tse
{
using boost::assign::list_of;

class ExpndKeepFareValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ExpndKeepFareValidatorTest);

  CPPUNIT_TEST(testMatchTagDefinitionNotInMap);
  CPPUNIT_TEST(testMatchTagDefinitionExactMatch);
  CPPUNIT_TEST(testMatchTagDefinitionDomesticNoMatchNationNation);
  CPPUNIT_TEST(testMatchTagDefinitionMatchCityCity);
  CPPUNIT_TEST(testMatchTagDefinitionMatchCityNation);
  CPPUNIT_TEST(testMatchTagDefinitionMatchNationCity);
  CPPUNIT_TEST(testMatchTagDefinitionMatchNationNation);
  CPPUNIT_TEST(testMatchSeasonalityDOWzeroTable);
  CPPUNIT_TEST(testMatchSeasonalityDOWnotA);
  CPPUNIT_TEST(testMatchFareBasisFirstDiff);
  CPPUNIT_TEST(testMatchFareBasisSame);
  CPPUNIT_TEST(testMatchFareBasisDOWFail);
  CPPUNIT_TEST(testMatchFareBasisBothFail);
  CPPUNIT_TEST(testMatchFareBasis23new23exc);
  CPPUNIT_TEST(testMatchFareBasisEndDiff23new23exc);
  CPPUNIT_TEST(testMatchFareBasisSame23new3exc);
  CPPUNIT_TEST(testMatchFareBasisSame23new0exc);
  CPPUNIT_TEST(testMatchFareBasisSame3new23exc);
  CPPUNIT_TEST(testMatchFareBasisSame2new23exc);
  CPPUNIT_TEST(testMatchFareBasisSame0new23exc);
  CPPUNIT_TEST(testMatchFareBasisSame0new0excShorter);
  CPPUNIT_TEST(testMatchFareBasisSame0new0excLonger);
  CPPUNIT_TEST(testMatchSeasonalityDOWNoCore);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    RexPricingTrx* trx = _memHandle(new RexPricingTrx);
    trx->exchangeItin().push_back(_memHandle(new ExcItin));
    trx->exchangeItin().front()->geoTravelType() = GeoTravelType::International;
    _validator = _memHandle(new ExpndKeepFareValidator(*trx));
  }

  void tearDown() { _memHandle.clear(); }

  void addPaxTypeFares(const std::vector<LocCode>& origCities,
                       const std::vector<LocCode>& destCities,
                       const std::vector<NationCode>& origNations,
                       const std::vector<NationCode>& destNations)
  {
    std::vector<LocCode>::const_iterator cci = origCities.begin();
    for (unsigned i = 0; cci != origCities.end(); ++cci, ++i)
      _validator->newExpndPtfs().push_back(
          getPtf(origCities[i], destCities[i], origNations[i], destNations[i]));

    _validator->setSearchScope();
  }

  PaxTypeFare* getPtf(LocCode origC, LocCode destC, NationCode origN, NationCode destN)
  {
    FareMarket* fm = _memHandle(new FareMarket);
    fm->boardMultiCity() = origC;
    fm->offMultiCity() = destC;
    Loc* orig = _memHandle(new Loc);
    orig->nation() = origN;
    Loc* dest = _memHandle(new Loc);
    dest->nation() = destN;
    fm->origin() = orig;
    fm->destination() = dest;

    PaxTypeFare* excPtf = _memHandle(new PaxTypeFare);
    excPtf->fareMarket() = fm;
    return excPtf;
  }

  void toExpndMap(const PaxTypeFare& excPtf)
  {
    // key is newFm, but for convenience we will use one from excFare
    _validator->_trx->expndKeepMap().insert(std::make_pair(excPtf.fareMarket(), &excPtf));

    CPPUNIT_ASSERT_EQUAL(_validator->newExpndPtfs()[0], _validator->matchTagDefinition(excPtf));
    CPPUNIT_ASSERT_EQUAL(_validator->newExpndPtfs()[0], _validator->_validationPairMap[&excPtf]);
    CPPUNIT_ASSERT_EQUAL(static_cast<ExpndKeepFareValidator::ValidationPairMap::size_type>(1),
                         _validator->_validationPairMap.size());
    CPPUNIT_ASSERT_EQUAL(*(_validator->newExpndPtfs().begin() + 1), *_validator->_searchBegin);
    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(0),
                         _validator->matchTagDefinition(excPtf));
  }

  void testMatchTagDefinitionNotInMap()
  {
    addPaxTypeFares(list_of("DFW")("CHI")("WAS"),
                    list_of("CHI")("WAS")("NYC"),
                    list_of("US")("US")("US"),
                    list_of("US")("US")("US"));

    PaxTypeFare* excPtf = getPtf("CHI", "WAS", "US", "US");

    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(0),
                         _validator->matchTagDefinition(*excPtf));
    CPPUNIT_ASSERT_EQUAL(*_validator->newExpndPtfs().begin(), *_validator->_searchBegin);
  }

  void testMatchTagDefinitionExactMatch()
  {
    addPaxTypeFares(list_of("DFW")("CHI")("WAS"),
                    list_of("CHI")("WAS")("NYC"),
                    list_of("US")("US")("US"),
                    list_of("US")("US")("US"));

    PaxTypeFare* excPtf = getPtf("CHI", "WAS", "US", "US");

    _validator->_trx->expndKeepMap().insert(
        std::make_pair(_validator->newExpndPtfs()[1]->fareMarket(), excPtf));

    CPPUNIT_ASSERT_EQUAL(_validator->newExpndPtfs()[0], _validator->matchTagDefinition(*excPtf));
    CPPUNIT_ASSERT_EQUAL(_validator->newExpndPtfs()[0], _validator->_validationPairMap[excPtf]);
    CPPUNIT_ASSERT_EQUAL(static_cast<ExpndKeepFareValidator::ValidationPairMap::size_type>(1),
                         _validator->_validationPairMap.size());
    CPPUNIT_ASSERT_EQUAL(_validator->newExpndPtfs()[0], _validator->_validationPairMap[excPtf]);
    CPPUNIT_ASSERT_EQUAL(*(_validator->newExpndPtfs().begin() + 1), *_validator->_searchBegin);
  }

  void testMatchTagDefinitionDomesticNoMatchNationNation()
  {
    addPaxTypeFares(list_of("DFW")("CHI")("WAS"),
                    list_of("CHI")("WAS")("NYC"),
                    list_of("US")("CA")("US"),
                    list_of("US")("US")("CA"));

    PaxTypeFare* excPtf = getPtf("MIA", "BOS", "RU", "US");

    _validator->_trx->expndKeepMap().insert(std::make_pair(excPtf->fareMarket(), excPtf));
    _validator->_trx->exchangeItin().front()->geoTravelType() = GeoTravelType::Domestic;

    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(0),
                         _validator->matchTagDefinition(*excPtf));
    CPPUNIT_ASSERT_EQUAL(*_validator->newExpndPtfs().begin(), *_validator->_searchBegin);
  }

  void testMatchTagDefinitionMatchCityCity()
  {
    addPaxTypeFares(list_of("DFW")("CHI")("CHI"),
                    list_of("WAS")("WAS")("NYC"),
                    list_of("US")("US")("CA"),
                    list_of("CA")("US")("US"));

    PaxTypeFare* excPtf = getPtf("CHI", "WAS", "RU", "RU");

    toExpndMap(*excPtf);
  }

  void testMatchTagDefinitionMatchCityNation()
  {
    addPaxTypeFares(list_of("DFW")("CHI")("BOS"),
                    list_of("WAS")("BOS")("WAS"),
                    list_of("US")("US")("US"),
                    list_of("US")("US")("US"));

    PaxTypeFare* excPtf = getPtf("CHI", "WAS", "RU", "US");

    toExpndMap(*excPtf);
  }

  void testMatchTagDefinitionMatchNationCity()
  {
    addPaxTypeFares(list_of("CHI")("CHI")("WAS"),
                    list_of("BOS")("WAS")("NYC"),
                    list_of("US")("CA")("US"),
                    list_of("CA")("CA")("US"));

    PaxTypeFare* excPtf = getPtf("MIA", "WAS", "CA", "RU");

    toExpndMap(*excPtf);
  }

  void testMatchTagDefinitionMatchNationNation()
  {
    addPaxTypeFares(list_of("DFW")("CHI")("WAS"),
                    list_of("CHI")("WAS")("NYC"),
                    list_of("RU")("CA")("US"),
                    list_of("US")("US")("CA"));

    PaxTypeFare* excPtf = getPtf("MIA", "BOS", "RU", "US");

    toExpndMap(*excPtf);
  }

  ProcessTagInfo* getPti()
  {
    ReissueSequence* rs = _memHandle(new ReissueSequence);
    rs->seasonalityDOWTblItemNo() = 0;
    ProcessTagInfo* pti = _memHandle(new ProcessTagInfo);
    pti->reissueSequence()->orig() = rs;
    pti->paxTypeFare() = _memHandle(new PaxTypeFare);

    _validator->_IATAindicators = true;
    _validator->_seasons.insert('G');
    _validator->_seasons.insert('H');
    _validator->_DOWs.insert('I');
    _validator->_DOWs.insert('J');

    return pti;
  }

  void testMatchSeasonalityDOWzeroTable()
  {
    CPPUNIT_ASSERT(_validator->matchSeasonalityDOW(*_memHandle(new ProcessTagInfo)));
  }

  void testMatchSeasonalityDOWnotA()
  {
    ProcessTagInfo* pti = _memHandle(new ProcessTagInfo);
    pti->reissueSequence()->orig() = _memHandle(new ReissueSequence);
    CPPUNIT_ASSERT(_validator->matchSeasonalityDOW(*pti));
  }

  void testMatchFareBasisFirstDiff()
  {
    CPPUNIT_ASSERT(!_validator->matchFareBasis(
        *_memHandle(new ProcessTagInfo), std::string("HEN"), std::string("BEN")));
  }

  void testMatchFareBasisSame()
  {
    CPPUNIT_ASSERT(_validator->matchFareBasis(
        *_memHandle(new ProcessTagInfo), std::string("HEN"), std::string("HEN")));
  }

  void testMatchFareBasisSeasonFail()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(!_validator->matchFareBasis(*pti, std::string("CHCKEN"), std::string("CFCKEN")));
  }

  void testMatchFareBasisDOWFail()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(!_validator->matchFareBasis(*pti, std::string("CICKEN"), std::string("CKCKEN")));
  }

  void testMatchFareBasisBothFail()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(
        !_validator->matchFareBasis(*pti, std::string("CHICKEN"), std::string("CABCKEN")));
  }

  void testMatchFareBasis23new23exc()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(
        _validator->matchFareBasis(*pti, std::string("CHICKEN"), std::string("CGJCKEN")));
  }

  void testMatchFareBasisEndDiff23new23exc()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(
        !_validator->matchFareBasis(*pti, std::string("CHICKEN"), std::string("CGJCKEM")));
  }

  void testMatchFareBasisSame23new3exc()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(_validator->matchFareBasis(*pti, std::string("CHICKEN"), std::string("CJCKEN")));
  }

  void testMatchFareBasisSame23new0exc()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(_validator->matchFareBasis(*pti, std::string("CHICKEN"), std::string("CCKEN")));
  }

  void testMatchFareBasisSame3new23exc()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(_validator->matchFareBasis(*pti, std::string("CICKEN"), std::string("CHICKEN")));
  }

  void testMatchFareBasisSame2new23exc()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(_validator->matchFareBasis(*pti, std::string("CHCKEN"), std::string("CGJCKEN")));
  }

  void testMatchFareBasisSame0new23exc()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(_validator->matchFareBasis(*pti, std::string("ABCDE"), std::string("AGJBCDE")));
  }

  void testMatchFareBasisSame0new0excShorter()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(!_validator->matchFareBasis(*pti, std::string("ABCDE"), std::string("ABCDEF")));
  }

  void testMatchFareBasisSame0new0excLonger()
  {
    ProcessTagInfo* pti = getPti();

    CPPUNIT_ASSERT(!_validator->matchFareBasis(*pti, std::string("ABCDEX"), std::string("ABCDE")));
  }

  void testMatchSeasonalityDOWNoCore()
  {
    ProcessTagInfo& pti = *_memHandle(new ProcessTagInfo);
    CPPUNIT_ASSERT(_validator->matchSeasonalityDOW(pti));
  }

protected:
  ExpndKeepFareValidator* _validator;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ExpndKeepFareValidatorTest);
}
