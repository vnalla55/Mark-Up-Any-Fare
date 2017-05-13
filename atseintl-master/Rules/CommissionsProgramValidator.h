//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"

namespace tse
{
class CommissionProgramInfo;
class Diag867Collector;
class FarePath;
class FareUsage;
class Loc;
class LocKey;
class Logger;
class PricingTrx;

class CommissionsProgramValidator
{
  friend class CommissionsProgramValidatorTest;

public:
  CommissionsProgramValidator(PricingTrx& trx,
                              const FarePath& fp,
                              const FareUsage& fu,
                              Diag867Collector* diag867)
    : _trx(trx), _fp(fp), _fu(fu), _diag867(diag867)
  {
  }

  bool isCommissionProgramMatched(const CommissionProgramInfo& commissionProgram,
                                  const CarrierCode& valCxr);
  virtual ~CommissionsProgramValidator() = default;

protected:

  CommissionValidationStatus
  validateCommissionProgram(const CommissionProgramInfo& cpi, const CarrierCode& valCxr);

  bool matchPointOfSale(const CommissionProgramInfo& cri,
                        const CarrierCode& valCxr);

  bool matchPointOfOrigin(const CommissionProgramInfo& cri,
                          const CarrierCode& valCxr);

  bool matchMarket(const CommissionProgramInfo& cri,
                   const CarrierCode& valCxr);
  bool matchTravelDates(const CommissionProgramInfo& cri) const;
  bool matchTicketingDates(const CommissionProgramInfo& cri) const;

  void printCommissionProgramProcess(const CommissionProgramInfo& cri,
                                     CommissionValidationStatus rc) const;
  bool isDiagProgramMatch(const CommissionProgramInfo& cri) const;


  bool validateLocation(const CommissionProgramInfo& cpi,
                        const VendorCode& vendor,
                        const LocKey& locKey,
                        const Loc& loc,
                        const CarrierCode& carrier);

  virtual bool isInZone(const VendorCode& vendor,
                        const LocCode& zone,
                        const Loc& loc,
                        const CarrierCode& carrier);

  virtual bool isInLoc(const VendorCode& vendor,
                       const LocKey& locKey,
                       const Loc& loc,
                       const CarrierCode& carrier);

  bool checkIsDateBetween(DateTime startDate,
                          DateTime endDate,
                          DateTime& betweenDate) const;

  bool notValidToValidate();

  PricingTrx& _trx;
  const FarePath& _fp;
  const FareUsage& _fu;
  Diag867Collector* _diag867;
  static Logger _logger;

private:
  CommissionsProgramValidator(const CommissionsProgramValidator& rhs);
  CommissionsProgramValidator& operator=(const CommissionsProgramValidator& rhs);

};
} // tse namespace

