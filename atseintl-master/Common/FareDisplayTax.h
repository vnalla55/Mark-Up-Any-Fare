//----------------------------------------------------------------------------
//
//  File:            FareDisplayTax.h
//  Created:     12/21/2005
//  Authors:     Partha Kumar Chakraborti
//
//  Description:    Common functions required for Tax Calculation of ATSE fare display
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class FareDisplayTrx;
class FareMarket;
class FarePath;
class PaxTypeFare;

namespace FareDisplayTax
{
extern const std::string TAX_CODE_US1;
extern const std::string TAX_CODE_US2;
extern const std::string TAX_CODE_YZ1;
extern const std::string TAX_CODE_UO2;
extern const std::string TAX_CODE_NZ1;

void
getTaxCodeForFQ(const FareDisplayTrx& trx,
                std::set<TaxCode>& taxCode,
                const GlobalDirection globalDirection,
                const FareMarket* fareMarket = nullptr);

void
getTotalTax(const FareDisplayTrx& trx,
            const PaxTypeFare& paxTypeFare,
            MoneyAmount& owTaxAmount,
            MoneyAmount& rtTaxAmount);

bool
getOWTax(const FareDisplayTrx& trx,
         const PaxTypeFare& paxTypeFare,
         const TaxCode& taxCode,
         MoneyAmount& owTaxAmount,
         bool overrideFareOWRTCheck = false);

bool
getRTTax(const FareDisplayTrx& trx,
         const PaxTypeFare& paxTypeFare,
         const TaxCode& taxCode,
         MoneyAmount& rtTaxAmount);

bool
getTotalOWTax(const FareDisplayTrx& trx, const PaxTypeFare& paxTypeFare, MoneyAmount& owTaxAmount);

bool
getTotalRTTax(const FareDisplayTrx& trx, const PaxTypeFare& paxTypeFare, MoneyAmount& rtTaxAmount);

bool
shouldCalculateTax(FareDisplayTrx& trx, const FareMarket* fareMarket = nullptr);

bool
shouldBypassTax(const TaxCode& taxCode);

bool
initializeFarePath(const FareDisplayTrx& trx, FarePath* farePath);

bool
populateFarePath(const FareDisplayTrx& trx, const PaxTypeFare* paxTypefare, FarePath* fp);

inline bool
isSupplementalFee(const long& specialProcessNo)
{
  return specialProcessNo == 64;
}
}
} // namespace

