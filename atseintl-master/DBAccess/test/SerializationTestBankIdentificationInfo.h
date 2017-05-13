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
#ifndef SERIALIZATION_TEST_BANKIDENTIFICATIONINFO_H
#define SERIALIZATION_TEST_BANKIDENTIFICATIONINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/BankIdentificationInfo.h"

namespace tse
{
class SerializationTestBankIdentificationInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestBankIdentificationInfo);
  CPPUNIT_TEST(testInfoType<BankIdentificationInfo>);
  CPPUNIT_TEST_SUITE_END();
};
}

#endif // SERIALIZATION_TEST_BANKIDENTIFICATIONINFO_H
