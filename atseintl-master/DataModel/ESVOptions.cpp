//-------------------------------------------------------------------
//
//  File:        ESVOptions.cpp
//  Created:     November 13, 2007
//  Authors:
//
//  Description: Options connected to ESV Diversity logic
//
//  Updates:
//
//  Copyright Sabre 2007
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

#include "DataModel/ESVOptions.h"

using namespace tse;

const int16_t ESVOptions::DEFAULT_NUM_OF_SOLUTIONS = 500;

const int16_t ESVOptions::DEFAULT_NUM_MUST_PRICE_ONLINE = 50; // Q60
const int16_t ESVOptions::DEFAULT_NUM_MUST_PRICE_INTERLINE = 200; // Q61
const int16_t ESVOptions::DEFAULT_NUM_MUST_PRICE_NONSTOP_ONLINE = 10; // Q62
const int16_t ESVOptions::DEFAULT_NUM_MUST_PRICE_NONSTOP_INTERLINE = 10; // Q63
const int16_t ESVOptions::DEFAULT_NUM_MUST_PRICE_ONESTOP_ONLINE = 10; // Q64
const int16_t ESVOptions::DEFAULT_NUM_MUST_PRICE_NONANDONE_ONLINE = 10; // Q6Q
const int16_t ESVOptions::DEFAULT_NUM_MUST_PRICE_NONANDONE_INTERLINE = 10; // Q6R

const float ESVOptions::DEFAULT_MAX_ALLOWED_OVERAGE_PER_CARRIER = 0.1f; // Q68
const int16_t ESVOptions::DEFAULT_FLIGHT_OPTION_REUSE_LIMIT = 10; // Q69
const float ESVOptions::DEFAULT_UPPER_BOUND_FACTOR_NOT_NONSTOP = 2.0; // Q6A
const float ESVOptions::DEFAULT_UPPER_BOUND_FACTOR_NONSTOP = 2.0; // Q6S
const float ESVOptions::DEFAULT_UPPER_BOUND_FACTOR_LFS = 2.0; // Q6U

const int16_t ESVOptions::DEFAULT_STOP_PENALTY = 50; // Q65
const int16_t ESVOptions::DEFAULT_TRAV_DUR_PENALTY = 30; // Q66
const int16_t ESVOptions::DEFAULT_DEP_TIME_DEV_PENALTY = 0; // Q67

const int16_t ESVOptions::DEFAULT_NUM_ESV_LOWFARE = 200; // Q0E
const float ESVOptions::DEFAULT_MAX_ALLOWED_OVERAGE_PER_CARRIER_LFS = 10; // Q5T
const int16_t ESVOptions::DEFAULT_NUM_MIN_ONLINE_PER_CARRIER = 5; // Q5U
const float ESVOptions::DEFAULT_MIN_PERCENT_OF_ONLINE = 5; // Q5V
const int16_t ESVOptions::DEFAULT_FLIGHT_OPTION_REUSE_FOR_AVS = 20; // Q5W
const int16_t ESVOptions::DEFAULT_FLIGHT_OPTION_REUSE_FOR_NONAVS = 40; // Q5X

ESVOptions::ESVOptions()
  : _mustPriceFlag(true),
    _noOfMustPriceOnlineSolutions(DEFAULT_NUM_MUST_PRICE_ONLINE),
    _noOfMustPriceInterlineSolutions(DEFAULT_NUM_MUST_PRICE_INTERLINE),
    _noOfMustPriceNonstopOnlineSolutions(DEFAULT_NUM_MUST_PRICE_NONSTOP_ONLINE),
    _noOfMustPriceNonstopInterlineSolutions(DEFAULT_NUM_MUST_PRICE_NONSTOP_INTERLINE),
    _noOfMustPriceSingleStopOnlineSolutions(DEFAULT_NUM_MUST_PRICE_ONESTOP_ONLINE),
    _noOfMustPriceNonAndOneStopOnlineSolutions(DEFAULT_NUM_MUST_PRICE_NONANDONE_ONLINE),
    _noOfMustPriceNonAndOneStopInterlineSolutions(DEFAULT_NUM_MUST_PRICE_NONANDONE_INTERLINE),
    _percentFactor(DEFAULT_MAX_ALLOWED_OVERAGE_PER_CARRIER),
    _flightOptionReuseLimit(DEFAULT_FLIGHT_OPTION_REUSE_LIMIT),
    _upperBoundFactorForNotNonstop(DEFAULT_UPPER_BOUND_FACTOR_NOT_NONSTOP),
    _upperBoundFactorForNonstop(DEFAULT_UPPER_BOUND_FACTOR_NONSTOP),
    _upperBoundFactorForLFS(DEFAULT_UPPER_BOUND_FACTOR_LFS),
    _perStopPenalty(DEFAULT_STOP_PENALTY),
    _travelDurationPenalty(DEFAULT_TRAV_DUR_PENALTY),
    _departureTimeDeviationPenalty(DEFAULT_DEP_TIME_DEV_PENALTY),
    _noOfESVLowFareSolutionsReq(DEFAULT_NUM_ESV_LOWFARE),
    _esvPercent(DEFAULT_MAX_ALLOWED_OVERAGE_PER_CARRIER_LFS),
    _noOfMinOnlinePerCarrier(DEFAULT_NUM_MIN_ONLINE_PER_CARRIER),
    _onlinePercent(DEFAULT_MIN_PERCENT_OF_ONLINE),
    _loMaximumPerOption(DEFAULT_FLIGHT_OPTION_REUSE_FOR_AVS),
    _hiMaximumPerOption(DEFAULT_FLIGHT_OPTION_REUSE_FOR_NONAVS)
{
  if (_requestedNumberOfSolutions == 0)
  {
    _requestedNumberOfSolutions = DEFAULT_NUM_OF_SOLUTIONS;
  }
}

ESVOptions::~ESVOptions() {}
