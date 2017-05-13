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

#include "TaxDisplay/Response/ResponseFormatter.h"

#include "TaxDisplay/Response/LineParams.h"

#include <boost/algorithm/string.hpp>
#include <boost/next_prior.hpp>

namespace tax
{
namespace display
{
namespace
{
std::string getIndentStr(const LineParams& params)
{
  int8_t indent = params.getLeftMargin();
  if (params.isUserDefined())
    indent += params.getParagraphIndent();

  if (indent <= 0)
    return std::string();

  return std::string(indent, ' ');
}
} // anonymous namespace


const unsigned short ResponseFormatter::DISPLAY_WIDTH = 64;

void ResponseFormatter::format(std::ostringstream& response)
{
  for(LinesList::iterator lineIt = _linesList.begin();
      lineIt != _linesList.end();
      ++lineIt)
  {
    std::string formatted;
    formatLine(lineIt, formatted);
    response << formatted << std::endl;
  }

  _linesList.clear(); // there's no need no more to keep this
}

void ResponseFormatter::formatLine(LinesList::iterator lineIt,
                                   std::string& formatted)
{
  std::string& lineString = lineIt->str();
  formatted.swap(lineString);
  boost::trim(formatted);
  formatted.insert(0, getIndentStr(lineIt->params()));

  if (formatted.size() > DISPLAY_WIDTH)
  {
    switch (lineIt->params().longLineFormatting())
    {
    case LineParams::LongLineFormatting::BREAK:
    {
      std::string::size_type lastSpacePos = formatted.rfind(' ', DISPLAY_WIDTH);

      LinesList::iterator nextLineIt = boost::next(lineIt);
      if (nextLineIt == _linesList.end() ||
          nextLineIt->params() != lineIt->params() ||  // next line arguments must be the same
          nextLineIt->params().isUserDefined()) // do not touch user-defined lines!
      {
        nextLineIt = addLineBefore(nextLineIt, lineIt->params());
        nextLineIt->params().isUserDefined() = false;
      }

      std::string::size_type strToMoveSize = formatted.size() - lastSpacePos;
      nextLineIt->str().insert(0, formatted.substr(lastSpacePos, strToMoveSize) + " ");
      formatted.erase(lastSpacePos, strToMoveSize);
    }
      break;
    case LineParams::LongLineFormatting::TRUNCATE:
      formatted.resize(DISPLAY_WIDTH);
      break;
    } // switch
  } // if
}

} /* namespace display */
} /* namespace tax */
