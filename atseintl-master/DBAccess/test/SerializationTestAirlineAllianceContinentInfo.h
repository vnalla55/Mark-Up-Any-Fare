// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#ifndef SERIALIZATION_TEST_AIRLINEALLIANCECONTINENT_H
#define SERIALIZATION_TEST_AIRLINEALLIANCECONTINENT_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/AirlineAllianceContinentInfo.h"

namespace tse
{
class SerializationTestAirlineAllianceContinentInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestAirlineAllianceContinentInfo);
  CPPUNIT_TEST(testInfoType<AirlineAllianceContinentInfo>);
  CPPUNIT_TEST_SUITE_END();
};
}

#endif // SERIALIZATION_TEST_AIRLINEALLIANCECONTINENT_H
