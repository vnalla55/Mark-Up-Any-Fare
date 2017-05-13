//----------------------------------------------------------------------------
//  File:           PfcDisplayBuilder.h
//  Authors:        Piotr Lach
//  Created:        4/16/2008
//  Description:    PfcDisplayFormatter header file for ATSE V2 PFC Display Project.
//                  String utilities for PFC Display.
//
//  Copyright Sabre 2008
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
#ifndef PFC_DISPLAY_FORMATTER_H
#define PFC_DISPLAY_FORMATTER_H

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseConsts.h"
#include "Common/CurrencyConverter.h"
#include "Common/Money.h"
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

namespace tse
{

class PfcDisplayFormatter
{
public:
  static const uint32_t GREEN_SCREEN_WIDTH = 63;
  static const uint32_t PXC_AMOUNT_SIZE = 4;
  static const uint32_t PXC_NOTE_COUNTER_SIZE = 3;
  static constexpr char SPACE = ' ';
  static const std::string NOT_APPLICABLE;
  static const std::string DATE_TIME_NOT_APPLICABLE;

  PfcDisplayFormatter(const std::string& columnHeader = EMPTY_STRING());
  virtual ~PfcDisplayFormatter();

  std::string margin(const std::string& s, size_t marginSize);
  std::string center(const std::string& s);
  std::string left(const std::string& s);
  std::string right(const std::string& s);

  MoneyAmount round(const MoneyAmount& amt,
                    RoundingFactor roundingUnit = 0.01,
                    RoundingRule roundingRule = NEAREST);

  template <typename T>
  std::string toString(const T& t)
  {
    std::ostringstream os;
    os.setf(std::ios::fixed);
    os.precision(2);
    os << t;
    return os.str();
  }

  std::string toString(const std::vector<std::string>& t)
  {
    std::string s;

    for (size_t i = 0; i != t.size(); i++)
    {
      s += t[i];
    }

    return s;
  }

  std::string toString(const DateTime& dt)
  {
    if (!dt.isValid())
    {
      return std::string(DATE_TIME_NOT_APPLICABLE);
    }

    return dt.dateToString(tse::DDMMMYY, nullptr);
  }

  std::string pfcAmount(MoneyAmount amount);
  std::string counter(uint32_t counter);

protected:
  virtual void getTableInfo(const std::string& tableHeader);
  std::vector<std::string> _tableHeaderFields;
  std::vector<uint32_t> _tableHeaderFieldsPos;

private:
  unsigned int _width;
};

class PfcDisplayFormatterPXC : public PfcDisplayFormatter
{
public:
  static const uint32_t PXC_FLIGHT_SIZE = 4;

  PfcDisplayFormatterPXC(const std::string& columnHeader = EMPTY_STRING());
  virtual ~PfcDisplayFormatterPXC() {}

  std::string line(const std::string& column1,
                   const std::string& column2,
                   const std::string& column3,
                   const std::string& column4,
                   const std::string& column5,
                   const std::string& column6);

  std::string warningLine(const std::string& airportCode, const std::string& warning);

  std::string flight(uint32_t flight);
};

class PfcDisplayFormatterPXA : public PfcDisplayFormatter
{
public:
  PfcDisplayFormatterPXA(const std::string& columnHeader = EMPTY_STRING());
  virtual ~PfcDisplayFormatterPXA() {}

  std::string line(const std::string& column1,
                   const std::string& column2,
                   const std::string& column3,
                   const std::string& column4,
                   const std::string& column5);

  std::string line(const std::string& column1,
                   const std::string& column2,
                   const std::string& column3,
                   const std::string& column4);

  std::string
  line(const std::string& column1, const std::string& column2, const std::string& column3);

  std::string fare(MoneyAmount amount);
  std::string tax(MoneyAmount amount);
};

class PfcDisplayFormatterPXH : public PfcDisplayFormatter
{
public:
  PfcDisplayFormatterPXH(const std::string& columnHeader = EMPTY_STRING());
  virtual ~PfcDisplayFormatterPXH() {}

  std::string line(const std::string& column1,
                   const std::string& column2,
                   const std::string& column3,
                   const std::string& column4,
                   const std::string& column5,
                   const std::string& column6,
                   const std::string& column7);

  std::string dateTime(const DateTime& dt);
};

class PfcDisplayFormatterPXE : public PfcDisplayFormatter
{
public:
  PfcDisplayFormatterPXE(const std::string& columnHeader = EMPTY_STRING());
  virtual ~PfcDisplayFormatterPXE() {}

  std::string line(const std::string& column1,
                   const std::string& column2,
                   const std::string& column3,
                   const std::string& column4,
                   const std::string& column5,
                   const std::string& column6);
};

class PfcDisplayFormatterPXM : public PfcDisplayFormatter
{
public:
  PfcDisplayFormatterPXM(const std::string& columnHeader = EMPTY_STRING());
  virtual ~PfcDisplayFormatterPXM() {}

  std::string
  line(const std::string& column1, const std::string& column2, const std::string& column3);
};

class PfcDisplayFormatterPXQ : public PfcDisplayFormatter
{
public:
  PfcDisplayFormatterPXQ(const std::string& columnHeader = EMPTY_STRING());
  virtual ~PfcDisplayFormatterPXQ() {};

  std::string line(const std::string& column1,
                   const std::string& column2,
                   const std::string& column3,
                   const std::string& column4,
                   const std::string& column5);
};

class PfcDisplayFormatterPXT : public PfcDisplayFormatter
{
public:
  PfcDisplayFormatterPXT(const std::string& columnHeader = EMPTY_STRING());
  virtual ~PfcDisplayFormatterPXT() {};

  std::string line(const std::string& column1,
                   const std::string& column2,
                   const std::string& column3,
                   const std::string& column4,
                   const std::string& column5);

  std::string colOptToStr(const Indicator colOpt);
};

} // namespace tse
#endif
