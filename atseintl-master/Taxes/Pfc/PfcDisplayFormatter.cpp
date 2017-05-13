
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "Taxes/Pfc/PfcDisplayFormatter.h"
#include "Common/TseConsts.h"
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lambda/lambda.hpp>

using namespace tse;
using namespace boost::lambda;

const std::string PfcDisplayFormatter::NOT_APPLICABLE = "N/A";

const std::string PfcDisplayFormatter::DATE_TIME_NOT_APPLICABLE = "NONE";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatter::PfcDisplayFormatter
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayFormatter::PfcDisplayFormatter(const std::string& tableHeader)
  : _width(GREEN_SCREEN_WIDTH)
{
  if (!tableHeader.empty())
  {
    getTableInfo(tableHeader);
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatter::~PfcDisplayFormatter
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayFormatter::~PfcDisplayFormatter() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatter::right
//
// Description:  Get table info.
//
// </PRE>
// ----------------------------------------------------------------------------
void
PfcDisplayFormatter::getTableInfo(const std::string& tableHeader)
{
  std::string firstTableHeaderLine = tableHeader.substr(0, tableHeader.find("\n"));

  boost::regex reg("\\s{2,}");
  boost::sregex_token_iterator it(
      firstTableHeaderLine.begin(), firstTableHeaderLine.end(), reg, -1);
  boost::sregex_token_iterator end;

  std::string s;
  size_t pos = 0;
  while (it != end)
  {
    s = *it++;
    _tableHeaderFieldsPos.push_back(firstTableHeaderLine.find(s, pos));
    _tableHeaderFields.push_back(s);
    pos += s.size();
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatter::margin
//
// Description:  Left margin.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatter::margin(const std::string& s, size_t marginSize)
{
  std::string margin;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("\n");
  tokenizer tokens(s, sep);

  for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
  {
    margin += std::string(marginSize, SPACE) + *tok_iter + "\n";
  }

  return margin;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatter::center
//
// Description:  Center string.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatter::center(const std::string& s)
{
  std::string center;
  size_t sLength;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("\n");
  tokenizer tokens(s, sep);

  for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
  {
    sLength = (*tok_iter).size();
    center += std::string(_width / 2 - sLength / 2, SPACE) + *tok_iter + "\n";
  }

  return center;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatter::left
//
// Description:  Move string to the left.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatter::left(const std::string& s)
{
  std::ostringstream ss;
  ss.width(_width);
  ss.setf(std::ios_base::left, std::ios_base::adjustfield);
  ss << s;
  return ss.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatter::right
//
// Description:  Move string to the right.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatter::right(const std::string& s)
{
  std::ostringstream ss;
  ss.width(_width);
  ss.setf(std::ios_base::right, std::ios_base::adjustfield);
  ss << s;
  return ss.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatter::round
//
// Description:  Round Money Amt.
//
// </PRE>
// ----------------------------------------------------------------------------
MoneyAmount
PfcDisplayFormatter::round(const MoneyAmount& amt,
                           RoundingFactor roundingUnit,
                           RoundingRule roundingRule)
{

  Money amtMoney(amt, US_DOLLARS);

  CurrencyConverter curConverter;
  if (curConverter.round(amtMoney, roundingUnit, roundingRule))
  {
    return amtMoney.value();
  }
  else
  {
    return 0.0;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXC::pfcAmount
//
// Description:  PXC money amount formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatter::pfcAmount(MoneyAmount amount)
{
  if (amount == 0.0)
  {
    return std::string();
  }

  std::ostringstream ss;
  ss.fill(' ');
  ss.precision(2);
  ss.setf(std::ios::fixed, std::ios::floatfield);
  ss.setf(std::ios_base::right, std::ios_base::adjustfield);
  ss.width(PXC_AMOUNT_SIZE);
  ss << amount;

  return US_DOLLARS + std::string(1, SPACE) + ss.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXC::counter
//
// Description:  PXC note/seg counter formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatter::counter(uint32_t counter)
{
  std::ostringstream ss;
  ss.fill('0');
  ss.width(PXC_NOTE_COUNTER_SIZE);
  ss << counter;

  return ss.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXC::PfcDisplayFormatter
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayFormatterPXC::PfcDisplayFormatterPXC(const std::string& tableHeader)
  : PfcDisplayFormatter(tableHeader)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXC::flight
//
// Description:  PXC flight number formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXC::flight(uint32_t flight)
{
  std::ostringstream ss;
  ss.fill(' ');
  ss.setf(std::ios_base::right, std::ios_base::adjustfield);
  ss.width(PXC_FLIGHT_SIZE);
  if (flight)
  {
    ss << flight;
  }
  else
  {
    ss << SPACE;
  }

  return ss.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXC::line
//
// Description:  PXC line formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXC::line(const std::string& column1,
                             const std::string& column2,
                             const std::string& column3,
                             const std::string& column4,
                             const std::string& column5,
                             const std::string& column6)
{

  std::ostringstream ss;

  ss.setf(std::ios_base::left, std::ios_base::adjustfield);

  ss << std::setw(_tableHeaderFieldsPos[1]) << column1
     << std::setw(_tableHeaderFieldsPos[2] - _tableHeaderFieldsPos[1]) << column2
     << std::setw(_tableHeaderFieldsPos[3] - _tableHeaderFieldsPos[2]) << column3
     << std::setw(_tableHeaderFieldsPos[4] - _tableHeaderFieldsPos[3]) << column4;

  ss.setf(std::ios_base::right, std::ios_base::adjustfield);

  ss << std::setw(_tableHeaderFields[4].size()) << column5
     << std::string(_tableHeaderFieldsPos[5] - _tableHeaderFieldsPos[4] -
                        _tableHeaderFields[4].size(),
                    SPACE) << std::setw(_tableHeaderFields[5].size()) << column6 << std::endl;

  return ss.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXC::warningLine
//
// Description:  PXC warning line formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXC::warningLine(const std::string& airportCode, const std::string& warning)
{
  return airportCode + std::string(6, SPACE) + warning + "\n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXC::PfcDisplayFormatter
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayFormatterPXA::PfcDisplayFormatterPXA(const std::string& tableHeader)
  : PfcDisplayFormatter(tableHeader)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXA::line
//
// Description:  PXA line formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXA::line(const std::string& column1,
                             const std::string& column2,
                             const std::string& column3,
                             const std::string& column4,
                             const std::string& column5)
{
  std::ostringstream ss;
  ss.setf(std::ios_base::left, std::ios_base::adjustfield);

  if (column5.size() > 3)
  {
    ss << std::setw(_tableHeaderFieldsPos[1]) << column1
       << std::setw(_tableHeaderFieldsPos[2] - _tableHeaderFieldsPos[1]) << column2
       << std::setw(_tableHeaderFieldsPos[3] - _tableHeaderFieldsPos[2]) << column3
       << std::setw(_tableHeaderFieldsPos[4] - _tableHeaderFieldsPos[3] - 1) << column4 << column5
       << std::endl;
  }
  else
  {
    ss << std::setw(_tableHeaderFieldsPos[1]) << column1
       << std::setw(_tableHeaderFieldsPos[2] - _tableHeaderFieldsPos[1]) << column2
       << std::setw(_tableHeaderFieldsPos[3] - _tableHeaderFieldsPos[2]) << column3
       << std::setw(_tableHeaderFieldsPos[4] - _tableHeaderFieldsPos[3]) << column4 << column5
       << std::endl;
  }

  return ss.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXA::line
//
// Description:  PXA line formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXA::line(const std::string& column1,
                             const std::string& column2,
                             const std::string& column3,
                             const std::string& column4)
{

  std::ostringstream ss;

  ss.setf(std::ios_base::left, std::ios_base::adjustfield);

  ss << std::setw(_tableHeaderFieldsPos[1]) << column1
     << std::setw(_tableHeaderFieldsPos[2] - _tableHeaderFieldsPos[1]) << column2
     << std::setw(_tableHeaderFieldsPos[3] - _tableHeaderFieldsPos[2]) << column3 << column4
     << std::endl;

  return ss.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXA::line
//
// Description:  PXA line formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXA::line(const std::string& column1,
                             const std::string& column2,
                             const std::string& column3)
{
  std::ostringstream ss;
  ss.setf(std::ios_base::left, std::ios_base::adjustfield);

  ss << std::string(2, SPACE) << std::setw(_tableHeaderFieldsPos[1])
     << (_tableHeaderFields[0] + column1)
     << std::setw(_tableHeaderFieldsPos[2] - _tableHeaderFieldsPos[1])
     << (_tableHeaderFields[1] + column2) << (_tableHeaderFields[2] + column3) << std::endl;

  return ss.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXA::fare
//
// Description:  PXA fare amount formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXA::fare(MoneyAmount amount)
{
  if (amount == 0.0)
  {
    return NOT_APPLICABLE;
  }
  else
  {
    return toString(amount) + "-";
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXA::tax
//
// Description:  PXA tax amount formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXA::tax(MoneyAmount amount)
{

  std::string tax;

  std::ostringstream ss;
  ss.precision(2);
  ss.setf(std::ios::fixed, std::ios::floatfield);
  ss.setf(std::ios_base::right, std::ios_base::adjustfield);
  ss << amount;

  tax = ss.str();

  if (tax.substr(0, 2) == std::string("0."))
  {
    tax = std::string(tax, 1);
  }

  if (tax.empty())
  {
    tax = ".00";
  }

  return tax;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXH::PfcDisplayFormatter
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayFormatterPXH::PfcDisplayFormatterPXH(const std::string& tableHeader)
  : PfcDisplayFormatter(tableHeader)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXH::line
//
// Description:  PXH line formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXH::line(const std::string& column1,
                             const std::string& column2,
                             const std::string& column3,
                             const std::string& column4,
                             const std::string& column5,
                             const std::string& column6,
                             const std::string& column7)
{

  std::ostringstream ss;

  ss.setf(std::ios_base::left, std::ios_base::adjustfield);

  ss << std::setw(_tableHeaderFieldsPos[1]) << column1
     << std::setw(_tableHeaderFieldsPos[2] - _tableHeaderFieldsPos[1]) << column2
     << std::setw(_tableHeaderFieldsPos[3] - _tableHeaderFieldsPos[2]) << column3
     << std::setw(_tableHeaderFieldsPos[4] - _tableHeaderFieldsPos[3]) << column4
     << std::setw(_tableHeaderFieldsPos[5] - _tableHeaderFieldsPos[4]) << column5
     << std::setw(_tableHeaderFieldsPos[6] - _tableHeaderFieldsPos[5]) << column6 << column7
     << std::endl;

  return ss.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXH::dateTime
//
// Description:  PXH DateTime formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXH::dateTime(const DateTime& dt)
{
  if (!dt.isValid())
  {
    return std::string(DATE_TIME_NOT_APPLICABLE);
  }
  std::string dateTime = dt.dateToString(tse::DDMMMYY, nullptr) + "/" + dt.timeToString(HHMM, ":");

  return dateTime;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXE::PfcDisplayFormatter
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayFormatterPXE::PfcDisplayFormatterPXE(const std::string& tableHeader)
  : PfcDisplayFormatter(tableHeader)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXE::line
//
// Description:  PXE line formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXE::line(const std::string& column1,
                             const std::string& column2,
                             const std::string& column3,
                             const std::string& column4,
                             const std::string& column5,
                             const std::string& column6)
{
  std::ostringstream ss;

  ss.setf(std::ios_base::left, std::ios_base::adjustfield);

  ss << std::setw(_tableHeaderFieldsPos[1]) << column1
     << std::setw(_tableHeaderFieldsPos[2] - _tableHeaderFieldsPos[1]) << column2
     << std::setw(_tableHeaderFieldsPos[3] - _tableHeaderFieldsPos[2]) << column3
     << std::setw(_tableHeaderFieldsPos[4] - _tableHeaderFieldsPos[3])
     << std::string((_tableHeaderFields[3].size() - column4.size()) / 2, SPACE) + column4
     << std::setw(_tableHeaderFieldsPos[5] - _tableHeaderFieldsPos[4]) << column5 << column6
     << std::endl;

  return ss.str();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXQ::line
//
// Description:  PXQ line formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXQ::line(const std::string& column1,
                             const std::string& column2,
                             const std::string& column3,
                             const std::string& column4,
                             const std::string& column5)
{

  std::ostringstream ss;
  ss.setf(std::ios_base::left, std::ios_base::adjustfield);

  ss << std::setw(11) << column1;

  ss.setf(std::ios_base::right, std::ios_base::adjustfield);

  ss << std::setw(3) << column2 << "        ";

  ss.setf(std::ios_base::left, std::ios_base::adjustfield);

  ss << std::setw(12) << column3 << std::setw(14) << column4 << column5 << std::endl;

  return ss.str();
}

PfcDisplayFormatterPXQ::PfcDisplayFormatterPXQ(const std::string& tableHeader)
  : PfcDisplayFormatter(tableHeader)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXQ::line
//
// Description:  PXQ line formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXT::line(const std::string& column1,
                             const std::string& column2,
                             const std::string& column3,
                             const std::string& column4,
                             const std::string& column5)
{
  const std::string::size_type COLUMN5_MAX_SIZE = 28;
  std::ostringstream ss;
  ss.setf(std::ios_base::left, std::ios_base::adjustfield);

  ss << std::setw(_tableHeaderFieldsPos[1]) << column1
     << std::setw(_tableHeaderFieldsPos[2] - _tableHeaderFieldsPos[1]) << column2 << std::setw(10)
     << column3 << std::setw(10) << column4;

  if (column5.empty())
  {
    ss << std::endl;
    return ss.str();
  }

  std::string::size_type offsets[] = { COLUMN5_MAX_SIZE };
  boost::offset_separator separator(offsets, offsets + 1);
  boost::tokenizer<boost::offset_separator> tokens(column5, separator);

  boost::tokenizer<boost::offset_separator>::iterator token = tokens.begin();

  ss << *token++ << std::endl;

  std::for_each(token,
                tokens.end(),
                ss << constant(std::setw(_tableHeaderFieldsPos[2] + 20)) << constant(' ') << _1
                   << constant('\n'));

  return ss.str();
}

std::string
PfcDisplayFormatterPXT::colOptToStr(const Indicator colOpt)
{
  switch (colOpt)
  {
  case '1':
    return "DEP";
  case '2':
    return "ALL";
  case '3':
    return "BYP";
  default:
    return "???";
  }
}

PfcDisplayFormatterPXT::PfcDisplayFormatterPXT(const std::string& tableHeader)
  : PfcDisplayFormatter(tableHeader)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXM::PfcDisplayFormatterPXM
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayFormatterPXM::PfcDisplayFormatterPXM(const std::string& tableHeader)
  : PfcDisplayFormatter(tableHeader)
{
}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayFormatterPXM::line
//
// Description:  PXM line formatter.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayFormatterPXM::line(const std::string& column1,
                             const std::string& column2,
                             const std::string& column3)
{
  std::ostringstream ss;

  ss.setf(std::ios_base::left, std::ios_base::adjustfield);

  ss << std::setw(_tableHeaderFieldsPos[1] - column2.size() / 2 + _tableHeaderFields[1].size() / 2)
     << column1 << std::setw(_tableHeaderFieldsPos[2] - _tableHeaderFieldsPos[1] +
                             column2.size() / 2 - _tableHeaderFields[1].size() / 2 +
                             _tableHeaderFields[2].size() / 2 - column3.size() / 2 - 1) << column2
     << column3 << std::endl;

  return ss.str();
}
