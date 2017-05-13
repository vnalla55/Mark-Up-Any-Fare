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

#include "TaxDisplay/Response/Line.h"

#include <boost/format.hpp>

#include <algorithm>
#include <list>
#include <sstream>
#include <string>
#include <vector>

namespace tax
{
namespace display
{

class LineParams;

class ResponseFormatter
{
  typedef std::list<Line> LinesList;
  friend class ResponseFormatterTest;

public:
  ResponseFormatter() {}

  ResponseFormatter&
  addBlankLine() { return addLine(std::string()); }

  ResponseFormatter&
  addLine(const Line& line)
  {
    _linesList.push_back(line);
    return *this;
  }

  ResponseFormatter&
  addLine(const std::string& str) { return addLine(Line(str)); }

//  ResponseFormatter&
//  addLine(const std::list<Line>& lines)
//  {
//    std::copy(lines.begin(), lines.end(), std::back_inserter(_linesList));
//    return *this;
//  }

  ResponseFormatter&
  addLine(std::list<Line>&& lines)
  {
    _linesList.splice(_linesList.end(), lines);
    return *this;
  }

  ResponseFormatter&
  addLine(const std::string& str,
          const LineParams& params) { return addLine(Line(str, params)); }

  ResponseFormatter&
  addLine(boost::format& format) { return addLine(Line(format.str())); }

  LinesList::iterator
  addLineBefore(LinesList::iterator it) { return addLineBefore(it, Line()); }

  LinesList::iterator
  addLineBefore(LinesList::iterator it,
                const std::string& str) { return addLineBefore(it, Line(str)); }

  LinesList::iterator
  addLineBefore(LinesList::iterator it,
                const LineParams& params) { return addLineBefore(it, Line(params)); }

  LinesList::iterator
  addLineBefore(LinesList::iterator it, const Line& line)
  {
    _linesList.insert(it, line);
    return --it;
  }

  ResponseFormatter&
  addSeparatorLine(char separatorChar)
  {
    return addLine(std::string(DISPLAY_WIDTH, separatorChar));
  }

  void
  format(std::ostringstream& response);

  LinesList&
  linesList() { return _linesList; }

  const LinesList&
  linesList() const { return _linesList; }

private:
  void formatLine(LinesList::iterator line, std::string& formattedLine);

  static const unsigned short DISPLAY_WIDTH;
  LinesList _linesList;
};

} /* namespace display */
} /* namespace tax */
