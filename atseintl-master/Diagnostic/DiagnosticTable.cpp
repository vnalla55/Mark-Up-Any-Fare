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

#include "Diagnostic/DiagnosticTable.h"
#include "DataModel/PricingTrx.h"

#include <iostream>

namespace tse
{

DiagnosticTable::DiagnosticTable(const PricingTrx& trx)
{
  if (!trx.isMip() && !trx.isShopping())
    _verticalLineChar = ':';
}

std::string
DiagnosticTable::Column::getField(const std::string& value,
                          const uint32_t size,
                          const TextAlign textAlign)
{
  if (value.size() > size)
    return value.substr(0, size-1) + '*';

  const std::string filling(size - value.size(), ' ');

  if (textAlign == TextAlign::RIGHT)
    return filling + value;
  else if (textAlign == TextAlign::LEFT)
    return value + filling;
  else
    return filling.substr(0, filling.size()/2) + value + filling.substr(filling.size()/2);
}

void
DiagnosticTable::printFrame()
{
  for (const auto& column : _columns)
    _os << '+' << std::string(column.getSize(), '-');
  _os << '+' << std::endl;
}

void
DiagnosticTable::printValues()
{
  for (uint32_t row = 0; row < _columns[0].getRows(); ++row)
  {
    _os << _verticalLineChar;
    for (const auto& column : _columns)
      _os << Column::getField(column[row], column.getSize(), column.getTextAlign()) << _verticalLineChar;
    _os << std::endl;
  }
}

std::vector<std::string>
DiagnosticTable::Column::divideNameBasedOnColumnSize() const
{
  std::vector<std::string> name;
  std::size_t first = 0;
  const std::size_t last = getName().size();

  while (true)
  {
    std::size_t space = std::string::npos;
    if (last - first > getSize())
      for (std::size_t i = first + getSize(); i != first; --i)
        if (getName()[i] == ' ')
        {
          space = i - first;
          break;
        }

    name.push_back(getField(getName().substr(first, space), getSize()));

    if (space == std::string::npos)
      break;

    first += space + 1;
  }

  return name;
}

std::vector<std::vector<std::string>>
DiagnosticTable::prepareMultiLineHeader() const
{
  std::vector<std::vector<std::string>> header;
  for (const auto& column : _columns)
    header.push_back(column.divideNameBasedOnColumnSize());

  return header;
}

namespace
{
uint8_t
countNumberOfRowsForHeader(const std::vector<std::vector<std::string>>& header)
{
  uint8_t nbOfRows = 1;
  for (const auto& name : header)
    if (nbOfRows < name.size())
      nbOfRows = name.size();

  return nbOfRows;
}
}

void
DiagnosticTable::printHeader()
{
  const auto& header = prepareMultiLineHeader();

  const uint8_t nbOfRows = countNumberOfRowsForHeader(header);

  for (uint32_t i = 0; i < nbOfRows; ++i)
  {
    _os << _verticalLineChar;
    for (const auto& name : header)
      if (name.size() > i)
        _os << name[i] << _verticalLineChar;
      else
        _os << std::string(name[0].size(), ' ') << _verticalLineChar;
    _os << std::endl;
  }
}

}
