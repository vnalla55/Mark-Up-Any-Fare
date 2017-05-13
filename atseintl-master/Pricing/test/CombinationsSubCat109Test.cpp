#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/list_of.hpp>
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "Diagnostic/DiagCollector.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/OpenJawRestriction.h"
#include "Pricing/CombinationsSubCat109.h"

namespace tse
{
using boost::assign::list_of;

namespace
{
class MyNewDataHandleMock : public DataHandleMock
{
public:
  const std::vector<OpenJawRestriction*>* _openJawSetVector;

  const std::vector<OpenJawRestriction*>&
  getOpenJawRestriction(const VendorCode& vendor, int itemNo)
  {
    return *_openJawSetVector;
  }
};
}

class CombinationsSubCat109Stub : public CombinationsSubCat109
{
public:
  CombinationsSubCat109Stub(PricingTrx& trx,
                            DiagCollector& diag,
                            const VendorCode& vendor,
                            const uint32_t itemNo,
                            const PricingUnit& pu,
                            const FareUsage& fu,
                            Combinations::ValidationFareComponents& components,
                            bool& negativeApplication)
    : CombinationsSubCat109(
          trx, diag, vendor, itemNo, pu, fu, components, negativeApplication)
  {
  }

  virtual ~CombinationsSubCat109Stub() {}

protected:
  virtual bool
  isBetween(const Loc& ojFrom, const Loc& ojTo, const LocKey& setLoc1, const LocKey& setLoc2) const
  {
    return ojFrom.loc() == setLoc1.loc() && ojTo.loc() == setLoc2.loc();
  }

