// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include <string>

namespace tse
{
class PreviousTicketTaxInfo
{
  const std::string _sabreTaxCode;
  const double _percentage;
  const bool _isCanadaExch;

public:
  PreviousTicketTaxInfo(const std::string& sabreTaxCode, double percentage, bool isCanadaExch)
    : _sabreTaxCode(sabreTaxCode), _percentage(percentage), _isCanadaExch(isCanadaExch)
  {
  }

  bool operator<(const PreviousTicketTaxInfo& that) const
  {
    if (_sabreTaxCode < that.getSabreTaxCode())
      return true;

    if (_sabreTaxCode == that.getSabreTaxCode())
    {
      if (_percentage < that.getPercentage())
        return true;

      if (_percentage == that.getPercentage())
        return _isCanadaExch < that.isCanadaExch();
    }

    return false;
  }

  const std::string& getSabreTaxCode() const { return _sabreTaxCode; }

  double getPercentage() const { return _percentage; }

  bool isCanadaExch() const { return _isCanadaExch; }
};

} // end of tse namespace
