//----------------------------------------------------------------------------
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

#include "MinFares/MinimumFare.h"

namespace tse
{
class PricingTrx;
class FarePath;
class FareUsage;
class Itin;
class Loc;

/**
 * @class EPQMinimumFare
 * This class will process the EPQ(Exception Provision Qualification) to
 * defines minimum fares processing as it pertain to qualified and non-qualified
 * one-way journey type.
*/
class EPQMinimumFare : public MinimumFare
{
  friend class EPQMinimumFareTest;

public:
  EPQMinimumFare();
  virtual ~EPQMinimumFare();

  bool process(PricingTrx& trx, const FarePath& farePath);

  // Prevalidation called from Itinalyzer for simple trip
  bool process(PricingTrx& trx, Itin& itin);

private:
  bool isSITI(const Itin& itin);
  bool isSIUsTerritories(const Loc& saleLoc);
  bool isSIAndOriginInArea1(const Loc& saleLoc, const Loc& orig);
  bool isOriginInWesternAfrica(const Loc& saleLoc, const Loc& orig);
  bool hasHip(const FareUsage& fu);
  void displayFarePath(const FarePath& farePath, DiagCollector& diag);
};
} // namespace tse
