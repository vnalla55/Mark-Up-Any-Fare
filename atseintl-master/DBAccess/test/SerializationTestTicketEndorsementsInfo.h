//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_TICKETENDORSEMENTSINFO_H
#define SERIALIZATION_TEST_TICKETENDORSEMENTSINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/TicketEndorsementsInfo.h"

namespace tse
{
class SerializationTestTicketEndorsementsInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestTicketEndorsementsInfo);
  CPPUNIT_TEST(testInfoType<TicketEndorsementsInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_TICKETENDORSEMENTSINFO_H
