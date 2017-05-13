//-------------------------------------------------------------------------------
// Copyright 2016, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_SPANISHREFERENCEFAREINFO_H
#define SERIALIZATION_TEST_SPANISHREFERENCEFAREINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/SpanishReferenceFareInfo.h"

namespace tse
{
class SerializationTestSpanishReferenceFareInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestSpanishReferenceFareInfo);
  CPPUNIT_TEST(testInfoType<SpanishReferenceFareInfo>);
  CPPUNIT_TEST(testInfoVectorType<SpanishReferenceFareInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_SPANISHREFERENCEFAREINFO_H
