//----------------------------------------------------------------------------
//  File:           PfcDisplayBuilderPXC.h
//  Authors:        Piotr Lach
//  Created:        4/14/2008
//  Description:    PfcDisplayBuilderPXC header file for ATSE V2 PFC Display Project.
//                  The object of this class build PXC message.
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
#ifndef PFC_DISPLAY_BUILDER_PXC_H
#define PFC_DISPLAY_BUILDER_PXC_H

#include "Taxes/Pfc/PfcDisplayBuilder.h"
#include "Taxes/Pfc/PfcDisplayDataPXC.h"
#include "Taxes/Pfc/PfcDisplayFormatter.h"
#include <vector>

namespace tse
{

class PfcDisplayBuilderPXC : public PfcDisplayBuilder
{
public:
  static const std::string TABLE_HEADER;
  static const std::string ITINERARY_HEADER;

  static const std::string CONNEX_CITY_LOCAL_ABSORPTION_SIGN;
  static const std::string ESSENTIAL_AIR_SVC_EXEMPTION_SIGN;
  static const std::string AIR_TAX_EXEMPTION_SIGN;
  static const std::string CHARTER_EXEMPTION_SIGN;
  static const std::string FREQUENT_FLYER_EXEMPTION_SIGN;

  static const std::string CONNEX_CITY_LOCAL_ABSORPTION_DESCRIPTION;
  static const std::string ESSENTIAL_AIR_SVC_EXEMPTION_DESCRIPTION;
  static const std::string AIR_TAX_EXEMPTION_DESCRIPTION;
  static const std::string CHARTER_EXEMPTION_DESCRIPTION;
  static const std::string FREQUENT_FLYER_EXEMPTION_DESCRIPTION;

  static const size_t SPECIAL_SIGN_NUMBER = 5;

  PfcDisplayBuilderPXC(TaxTrx* trx, PfcDisplayData* data);
  virtual ~PfcDisplayBuilderPXC();

protected:
  virtual std::string buildHeader() override;
  virtual std::string buildBody() override;
  virtual std::string buildFootnote() override;

  PfcDisplayDataPXC* data() override { return (PfcDisplayDataPXC*)_data; }
  const PfcDisplayDataPXC* data() const override { return (PfcDisplayDataPXC*)_data; }
  PfcDisplayFormatterPXC& fmt() { return _formatter; }
  const std::vector<PfcPFC*>& pfcV() { return _pfcV; }
  PfcDisplayDataPXC::WarningMap& pfcWarning() { return _warningMap; }

  virtual std::vector<std::string> specialSigns(PfcPFC& pfc);

private:
  std::string specialSignsDescription();

  PfcDisplayFormatterPXC _formatter;
  PfcDisplayDataPXC::WarningMap _warningMap;
  std::vector<PfcPFC*> _pfcV;

  friend class PfcDisplayBuilderPXCTest;
};

} // namespace tse
#endif
