//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#ifndef GTESTHELPERMACROS_H
#define GTESTHELPERMACROS_H

#include <fstream>
#include "test/include/GetSuiteName.h"

/* It is a copy of macro from include/gtest/internal/gtest-internal.h
 * from gtest source code with replaced "#test_case_name" to
 * "(getSuiteName()+"/."+std::string(#test_case_name)).c_str()".
 * This is modyfied macro generating test_case_name with path name.
 */
#undef GTEST_TEST_
#define GTEST_TEST_(test_case_name, test_name, parent_class, parent_id)\
class GTEST_TEST_CLASS_NAME_(test_case_name, test_name) : public parent_class {\
 public:\
  GTEST_TEST_CLASS_NAME_(test_case_name, test_name)() {}\
 private:\
  virtual void TestBody();\
  static ::testing::TestInfo* const test_info_ GTEST_ATTRIBUTE_UNUSED_;\
  GTEST_DISALLOW_COPY_AND_ASSIGN_(\
      GTEST_TEST_CLASS_NAME_(test_case_name, test_name));\
};\
\
::testing::TestInfo* const GTEST_TEST_CLASS_NAME_(test_case_name, test_name)\
  ::test_info_ =\
    ::testing::internal::MakeAndRegisterTestInfo(\
        (getSuiteName()+"/."+std::string(#test_case_name)).c_str(), #test_name, NULL, NULL, \
        (parent_id), \
        parent_class::SetUpTestCase, \
        parent_class::TearDownTestCase, \
        new ::testing::internal::TestFactoryImpl<\
            GTEST_TEST_CLASS_NAME_(test_case_name, test_name)>);\
void GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::TestBody()

/* This macro is for type-parametrized tests.
 * It is a copy of macro from include/gtest/gtest-typed-test.h
 * from gtest source code with replaced "#Prefix" to
 * "(getSuiteName()+"/."+std::string(#Prefix)).c_str()".
 * This is modyfied macro generating Prefix with path name.
 */
#undef INSTANTIATE_TYPED_TEST_CASE_P
# define INSTANTIATE_TYPED_TEST_CASE_P(Prefix, CaseName, Types) \
  bool gtest_##Prefix##_##CaseName GTEST_ATTRIBUTE_UNUSED_ = \
      ::testing::internal::TypeParameterizedTestCase<CaseName, \
          GTEST_CASE_NAMESPACE_(CaseName)::gtest_AllTests_, \
          ::testing::internal::TypeList< Types >::type>::Register(\
              (getSuiteName()+"/."+std::string(#Prefix)).c_str(), #CaseName, GTEST_REGISTERED_TEST_NAMES_(CaseName))

#endif // GTESTHELPERMACROS_H
