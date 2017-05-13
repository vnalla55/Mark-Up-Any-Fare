//----------------------------------------------------------------------------
//
//  File:        TSEAlgorithms.h
//  Created:     03/01/2005
//  Authors:     Stephen Suggs
//
//  Description: Common algorithms for transaction processing
//
//  Updates:
//
//  Copyright Sabre 2005
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

#include "Common/Thread/TseThreadingConst.h"
#include "Common/TseStringTypes.h"

#include <boost/function.hpp>

namespace tse
{

class PricingTrx;
class Itin;
class FareMarket;

typedef boost::function<void(PricingTrx&, Itin&)> TrxItinFunctor;
typedef boost::function<void(PricingTrx&, Itin&, FareMarket&)> TrxItinFareMarketFunctor;

// The 'exec' funcions create a new thread for each fareMarket that invoke the given function

void
exec_foreach_fareMarket(TseThreadingConst::TaskId taskId,
                        PricingTrx& trx,
                        TrxItinFareMarketFunctor func);
void
exec_foreach_valid_fareMarket(TseThreadingConst::TaskId taskId,
                              PricingTrx& trx,
                              TrxItinFareMarketFunctor func,
                              uint8_t serviceBit = 0);
void
exec_foreach_valid_not_dummy_fareMarket(TseThreadingConst::TaskId taskId,
                                        PricingTrx& trx,
                                        TrxItinFareMarketFunctor func,
                                        uint8_t serviceBit = 0);

void
exec_foreach_valid_fareMarket(TseThreadingConst::TaskId taskId,
                              PricingTrx& trx,
                              Itin& itin,
                              TrxItinFareMarketFunctor func,
                              uint8_t serviceBit = 0);
void
exec_foreach_valid_common_fareMarket(TseThreadingConst::TaskId taskId,
                                     PricingTrx& trx,
                                     std::vector<FareMarket*>& fmv,
                                     TrxItinFareMarketFunctor func,
                                     uint8_t serviceBit = 0);

// The 'invoke' functions invoke the function directly

void
invoke_foreach_fareMarket(PricingTrx& trx, TrxItinFareMarketFunctor func);
void
invoke_foreach_valid_fareMarket(PricingTrx& trx,
                                TrxItinFareMarketFunctor func,
                                uint8_t serviceBit = 0);
void
invoke_foreach_valid_fareMarket(PricingTrx& trx,
                                Itin& itin,
                                TrxItinFareMarketFunctor func,
                                uint8_t serviceBit = 0);
}
