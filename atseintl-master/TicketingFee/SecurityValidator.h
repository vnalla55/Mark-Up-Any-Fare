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

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"


#include <vector>

namespace tse
{
class PricingTrx;
class SvcFeesSecurityInfo;
class Loc;
class SvcFeesDiagCollector;
class TravelSeg;

class SecurityValidator
{
  friend class SecurityValidatorTest;
  friend class BaggageSecurityValidatorTest;

public:
  SecurityValidator(PricingTrx& trx,
                    const std::vector<TravelSeg*>::const_iterator segI,
                    const std::vector<TravelSeg*>::const_iterator segIE);
  virtual ~SecurityValidator();

  bool validate(int s4SeqNo,
                int t183ItemNo,
                bool& view,
                VendorCode vc = ATPCO_VENDOR_CODE,
                SvcFeesDiagCollector* dc = nullptr);

protected:
  virtual const std::vector<SvcFeesSecurityInfo*>& getSecurityInfo(VendorCode vc, int t183ItemNo);
  virtual StatusT183Security validateSequence(const SvcFeesSecurityInfo* secInfo, bool& view) const;
  bool checkTvlAgency(const SvcFeesSecurityInfo* secInfo) const;
  virtual bool checkGds(const SvcFeesSecurityInfo* secInfo) const;
  virtual bool checkDutyCode(const SvcFeesSecurityInfo* secInfo) const;
  bool checkLoc(const SvcFeesSecurityInfo* secInfo) const;
  bool checkCode(const SvcFeesSecurityInfo* secInfo) const;
  virtual const Loc* getLocation() const;
  virtual const Loc* locationOverride(const LocCode& loc) const;
  virtual bool isInLoc(const SvcFeesSecurityInfo* secInfo, const Loc& loc) const;

  void createDiag(int s4SeqNo, int t183ItemNo);
  bool matchFareMarketInRequest() const;
  void detailDiag(const SvcFeesSecurityInfo* feeSec, const StatusT183Security status);
  void endDiag();
  void emptyMsg(VendorCode vc);

  virtual bool shouldCreateDiag() const;
  virtual bool shouldCollectDiag() const;
  virtual bool processAirlineSpecificX(const SvcFeesSecurityInfo* secInfo) const;

  static constexpr Indicator MUST_BE_TVL_AGENCY = 'X';
  const static LocTypeCode AIRLINE_SPECIFIC_X = 'X';
  const static LocTypeCode AIRLINE_SPECIFIC_V = 'V';
  const static LocTypeCode AIRLINE_SPECIFIC_A = 'A';
  const static LocTypeCode ERSP_NUMBER = 'E';
  const static LocTypeCode LNIATA_NUMBER = 'L';

  PricingTrx& _trx;
  SvcFeesDiagCollector* _diag;
  const std::vector<TravelSeg*>::const_iterator _segI;
  const std::vector<TravelSeg*>::const_iterator _segIE;

private:
  SecurityValidator(const SecurityValidator& rhs);
  SecurityValidator& operator=(const SecurityValidator& rhs);
};
} // tse namespace

