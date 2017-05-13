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

#include "TaxDisplay/Response/LineParams.h"

namespace tax
{
namespace display
{

LineParams LineParams::withLeftMargin(int8_t margin)
{
  LineParams params;
  params.setLeftMargin(margin);
  return params;
}

LineParams LineParams::withTruncateFlag()
{
  LineParams params;
  params.longLineFormatting() = LineParams::LongLineFormatting::TRUNCATE;
  return params;
}

bool LineParams::operator==(const LineParams& rhs) const
{
  return _leftMargin == rhs._leftMargin &&
         _isUserDefined == rhs._isUserDefined &&
         _longLineFormatting == rhs._longLineFormatting;
}

} /* namespace display */
} /* namespace tax */
