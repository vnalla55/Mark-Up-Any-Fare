//----------------------------------------------------------------------------
//  File:        Diag912Collector.C
//  Created:     2008-06-05
//
//  Description: Diagnostic 912 formatter
//
//  Updates:
//
//  Copyright Sabre 2004
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

#include "Diagnostic/Diag912Collector.h"

#include "Diagnostic/DiagCollector.h"

#include <iomanip>

namespace tse
{
void
Diag912Collector::printHeader(DiagCollector& dc)
{
  dc << "912 : FARE AND BITMAP VALIDATION AFTER TRANSACTION" << std::endl;
  dc << std::endl;
  dc << "RULE FAILED OR ROUTE NUMBER LEGEND" << std::endl;
  dc << "CAT-n : FARE FAILED DURING SHOPPING COMPONENT VALIDATION ON RULE" << std::endl;
  dc << "nnnn  : ROUTE NUMBER / FARE PASSED SHOPPING COMPONENT VALIDATION" << std::endl;
  dc << "FAIL  : FARE CONSIDERED FAILED DUE TO ALL BITS IN BITMAP INVALID" << std::endl;
  dc << "DIR   : FARE CONSIDERED FAILED DUE TO DIRECTIONALITY CHECK" << std::endl;
  dc << "#     : FARE VALIDATED BY FIRST PASS VALIDATION" << std::endl;
  dc << "%     : FARE VALIDATED BY CALLBACK VALIDATION" << std::endl;
  dc << "" << std::endl;
  dc << "BITMAP LEGEND" << std::endl;
  dc << "G : FAIL GLOBAL DIRECTION" << std::endl;
  dc << "2 : FAIL CAT 2 DATE TIME" << std::endl;
  dc << "4 : FAIL CAT 4 FLIGHT APPLICATION" << std::endl;
  dc << "9 : FAIL CAT 9 TRANSFER" << std::endl;
  dc << "T : FAIL CAT 14 TRAVEL RESTRICTION" << std::endl;
  dc << "8 : FAIL CAT 8 STOPOVER" << std::endl;
  dc << "Q : FAIL QUALIFY CAT 4" << std::endl;
  dc << "R : FAIL ROUTING" << std::endl;
  dc << "B : FAIL BOOKING CODE" << std::endl;
  dc << "X : FAIL EXCEEDS CONNECTION POINT LIMIT FOR ASO LEG" << std::endl;
  dc << "E : FAIL FARE EFFECTIVE DATE" << std::endl;
  dc << "F : FAIL FARE EXPIRED DATE" << std::endl;
  dc << "D : NO DATE FOUND" << std::endl;
  dc << "N : NOT APPLICABLE FOR THIS CARRIER" << std::endl;
  dc << "L : FAIL DUE TO TOO MANY FAILED BITS EARLIER" << std::endl;
  dc << "P : NOT APPLICABLE FOR ORIGIN AND/OR DESTINATION AIRPORT" << std::endl;
  dc << "- : PASS" << std::endl;
  dc << "***************************************************" << std::endl;
  dc << "" << std::endl;
}
}
