//---------------------------------------------------------------------------
//  Copyright Sabre 2014
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
#ifndef TAX_SP1801_H
#define TAX_SP1801_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include <log4cxx/helpers/objectptr.h>
#include <set>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class TaxLocIterator;
class PfcMultiAirport;

class TaxSP1801 : public Tax
{
  friend class TaxAYTest;

public:
  TaxSP1801();
  virtual ~TaxSP1801();

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

  bool validateFareClass(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex) override;

  bool validateFinalGenericRestrictions(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        uint16_t& startIndex,
                                        uint16_t& endIndex) override;

protected:

  bool isConUS(const Loc* loc);
  bool isUS(const Loc* loc);
  bool isCoterminal(PricingTrx& trx, const PfcMultiAirport* pfcMultiAirport, const Loc* loc) const;
  void setStopTime(TaxLocIterator* locIt);
  void findCircleClosing(PricingTrx& trx, TaxResponse& taxResponse);
  bool isCT(PricingTrx& trx, TaxResponse& taxResponse, uint16_t travelSegIndex);

  uint16_t _validTransitLastSegOrder;
  std::vector<uint16_t> _circleClosings;
  bool _cicrleClosingInitialized;
  bool _isFMMHInternational;
  bool _treatTrainBusAsNonAir;

private:
  static log4cxx::LoggerPtr _logger;
};

} /* end tse namespace */

#endif /* TAX_SP1801_H */
