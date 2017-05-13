// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <vector>
#include "Common/TaxName.h"
#include "Rules/LimitGroup.h"
#include "Rules/SpecialTrips.h"
#include "Rules/TaxLimitInfo.h"

namespace tax
{
typedef std::vector<type::TaxApplicationLimit> TaxApplicationLimits;
typedef std::vector<type::MoneyAmount> MoneyAmounts;
typedef std::vector<const BusinessRule*> BusinessRules;

bool
isLimitOnly(TaxLimitInfoIter first, TaxLimitInfoIter last,
    const type::TaxApplicationLimit& limitType);

bool
isLimit(TaxLimitInfoIter first, TaxLimitInfoIter last,
    const type::TaxApplicationLimit& limitType);

bool
isLimitOnly(TaxLimitInfoIter first, TaxLimitInfoIter last,
    const type::TaxApplicationLimit& firstLimitType,
    const type::TaxApplicationLimit& secondLimitType);

std::vector<type::Index>
findHighest(TaxLimitInfoIter first, TaxLimitInfoIter last);

std::vector<type::Index>
findHighestForOnceForItin(TaxLimitInfoIter first, TaxLimitInfoIter last,
    const TaxPointMap& taxPointMap,
    const std::vector<TaxLimitInfoIter>& blanks);

std::vector<type::Index>
findHighestAndBlank(TaxLimitInfoIter first, TaxLimitInfoIter last);

std::vector<type::Index>
findFirst(TaxLimitInfoIter first, TaxLimitInfoIter last);

std::vector<type::Index>
findFirstTwo(TaxLimitInfoIter first, TaxLimitInfoIter last);

std::vector<type::Index>
findFirstTwoAndBlank(TaxLimitInfoIter first, TaxLimitInfoIter last);

std::vector<type::Index>
findFirstFour(TaxLimitInfoIter first, TaxLimitInfoIter last);

std::vector<type::Index>
findFirstFourAndBlank(TaxLimitInfoIter first, TaxLimitInfoIter last);

std::vector<TaxLimitInfoIter>
findAllBlank(TaxLimitInfoIter first, TaxLimitInfoIter last);

TaxLimitInfoIter
findBegin(TaxLimitInfoIter first, TaxLimitInfoIter last, type::Index value);

TaxLimitInfoIter
findEnd(TaxLimitInfoIter first, TaxLimitInfoIter last, type::Index value);
}
