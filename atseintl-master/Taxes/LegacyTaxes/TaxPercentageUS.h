//----------------------------------------------------------------------------
//  File:           TaxPercentageUS.h
//  Description:    TaxPercentageUS header file for ATSE International Project
//  Created:        11/16/2007
//  Authors:        Piotr Lach
//
//  Description: This Object will be used for Tax Display functionality.
//
//  Updates:
//
//  Copyright Sabre 2007
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

#ifndef TAX_PERCENTAGE_US_H
#define TAX_PERCENTAGE_US_H

#include <utility>
#include <string>
#include "DataModel/TaxRequest.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseConsts.h"

namespace tse
{
class TaxTrx;
class TaxCodeReg;
class Loc;

class TaxPercentageUS
{
  friend class TaxPercentageUSTest;

public:
  typedef std::pair<MoneyAmount, // fare amount
                    MoneyAmount> // tax amount
      FareAndTax;
  typedef Indicator AmountType;
  typedef Indicator TripType;

  TaxPercentageUS(TaxRequest& r);
  virtual ~TaxPercentageUS() {}

  std::string display() { return _output; }
  void build();

  void taxFactorsBuild();
  void taxContinentalBuild();

  static bool validAmt(TaxRequest* req);
  static bool validConUSTrip(TaxRequest* req);
  static bool validHiAkConUSTrip(TaxRequest* req);

  TaxCode TAX_CODE_US1;
  TaxCode TAX_CODE_US2;
  static const unsigned int DISP_PRECISION = 2;
  static const AmountType TOTAL_FARE = 'T';
  static const AmountType BASE_FARE = 'B';
  static const TripType ONE_WAY_TRIP = 'X';
  static const TripType ROUND_TRIP = 'R';
  static const MoneyAmount MIN_AMOUNT;

private:
  TaxRequest& _request;
  std::string _output;

  FareAndTax calculateFareAndTax(const MoneyAmount& factor, const double& taxUS2 = 0);
  MoneyAmount locateHiAkFactor(TaxTrx& trx, const Loc* origin, const Loc* destination) const;
  bool validAmount() const;
  bool validConUSTrip() const;
  bool validHiAkConUSTrip() const;
  bool validHiAkConUSLoc(const Loc& origin, const Loc& destination) const;
  MoneyAmount taxRound(TaxTrx& trx, const TaxCodeReg& taxCodeReg, const MoneyAmount& taxAmt);
  std::string amtFmt(const MoneyAmount& amt, unsigned int precision = DISP_PRECISION);

  TaxPercentageUS(const TaxPercentageUS&);
  TaxPercentageUS& operator=(const TaxPercentageUS&);
};

} // namespace tse
#endif // TAX_PERCENTAGE_US_H
