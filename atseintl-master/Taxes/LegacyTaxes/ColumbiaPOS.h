// ---------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------
#ifndef COLUMBIA_POS_H
#define COLUMBIA_POS_H

#include <set>

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class PricingTrx;
class TaxResponse;
class Logger;

// ----------------------------------------------------------------------------
// <PRE>
//
// @class Columbia Point of Sale Exception - whether to charge US Taxes
// Description:
//
// </PRE>
// ----------------------------------------------------------------------------

class ColumbiaPOS
{
  friend class ColumbiaPOSTest;

public:
  ColumbiaPOS();
  virtual ~ColumbiaPOS();

  const bool& canadianPt() const { return _canadianPt; }
  const bool& usPoint() const { return _usPoint; }
  const bool& mexicanPt() const { return _mexicanPt; }
  const bool& otherIntl() const { return _otherIntl; }

  bool chargeUSTaxes(PricingTrx& trx, TaxResponse& taxResponse);

  bool chargeMXTaxes(PricingTrx& trx, TaxResponse& taxResponse);

  bool chargeAllTaxes(PricingTrx& trx, TaxResponse& taxResponse);

  class CarrierList
  {
    friend class ColumbiaPOSTest;
    friend class CarrierListMock;

  public:
    CarrierList(const std::string& paramName);
    virtual ~CarrierList();

    bool operator()(const CarrierCode&) const;
    bool operator()(const CarrierCode&, const CarrierCode&) const;

  private:
    CarrierList();

    void getCarriers(const std::string& paramName,
                     std::set<std::string>& _carriers,
                     tse::ConfigMan* config = &(Global::config()));

    virtual void
    parseCarriers(const std::string& carriersStr, std::set<std::string>& _carriers) const;

    std::set<std::string> _carriers;
  };

private:
  static const CarrierList NotChargeUSTaxes;
  static const CarrierList ChargeAllTaxes;
  static const CarrierList ChargeMXTaxes;
  static const CarrierList ChargeUSTaxes;

  bool _canadianPt;
  bool _usPoint;
  bool _mexicanPt;
  bool _otherIntl;

  static Logger _logger;

  ColumbiaPOS(const ColumbiaPOS& to);
  ColumbiaPOS& operator=(const ColumbiaPOS& to);
};

} /* end tse namespace */

#endif /* COLUMBIA_POS_H */
