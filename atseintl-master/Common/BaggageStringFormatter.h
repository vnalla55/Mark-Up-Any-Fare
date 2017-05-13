//-------------------------------------------------------------------
//  Copyright Sabre 2012
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

#include "DataModel/BaggageTravel.h"

#include <vector>

namespace tse
{
namespace BaggageStringFormatter
{

void
old_printBaggageTravelSegments(const BaggageTravel& bt, std::ostringstream& output);

void
old_printBaggageTravelSegments(TravelSegPtrVecCI begin, TravelSegPtrVecCI end, std::ostringstream& output);

std::string
printBaggageTravelSegmentsWithNumbering(const BaggageTravel& bt);

std::string
printBaggageTravelSegmentsWithoutNumbering(const BaggageTravel& bt);

std::string
printBaggageTravelSegmentsWithNumbering(TravelSegPtrVecCI begin, TravelSegPtrVecCI end);

std::string
printBaggageTravelSegmentsWithoutNumbering(TravelSegPtrVecCI begin, TravelSegPtrVecCI end);

void
printBtCarriers(const BaggageTravel& bt,
                const bool usDot,
                const bool defer,
                std::ostringstream& out);
}
} // tse

