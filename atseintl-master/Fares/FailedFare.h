//--------------------------------------------------------------------
//
//  File:        FailedFare.h
//
//  Copyright Sabre 2012
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//--------------------------------------------------------------------

#pragma once

#include <string>

namespace tse
{
class DiagCollector;
class PricingTrx;
class PaxTypeFare;

class FailedFare final
{
public:
  FailedFare(PricingTrx& trx, DiagCollector& diag, bool specialRtgFound);

  bool operator()(const PaxTypeFare* ptFare);

  bool checkFare(const PaxTypeFare* ptFare);

  std::string toString(const PaxTypeFare& paxFare);

private:
  bool isValidForCategory35(const PaxTypeFare* ptFare) const;

  bool isNotSpecialRoutingAndDataMissing(const PaxTypeFare* ptFare) const;

  bool isValidForOtherCats(const PaxTypeFare* ptFare);

  PricingTrx& _trx;
  DiagCollector& _diag;
  bool _specialRtgFound;
  bool _diagEnabled = false;
};
}

