//-------------------------------------------------------------------
//
//  File:        FQDisplayDocument.h
//  Created:     July 24, 2005
//  Authors:     Mike Carroll
//
//  Updates:
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "FareDisplay/Templates/Document.h"

#include <vector>

namespace tse
{
class FareDisplayPref;
class FareDispTemplateSeg;

class FQDisplayDocument : public Document
{
public:
  FQDisplayDocument(const FareDisplayPref* prefs,
                    std::vector<FareDispTemplateSeg*>* templateSegRecs)
    : _prefs(prefs), _templateSegRecs(templateSegRecs)
  {
  }

  const FareDisplayPref& prefs() const { return *_prefs; }

  std::vector<FareDispTemplateSeg*>* templateSegRecs() { return _templateSegRecs; }

private:
  const FareDisplayPref* _prefs = nullptr;
  std::vector<FareDispTemplateSeg*>* _templateSegRecs = nullptr;
};
} // namespace tse
