#include "DBAccess/test/DiscountNotifyTest.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/ObjectKey.h"
#include "DBAccess/DiscountDAO.h"

CPPUNIT_TEST_SUITE_REGISTRATION(tse::DiscountNotifyTest);

namespace tse
{
const char* DISCOUNT_CACHE_NAME = "DISCOUNT";
const int NUM = 5;

const std::string TYPES[NUM] = { DISCOUNT_TYPE_CHILDREN, DISCOUNT_TYPE_TOUR, DISCOUNT_TYPE_AGENT,
                                 DISCOUNT_TYPE_OTHER, DISCOUNT_TYPE_INVALID // keep this last
};

const int CATEGORIES[NUM] = { DiscountInfo::CHILD, DiscountInfo::TOUR, DiscountInfo::AGENT,
                              DiscountInfo::OTHERS, 0 // keep this last
};

const char* VENDORS[NUM] = { "ABC ", "DEF ", "GHI ", "JKL ", "" // keep this last
};

const int ITEMNOS[NUM] = { 11111, 22222, 33333, 44444, 0 // keep this last
};

void
DiscountNotifyTest::testTranslate()
{
  // std::cout << std::endl ;

  for (int i = 0; i < NUM; ++i)
  {
    ObjectKey objectKeyA(DISCOUNT_CACHE_NAME);
    ObjectKey objectKeyB(DISCOUNT_CACHE_NAME);
    DiscountKey key;
    char itemNoString[8];
    sprintf(itemNoString, "%d", ITEMNOS[i]);

    objectKeyA.entityType() = TYPES[i];
    objectKeyA.keyFields()["VENDOR"] = VENDORS[i];
    objectKeyA.keyFields()["ITEMNO"] = itemNoString;
    // std::cout << "Translating notification [" << objectKeyA.toString() << "] to object key..." <<
    // std::endl ;
    bool initialized(DiscountDAO::keyFromNotify(objectKeyA, key));
    if (i == (NUM - 1))
    {
      CPPUNIT_ASSERT(!initialized);
      CPPUNIT_ASSERT(!key.initialized);
    }
    else
    {
      CPPUNIT_ASSERT(initialized);
      CPPUNIT_ASSERT(key.initialized);
    }
    CPPUNIT_ASSERT(key._a == VENDORS[i]);
    CPPUNIT_ASSERT(key._b == ITEMNOS[i]);
    CPPUNIT_ASSERT(key._c == CATEGORIES[i]);

    // std::cout << "Translating cache key [" << key.toString() << "] to notification..." <<
    // std::endl ;
    DiscountDAO::notifyFromKey(key, objectKeyB);
    CPPUNIT_ASSERT(objectKeyB.entityType() == TYPES[i]);
    CPPUNIT_ASSERT(objectKeyB.tableName() == DISCOUNT_CACHE_NAME);
    CPPUNIT_ASSERT(objectKeyB.keyFields()["VENDOR"] == VENDORS[i]);
    CPPUNIT_ASSERT(objectKeyB.keyFields()["ITEMNO"] == itemNoString);
  }

  // std::cout << std::endl ;
}

} // tse
