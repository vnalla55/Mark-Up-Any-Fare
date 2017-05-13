// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <iomanip>

#include "Diagnostic/AtpcoDiagnostic.h"

namespace tax
{
namespace
{

boost::format
fullLineFormat()
{
  return boost::format("%|=" + boost::lexical_cast<std::string>(AtpcoDiagnostic::LINE_LENGTH) + "|");
}
}

const size_t AtpcoDiagnostic::LINE_LENGTH = 63;

AtpcoDiagnostic::AtpcoDiagnostic(void) {}

AtpcoDiagnostic::~AtpcoDiagnostic(void) {}

void
AtpcoDiagnostic::printLine(const char c)
{
  _result << fullLineFormat() % boost::io::group(std::setfill(c), std::setw(LINE_LENGTH), "");
}

void
AtpcoDiagnostic::printHeaderLong(const std::string& content)
{
  _result << fullLineFormat() %
    boost::io::group(std::setfill('-'), std::setw(LINE_LENGTH), " " + content + " ");
}

void
AtpcoDiagnostic::printHeaderShort(const std::string& content)
{
  _result << fullLineFormat() % str(boost::format("----%|=32|----") % content);
}

void
AtpcoDiagnostic::createMessages(boost::ptr_vector<Message>& messages)
{
  applyParameters();
  printHeader();
  runAll();

  for (std::string line; _result.getline(line);)
  {
    for (size_t i = 0; i <= line.length(); i += LINE_LENGTH)
    {
      messages.push_back(new Message());
      messages.back()._content = std::string(line, i, LINE_LENGTH);
    }
  }
}

void
AtpcoDiagnostic::printInputFilterHelp()
{
  _result << "INPUT FILTERS - PROCESS ONLY SEQUENCES MATCHING FILTERS:\n"
    << "IVXXXXX - PROCESS RECORDS WITH VENDOR XXXXX (ATP/SABR)\n"
    << "INXX - PROCESS RECORDS WITH NATION XX\n"
    << "ICXX - PROCESS RECORDS WITH TAX CODE XX\n"
    << "ITX - PROCESS RECORDS WITH TAX TYPE X\n"
    << "ISXXX - PROCESS RECORDS WITH SEQUENCE XXX\n"
    << "ISXXX- - PROCESS RECORDS WITH SEQUENCE XXX AND HIGHER\n"
    << "ISXXX-YYY - PROCESS RECORDS WITH SEQUENCE RANGE XXX-YYY\n"
    << "    EG. /IVATPCO/INUS/ICCE/ITF/IS17500-";
}

}
