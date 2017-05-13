#include "DataModel/ValidatingCxrGSAData.h"
#include "test/include/CppUnitHelperMacros.h"

#include "DBAccess/DataManager.h"

using namespace std;

namespace tse
{
class ValidatingCxrGSADataTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ValidatingCxrGSADataTest);
  CPPUNIT_TEST(testHash);
  CPPUNIT_TEST(testGetSwapCarriers);
  CPPUNIT_TEST_SUITE_END();

public:

  void testHash()
  {
    ValidatingCxrGSADataHashMap v;

    ValidatingCxrGSAData v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;

    v["AA|AA"] = &v1;
    v["AA|AABB"] = &v2;
    v["AABB|AA"] = &v3;
    v["AABB|AABB"] = &v4;
    v["AABB|AABBCC"] = &v5;
    v["AABBCC|AABBCC"] = &v6;
    v["AABBCC|AABBCCDD"] = &v7;
    v["AABBCCDD|AABBCCDD"] = &v8;
    v["AABBCCDD|AABBCCDDEE"] = &v9;
    v["AABBCCDDEE|AABBCCDDEE"] = &v10;

    CPPUNIT_ASSERT(v.size() == 10);

    CPPUNIT_ASSERT(v["AA|AA"] == &v1);
    CPPUNIT_ASSERT(v["AA|AABB"] == &v2);
    CPPUNIT_ASSERT(v["AABB|AA"] == &v3);
    CPPUNIT_ASSERT(v["AABB|AABB"] == &v4);
    CPPUNIT_ASSERT(v["AABB|AABBCC"] == &v5);
    CPPUNIT_ASSERT(v["AABBCC|AABBCC"] == &v6);
    CPPUNIT_ASSERT(v["AABBCC|AABBCCDD"] == &v7);
    CPPUNIT_ASSERT(v["AABBCCDD|AABBCCDD"] == &v8);
    CPPUNIT_ASSERT(v["AABBCCDD|AABBCCDDEE"] == &v9);
    CPPUNIT_ASSERT(v["AABBCCDDEE|AABBCCDDEE"] == &v10);
  }

  void testGetSwapCarriers()
  {
    ValidatingCxrGSAData v;
    v.gsaSwapMap()["AA"].insert("BB");
    v.gsaSwapMap()["AA"].insert("CC");
    v.gsaSwapMap()["ZZ"].insert("CC");

    set<CarrierCode> result;
    v.getSwapCarriers("DD", result);
    CPPUNIT_ASSERT(result.empty());

    v.getSwapCarriers("AA", result);
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT(result.count("BB") == 1);
    CPPUNIT_ASSERT(result.count("CC") == 1);

    result.clear();
    v.getSwapCarriers("ZZ", result);
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT(result.count("CC") == 1);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ValidatingCxrGSADataTest);
}
