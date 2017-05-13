//-------------------------------------------------------------------
//
//  File:        TourCodeUtil.h
//  Created:     December 07, 2010
//  Authors:     Artur Krezel
//
//  Description:
//
//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <tuple>

namespace tse
{
class FarePath;
class PricingTrx;
class FareUsage;
class PaxTypeFare;
class PricingTrx;

class TourCodeUtil
{
  friend class TourCodeUtilTest;
  typedef std::tuple<std::string&, Indicator&, Indicator&> TourCodeTuple;

public:
  TourCodeUtil(const std::vector<const PaxTypeFare*>& paxTypeFares);
  virtual ~TourCodeUtil();

  bool validate(PricingTrx& trx);
  bool useOptimusNetRemit(PricingTrx& trx) const;

  static void saveTourCodeForNetRemitSMF(PricingTrx& trx, const FarePath& farePath);

private:
  bool hasTourCodeCombInd() const;
  std::vector<TourCodeUtil::TourCodeTuple>::iterator collectTourCodes(PricingTrx& trx);
  static bool matchTourCodeCombination(const TourCodeUtil::TourCodeTuple& first,
                                       const TourCodeUtil::TourCodeTuple& second);

  static bool getTourCodeInfo(PricingTrx& trx,
                              const FareUsage* fareUsage,
                              TourCodeUtil::TourCodeTuple& codesTuple);

  static bool getTourCodeInfo(PricingTrx& trx,
                              const PaxTypeFare* ptf,
                              TourCodeUtil::TourCodeTuple& codesTuple,
                              bool forcePassForNA);

  static bool createTourCodeForNetRemitSMF(PricingTrx& trx,
                                           const std::vector<const FareUsage*>& fareUsages,
                                           std::string& tourCode,
                                           Indicator& indTypeTC);
  // Data
private:
  const std::vector<const PaxTypeFare*>& _paxTypeFares;
  std::vector<std::string> _tourCodes;
  std::vector<Indicator> _tourCodeInds;
  std::vector<Indicator> _tourCodeTypes;
  std::vector<TourCodeTuple> _tourCodeTuples;
};
}

