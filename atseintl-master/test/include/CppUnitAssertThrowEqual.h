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

#ifndef CPPUNITASSERTTHROWEQUAL_H
#define CPPUNITASSERTTHROWEQUAL_H

#define CPPUNIT_ASSERT_THROW_EQUAL(expression, ExceptionType, initValue)                           \
  do                                                                                               \
  {                                                                                                \
    bool cpputExceptionThrown_ = false;                                                            \
    try { expression; }                                                                            \
    catch (const ExceptionType& e)                                                                 \
    {                                                                                              \
      cpputExceptionThrown_ = true;                                                                \
      CPPUNIT_NS::assertEquals(ExceptionType(initValue), e, CPPUNIT_SOURCELINE(), "");             \
    }                                                                                              \
    catch (...) {}                                                                                 \
    if (cpputExceptionThrown_)                                                                     \
      break;                                                                                       \
    CPPUNIT_NS::Asserter::fail("Expected exception: " #ExceptionType " not thrown.",               \
                               CPPUNIT_SOURCELINE());                                              \
  } while (false);

#endif // CPPUNITASSERTTHROWEQUAL_H
