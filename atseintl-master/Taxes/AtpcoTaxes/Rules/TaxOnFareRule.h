// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include <set>
#include <vector>

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{

class TaxOnFareApplicator;
class Request;
class Services;

class TaxOnFareRule : public BusinessRule
{
public:
  typedef TaxOnFareApplicator ApplicatorType;
  TaxOnFareRule(type::NetRemitApplTag const& netRemitApplTag,
                type::TaxAppliesToTagInd const& taxAppliesToTagInd,
                type::Index const& itemNo,
                type::Vendor const& vendor,
                type::TaxProcessingApplTag processingApplTag);

  virtual ~TaxOnFareRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  type::NetRemitApplTag const& netRemitApplTag() const { return _netRemitApplTag; }
  type::TaxAppliesToTagInd const& taxAppliesToTagInd() const { return _taxAppliesToTagInd; }
  type::TaxProcessingApplTag const& processingApplTag() const { return _processingApplTag; }
  type::Index const& itemNo() const { return _itemNo; }

private:
  type::NetRemitApplTag _netRemitApplTag;
  type::TaxAppliesToTagInd _taxAppliesToTagInd;
  type::Index _itemNo;
  type::Vendor _vendor;
  type::TaxProcessingApplTag _processingApplTag;
};
}
