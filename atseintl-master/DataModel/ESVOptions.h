//-------------------------------------------------------------------
//
//  File:        ESVptions.h
//  Created:     November 13, 2007
//  Authors:
//
//  Description: Options connected to ESV diversity logic
//
//  Updates:
//
//  Copyright Sabre 2007
//;
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

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TypeConvert.h"
#include "DataModel/PricingOptions.h"

#include <set>
#include <vector>

namespace tse
{

class ESVOptions : public PricingOptions
{
public:
  static const int16_t DEFAULT_NUM_OF_SOLUTIONS;
  static const int16_t DEFAULT_NUM_MUST_PRICE_ONLINE; // Q60
  static const int16_t DEFAULT_NUM_MUST_PRICE_INTERLINE; // Q61
  static const int16_t DEFAULT_NUM_MUST_PRICE_NONSTOP_ONLINE; // Q62
  static const int16_t DEFAULT_NUM_MUST_PRICE_NONSTOP_INTERLINE; // Q63
  static const int16_t DEFAULT_NUM_MUST_PRICE_ONESTOP_ONLINE; // Q64
  static const int16_t DEFAULT_NUM_MUST_PRICE_NONANDONE_ONLINE; // Q6Q
  static const int16_t DEFAULT_NUM_MUST_PRICE_NONANDONE_INTERLINE; // Q6R

  static const float DEFAULT_MAX_ALLOWED_OVERAGE_PER_CARRIER; // Q68
  static const int16_t DEFAULT_FLIGHT_OPTION_REUSE_LIMIT; // Q69
  static const float DEFAULT_UPPER_BOUND_FACTOR_LFS; // Q6U
  static const float DEFAULT_UPPER_BOUND_FACTOR_NONSTOP; // Q6S
  static const float DEFAULT_UPPER_BOUND_FACTOR_NOT_NONSTOP; // Q6A

  static const int16_t DEFAULT_STOP_PENALTY; // Q65
  static const int16_t DEFAULT_TRAV_DUR_PENALTY; // Q66
  static const int16_t DEFAULT_DEP_TIME_DEV_PENALTY; // Q67

  static const int16_t DEFAULT_NUM_ESV_LOWFARE; // Q0E
  static const float DEFAULT_MAX_ALLOWED_OVERAGE_PER_CARRIER_LFS; // Q5T
  static const int16_t DEFAULT_NUM_MIN_ONLINE_PER_CARRIER; // Q5U
  static const float DEFAULT_MIN_PERCENT_OF_ONLINE; // Q5V
  static const int16_t DEFAULT_FLIGHT_OPTION_REUSE_FOR_AVS; // Q5W
  static const int16_t DEFAULT_FLIGHT_OPTION_REUSE_FOR_NONAVS; // Q5X

private:
  // diversity options
  bool _mustPriceFlag; // P80
  int16_t _noOfMustPriceOnlineSolutions; // Q60
  int16_t _noOfMustPriceInterlineSolutions; // Q61
  int16_t _noOfMustPriceNonstopOnlineSolutions; // Q62
  int16_t _noOfMustPriceNonstopInterlineSolutions; // Q63
  int16_t _noOfMustPriceSingleStopOnlineSolutions; // Q64
  int16_t _noOfMustPriceNonAndOneStopOnlineSolutions; // Q6Q
  int16_t _noOfMustPriceNonAndOneStopInterlineSolutions; // Q6R

  float _percentFactor; // Q68
  int16_t _flightOptionReuseLimit; // Q69
  float _upperBoundFactorForNotNonstop; // Q6A
  float _upperBoundFactorForNonstop; // Q6S
  float _upperBoundFactorForLFS; // Q6U

  // penalties
  int16_t _perStopPenalty; // Q65
  int16_t _travelDurationPenalty; // Q66
  int16_t _departureTimeDeviationPenalty; // Q67

  // low fare options
  int16_t _noOfESVLowFareSolutionsReq; // Q0E
  float _esvPercent; // Q5T
  int16_t _noOfMinOnlinePerCarrier; // Q5U
  float _onlinePercent; // Q5V
  int16_t _loMaximumPerOption; // Q5W
  int16_t _hiMaximumPerOption; // Q5X
  std::string _avsCarriersString; // S5A

public:
  ESVOptions(const ESVOptions&);
  ESVOptions& operator=(const ESVOptions&);
  ESVOptions();
  virtual ~ESVOptions();

  //-------------------------------------------------------------------------
  // Accessors
  //-------------------------------------------------------------------------

  // Must Price  ------------------------------------------------------------
  // P80
  bool& mustPriceFlag() { return _mustPriceFlag; }
  const bool& mustPriceFlag() const { return _mustPriceFlag; }

  // Q60
  int16_t& noOfMustPriceOnlineSolutions() { return _noOfMustPriceOnlineSolutions; }
  const int16_t& noOfMustPriceOnlineSolutions() const { return _noOfMustPriceOnlineSolutions; }

