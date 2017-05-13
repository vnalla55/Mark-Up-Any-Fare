//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_ETICKETPSEUDOINFO_H
#define SERIALIZATION_TEST_ETICKETPSEUDOINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/ETicketPseudoInfo.h"

namespace tse
{
class SerializationTestETicketPseudoInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestETicketPseudoInfo);
  CPPUNIT_TEST(testInfoType<ETicketPseudoInfo>);
  CPPUNIT_TEST(testInfoVectorType<ETicketPseudoInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_ETICKETPSEUDOINFO_H
