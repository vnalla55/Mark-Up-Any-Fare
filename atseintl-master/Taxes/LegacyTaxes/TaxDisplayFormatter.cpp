// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2007
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
#include "Taxes/LegacyTaxes/TaxDisplayFormatter.h"

using namespace tse;
using namespace std;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxDisplayFormatter::TaxDisplayFormatter
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

TaxDisplayFormatter::TaxDisplayFormatter(size_t lineLength)
  : _lineLength(lineLength), _isOffset(true), _offsetWidth(0)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxDisplayFormatter::~TaxDisplayFormatter
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------
TaxDisplayFormatter::~TaxDisplayFormatter() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxDisplayFormatter::format
//
// Description:  Format input string
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxDisplayFormatter::format(std::string& buff)
{
  std::string out;
  std::string line;
  std::string offset;
  size_t width;

  if (isOffset())
    offset = getOffset(buff);

  if (_lineLength - offset.size() >= 0)
    width = _lineLength - offset.size();
  else
    return;

  bool isFirstLine = true; // first line or line after '\n'

  while (buff.size() > 0)
  {

    if (isFirstLine)
      line = getLine(buff, offset, width + offset.size());
    else
      line = getLine(buff, offset, width);

    buff.erase(0, line.size()); // rem line from input string

    if (line.rfind('\n', line.size()) == std::string::npos)
    {
      if (isFirstLine)
      {
        out += line + "\n";
      }
      else
      {
        out += offset + line + "\n";
      }
      isFirstLine = false;
    }
    else
    {
      if (isFirstLine)
        out += line;
      else
        out += offset + line;

      // set new offset
      if (isOffset())
        offset = getOffset(buff);

      isFirstLine = true;
    }
  }

  buff = out;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxDisplayFormatter::getLine
//
// Description:  Get line from buff string
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
TaxDisplayFormatter::getLine(std::string& buff, const std::string& offset, const size_t width)
{
  std::string line;

  size_t wordEnd = findEndOfWord(buff, width); // calculate last word pos
  size_t endLine = buff.find("\n");

  if (endLine == std::string::npos)
  {
    if (buff.size() >= width)
      line = buff.substr(0, wordEnd);
    else
      line = buff;
  }
  else
  {
    if (endLine > width)
      line = buff.substr(0, wordEnd);
    else
      line = buff.substr(0, endLine + 1); // get also last '\n'
  }
  return line;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxDisplayFormatter::findEndOfWord
//
// Description:  Determine where line has end
//
// </PRE>
// ----------------------------------------------------------------------------
size_t
TaxDisplayFormatter::findEndOfWord(const std::string& buff, size_t width)
{
  size_t pos = buff.rfind(" ", width);

  if (pos < width)
    ++pos;

  if (pos == width && buff.size() > pos + 1 &&
      (buff.substr(pos, 1) == "\n" || buff.substr(pos, 1) == " "))
    ++pos;

  if (pos == std::string::npos)
    pos = width;

  return pos;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxDisplayFormatter::getOffset
//
// Description: Construct left side margin
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
TaxDisplayFormatter::getOffset(std::string& in)
{

  std::string offset;

  if (isOffset() && _offsetWidth == 0)
  {
    for (std::string::iterator i = in.begin(); i < in.end(); i++)
    {
      if (*i == ' ' || *i == '*')
        offset += " ";
      else
        break;
    }
  }
  else
    offset = std::string(_offsetWidth, ' ');

  return offset;
}
