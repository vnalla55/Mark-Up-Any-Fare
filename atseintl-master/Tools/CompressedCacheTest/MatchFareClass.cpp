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
#include <boost/cstdint.hpp>
#include "MatchFareClass.h"
#include "MatchFareClassC.h"

//g++ MatchFareClass.cpp -O2 -finline-functions -funroll-loops -minline-all-stringops -Wall -o matchFareClass

boost::uint64_t getCurrentMillis();

typedef bool (*FUNCTION) (const char *, const char *);

void matchFareClass()
{
  std::cout << __FUNCTION__ << std::endl;
  //FUNCTION functions[] = { tse::matchFareClassC, tse::preselectByFareClass, tse::matchFareClassN };
  FUNCTION functions[] = { tse::matchFareClassN, tse::matchFareClassC };
  size_t sz(sizeof(functions) / sizeof(FUNCTION));
  for (size_t k = 0; k < sz; ++k)
  {
    FUNCTION function(functions[k]);
    boost::uint64_t start(getCurrentMillis());
    //-E70 matches BHE70NR but does not match BHE701
    assert(function("-E70", "BHE70NR"));
    assert(!function("-E70", "BHE701"));
    //-E7K matches BHE7KNR and BHE7K1
    assert(function("-E7K", "BHE7KNR"));
    assert(function("-E7K", "BHE7K1"));
    assert(!function("H3EE", "H3EEX"));
    assert(!function("H3EEX", "H3EE"));
    //Y-E matches YHE, YHXE, Y8E, YLXE1A, and YE
    assert(function("Y-E", "YHE"));
    assert(function("Y-E", "YHXE"));
    assert(function("Y-E", "Y8E"));
    assert(function("Y-E", "YLXE1A"));
    assert(function("Y-E", "YE"));
    //-YE will match to AYE, B7YE, and ABCYE, but does not match YE
    assert(function("-YE", "AYE"));
    assert(function("-YE", "B7YE"));
    assert(function("-YE", "ABCYE"));
    assert(!function("-YE", "YE"));
    //Y-8 matches YH8. It does not match Y18 or YH89
    assert(function("Y-8", "YH8"));
    assert(!function("Y-8", "Y18"));
    assert(!function("Y-8", "YH89"));
    //Y8- matches Y8E, but does not match Y89, or Y80E
    assert(function("Y8-", "Y8E"));
    assert(!function("Y8-", "Y89"));
    assert(!function("Y8-", "Y80E"));
    //-E70 will match to BE70 but not to E70
    assert(function("-E70", "BE70"));
    assert(!function("-E70", "E70"));
    //fare family H-EE matches to HLEE1M, HKEE, and HAPEE3M
    assert(function("H-EE", "HLEE1M"));
    assert(function("H-EE", "HKEE"));
    assert(function("H-EE", "HAPEE3M"));
    /////
    assert(function("YA7-", "YA7"));
    assert(function("YA7-", "YA7WA"));
    /////
    assert(function("-1", "SLX13IQ1"));
    assert(function("-1", "DNE01AQ1"));
    assert(!function("-1", "DNE01A21"));
    //////
    assert(function("", "Y26"));
    assert(function("Y26", "Y26"));
    assert(!function("Y26", "Y26HX0"));
    for (size_t i = 0; i < 100000000; ++i)
    {
      function("H-EE", "HAPEE3M");
      function("-1", "SLX13IQ1");
      function("-1", "DNE01AQ1");
      function("-1", "DNE01A21");
      function("Y-8", "YH89");
      function("Y8-", "Y80E");
      function("YA7-", "YA7WA");
    }
    std::cout << (k + 1) << " time " << (getCurrentMillis() - start) << std::endl;
  }
}
