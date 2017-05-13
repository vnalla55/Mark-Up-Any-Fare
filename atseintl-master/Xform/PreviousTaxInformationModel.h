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

#include "Xform/AbstractPreviousTaxInformationModel.h"

namespace tse
{
class PricingTrx;

class PreviousTaxInformationModel : public AbstractPreviousTaxInformationModel
{
  std::vector<std::unique_ptr<AbstractTaxBreakdownModel>> _taxBreakdown;
  std::vector<std::unique_ptr<AbstractTaxBreakdownModel>> _taxInfoBreakdown;
  std::vector<std::unique_ptr<AbstractTaxInformationModel>> _taxInformation;
  bool _isEmpty;

  std::vector<std::unique_ptr<AbstractTaxBreakdownModel>> _netTaxBreakdown;
  std::vector<std::unique_ptr<AbstractTaxInformationModel>> _netTaxInformation;
  bool _isNetEmpty;

  PreviousTaxInformationModel(const PreviousTaxInformationModel&) = delete;
  PreviousTaxInformationModel& operator=(const PreviousTaxInformationModel&) = delete;

public:
  PreviousTaxInformationModel(PricingTrx& trx);

  const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&
  getTaxBreakdown() const override;

  const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&
  getTaxInfoBreakdown() const override;

  const std::vector<std::unique_ptr<AbstractTaxInformationModel>>&
  getTaxInformation() const override;
  const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&
  getNetTaxBreakdown() const override;
  const std::vector<std::unique_ptr<AbstractTaxInformationModel>>&
  getNetTaxInformation() const override;
  bool isEmpty() const override { return _isEmpty; }
  bool isNetEmpty() const override { return _isNetEmpty; }
};

} // end of tse namespace
