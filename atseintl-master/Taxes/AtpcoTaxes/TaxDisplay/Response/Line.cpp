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

#include "TaxDisplay/Response/Line.h"

namespace tax
{
namespace display
{

Line::Line() {}
Line::Line(const std::string& str) : _str(str) {}
Line::Line(std::string&& str) : _str(str) {}
Line::Line(const LineParams& params) : _params(params) {}
Line::Line(const std::string& str, const LineParams& params) :
    _params(params),
    _str(str)
{
}

} /* namespace display */
} /* namespace tax */
