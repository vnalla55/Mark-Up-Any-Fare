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

#pragma once

#include "Xform/AbstractTaxBreakdownModel.h"
#include "Xform/AbstractTaxInformationModel.h"

#include <memory>
#include <vector>

namespace tse
{
class AbstractPreviousTaxInformationModel
{
public:
  virtual const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&
  getTaxBreakdown() const = 0;
  virtual const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&
  getTaxInfoBreakdown() const = 0;
  virtual const std::vector<std::unique_ptr<AbstractTaxInformationModel>>&
  getTaxInformation() const = 0;
  virtual const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&
  getNetTaxBreakdown() const = 0;
  virtual const std::vector<std::unique_ptr<AbstractTaxInformationModel>>&
  getNetTaxInformation() const = 0;
  virtual bool isEmpty() const = 0;
  virtual bool isNetEmpty() const = 0;
  virtual ~AbstractPreviousTaxInformationModel() {}
};

} // end of tse namespace
