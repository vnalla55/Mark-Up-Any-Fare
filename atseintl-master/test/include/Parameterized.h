// -------------------------------------------------------------------
//
//
//  Copyright (C) Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include <boost/lexical_cast.hpp>
#include <cppunit/extensions/HelperMacros.h>

#define PARAM_TESTS(_Inputs_, _Outputs_, _InitMethod_, _TestMethod_)                               \
  _InitMethod_();                                                                                  \
  TSE_ASSERT(_Inputs_.size() == _Outputs_.size());                                                 \
  for (size_t i(0); i < _Inputs_.size(); ++i)                                                      \
  {                                                                                                \
    CPPUNIT_TEST_SUITE_ADD_TEST((new CPPUNIT_NS::TestCaller<TestFixtureType>(                      \
        context.getTestNameFor(#_TestMethod_ + boost::lexical_cast<std::string>(i)),               \
        &TestFixtureType::_TestMethod_,                                                            \
        context.makeFixture())));                                                                  \
  }
