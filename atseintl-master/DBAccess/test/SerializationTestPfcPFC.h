//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_PFCPFC_H
#define SERIALIZATION_TEST_PFCPFC_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/PfcPFC.h"

namespace tse
{
class SerializationTestPfcPFC : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestPfcPFC);
  CPPUNIT_TEST(testInfoType<PfcPFC>);
  CPPUNIT_TEST(testInfoVectorType<PfcPFC>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_PFCPFC_H
