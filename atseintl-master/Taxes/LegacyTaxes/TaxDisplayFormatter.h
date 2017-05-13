//----------------------------------------------------------------------------
//  File:           TaxDisplayFormatter.h
//  Description:    TaxDisplayFormatter header file for ATSE International Project
//  Created:        12/12/2007
//  Authors:        Piotr Lach
//
//  Description: This Object will be used for Tax Display functionality.
//
//  Updates:
//
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#ifndef TAX_DISPLAY_FORMATER_H
#define TAX_DISPLAY_FORMATER_H

#include <string>

namespace tse
{

class TaxDisplayFormatter
{

public:
  TaxDisplayFormatter(size_t lineLength = 60);
  ~TaxDisplayFormatter();

  void format(std::string& buff);

  bool isOffset() { return _isOffset; }

  void setOffset(bool state) { _isOffset = state; }

  void offsetWidth(size_t width)
  {
    if (_isOffset)
      _offsetWidth = width;
    else
      _offsetWidth = 0;
  }

private:
  std::string getOffset(std::string& in);
  size_t findEndOfWord(const std::string& buff, size_t width);
  std::string getLine(std::string& buff, const std::string& offset, const size_t width);

  size_t _lineLength;
  bool _isOffset;
  size_t _offsetWidth;

  TaxDisplayFormatter(const TaxDisplayFormatter&);
  TaxDisplayFormatter& operator=(const TaxDisplayFormatter&);
};

} // namespace tse
#endif // TAX_PERCENTAGE_US_H
