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
#include "Diagnostic/BuildInfoDiagnostic.h"

#include <boost/lexical_cast.hpp>

namespace tax
{

const uint32_t BuildInfoDiagnostic::NUMBER = 839;

BuildInfoDiagnostic::BuildInfoDiagnostic(const std::string& buildInfo,
                                         const boost::ptr_vector<Parameter>& parameters)
  : _buildInfo(buildInfo), _parameters(parameters)
{
}

BuildInfoDiagnostic::~BuildInfoDiagnostic(void)
{
}

void BuildInfoDiagnostic::runAll()
{
  _buildInfoDiagnosticPrinter.enable();
  _buildInfoDiagnosticPrinter.print(*this);
  printLine('-');
}

void BuildInfoDiagnostic::applyParameters()
{
}

void BuildInfoDiagnostic::printHeader()
{
  printLine('*');
  printHeaderShort("DIAGNOSTIC " + boost::lexical_cast<std::string>(NUMBER) + " - BUILD INFO");
  printLine('*');
}

void BuildInfoDiagnostic::printBuildInfo()
{
  _result << _buildInfo;
}

} // namespace tax
