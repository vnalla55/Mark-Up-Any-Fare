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

#include "Common/TNBrands/ItinGeometryCalculator.h"

namespace tse
{

namespace skipper
{

// Code readability hack
template <typename ContainerT> bool allAllowed(const ContainerT& container)
{
  return container.empty();
}

bool BrandInItinInclusionPolicy::isBrandCodeInItin(
    const BrandCode& brandCode, const BrandProgramRelations& relations,
    const BrandFilterMap& brandsAllowedForItin) const
{
  if (allAllowed(brandsAllowedForItin))
  {
    // If the filtering map is empty, it means no limitations.
    return true;
  }

  BrandFilterMap::const_iterator found = brandsAllowedForItin.find(brandCode);

  // We discard brands not present in the filtering map.
  if (found == brandsAllowedForItin.end())
  {
    return false;
  }

  // If brand alone is inserted into the filtering map
  // (no programs), we pass the brand.
  const std::set<ProgramID>& programIds = found->second;
  if (allAllowed(programIds))
  {
    return true;
  }

  // If any of specified programs match the brand, we pass it.
  for (const ProgramID& programId : programIds)
  {
    if (relations.isBrandCodeInProgram(brandCode, std::string(programId)))
    {
      return true;
    }
  }
  return false;
}

QualifiedBrandIndices BrandInItinInclusionPolicy::getIndicesForBrandCodeWithFiltering(
    const BrandCode& brandCode, const BrandProgramRelations& relations,
    const BrandFilterMap& brandsAllowedForItin) const
{
  ProgramIds programs = relations.getProgramsForBrand(brandCode);
  QualifiedBrandIndices qbIndices;

  if (allAllowed(brandsAllowedForItin))
  {
    // If the filtering map is empty, it means no limitations, all programs
    // allowed.
    for (const ProgramID& programId : programs)
    {
      qbIndices.insert(relations.getQualifiedBrandIndexForBrandProgramPair(brandCode, programId));
    }
    return qbIndices;
  }

  BrandFilterMap::const_iterator found = brandsAllowedForItin.find(brandCode);

  // We discard brands not present in the filtering map. No programs allowed
  // for this brand
  if (found == brandsAllowedForItin.end())
  {
    return qbIndices;
  }

  // If brand alone is inserted into the filtering map (no programs)
  // all programs are allowed.
  const std::set<ProgramID>& filteredProgramIds = found->second;
  if (allAllowed(filteredProgramIds))
  {
    for (const ProgramID& programId : programs)
    {
      qbIndices.insert(relations.getQualifiedBrandIndexForBrandProgramPair(brandCode, programId));
    }
    return qbIndices;
  }

  // Otherwise intersect programs for this brand and programs from filter
  ProgramIds intersection;
  std::set_intersection(
      filteredProgramIds.begin(),
      filteredProgramIds.end(),
      programs.begin(),
      programs.end(),
      std::inserter(intersection, intersection.begin()));
  for (const ProgramID& programId : intersection)
  {
    qbIndices.insert(relations.getQualifiedBrandIndexForBrandProgramPair(brandCode, programId));
  }
  return qbIndices;
}


} /* namespace skipper */

} // namespace tse
