//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/Assert.h"

#include <sstream>
#include <string>
#include <vector>

namespace tse
{
class PricingTrx;

class DiagnosticTable
{
public:
  DiagnosticTable(const PricingTrx& trx);

  DiagnosticTable& addColumn(std::string name,
                             const uint32_t size)
  {
    _columns.emplace_back(std::move(name), size);
    return *this;
  }

  DiagnosticTable& setLeftAlign()
  {
    _columns.back().setTextAlign(TextAlign::LEFT);
    return *this;
  }

  DiagnosticTable& setRightAlign()
  {
    _columns.back().setTextAlign(TextAlign::RIGHT);
    return *this;
  }

  /**
    * Add value to next column as string
    */
  template<typename T>
  DiagnosticTable& operator<<(const T& val)
  {
    if (_columns.size() > _column)
    {
      std::ostringstream os;
      os << val;
      _columns[_column].addValue(os.str());
      ++_column;
    }
    return *this;
  }

  /*
   * Append value to actual column as string
   */
  template<typename T>
  DiagnosticTable& operator>>(const T& val)
  {
    if (_columns.size() > _column - 1)
    {
     std::ostringstream os;
     os << val;
      _columns[_column - 1].appendValue(os.str());
    }
    return *this;
  }

  DiagnosticTable& nextRow()
  {
    _column = 0;
    return *this;
  }

  std::string toString()
  {
    _os.clear();
    printFrame();
    printHeader();
    printFrame();
    printValues();
    printFrame();
    return _os.str();
  }

private:
  enum class TextAlign
  {
    CENTER,
    LEFT,
    RIGHT
  };

  class Column
  {
  public:
    Column(std::string name, const uint32_t size)
        : _name(std::move(name)), _size(size) {}

    void appendValue(const std::string& value)
    {
      TSE_ASSERT(!_values.empty());
      _values.back() += value;
    }

    void addValue(std::string value) { _values.push_back(std::move(value)); }
    void setTextAlign(const TextAlign textAlign) { _textAlign = textAlign; }

    uint32_t getSize() const { return _size; }
    const std::string& getName() const { return _name; }
    uint32_t getRows() const { return _values.size(); }
    TextAlign getTextAlign() const { return _textAlign; }

    const std::string& operator[](const size_t i) const
    {
      return _values[i];
    }

    static std::string
    getField(const std::string& value,
             const uint32_t size,
             const TextAlign textAlign = TextAlign::CENTER);

    std::vector<std::string> divideNameBasedOnColumnSize() const;

  private:
    const std::string _name;
    TextAlign _textAlign;
    const uint32_t _size;
    std::vector<std::string> _values;
  };

  std::vector<std::vector<std::string>> prepareMultiLineHeader() const;
  void printFrame();
  void printHeader();
  void printValues();

private:
  std::ostringstream _os;
  uint32_t _column = 0;
  std::vector<Column> _columns;
  char _verticalLineChar = '|';
};

} // namespace tse
