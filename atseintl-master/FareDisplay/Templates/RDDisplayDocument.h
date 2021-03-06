//-------------------------------------------------------------------
//
//  File:        RDDisplayDocument.h
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
class FareDispTemplateSeg;

class RDDisplayDocument : public Document
{
public:
  RDDisplayDocument(std::vector<FareDispTemplateSeg*>* templateSegRecs)
    : _templateSegRecs(templateSegRecs)
  {
  }

  std::vector<FareDispTemplateSeg*>* templateSegRecs() { return _templateSegRecs; }

private:
  std::vector<FareDispTemplateSeg*>* _templateSegRecs = nullptr;
};
} // namespace tse
