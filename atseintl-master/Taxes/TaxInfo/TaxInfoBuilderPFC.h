//----------------------------------------------------------------------------
//  File:           PfcInfo.h
//  Authors:        Piotr Lach
//  Created:        12/12/2008
//  Description:    PfcInfo header file for ATSE V2 PFC Display Project.
//                  Tax Info Display for Ticketing functionality.
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
#ifndef TAX_INFO_BUILDER_PFC_H
#define TAX_INFO_BUILDER_PFC_H

#include "Taxes/TaxInfo/TaxInfoBuilder.h"
#include "DBAccess/PfcPFC.h"
#include "Taxes/Pfc/PfcDisplayDb.h"

namespace tse
{

class IsValidDate : public std::unary_function<PfcPFC*, bool>
{
  DateTime _date;

public:
  explicit IsValidDate(const DateTime& date) : _date(date) {}

  bool operator()(const PfcPFC* pfc) const
  {
    return (pfc->effDate() <= _date && pfc->expireDate() >= _date);
  }
};

class IsValidAirport : public std::unary_function<PfcPFC*, bool>
{
  LocCode _loc;

public:
  IsValidAirport(const LocCode& loc) : _loc(loc) {}

  bool operator()(PfcPFC* pfc) const { return pfc->pfcAirport() == _loc; }
};

class TaxInfoBuilderPFC : public TaxInfoBuilder
{
  friend class TaxInfoBuilderPFCTest;

public:
  typedef TaxInfoBuilder::Response Response;

  static const TaxCode TAX_CODE;
  static const std::string TAX_NOT_FOUND;
  static const std::string TAX_DESCRIPTION;

  TaxInfoBuilderPFC();
  virtual ~TaxInfoBuilderPFC();

  void buildDetails(TaxTrx& trx) override;

protected:
  bool validateTax(TaxTrx& trx) override;
  virtual const std::vector<PfcPFC*>& getPfc(TaxTrx& trx, const LocCode& airport);

private:
  void buildItem(TaxTrx& trx, LocCode& airport);

  std::vector<PfcPFC*>* _pfcV;
};

} // namespace tse
#endif
