//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class BaggageTravel;
class OCFees;
class OptionalServicesInfo;
class PaxTypeFare;
class SvcFeesCxrResultingFCLInfo;
class SvcFeesResBkgDesigInfo;
class TravelSeg;

class IS7RecordFieldsValidator
{
public:
  virtual ~IS7RecordFieldsValidator() {}

  virtual const TravelSeg* determineSpecialSegmentForT186(const BaggageTravel& bt) const = 0;

  virtual bool checkCabinInSegment(TravelSeg* segment, const Indicator cabin) const = 0;

  virtual bool checkRBDInSegment(TravelSeg* segment,
                                 OCFees& ocFees,
                                 uint32_t serviceFeesResBkgDesigTblItemNo,
                                 const std::vector<SvcFeesResBkgDesigInfo*>& rbdInfos) const = 0;

  virtual bool
  checkResultingFareClassInSegment(const PaxTypeFare* paxTypeFare,
                                   uint32_t serviceFeesCxrResultingFclTblItemNo,
                                   OCFees& ocFees,
                                   const std::vector<SvcFeesCxrResultingFCLInfo*>& resFCLInfo)
      const = 0;

  virtual bool checkOutputTicketDesignatorInSegment(TravelSeg* segment,
                                                    const PaxTypeFare* ptf,
                                                    const OptionalServicesInfo& s7) const = 0;

  virtual bool checkRuleInSegment(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                                  const RuleNumber& rule,
                                  OCFees& ocFees) const = 0;

  virtual bool checkRuleTariffInSegment(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                                        uint16_t ruleTariff,
                                        OCFees& ocFees) const = 0;

  virtual bool checkFareIndInSegment(const PaxTypeFare* paxTypeFareLinkedWithSegment,
                                     const Indicator& fareInd) const = 0;

  virtual bool checkCarrierFlightApplT186InSegment(const TravelSeg* segment,
                                                   const VendorCode& vendor,
                                                   uint32_t itemNo) const = 0;
};

} // tse namespace

