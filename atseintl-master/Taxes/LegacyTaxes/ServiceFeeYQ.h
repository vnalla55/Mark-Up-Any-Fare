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
#ifndef SERVICE_FEEYQ_H
#define SERVICE_FEEYQ_H

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/YQYR/YQYRCalculator.h"
#include "DBAccess/PaxTypeMatrix.h"
#include "Taxes/LegacyTaxes/TaxYQ.h"

#include <log4cxx/helpers/objectptr.h>

#include <list>
#include <vector>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class CarrierPreference;
class PricingTrx;
class TaxResponse;
class YQYRFees;

namespace YQYR
{
class ServiceFeeRec1Validator;
// ----------------------------------------------------------------------------
// <PRE>
//
// @class ServiceFee
// Description: ATPCO passses Fuel and Insurance Fees that are
//              required to be collected under S1 and S2 record conditions.
//              These surcharges will be validated and applied with this object.
//              The TaxResponse record Tax Items will be updated with the
//              qualifying Fuel and Insurnce Surcharges.
//
// </PRE>
// ----------------------------------------------------------------------------

class ServiceFee : public YQYR::Tax
{
public:
  friend class ServiceFeeYQTest;
  typedef std::list<PaxTypeCode> PaxTypeCodeList;
  typedef std::vector<const PaxTypeMatrix*> PaxTypeMatrixVector;

  void collectFees(PricingTrx& trx,
                   TaxResponse& taxResponse); // I couldn't find a more appropriate name...

  void applyCharges(PricingTrx &trx,
                    TaxResponse &taxResponse,
                    YQYRCalculator::YQYRFeesApplicationVec &fees);


private:
  void applyCharge(PricingTrx& trx,
                   TaxResponse& taxResponse,
                   tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator);

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex);

  void doTaxRound(PricingTrx& trx, tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator);
  void doAtpcoDefaultTaxRounding(tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator);

  static bool isNonRefundable(const TaxCode& taxCode, const CarrierPreference& carrierPreference);

  static const std::string SERVICE_FEE_DEFAULT_TEXT;
  static const std::string FUEL;
  static const std::string INSURANCE;
  static const std::string CARRIER_IMPOSED_MISC;

  static log4cxx::LoggerPtr _logger;
};
}
}
#endif
