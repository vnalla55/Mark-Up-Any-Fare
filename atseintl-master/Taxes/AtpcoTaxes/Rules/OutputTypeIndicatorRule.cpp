// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "Rules/DummyApplicator.h"
#include "Rules/OutputTypeIndicatorRule.h"

namespace tax
{

OutputTypeIndicatorRule::OutputTypeIndicatorRule(const type::OutputTypeIndicator& outputTypeIndicator)
  : _outputTypeIndicator(outputTypeIndicator)
{
}

OutputTypeIndicatorRule::~OutputTypeIndicatorRule() {}

OutputTypeIndicatorRule::ApplicatorType
OutputTypeIndicatorRule::createApplicator(type::Index const& /*itinIndex*/,
                                          const Request& /*request*/,
                                          Services& /*services*/,
                                          RawPayments& /*itinPayments*/) const
{
  return DummyApplicator(*this,
                         _outputTypeIndicator != type::OutputTypeIndicator::OnlyRATD,
                         "Sequence only for RATD!");
}

std::string
OutputTypeIndicatorRule::getDescription(Services&) const
{
  std::string result("VALIDATE OUTPUT TYPE INDICATOR: ");
  if (_outputTypeIndicator == type::OutputTypeIndicator::OnlyRATD)
    result += "FAIL, SEQUENCE EXCLUSIVE FOR RATD";
  else
    result += "PASS, SEQUENCE NOT EXCLUSIVE TO RATD";

  return result;
}
}
