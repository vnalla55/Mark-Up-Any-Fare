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

#include "TaxDisplay/Common/DetailLevelsSequencer.h"
#include "TaxDisplay/Response/ResponseFormatter.h"

#include <functional>
#include <vector>

namespace tax
{
namespace display
{

class Entry
{
public:
  virtual ~Entry() {}

  virtual bool buildHeader() { return true; }
  virtual bool buildBody()   { return true; }
  virtual bool buildFooter() { return true; }

protected:
  explicit Entry(ResponseFormatter& formatter) : _formatter(formatter) {}
  Entry(ResponseFormatter& formatter, const DetailLevels& detailLevels)
    : _formatter(formatter), _detailLevelSequencer(detailLevels) {}

  ResponseFormatter& _formatter;
  DetailLevelsSequencer _detailLevelSequencer;
};

} /* namespace display */
} /* namespace tax */
