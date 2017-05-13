//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

// macros to let us do branch prediction.
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)


#ifdef EXTEND_BRANCH_PREDICTION_ON
  // macros for new changes
  #define _LIKELY(x) __builtin_expect(!!(x), 1)
  #define _UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
  #define _LIKELY(x) (x)
  #define _UNLIKELY(x) (x)
#endif
