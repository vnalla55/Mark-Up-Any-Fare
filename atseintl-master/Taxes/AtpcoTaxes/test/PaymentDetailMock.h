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

#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"

namespace tax
{
class PaymentDetailMock : public PaymentDetail
{
public:
  PaymentDetailMock(type::TaxAppliesToTagInd taxAppliesToTag = type::TaxAppliesToTagInd::Blank);
  virtual ~PaymentDetailMock();

  void setLoc1Stopover(bool result);
  void setLoc1FareBreak(bool result);
  void setTaxPointBegin(const Geo& result);
  void setMustBeTicketed(bool result);
  void setTaxName(const TaxName& result);

  void setTaxableUnit(const type::TaxableUnit& taxableUnit);

  virtual bool isLoc1Stopover() const;
  virtual bool isLoc1FareBreak() const;
  virtual bool mustBeTicketed() const;
  virtual const TaxName& taxName() const;

private:
  bool _isLoc1Stopover;
  bool _isLoc1FareBreak;
  const Geo* _myLoc1;
  bool _mustBeTicketed;
  type::SeqNo _newSeqNo;
  Geo _taxPointBegin;
  Geo _taxPointLoc2;
  TaxName _taxName;
};

} // namespace tax
