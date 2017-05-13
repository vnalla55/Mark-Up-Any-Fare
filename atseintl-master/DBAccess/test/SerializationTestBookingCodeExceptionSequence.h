//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_BOOKINGCODEEXCEPTIONSEQUENCE_H
#define SERIALIZATION_TEST_BOOKINGCODEEXCEPTIONSEQUENCE_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/BookingCodeExceptionSequence.h"

namespace tse
{
class SerializationTestBookingCodeExceptionSequence : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestBookingCodeExceptionSequence);
  CPPUNIT_TEST(testInfoType<BookingCodeExceptionSequence>);
  CPPUNIT_TEST(testInfoVectorType<BookingCodeExceptionSequence>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_BOOKINGCODEEXCEPTIONSEQUENCE_H
