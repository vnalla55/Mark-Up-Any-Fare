//----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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

#pragma once

#include "Common/Money.h"
#include "Common/XMLConstruct.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "Xform/PricingResponseFormatter.h"

namespace tse
{

class TktFeesPricingResponseFormatter : public PricingResponseFormatter
{
public:
  //--------------------------------------------------------------------------
  // @function TktFeesPricingResponseFormatter::formatResponse
  //
  // Description: Prepare a OB Ticketing Fees Fee detail response XML for client
  //
  //--------------------------------------------------------------------------
  virtual std::string formatResponse(
      const std::string& responseString,
      TktFeesPricingTrx& tktFeesTrx,
      ErrorResponseException::ErrorResponseCode errCode = ErrorResponseException::NO_ERROR);

  void formatResponse(const ErrorResponseException& ere, std::string& response);

  virtual void formatTktFeesResponse(XMLConstruct& construct, TktFeesPricingTrx& tktFeesTrx);

  virtual void prepareResponseText(const std::string& responseString,
                                   XMLConstruct& construct,
                                   bool noSizeLImit = false) const;

  void buildTktFeesResponse(TktFeesPricingTrx& tktFeesTrx, Itin* itin, XMLConstruct& constructA);

private:
  Itin*& currentItin() { return _itin; }
  const Itin* currentItin() const { return _itin; }

  void prepareHostPortInfo(PricingTrx& tktFeesTrx, XMLConstruct& construct);

  Itin* _itin = nullptr;
  bool _isHeader = false;

}; // End class TktFeesPricingResponseFormatter
} // End namespace tse

