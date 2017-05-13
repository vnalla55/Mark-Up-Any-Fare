//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/RexShoppingTrx.h"

namespace tse {

class FarePath;
class PricingUnit;
class FareMarket;
class Itin;

class Cat31RestrictionMerger
{
public:
  using OADResponseDataMap = RexShoppingTrx::OADResponseDataMap;
  using R3SeqsConstraintMap = RexShoppingTrx::R3SeqsConstraintMap;
  using OADResponseData = RexShoppingTrx::OADResponseData;
  using OADDataPair = RexShoppingTrx::OADDataPair;
  using R3SeqsConstraintVec = std::vector<const RexShoppingTrx::R3SeqsConstraint*>;

  Cat31RestrictionMerger(OADResponseDataMap& oadMergedR3Data, bool oneCarrierTicket,
                         const Itin& itin, RexShoppingTrx& trx)
    : _oadResponseData(oadMergedR3Data), _oneCarrierTicket(oneCarrierTicket), _itin(itin), _trx(trx)
  {
  }

  void operator_impl_OLD(const OADDataPair& in);
  void operator_impl(const OADDataPair& in);
  void operator()(const OADDataPair& in);

  void mergeForcedConnection(const R3SeqsConstraintVec& rec3Constraint,
                             RexShoppingTrx::OADResponseData& oadMergedResponse);
  void mergeFirstBreakRestr(const R3SeqsConstraintVec& rec3Constraint,
                            RexShoppingTrx::OADResponseData& oadMergedResponse);
  void mergePortion(const R3SeqsConstraintVec& rec3Constraint,
                    RexShoppingTrx::OADResponseData& oadMergedResponse);
  void mergeFlightNumber(const R3SeqsConstraintVec& rec3Constraint,
                         RexShoppingTrx::OADResponseData& oadMergedResponse);
  void mergeFareByteCxrAppl(const R3SeqsConstraintVec& rec3Constraint,
                            RexShoppingTrx::OADResponseData& oadMergedResponse);
  void mergeOutboundPortion(const R3SeqsConstraintVec& rec3Constraint,
                            RexShoppingTrx::OADResponseData& oadMergedResponse);
  void mergeChangeInd(const R3SeqsConstraintVec& rec3Constraint,
                      OADResponseData& oadMergedResponse,
                      const FareMarket& fareMarket);
  void mergeCalendarRange(const std::vector<R3SeqsConstraintVec>& rec3Constraints,
                          std::vector<RexShoppingTrx::OADResponseData>& oadMergedResponseVec);

private:
  struct ConstraintsBucket
  {
    R3SeqsConstraintVec wholePeriod;
    R3SeqsConstraintVec sameDate;
    R3SeqsConstraintVec laterDate;
  };

  enum ChangeInd : unsigned
  {
    NO_RESTR = 0x000,
    FC_RESTR = 0x001,
    PU_RESTR = 0x011,
    J_RESTR  = 0x111
  };

  ChangeInd convert(Indicator changeInd) const;
  const PricingUnit* getPricingUnit(const FarePath& fp, const FareMarket& fareMarket) const;

  OADResponseDataMap& _oadResponseData;
  bool _oneCarrierTicket = false;
  const Itin& _itin;
  RexShoppingTrx& _trx;

protected:
  std::vector<std::vector<const RexShoppingTrx::R3SeqsConstraint*>>
  divideConstraintsToCorrectRanges(const RexShoppingTrx::R3SeqsConstraintMap& oadData);
};


} // tse
