//----------------------------------------------------------------------------
//  File:           PfcDisplayDataPXA.h
//  Authors:        Piotr Lach
//  Created:        5/22/2008
//  Description:    PfcDisplayDataPXA header file for ATSE V2 PFC Display Project.
//                  Requested data for PXA entry functionality.
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
#ifndef PFC_DISPLAY_DATA_PXA_H
#define PFC_DISPLAY_DATA_PXA_H

#include "Taxes/Pfc/PfcDisplayData.h"

namespace tse
{

class PfcDisplayDataPXA : public PfcDisplayData
{
public:
  enum CarrierId
  {
    FIRST_CARRIER,
    SECOND_CARRIER
  };

  PfcDisplayDataPXA(TaxTrx* trx, PfcDisplayDb* db);
  virtual ~PfcDisplayDataPXA();

  const std::vector<PfcAbsorb*>& getPfcAbsorb() const;
  const MoneyAmount getFareAmt(LocCode key) const;
  const MoneyAmount getTaxAmt(LocCode key) const;
  void releasePfcAbsorbData() { _pfcAbsorbV = nullptr; }
  void setCarrier(CarrierId id);
  bool isPNR() const
  {
    if (trx()->pfcDisplayRequest()->isPNR())
    {
      return trx()->pfcDisplayRequest()->isPNR();
    }
    else
    {
      if (!trx()->pfcDisplayRequest()->segments().empty())
      {
        PfcDisplayRequest::Segment segment = trx()->pfcDisplayRequest()->segments().front();
        if (!std::get<PfcDisplayRequest::CARRIER_CODE>(segment).empty())
        {
          return true;
        }
      }
    }

    return false;
  }

  // ----------------------------------------------------------------------------
  // <PRE>
  //
  // @function PfcDisplayBuilderPXA::ifNotEqualitySet
  //
  // Description: If currentFields == prevFields set new prevFields
  //
  // </PRE>
  // ----------------------------------------------------------------------------
  template <typename T1, typename T2, typename T3, typename T4>
  bool ifNotEqualitySet(const T1& currentField1,
                        const T2& currentField2,
                        const T3& currentField3,
                        const T4& currentField4,
                        T1& prevField1,
                        T2& prevField2,
                        T3& prevField3,
                        T4& prevField4) const
  {
    if (currentField1 == prevField1 && currentField2 == prevField2 && currentField3 == prevField3 &&
        currentField4 == prevField4)
    {
      return false;
    }

    prevField1 = currentField1;
    prevField2 = currentField2;
    prevField3 = currentField3;
    prevField4 = currentField4;

    return true;
  }

protected:
  CarrierCode& carrier()
  {
    return _carrier;
  };
  const CarrierCode carrier() const
  {
    return _carrier;
  };

private:
  mutable std::vector<PfcAbsorb*>* _pfcAbsorbV;
  CarrierCode _carrier;
};

} // namespace tse
#endif
