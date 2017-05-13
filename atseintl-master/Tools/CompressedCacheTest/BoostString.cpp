//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#include <cassert>
#include <ctime>
#include <iostream>
#include <boost/container/string.hpp>
#include <boost/serialization/string.hpp>
//g++ BoostString.cpp -I /opt/atseintl/3rdParty/boost_1_48_0/include -O2 -finline-functions -funroll-loops -minline-all-stringops -Wall -o boostString

extern uint64_t getCurrentMillis();

void boostString()
{
  std::cout << __FUNCTION__ << std::endl;
  const size_t size(10000000);
  boost::uint64_t start(getCurrentMillis());
  for (size_t i = 0; i < size; ++i)
  {
    std::string string1("HAPEE3M");
    std::string string2("SLX13IQ1");
    std::string string3("DNE01AQ1");
    std::string string4("DNE01A21");
    std::string string5("YH89");
    std::string string6("Y80E");
    std::string string7("YA7WA");
  }
  std::cout << "std::string" << " time:" << (getCurrentMillis() - start) << std::endl;
  start = getCurrentMillis();
  for (size_t i = 0; i < size; ++i)
  {
    boost::container::string string1("HAPEE3M");
    boost::container::string string2("SLX13IQ1");
    boost::container::string string3("DNE01AQ1");
    boost::container::string string4("DNE01A21");
    boost::container::string string5("YH89");
    boost::container::string string6("Y80E");
    boost::container::string string7("YA7WA");
    /*
    size_t sx(sizeof(string7));
    string2 = string3;
    assert(string3 == string2);
    string4 = string5;
    assert(string5 == string4);
    boost::container::string tmp(string1 + string2);
    std::cout << tmp << std::endl;
    */
  }
  std::cout << "boost::container::string" << " time:" << (getCurrentMillis() - start) << std::endl;
}
