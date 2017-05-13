//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#ifndef CPPUNITHELPERMACROS_H
#define CPPUNITHELPERMACROS_H

#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

#include "test/include/CppUnitSkipTest.h"
#include "test/include/CppUnitAssertThrowEqual.h"

#include "test/include/GetSuiteName.h"

#undef CPPUNIT_TEST_SUITE_REGISTRATION
#define CPPUNIT_TEST_SUITE_REGISTRATION(testName) \
  CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(testName, getSuiteName())\

#define REGISTRY_ADD \
  CPPUNIT_REGISTRY_ADD(getSuiteName(), "Atsev2")
REGISTRY_ADD;
#endif // CPPUNITHELPERMACROS_H
