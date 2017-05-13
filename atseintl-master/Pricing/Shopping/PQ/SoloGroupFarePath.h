//-------------------------------------------------------------------
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
//-------------------------------------------------------------------

#pragma once

#include "Pricing/GroupFarePath.h"

namespace tse
{
namespace shpq
{
class SolutionPattern;

class SoloGroupFarePath : public GroupFarePath
{
public:
  const SolutionPattern* getSolutionPattern() const { return _solutionPattern; }
  void setSolutionPattern(const SolutionPattern* sp) { _solutionPattern = sp; }

  bool getProcessThruOnlyHint() const { return _processThruOnlyHint; }
  void setProcessThruOnlyHint(bool value) { _processThruOnlyHint = value; }

  /**
   * @return result != 0 only when processing diag 941, 942
   */
  size_t getFPKey() const { return _fpKey; }
  void setFPKey(size_t fpKey) { _fpKey = fpKey; }

private:
  const SolutionPattern* _solutionPattern = nullptr;
  bool _processThruOnlyHint = false;
  size_t _fpKey = 0; // for diag
};

} /* namespace shpq */
} /* namespace tse */

