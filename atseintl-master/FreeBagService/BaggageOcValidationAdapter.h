//-------------------------------------------------------------------
//  Copyright Sabre 2010
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
class BaggageTravel;
class BagValidationOpt;
class Diag877Collector;
class OCFees;
class SubCodeInfo;

namespace BaggageOcValidationAdapter
{
std::vector<OCFees*>
matchS7AllowanceSkipFareChecks(const BagValidationOpt& opt,
                               const SubCodeInfo& s5,
                               uint32_t startSeqNo = 0);
std::vector<BaggageCharge*>
matchS7ChargeSkipFareChecks(const BagValidationOpt& opt,
                            const SubCodeInfo& s5,
                            uint32_t freePieces);
OCFees&
matchS7AllowanceRecord(const SubCodeInfo& subCodeInfo,
                       BaggageTravel& bt,
                       const CheckedPoint& fcp,
                       Diag877Collector* diag,
                       bool allowanceCarrierOverridden = false);

OCFees&
matchS7FastForwardAllowanceRecord(const SubCodeInfo& subCodeInfo,
                                  BaggageTravel& bt,
                                  const CheckedPoint& fcp,
                                  uint32_t lastSequence,
                                  Diag877Collector* diag);

OCFees&
matchS7CarryOnAllowanceRecord(const SubCodeInfo& subCodeInfo,
                              BaggageTravel& bt,
                              const CheckedPoint& fcp,
                              Diag877Collector* diag);

void
matchS7ChargesRecords(const SubCodeInfo& subCodeInfo,
                      BaggageTravel& bt,
                      const CheckedPoint& fcp,
                      Diag877Collector* diag,
                      ChargeVector& matchedCharges);

void
matchS7AncillaryChargesRecords(const SubCodeInfo& subCodeInfo,
                               BaggageTravel& bt,
                               const CheckedPoint& fcp,
                               Diag877Collector* diag,
                               ChargeVector& matchedCharges);

void
matchS7CarryOnChargesRecords(const SubCodeInfo& subCodeInfo,
                             BaggageTravel& bt,
                             const CheckedPoint& fcp,
                             Diag877Collector* diag,
                             ChargeVector& matchedCharges);

OCFees&
matchS7EmbargoRecord(const SubCodeInfo& subCodeInfo,
                     BaggageTravel& bt,
                     const CheckedPoint& fcp,
                     Diag877Collector* diag);
}
} // tse
