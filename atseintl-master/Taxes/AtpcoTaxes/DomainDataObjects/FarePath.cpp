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
#include "Common/MoneyUtil.h"
#include "DataModel/Common/CodeIO.h"
#include "DataModel/RequestResponse/InputFarePath.h"
#include "DomainDataObjects/FarePath.h"
#include "Factories/FareUsageFactory.h"

namespace tax
{

FarePath::FarePath(void) : _totalAmount(0), _totalMarkupAmount(0) {}

FarePath::~FarePath(void) {}

std::ostream&
FarePath::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  const std::string indent(indentLevel, indentChar);

  out << indent << "FAREUSAGES\n";
  for (const FareUsage & fareUsage : _fareUsages)
    fareUsage.print(out, indentLevel + 1);

  out << indent << "TOTALAMOUNT: " << amountToDouble(_totalAmount) << "\n"
      << indent << "TOTALMARKUPAMOUNT: " << amountToDouble(_totalMarkupAmount) << "\n"
      << indent << "VALIDATINGCARRIER: " << _validatingCarrier << "\n";

  return out;
}
}
