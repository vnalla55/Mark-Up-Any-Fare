//----------------------------------------------------------------------------
//
//  Description: TimeBomb class for the cppUnit tests.
//
//  Provides a mechanism so a test can be marked to not run until a specific date.
//
//  Copyright Sabre 2009
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
#ifndef TIME_BOMB_H
#define TIME_BOMB_H

#include "Common/DateTime.h"
#include <iostream>

// This macro allows you to bypass a test until a specific date.  However,
// the date must be less than 15 days in the future.
// This macro should be the first line in the test you wish to bypass.

#define TIME_BOMB(year, month, day)                                                                \
  if (((DateTime::localTime().addDays(15)) > DateTime(year, month, day)) &&                        \
      (DateTime::localTime() < DateTime(year, month, day)))                                        \
    return;                                                                                        \
  std::cout << "TIME BOMB EXPIRED --- please fix this test now and remove me" << std::endl

#endif // End #ifndef TIME_BOMB_H
