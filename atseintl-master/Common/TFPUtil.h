//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include <vector>

namespace tse
{
class FareMarket;
class TravelSeg;
class Trx;

namespace TFPUtil
{
bool isThroughFarePrecedencePossibleAtGeography(const std::vector<TravelSeg*>& segments);
bool isThroughFarePrecedenceNeeded(const Trx& trx, const std::vector<TravelSeg*>& segments);
bool isThroughFarePrecedenceNeededNGS(const Trx& trx, const FareMarket& fareMarket);
}
}
