//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//               Andrzej Fediuk
//
//  Copyright Sabre 2014
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


#include "Common/TNBrands/BrandProgramRelations.h"

#include "Common/Assert.h"

namespace tse {

namespace skipper
{

void BrandProgramRelations::addBrandProgramPair(
    const BrandCode& brandCode, const ProgramID& programId, int qualifiedBrandIndex)
{
  _programsPerCode[brandCode][programId] = qualifiedBrandIndex;
}

UnorderedBrandCodes BrandProgramRelations::getAllBrands() const
{
  UnorderedBrandCodes allBrands;
  for (const auto& elem : _programsPerCode)
  {
    allBrands.insert(elem.first);
  }
  return allBrands;
}

bool BrandProgramRelations::isBrandCodeInProgram(
    const BrandCode& brandCode, const ProgramID& programId) const
{
  TSE_ASSERT(_programsPerCode.find(brandCode) != _programsPerCode.end());

  const ProgramsQualifiedBrandsRelation& program = _programsPerCode.at(brandCode);
  for (const auto& elem : program)
  {
    if (elem.first == programId)
    {
      return true;
    }
  }
  return false;
}

const ProgramIds BrandProgramRelations::getProgramsForBrand(
    const BrandCode& brandCode) const
{
  ProgramIdsPerBrandCode::const_iterator found =
    _programsPerCode.find(brandCode);
  TSE_ASSERT(found != _programsPerCode.end());

  ProgramIds programs;
  for (const auto& elem : found->second)
  {
    programs.insert(elem.first);
  }
  return programs;
}

int BrandProgramRelations::getQualifiedBrandIndexForBrandProgramPair(
    const BrandCode& brandCode, const ProgramID& programId) const
{
  ProgramIdsPerBrandCode::const_iterator found =
      _programsPerCode.find(brandCode);
  TSE_ASSERT(found != _programsPerCode.end());

  ProgramsQualifiedBrandsRelation::const_iterator secondFound =
      found->second.find(programId);
  TSE_ASSERT(secondFound != found->second.end());

  return secondFound->second;
}


} /* namespace skipper */

} /* namespace tse */