  // Q61
  int16_t& noOfMustPriceInterlineSolutions() { return _noOfMustPriceInterlineSolutions; }
  const int16_t& noOfMustPriceInterlineSolutions() const
  {
    return _noOfMustPriceInterlineSolutions;
  }

  // Q62
  int16_t& noOfMustPriceNonstopOnlineSolutions() { return _noOfMustPriceNonstopOnlineSolutions; }
  const int16_t& noOfMustPriceNonstopOnlineSolutions() const
  {
    return _noOfMustPriceNonstopOnlineSolutions;
  }

  // Q63
  int16_t& noOfMustPriceNonstopInterlineSolutions()
  {
    return _noOfMustPriceNonstopInterlineSolutions;
  }
  const int16_t& noOfMustPriceNonstopInterlineSolutions() const
  {
    return _noOfMustPriceNonstopInterlineSolutions;
  }

  // Q64
  int16_t& noOfMustPriceSingleStopOnlineSolutions()
  {
    return _noOfMustPriceSingleStopOnlineSolutions;
  }
  const int16_t& noOfMustPriceSingleStopOnlineSolutions() const
  {
    return _noOfMustPriceSingleStopOnlineSolutions;
  }

  // Q6Q
  int16_t& noOfMustPriceNonAndOneStopOnlineSolutions()
  {
    return _noOfMustPriceNonAndOneStopOnlineSolutions;
  }
  const int16_t& noOfMustPriceNonAndOneStopOnlineSolutions() const
  {
    return _noOfMustPriceNonAndOneStopOnlineSolutions;
  }

  // Q6R
  int16_t& noOfMustPriceNonAndOneStopInterlineSolutions()
  {
    return _noOfMustPriceNonAndOneStopInterlineSolutions;
  }
  const int16_t& noOfMustPriceNonAndOneStopInterlineSolutions() const
  {
    return _noOfMustPriceNonAndOneStopInterlineSolutions;
  }

  // Q68
  float& percentFactor() { return _percentFactor; }
  const float& percentFactor() const { return _percentFactor; }

  // Q69
  int16_t& flightOptionReuseLimit() { return _flightOptionReuseLimit; }
  const int16_t& flightOptionReuseLimit() const { return _flightOptionReuseLimit; }

  // Q6A
  float& upperBoundFactorForNotNonstop() { return _upperBoundFactorForNotNonstop; }
  const float& upperBoundFactorForNotNonstop() const { return _upperBoundFactorForNotNonstop; }

  // Q6S
  float& upperBoundFactorForLFS() { return _upperBoundFactorForLFS; }
  const float& upperBoundFactorForLFS() const { return _upperBoundFactorForLFS; }

  // Q6U
  float& upperBoundFactorForNonstop() { return _upperBoundFactorForNonstop; }
  const float& upperBoundFactorForNonstop() const { return _upperBoundFactorForNonstop; }

  // Q65
  int16_t& perStopPenalty() { return _perStopPenalty; }
  const int16_t& perStopPenalty() const { return _perStopPenalty; }

  // Q66
  int16_t& travelDurationPenalty() { return _travelDurationPenalty; }
  const int16_t& travelDurationPenalty() const { return _travelDurationPenalty; }

  // Q67
  int16_t& departureTimeDeviationPenalty() { return _departureTimeDeviationPenalty; }
  const int16_t& departureTimeDeviationPenalty() const { return _departureTimeDeviationPenalty; }

  // LFS ---------------------------------------------------------------------
  // Q0E
  int16_t& noOfESVLowFareSolutionsReq() { return _noOfESVLowFareSolutionsReq; }
  const int16_t& noOfESVLowFareSolutionsReq() const { return _noOfESVLowFareSolutionsReq; }

  // Q5T
  float& esvPercent() { return _esvPercent; }
  const float& esvPercent() const { return _esvPercent; }

  // Q5U
  int16_t& noOfMinOnlinePerCarrier() { return _noOfMinOnlinePerCarrier; }
  const int16_t& noOfMinOnlinePerCarrier() const { return _noOfMinOnlinePerCarrier; }

  // Q5V
  float& onlinePercent() { return _onlinePercent; }
  const float& onlinePercent() const { return _onlinePercent; }

  // Q5W
  int16_t& loMaximumPerOption() { return _loMaximumPerOption; }
  const int16_t& loMaximumPerOption() const { return _loMaximumPerOption; }

  // Q5X
  int16_t& hiMaximumPerOption() { return _hiMaximumPerOption; }
  const int16_t& hiMaximumPerOption() const { return _hiMaximumPerOption; }

  // S5A
  std::string& avsCarriersString() { return _avsCarriersString; }
  const std::string& avsCarriersString() const { return _avsCarriersString; }
};
} // tse namespace

