#include <algorithm>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "Common/TaxShoppingConfig.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
namespace
{
template <class Iterator>
bool
isSorted(Iterator first, Iterator last)
{
  if (first == last)
    return true;

  Iterator next = first;
  while (++next != last)
  {
    if (*next < *first)
      return false;
    ++first;
  }
  return true;
}

template <class Container>
bool
hasValue(const Container& container, const typename Container::value_type& value)
{
  return std::find(container.begin(), container.end(), value) != container.end();
}
}

class TaxShoppingConfigTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxShoppingConfigTest);
  CPPUNIT_TEST(test_emptyValues);
  CPPUNIT_TEST(test_nationLists);
  CPPUNIT_TEST(test_taxCodesList);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }
  void tearDown() { _memHandle.clear(); }

  void test_emptyValues()
  {
    CPPUNIT_ASSERT_MESSAGE("Test config initialization", Global::hasConfig());
    TaxShoppingConfig taxConfig(Global::config());
    CPPUNIT_ASSERT_MESSAGE("TvlDateDepTaxNations vector empty",
                           taxConfig.getTvlDateDepTaxNations().empty());
    CPPUNIT_ASSERT_MESSAGE("FltNoDepTaxNations vector empty",
                           taxConfig.getFltNoDepTaxNations().empty());
    CPPUNIT_ASSERT_MESSAGE("SameDayDepTaxNations vector empty",
                           taxConfig.getSameDayDepTaxNations().empty());
  }

  void test_nationLists()
  {
    CPPUNIT_ASSERT_MESSAGE("Test config initialization", Global::hasConfig());
    // in random order, it should be sorted in TaxShoppingConfig
    const std::string nationList("NO|SX|LT|MG|DE|ER|VN|HK|NF|HN|AU|BE|CC|CO|IE|JP|LC|NO|SX|LT|MG|"
                                 "DE|ER|VN|HK|NF|HN|AU|BE|CC|CO|IE|JP|LC");
    // number of nations without duplications
    const size_t nationsCount = 17u;
    TestConfigInitializer::setValue("TVL_DEP_TAX_NATIONS", nationList, "TAX_SVC");
    TestConfigInitializer::setValue("FLTNO_DEP_TAX_NATIONS", nationList, "TAX_SVC");
    TestConfigInitializer::setValue("SAME_DAY_DEP_TAX_NATIONS", nationList, "TAX_SVC");

    TaxShoppingConfig taxConfig(Global::config());
    const NationCodesVec& tvlDateDepTaxNations = taxConfig.getTvlDateDepTaxNations();
    const NationCodesVec& fltNoDepTaxNations = taxConfig.getFltNoDepTaxNations();
    const NationCodesVec& sameDayDepTaxNations = taxConfig.getSameDayDepTaxNations();

    // check if duplication were skipped
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "TvlDateDepTaxNations size check", tvlDateDepTaxNations.size(), nationsCount);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "FltNoDepTaxNations size check", fltNoDepTaxNations.size(), nationsCount);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "SameDayDepTaxNations size check", sameDayDepTaxNations.size(), nationsCount);

    CPPUNIT_ASSERT_MESSAGE("TvlDateDepTaxNations vector sorted",
                           isSorted(tvlDateDepTaxNations.begin(), tvlDateDepTaxNations.end()));
    CPPUNIT_ASSERT_MESSAGE("FltNoDepTaxNations vector sorted",
                           isSorted(fltNoDepTaxNations.begin(), fltNoDepTaxNations.end()));
    CPPUNIT_ASSERT_MESSAGE("SameDayDepTaxNations vector sorted",
                           isSorted(sameDayDepTaxNations.begin(), sameDayDepTaxNations.end()));

    typedef std::vector<std::string> StringVector;
    StringVector testNations;
    boost::split(testNations, nationList, boost::is_any_of("|"));

    for (StringVector::iterator it = testNations.begin(); it != testNations.end(); ++it)
    {
      CPPUNIT_ASSERT_MESSAGE("TvlDateDepTaxNations contains " + *it,
                             hasValue(tvlDateDepTaxNations, *it));
      CPPUNIT_ASSERT_MESSAGE("FltNoDepTaxNations contains " + *it,
                             hasValue(fltNoDepTaxNations, *it));
      CPPUNIT_ASSERT_MESSAGE("SameDayDepTaxNations contains " + *it,
                             hasValue(sameDayDepTaxNations, *it));
    }

    const std::string nationAA("AA");
    CPPUNIT_ASSERT_MESSAGE("TvlDateDepTaxNations doesn't contain " + nationAA,
                           !hasValue(tvlDateDepTaxNations, nationAA));
    CPPUNIT_ASSERT_MESSAGE("FltNoDepTaxNations doesn't contain " + nationAA,
                           !hasValue(fltNoDepTaxNations, nationAA));
    CPPUNIT_ASSERT_MESSAGE("SameDayDepTaxNations doesn't contain " + nationAA,
                           !hasValue(sameDayDepTaxNations, nationAA));
  }

  void test_taxCodesList()
  {
    CPPUNIT_ASSERT_MESSAGE("Test config initialization - tax codes list", Global::hasConfig());
    // in random order, it should be sorted in TaxShoppingConfig
    const std::string taxList("WS|US|GR|TS|TS|GR|WS|US|GR|TS|TS|GR");
    // number of taxes without duplication
    const size_t taxListCount = 4u;
    TestConfigInitializer::setValue("TAX_TRANSIT_MINUTES_ROUNDING", taxList, "TAX_SVC");

    TaxShoppingConfig taxConfig(Global::config());
    const TaxCodesVec& taxRoundVec = taxConfig.roundTransitMinutesTaxCodes();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Tax rounding list size", taxListCount, taxRoundVec.size());

    CPPUNIT_ASSERT_MESSAGE("roundTransitMinutestTaxCodes vector sorted",
                           isSorted(taxRoundVec.begin(), taxRoundVec.end()));

    typedef std::vector<std::string> StringVector;
    StringVector testTaxCodes;
    boost::split(testTaxCodes, taxList, boost::is_any_of("|"));

    TaxCodesVec::const_iterator it = taxRoundVec.begin();
    for (StringVector::iterator it = testTaxCodes.begin(); it != testTaxCodes.end(); ++it)
    {
      CPPUNIT_ASSERT_MESSAGE("roundTransitMinutesTaxCodes contains " + *it,
                             hasValue(taxRoundVec, *it));
    }
    const TaxCode taxCode("AY");
    CPPUNIT_ASSERT_MESSAGE("roundTransitMinutesTaxCodes doesn't contain" + taxCode,
                           !hasValue(taxRoundVec, taxCode));

  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxShoppingConfigTest);
}
