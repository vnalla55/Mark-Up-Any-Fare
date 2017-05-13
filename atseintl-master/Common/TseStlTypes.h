//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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
#include "Common/TseStringTypes.h"

#include <map>
#include <set>
#include <utility>
#include <vector>

namespace tse
{
class BrandInfo;
class BrandProgram;
class ClassOfService;

typedef std::vector<NationCode> NationCodesVec;
typedef std::vector<TaxCode> TaxCodesVec;
typedef std::set<BrandCode> BrandCodeSet;
typedef std::vector<BrandCodeSet> BrandCodeSetVec;
typedef std::pair<uint32_t, uint32_t> IndexPair;
typedef std::vector<uint32_t> IndexVector;
typedef std::vector<IndexVector> IndexVectors;
typedef std::vector<IndexPair> IndexPairVector;
typedef std::vector<IndexPairVector> IndexPairVectors;
typedef std::vector<SopId> SopIdVec;
typedef std::map<LegId, IbfErrorMessage> IbfLegErrorMessageMap;
typedef std::map<BrandCode, IbfLegErrorMessageMap> IbfErrorMessageMapPerLeg;
typedef std::map<SettlementPlanType, std::vector<CarrierCode>> SettlementPlanValCxrsMap;
typedef std::pair<BrandProgram*, BrandInfo*> QualifiedBrand;
typedef IndexVector::iterator IndexVectorIterator;
typedef IndexVector::const_iterator IndexVectorConstIterator;
typedef IndexVectors::iterator IndexVectorsIterator;
typedef IndexVectors::const_iterator IndexVectorsConstIterator;
typedef IndexPairVector::iterator IndexPairVectorIterator;
typedef IndexPairVector::const_iterator IndexPairVectorConstIterator;
typedef IndexPairVectors::iterator IndexPairVectorsIterator;
typedef IndexPairVectors::const_iterator IndexPairVectorsConstIterator;
using ClassOfServiceList = std::vector<ClassOfService*>;
using AvailabilityMap = std::map<uint64_t, std::vector<ClassOfServiceList>*>;

}
