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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/TNBrandsTypes.h"

#include <map>
#include <set>
#include <string>

namespace tse
{

namespace skipper
{

// Maintains relations between brands and programs.
// A particular program may contain many brands.
// A particular brand may exist in many programs.
class BrandProgramRelations
{
public:

  // Add a relation telling that brandCode exists in program with programId and
  // is defined in qualifiedBrands vector at given index.
  void addBrandProgramPair(const BrandCode& brandCode,
                           const ProgramID& programId,
                           int qualifiedBrandIndex);

  // Returns brands from all programs. Only unique brandCodes
  // are returned: if a brand exists in multiple programs,
  // it will be returned once.
  UnorderedBrandCodes getAllBrands() const;

  // Tells if a particular brandCode exists in program with programId.
  // If brandCode does not exist, an exception is raised.
  // If programId does not exist (while brandCode does), false is returned.
  bool isBrandCodeInProgram(const BrandCode& brandCode,
                            const ProgramID& programId) const;

  // Returns set of program Ids defined for given brandCode.
  // If brandCode does not exist, an exception is raised.
  const ProgramIds getProgramsForBrand(const BrandCode& brandCode) const;

  // Returns index of brandCode/programId pair in qualifiedBrands vector.
  // If brandCode/programId does not exist an exception is raised.
  int getQualifiedBrandIndexForBrandProgramPair(
      const BrandCode& brandCode, const ProgramID& programId) const;

  bool operator==(const BrandProgramRelations& right) const
  {
    return (_programsPerCode == right._programsPerCode);
  }

private:
  typedef std::map<ProgramID, int> ProgramsQualifiedBrandsRelation;
  typedef std::map<BrandCode, ProgramsQualifiedBrandsRelation> ProgramIdsPerBrandCode;
  ProgramIdsPerBrandCode _programsPerCode;
};

} /* namespace skipper */

} /* namespace tse */

