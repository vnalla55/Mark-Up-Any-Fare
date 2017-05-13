//-------------------------------------------------------------------
//
//  File:        Document.h
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

#include "DataModel/FareDisplayRequest.h"

#include <vector>

namespace tse
{
class Section;

class Document
{
public:
  Document() = default;
  virtual ~Document() = 0;

  Document(const Document&) = delete;
  Document& operator=(const Document&) = delete;

  virtual void buildDisplay();

  std::vector<Section*>& sections() { return _sections; }
  const std::vector<Section*>& sections() const { return _sections; }

private:
  std::vector<Section*> _sections;
};
inline Document::~Document() = default;
} // namespace tse
