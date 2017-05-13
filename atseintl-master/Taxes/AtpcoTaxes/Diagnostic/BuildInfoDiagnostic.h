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
#pragma once

#include <stdint.h>
#include "Diagnostic/AtpcoDiagnostic.h"

namespace tax
{

class BuildInfoDiagnostic : public AtpcoDiagnostic
{
public:
  static const uint32_t NUMBER;

  BuildInfoDiagnostic(const std::string& buildInfo, const boost::ptr_vector<Parameter>& parameters);
  ~BuildInfoDiagnostic();

private:
  void printBuildInfo();
  typedef Printer<BuildInfoDiagnostic, &BuildInfoDiagnostic::printBuildInfo>
  BuildInfoDiagnosticPrinter;

  virtual void runAll() override;
  virtual void printHeader() override;

  BuildInfoDiagnosticPrinter _buildInfoDiagnosticPrinter;
  const std::string _buildInfo;
  const boost::ptr_vector<Parameter>& _parameters;
  void applyParameters() override;
};

} // namespace tax
