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

#include <string>

namespace tax
{
namespace display
{

class LineParams
{
public:
  enum class LongLineFormatting
  {
    BREAK,
    TRUNCATE
  };

  LineParams() = default;
  virtual ~LineParams() = default;
  bool operator==(const LineParams& rhs) const;
  bool operator!=(const LineParams& rhs) const { return !(*this == rhs); }

  void setLeftMargin(int8_t margin) { _leftMargin = margin; }
  int8_t getLeftMargin() const { return _leftMargin; }

  void setParagraphIndent(int8_t indent) { _paragraphIndent = indent; }
  int8_t getParagraphIndent() const { return _paragraphIndent; }

  bool& isUserDefined() { return _isUserDefined; }
  const bool& isUserDefined() const { return _isUserDefined; }

  LongLineFormatting& longLineFormatting() { return _longLineFormatting; }
  const LongLineFormatting& longLineFormatting() const { return _longLineFormatting; }

  // tool functions with most often used params
  static LineParams withLeftMargin(int8_t margin);
  static LineParams withTruncateFlag();

protected:
  int8_t _leftMargin{0};
  int8_t _paragraphIndent{0};
  bool _isUserDefined{true};
  LongLineFormatting _longLineFormatting{LongLineFormatting::BREAK};
};

} /* namespace display */
} /* namespace tax */