  virtual bool isInLoc(const Loc& loc, const LocKey& locKey) const
  {
    return loc.loc() == locKey.loc();
  }
};

class CombinationsSubCat109Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CombinationsSubCat109Test);

  CPPUNIT_TEST(testV1Seq2Set1Match);
  CPPUNIT_TEST(testV1Seq2Set2Match);
  CPPUNIT_TEST(testV1Seq3Set1NoMatch);
  CPPUNIT_TEST(testV1Seq2Set2NoMatch);
  CPPUNIT_TEST(testV1Seq2Set2ExceptionNoMatchSameItem);

  CPPUNIT_TEST(testV2Set1Match);
  CPPUNIT_TEST(testV2Set2Match);
  CPPUNIT_TEST(testV2Set1NoMatch);
  CPPUNIT_TEST(testV2Set2NoMatch);

  CPPUNIT_TEST(testV3BlankLocsSet1Match);
  CPPUNIT_TEST(testV3BlankLocsSet2Match);
  CPPUNIT_TEST(testV3Set1Match);
  CPPUNIT_TEST(testV3Set2Match);
  CPPUNIT_TEST(testV3Set1NoMatch);
  CPPUNIT_TEST(testV3Set2NoMatch);

  CPPUNIT_TEST(testNegativeSet1Match);
  CPPUNIT_TEST(testNegativeSet2Match);
  CPPUNIT_TEST(testNegativeSet2MatchOpposite);
  CPPUNIT_TEST(testNegativeSingleLocSet1Match);
  CPPUNIT_TEST(testNegativeSingleLocSet2Match);
  CPPUNIT_TEST(testNegativeSingleLocSet2MatchOpposite);
  CPPUNIT_TEST(testNegativeSet1NoMatch);
  CPPUNIT_TEST(testNegativeSet2NoMatch);
  CPPUNIT_TEST(testNegativeSingleLocSet1NoMatch);
  CPPUNIT_TEST(testNegativeSingleLocSet2NoMatch);

  CPPUNIT_TEST(testNegativeSet1V2Set2MatchSet2);
  CPPUNIT_TEST(testV3Set1NegativeSet2MatchSet1);
  CPPUNIT_TEST(testV3Set1NegativeSet1MatchSet1Negative);
  CPPUNIT_TEST(testV3Set1NegativeSet1V2Set2MatchSet2);
  CPPUNIT_TEST(testSameSetsMatchSet1);

  CPPUNIT_TEST(testOrigOJMatch);
  CPPUNIT_TEST(testOrigOJNoMatch);
  CPPUNIT_TEST(testDestOJMatch);
  CPPUNIT_TEST(testDestOJNoMatch);
  CPPUNIT_TEST(testDoubleOJMatch);
  CPPUNIT_TEST(testDoubleOJNoMatchOrigin);
  CPPUNIT_TEST(testDoubleOJNoMatchDestination);
  CPPUNIT_TEST(testUralRussiaException);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    PricingTrx* trx = _memHandle(new PricingTrx);
    DiagCollector* diag = _memHandle(new DiagCollector);
    PricingUnit* pu = _memHandle(new PricingUnit);
    pu->geoTravelType() = GeoTravelType::Domestic;
    pu->puType() = PricingUnit::Type::OPENJAW;
    FareUsage* fu = _memHandle(new FareUsage);
    Combinations::ValidationFareComponents* components =
        _memHandle(new Combinations::ValidationFareComponents);
    bool* negativeApplication = _memHandle(new bool(false));
    _subCat109 = _memHandle(new CombinationsSubCat109Stub(
        *trx, *diag, "ATP", 0, *pu, *fu, *components, *negativeApplication));
  }

  void tearDown() { _memHandle.clear(); }

  void makeOJRestrictionVector(const std::vector<Indicator>& set1ApplInd,
                               const std::vector<LocCode>& set1Loc1,
                               const std::vector<LocCode>& set1Loc2,
                               const std::vector<Indicator>& set2ApplInd,
                               const std::vector<LocCode>& set2Loc1,
                               const std::vector<LocCode>& set2Loc2,
                               MyNewDataHandleMock* myDataHandle = 0)
  {
    std::vector<OpenJawRestriction*>& jorv =
        *_memHandle(new std::vector<OpenJawRestriction*>(set1ApplInd.size()));
    for (unsigned i = 0; i < jorv.size(); ++i)
    {
      jorv[i] = _memHandle(new OpenJawRestriction);
      jorv[i]->set1ApplInd() = set1ApplInd[i];
      jorv[i]->set1Loc1().loc() = set1Loc1[i];
      jorv[i]->set1Loc2().loc() = set1Loc2[i];
      jorv[i]->set2ApplInd() = set2ApplInd[i];
      jorv[i]->set2Loc1().loc() = set2Loc1[i];
      jorv[i]->set2Loc2().loc() = set2Loc2[i];
    }

    if (myDataHandle)
      myDataHandle->_openJawSetVector = &jorv;
    else
      _subCat109->_openJawSetVector = &jorv;
  }

  Loc& makeLoc(const LocCode& locCode)
  {
    Loc& loc = *_memHandle(new Loc);
    loc.loc() = locCode;
    loc.city() = locCode;
    return loc;
  }

  void assertions(CombinationsSubCat109::SetNumber realSetMatched,
                  CombinationsSubCat109::SetNumber expectedSetMatched,
                  bool expectedNegativeApplication)
  {
    CPPUNIT_ASSERT_EQUAL(expectedSetMatched, realSetMatched);
    CPPUNIT_ASSERT_EQUAL(expectedNegativeApplication, _subCat109->_negativeApplication);
  }

  // value 1 matching

  void testV1Seq2Set1Match()
  {
    makeOJRestrictionVector(list_of('1')('1'),
                            list_of("CHI")("DFW"),
                            list_of("")(""),
                            list_of(' ')(' '),
                            list_of("")(""),
                            list_of("")(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::FIRST_SET,
               false);
  }

  void testV1Seq2Set2Match()
  {
    makeOJRestrictionVector(list_of(' ')(' '),
                            list_of("")(""),
                            list_of("")(""),
                            list_of('1')('1'),
                            list_of("JFK")("KRK"),
                            list_of("")(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("KRK"), makeLoc("JFK")),
               CombinationsSubCat109::SECOND_SET,
               false);
  }

  void testV1Seq3Set1NoMatch()
  {
    makeOJRestrictionVector(list_of('1')('1')('1'),
                            list_of("CHI")("DFW")("JFK"),
                            list_of("")("")(""),
                            list_of(' ')(' ')(' '),
                            list_of("")("")(""),
                            list_of("")("")(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("KRK"), makeLoc("DFW")),
               CombinationsSubCat109::NO_MATCH_SET,
               false);
  }

  void testV1Seq2Set2NoMatch()
  {
    makeOJRestrictionVector(list_of(' ')(' ')(' '),
                            list_of("")("")(""),
                            list_of("")("")(""),
                            list_of('1')('1')('1'),
                            list_of("CHI")("KRK")("DFW"),
                            list_of("")("")(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("KRK"), makeLoc("JFK")),
               CombinationsSubCat109::NO_MATCH_SET,
               false);
  }

  void testV1Seq2Set2ExceptionNoMatchSameItem()
  {
    makeOJRestrictionVector(list_of(' ')(' ')(' '),
                            list_of("")("")(""),
                            list_of("")("")(""),
                            list_of('1')('1')('1'),
                            list_of("CHI")("KRK")("DFW"),
                            list_of("")("")(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("KRK"), makeLoc("KRK")),
               CombinationsSubCat109::NO_MATCH_SET,
               false);
  }

  // value 2 matching

  void testV2Set1Match()
  {
    makeOJRestrictionVector(
        list_of('2'), list_of("CHI"), list_of("DFW"), list_of(' '), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::FIRST_SET,
               false);
  }

  void testV2Set2Match()
  {
    makeOJRestrictionVector(
        list_of(' '), list_of(""), list_of(""), list_of('2'), list_of("KRK"), list_of("JFK"));

    assertions(_subCat109->passOJSetRestriction(makeLoc("KRK"), makeLoc("JFK")),
               CombinationsSubCat109::SECOND_SET,
               false);
  }

  void testV2Set1NoMatch()
  {
    makeOJRestrictionVector(
        list_of('2'), list_of("CHI"), list_of("DFW"), list_of(' '), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("WAW"), makeLoc("DFW")),
               CombinationsSubCat109::NO_MATCH_SET,
               false);
  }

  void testV2Set2NoMatch()
  {
    makeOJRestrictionVector(
        list_of(' '), list_of(""), list_of(""), list_of('2'), list_of("KRK"), list_of("JFK"));

    assertions(_subCat109->passOJSetRestriction(makeLoc("KRK"), makeLoc("LON")),
               CombinationsSubCat109::NO_MATCH_SET,
               false);
  }

  // value 3 matching

  void testV3BlankLocsSet1Match()
  {
    makeOJRestrictionVector(
        list_of('3'), list_of(""), list_of(""), list_of(' '), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::FIRST_SET,
               false);
  }

  void testV3BlankLocsSet2Match()
  {
    makeOJRestrictionVector(
        list_of(' '), list_of(""), list_of(""), list_of('3'), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("JFK")),
               CombinationsSubCat109::SECOND_SET,
               false);
  }

  void testV3Set1Match()
  {
    makeOJRestrictionVector(
        list_of('3'), list_of(""), list_of(""), list_of(' '), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::FIRST_SET,
               false);
  }

  void testV3Set2Match()
  {
    makeOJRestrictionVector(
        list_of(' '), list_of(""), list_of(""), list_of('3'), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("KRK"), makeLoc("JFK")),
               CombinationsSubCat109::SECOND_SET,
               false);
  }

  void testV3Set1NoMatch()
  {
    makeOJRestrictionVector(
        list_of('3'), list_of("DFW"), list_of(""), list_of(' '), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::NO_MATCH_SET,
               false);
  }

  void testV3Set2NoMatch()
  {
    makeOJRestrictionVector(
        list_of(' '), list_of(""), list_of(""), list_of('3'), list_of("CHI"), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("KRK"), makeLoc("JFK")),
               CombinationsSubCat109::NO_MATCH_SET,
               false);
  }

  // value X matching

  void testNegativeSet1Match()
  {
    makeOJRestrictionVector(
        list_of('X'), list_of("CHI"), list_of("DFW"), list_of(' '), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::FIRST_SET,
               true);
  }

  void testNegativeSet2Match()
  {
    makeOJRestrictionVector(
        list_of(' '), list_of(""), list_of(""), list_of('X'), list_of("CHI"), list_of("DFW"));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::SECOND_SET,
               true);
  }

  void testNegativeSet2MatchOpposite()
  {
    makeOJRestrictionVector(
        list_of(' '), list_of(""), list_of(""), list_of('X'), list_of("DFW"), list_of("CHI"));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::SECOND_SET,
               true);
  }

  void testNegativeSingleLocSet1Match()
  {
    makeOJRestrictionVector(
        list_of('X'), list_of("CHI"), list_of(""), list_of(' '), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::FIRST_SET,
               true);
  }

  void testNegativeSingleLocSet2Match()
  {
    makeOJRestrictionVector(
        list_of(' '), list_of(""), list_of(""), list_of('X'), list_of("CHI"), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("KRK")),
               CombinationsSubCat109::SECOND_SET,
               true);
  }

  void testNegativeSingleLocSet2MatchOpposite()
  {
    makeOJRestrictionVector(
        list_of(' '), list_of(""), list_of(""), list_of('X'), list_of("KRK"), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("KRK")),
               CombinationsSubCat109::SECOND_SET,
               true);
  }

  void testNegativeSet1NoMatch()
  {
    makeOJRestrictionVector(
        list_of('X'), list_of("CHI"), list_of("DFW"), list_of(' '), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("KRK"), makeLoc("DFW")),
               CombinationsSubCat109::NO_MATCH_SET,
               false);
  }

  void testNegativeSet2NoMatch()
  {
    makeOJRestrictionVector(
        list_of(' '), list_of(""), list_of(""), list_of('X'), list_of("CHI"), list_of("DFW"));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("KRK")),
               CombinationsSubCat109::NO_MATCH_SET,
               false);
  }

  void testNegativeSingleLocSet1NoMatch()
  {
    makeOJRestrictionVector(
        list_of('X'), list_of("CHI"), list_of(""), list_of(' '), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("KRK"), makeLoc("DFW")),
               CombinationsSubCat109::NO_MATCH_SET,
               false);
  }

  void testNegativeSingleLocSet2NoMatch()
  {
    makeOJRestrictionVector(
        list_of(' '), list_of(""), list_of(""), list_of('X'), list_of("CHI"), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("DFW"), makeLoc("KRK")),
               CombinationsSubCat109::NO_MATCH_SET,
               false);
  }

  // mixed values

  void testNegativeSet1V2Set2MatchSet2()
  {
    makeOJRestrictionVector(
        list_of('X'), list_of("CHI"), list_of("DFW"), list_of('2'), list_of("CHI"), list_of("DFW"));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::SECOND_SET,
               false);
  }

  void testV3Set1NegativeSet2MatchSet1()
  {
    makeOJRestrictionVector(
        list_of('3'), list_of(""), list_of(""), list_of('X'), list_of("CHI"), list_of("DFW"));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::FIRST_SET,
               false);
  }

  void testV3Set1NegativeSet1MatchSet1Negative()
  {
    makeOJRestrictionVector(list_of('3')('X'),
                            list_of("")("CHI"),
                            list_of("")("DFW"),
                            list_of(' ')(' '),
                            list_of("")(""),
                            list_of("")(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::FIRST_SET,
               true);
  }

  void testV3Set1NegativeSet1V2Set2MatchSet2()
  {
    makeOJRestrictionVector(list_of('3')('X'),
                            list_of("")("CHI"),
                            list_of("")("DFW"),
                            list_of('1')('1'),
                            list_of("DFW")("CHI"),
                            list_of("")(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::SECOND_SET,
               false);
  }

  void testSameSetsMatchSet1()
  {
    makeOJRestrictionVector(
        list_of('3'), list_of(""), list_of(""), list_of('3'), list_of(""), list_of(""));

    assertions(_subCat109->passOJSetRestriction(makeLoc("CHI"), makeLoc("DFW")),
               CombinationsSubCat109::FIRST_SET,
               false);
  }

  // complex integration tests

  void prepareForIntegrationTest()
  {
    makeOJRestrictionVector(list_of('2'),
                            list_of("CHI"),
                            list_of("DFW"),
                            list_of('2'),
                            list_of("BOS"),
                            list_of("WAS"),
                            _memHandle(new MyNewDataHandleMock));

    _subCat109->_components.push_back(*_memHandle(new Combinations::ValidationElement));
    _subCat109->_components.back().resetAll();

    CPPUNIT_ASSERT_EQUAL(false, _subCat109->_components[0]._passMinor);
    CPPUNIT_ASSERT_EQUAL(false, _subCat109->_diag.isActive());
  }

  void addFareUsage(const LocCode& boardCity, const LocCode& offCity)
  {
    FareUsage& fu = *_memHandle(new FareUsage);
    fu.paxTypeFare() = _memHandle(new PaxTypeFare);
    fu.paxTypeFare()->fareMarket() = _memHandle(new FareMarket);
    fu.paxTypeFare()->fareMarket()->origin() = &makeLoc(boardCity);
    fu.paxTypeFare()->fareMarket()->destination() = &makeLoc(offCity);
    fu.paxTypeFare()->fareMarket()->boardMultiCity() = boardCity;
    fu.paxTypeFare()->fareMarket()->offMultiCity() = offCity;

    const_cast<PricingUnit&>(_subCat109->_prU).fareUsage().push_back(&fu);
  }

  void executeIntegrationTest(PricingUnit::PUSubType subType,
                              const std::vector<LocCode>& boardings,
                              const std::vector<LocCode>& landings,
                              char matchStatus)
  {
    prepareForIntegrationTest();

    const_cast<PricingUnit&>(_subCat109->_prU).puSubType() = subType;

    for (unsigned i = 0; i < landings.size(); ++i)
      addFareUsage(boardings[i], landings[i]);

    CPPUNIT_ASSERT_EQUAL(true, _subCat109->match());
    CPPUNIT_ASSERT_EQUAL(matchStatus, _subCat109->_components[0].getSubCat(Combinations::m109));
  }

  void testOrigOJMatch()
  {
    executeIntegrationTest(
        PricingUnit::ORIG_OPENJAW, list_of("CHI"), list_of("DFW"), Combinations::MATCH);
  }

  void testOrigOJNoMatch()
  {
    executeIntegrationTest(
        PricingUnit::ORIG_OPENJAW, list_of("KRK"), list_of("DFW"), Combinations::NO_MATCH);
  }

  void testDestOJMatch()
  {
    executeIntegrationTest(PricingUnit::DEST_OPENJAW,
                           list_of("KRK")("DFW"),
                           list_of("CHI")("KRK"),
                           Combinations::MATCH);
  }

  void testDestOJNoMatch()
  {
    executeIntegrationTest(PricingUnit::DEST_OPENJAW,
                           list_of("KRK")("DFW"),
                           list_of("WAS")("KRK"),
                           Combinations::NO_MATCH);
  }

  void testDoubleOJMatch()
  {
    executeIntegrationTest(PricingUnit::DOUBLE_OPENJAW,
                           list_of("BOS")("DFW"),
                           list_of("CHI")("WAS"),
                           Combinations::MATCH);
  }

  void testDoubleOJNoMatchOrigin()
  {
    executeIntegrationTest(PricingUnit::DOUBLE_OPENJAW,
                           list_of("BOS")("DFW"),
                           list_of("CHI")("KRK"),
                           Combinations::NO_MATCH);
  }

  void testDoubleOJNoMatchDestination()
  {
    executeIntegrationTest(PricingUnit::DOUBLE_OPENJAW,
                           list_of("BOS")("DFW"),
                           list_of("KRK")("WAS"),
                           Combinations::NO_MATCH);
  }

  void testUralRussiaException()
  {
    const_cast<PricingUnit&>(_subCat109->_prU).geoTravelType() = GeoTravelType::International;

    Loc& moscow = makeLoc("MOW");
    moscow.nation() = "RU";
    Loc& krasnoyarsk = makeLoc("KJA");
    krasnoyarsk.nation() = "XU";
    CPPUNIT_ASSERT(_subCat109->sameCountrySegment(moscow, krasnoyarsk));
    CPPUNIT_ASSERT(_subCat109->sameCountrySegment(krasnoyarsk, moscow));
  }

protected:
  CombinationsSubCat109* _subCat109;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinationsSubCat109Test);
}
