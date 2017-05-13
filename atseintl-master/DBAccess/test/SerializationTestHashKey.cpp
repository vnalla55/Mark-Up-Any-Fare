//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/test/SerializationTestHashKey.h"
#include "DBAccess/HashKey.h"
#include "DBAccess/ObjectKey.h"

CPPUNIT_TEST_SUITE_REGISTRATION(tse::SerializationTestHashKey);

namespace tse
{
typedef HashKey<std::string,
                int,
                char,
                long long int,
                RecordScope,
                GlobalDirection,
                uint64_t,
                DateTime,
                char> MyTestKey;

static const std::string
keyValueA("ABCD");
static const int
keyValueB(11);
static const char
keyValueC('E');
static const long long int
keyValueD(2222);
static const RecordScope
keyValueE(DOMESTIC);
static const GlobalDirection
keyValueF(GlobalDirection::US);
static const uint64_t
keyValueG(33333333);
static DateTime
keyValueH(time(NULL));
static const char
keyValueI(' ');

static const char*
dummyTableName("NONAME");

static const char*
keyNameA("_a_");
static const char*
keyNameB("_b_");
static const char*
keyNameC("_c_");
static const char*
keyNameD("_d_");
static const char*
keyNameE("_e_");
static const char*
keyNameF("_f_");
static const char*
keyNameG("_g_");
static const char*
keyNameH("_h_");
static const char*
keyNameI("_i_");

void
SerializationTestHashKey::testObjectToHash()
{
  MyTestKey originalHash(keyValueA,
                         keyValueB,
                         keyValueC,
                         keyValueD,
                         keyValueE,
                         keyValueF,
                         keyValueG,
                         keyValueH,
                         keyValueI);
  originalHash.initialized = true;

  // std::cout << std::endl << std::endl << "ORIGINAL HASHKEY:" << std::endl <<
  // originalHash.toString() << std::endl ;

  ObjectKey originalObj(dummyTableName);
  originalObj.setValue(keyNameA, keyValueA);
  originalObj.setValue(keyNameB, keyValueB);
  originalObj.setValue(keyNameC, keyValueC);
  originalObj.setValue(keyNameD, keyValueD);
  originalObj.setValue(keyNameE, keyValueE);
  originalObj.setValue(keyNameF, keyValueF);
  originalObj.setValue(keyNameG, keyValueG);
  originalObj.setValue(keyNameH, keyValueH);
  originalObj.setValue(keyNameI, keyValueI);

  // std::cout << std::endl << "TRANSLATED OBJECTKEY: [" << originalObj.toString() << "]" <<
  // std::endl ;

  MyTestKey convertedHash;

  convertedHash.initialized = (originalObj.getValue(keyNameA, convertedHash._a) &&
                               originalObj.getValue(keyNameB, convertedHash._b) &&
                               originalObj.getValue(keyNameC, convertedHash._c) &&
                               originalObj.getValue(keyNameD, convertedHash._d) &&
                               originalObj.getValue(keyNameE, convertedHash._e) &&
                               originalObj.getValue(keyNameF, convertedHash._f) &&
                               originalObj.getValue(keyNameG, convertedHash._g) &&
                               originalObj.getValue(keyNameH, convertedHash._h) &&
                               originalObj.getValue(keyNameI, convertedHash._i));

  // std::cout << std::endl << "CONVERTED HASHKEY:" << std::endl << convertedHash.toString() <<
  // std::endl << std::endl ;

  CPPUNIT_ASSERT_EQUAL(originalHash, convertedHash);
}

void
SerializationTestHashKey::testToAndFromString()
{
  MyTestKey originalHash(keyValueA,
                         keyValueB,
                         keyValueC,
                         keyValueD,
                         keyValueE,
                         keyValueF,
                         keyValueG,
                         keyValueH,
                         keyValueI);

  originalHash.initialized = true;

  std::string str(originalHash.toString());

  // std::cout << std::endl << "STRINGIFIED HASHKEY: [" << str << "]" << std::endl ;

  MyTestKey newHash;

  newHash.fromString(str);
  newHash.initialized = true;

  CPPUNIT_ASSERT_EQUAL(originalHash, newHash);

  std::string secondStr(newHash.toString());

  // std::cout << std::endl << "2ND STRINGIFIED HASHKEY: [" << secondStr << "]" << std::endl ;

  CPPUNIT_ASSERT_EQUAL(str, secondStr);
}
}
