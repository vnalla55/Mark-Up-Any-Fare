//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"

#include <memory>

namespace tse
{
class DiagCollector;
class FareMarket;
class FarePath;
class FareUsage;
class Itin;
class PaxTypeFare;
class PricingTrx;

class Cat31FareBookingCodeValidator
{
public:
  enum Result
  { FAILED,
    PASSED,
    SKIPPED,
    POSTPONED_TO_PHASE2 };

  Cat31FareBookingCodeValidator(PricingTrx& trx, const FareMarket& mkt, Itin& itin);
  ~Cat31FareBookingCodeValidator();

  bool isActive() const;

  Result validateCat31(const PaxTypeFare& ptf,
                       const FarePath* farePath = nullptr,
                       DiagCollector* diag = nullptr) const;

  bool validateCat33(const PaxTypeFare& ptf, DiagCollector* diag = nullptr) const;

  void printDiagHeader(DiagCollector& dc, const FareUsage* fareUsage) const;

  void printResultDiag(DiagCollector& dc, const PaxTypeFare& paxTFare, Result result) const;

  class FareAndCabin;

private:
  Result validateSameFareBreak(const PaxTypeFare& ptf,
                               const FareAndCabin& prevFare,
                               DiagCollector* diag) const;

  Result validateUsCa(const PaxTypeFare& ptf, DiagCollector* diag) const;

  Result validateInternationalNormal(const PaxTypeFare& ptf, DiagCollector* diag) const;

  Result validateInternationalSpecial(const PaxTypeFare& ptf, DiagCollector* diag) const;

  void printFareDiag(DiagCollector& dc, const FareUsage& fareUsage) const;

private:
  const PricingTrx& _trx;
  const FareMarket& _mkt;
  const Itin& _itin;
  bool _partialFullFlown;
  bool _onlyOneFlownSeg;
  std::unique_ptr<FareAndCabin> _prevCat31Fare;
  std::vector<FareAndCabin> _prevFares;
  std::vector<const PaxTypeFare*> _rbdHierarchy;
  bool _cat31PrevFareNormal;
};

} // tse
