//-------------------------------------------------------------------
//
//  File:	CategoryRuleItemInfoSet.h
//  Authors:	Piotr Bartosik
//
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

#include <vector>

namespace tse
{

class CategoryRuleItemInfo;
class CombinabilityRuleItemInfo;

template <class T> class CategoryRuleInfoT;
typedef CategoryRuleInfoT<CategoryRuleItemInfo> CategoryRuleInfo;
class CombinabilityRuleInfo;

template <class T> using CategoryRuleItemInfoSetT = std::vector<T>;
typedef CategoryRuleItemInfoSetT<CategoryRuleItemInfo> CategoryRuleItemInfoSet;
typedef CategoryRuleItemInfoSetT<CombinabilityRuleItemInfo> CombinabilityRuleItemInfoSet;

} // tse namespace
