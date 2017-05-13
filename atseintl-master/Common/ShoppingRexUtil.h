//-------------------------------------------------------------------
//
//  File:        ShoppingRexUtil.h
//  Created:     March 17, 2009
//  Authors:     Miroslaw Bartyna
//
//  Description: Reissue existing ticket
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/RexShoppingTrx.h"

#include <vector>

namespace tse
{
class DiagCollector;

using FareByteCxrApplVect = RexShoppingTrx::FareByteCxrApplVect;
using PortionMergeTvlVectType = RexShoppingTrx::PortionMergeTvlVectType;
using FareByteCxrAppl = RexShoppingTrx::FareByteCxrAppl;

class ShoppingRexUtil
{
public: 
  ShoppingRexUtil() = delete;
  
  static void
  mergeFareByteCxrApplRestrictions(const FareByteCxrApplVect& fareByteCxrApplVect,
                                   FareByteCxrAppl& fareByteCxrAppl,
                                   DiagCollector* dc = nullptr);
  static void
  mergeRestrictedCrxAppl(const FareByteCxrApplVect& fareByteCxrApplVect,
                         std::set<CarrierCode>& mergedRestrictedCxrAppl);
  static void
  mergeApplicableCrxAppl(const FareByteCxrApplVect& fareByteCxrApplVect,
                         std::set<CarrierCode>& mergedApplicableCxrAppl);
  static FareByteCxrApplVect::const_iterator
  findFirstNotEmptyCxr(const FareByteCxrApplVect& fareByteCxrApplVect);  
  
  static void
  printCxr(const std::set<CarrierCode>& mergedCxrAppl, DiagCollector* dc);

  static void
  mergePortion(const std::vector<PortionMergeTvlVectType>& restrictions,
               PortionMergeTvlVectType& mergedRestrictions,
               DiagCollector* dc = nullptr);
};
} //tse

