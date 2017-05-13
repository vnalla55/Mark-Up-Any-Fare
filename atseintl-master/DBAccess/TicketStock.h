//----------------------------------------------------------------------------
//  � 2004, Sabre Inc.  All rights reserved.  This software/documentation is
//  the confidential and proprietary product of Sabre Inc. Any unauthorized use,
//  reproduction, or transfer of this software/documentation, in any medium, or
//  incorporation of this software/documentation into any system or publication,
//  is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

class TicketStock
{
public:
  TicketStock()
    : _tktStockCode(0), _couponsPerBook(0), _linesPermitted(0), _totalCharacters(0), _inhibit(' ')
  {
  }

  ~TicketStock() {}

  int& tktStockCode() { return _tktStockCode; }
  const int& tktStockCode() const { return _tktStockCode; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  std::string& ticketStock() { return _ticketStock; }
  const std::string& ticketStock() const { return _ticketStock; }

  int& couponsPerBook() { return _couponsPerBook; }
  const int& couponsPerBook() const { return _couponsPerBook; }

  int& linesPermitted() { return _linesPermitted; }
  const int& linesPermitted() const { return _linesPermitted; }

  int& totalCharacters() { return _totalCharacters; }
  const int& totalCharacters() const { return _totalCharacters; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  void addSegInfo(int lineNo, int lineLength)
  {
    _lineLength.insert(std::make_pair(lineNo, lineLength));
  }

  int lineLength(int lineNo) const
  {
    if (lineNo > _linesPermitted)
      return 0;

    std::map<int, int>::const_iterator iter = _lineLength.find(lineNo);
    if (iter != _lineLength.end())
      return iter->second;

    return 0;
  }

  bool operator==(const TicketStock& rhs) const
  {
    return ((_tktStockCode == rhs._tktStockCode) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_ticketStock == rhs._ticketStock) &&
            (_couponsPerBook == rhs._couponsPerBook) && (_linesPermitted == rhs._linesPermitted) &&
            (_totalCharacters == rhs._totalCharacters) && (_inhibit == rhs._inhibit) &&
            (_lineLength == rhs._lineLength));
  }

  static void dummyData(TicketStock& obj)
  {
    obj._tktStockCode = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._ticketStock = "aaaaaaaa";
    obj._couponsPerBook = 2;
    obj._linesPermitted = 3;
    obj._totalCharacters = 4;
    obj._inhibit = 'A';

    obj._lineLength[5] = 6;
    obj._lineLength[7] = 8;
  }

protected:
  int _tktStockCode;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  std::string _ticketStock;
  int _couponsPerBook;
  int _linesPermitted;
  int _totalCharacters;
  Indicator _inhibit;
  std::map<int, int> _lineLength;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _tktStockCode);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _ticketStock);
    FLATTENIZE(archive, _couponsPerBook);
    FLATTENIZE(archive, _linesPermitted);
    FLATTENIZE(archive, _totalCharacters);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _lineLength);
  }

protected:
private:
  TicketStock(const TicketStock&);
  TicketStock& operator=(const TicketStock&);
};
}
