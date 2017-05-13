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
#ifndef TAX_APPLYYQ_H
#define TAX_APPLYYQ_H

#include <string>

#include <log4cxx/helpers/objectptr.h>
namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class PricingTrx;
class TaxResponse;

namespace YQYR
{
class ServiceFee;
class ServiceFeeRec1Validator;
// ----------------------------------------------------------------------------
// TaxApply will call all the TaxValidator functions for all special taxes
// ----------------------------------------------------------------------------

class TaxApply
{
public:
  TaxApply() : _travelSegStartIndex(0), _travelSegEndIndex(0) {}
  virtual ~TaxApply() {}

  void initializeTaxItem(PricingTrx& trx,
                         tse::YQYR::ServiceFee& tax,
                         TaxResponse& taxResponse,
                         tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator);

protected:
private:
  uint16_t _travelSegStartIndex;
  uint16_t _travelSegEndIndex;
  TaxApply(const TaxApply& apply);
  TaxApply& operator=(const TaxApply& apply);

  static log4cxx::LoggerPtr _logger;
};
}
}
#endif
