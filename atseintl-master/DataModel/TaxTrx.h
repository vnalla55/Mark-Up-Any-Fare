//-------------------------------------------------------------------
//
//  Description: Tax Transaction object
//
//  Updates:
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxType.h"
#include "DataModel/PfcDisplayRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxInfoRequest.h"
#include "DataModel/TaxInfoResponse.h"
#include "DataModel/TaxRequest.h"
#include "Service/Service.h"

#include <string>

namespace tse
{
class Itin;

using TaxRequestType = std::string;
extern const TaxRequestType OTA_REQUEST;
extern const TaxRequestType DISPLAY_REQUEST;
extern const TaxRequestType PFC_DISPLAY_REQUEST;
extern const TaxRequestType ATPCO_DISPLAY_REQUEST;
extern const TaxRequestType NON_OTA_REQUEST;
extern const TaxRequestType TAX_INFO_REQUEST;
extern const TaxRequestType NEW_OTA_REQUEST;

class TaxTrx : public PricingTrx
{
public:
  virtual bool process(Service& srv) override { return srv.process(*this); }

  bool convert(std::string& response) override;

  void setRequest(TaxRequest* request) { _request = request; }
  const TaxRequest* getRequest() const { return static_cast<TaxRequest*>(_request); }
  TaxRequest* getRequest() { return static_cast<TaxRequest*>(_request); }

  PfcDisplayRequest*& pfcDisplayRequest() { return _pfcDisplayRequest; }
  const PfcDisplayRequest* pfcDisplayRequest() const { return _pfcDisplayRequest; }

  TaxInfoRequest*& taxInfoRequest() { return _taxInfoRequest; }
  const TaxInfoRequest* taxInfoRequest() const { return _taxInfoRequest; }

  std::vector<TaxInfoResponse>& taxInfoResponseItems() { return _taxInfoResponseItems; }
  const std::vector<TaxInfoResponse> taxInfoResponseItems() const { return _taxInfoResponseItems; }

  LocCode& saleLoc() { return _saleLoc; }
  const LocCode& saleLoc() const { return _saleLoc; }

  std::map<uint16_t, std::string>& warningMsg() { return _warningMsg; }
  const std::map<uint16_t, std::string>& warningMsg() const { return _warningMsg; }

  std::string& errorMsg() { return _errorMsg; }
  const std::string& errorMsg() const { return _errorMsg; }

  std::ostringstream& response() { return _response; }

  boost::mutex& mutexWarningMsg() { return _mutexWarningMsg; }
  const boost::mutex& mutexWarningMsg() const { return _mutexWarningMsg; }
  TaxRequestType& requestType() { return _requestType; }
  const TaxRequestType& requestType() const { return _requestType; }

  std::string& otaRequestRootElementType() { return _otaRequestRootElementType; }
  const std::string& otaRequestRootElementType() const { return _otaRequestRootElementType; }

  std::string& taxDisplayRequestRootElementType() { return _taxDisplayRequestRootElementType; }
  const std::string& taxDisplayRequestRootElementType() const
  {
    return _taxDisplayRequestRootElementType;
  }

  std::string& otaXmlVersion() { return _otaXmlVersion; }
  const std::string& otaXmlVersion() const { return _otaXmlVersion; }

  bool isShoppingPath() const { return _shoppingPath; }
  void setShoppingPath(bool shoppingPath) { _shoppingPath = shoppingPath; }

  bool isRepricingForTaxShoppingDisabled() const
  {
    return isShoppingPath() && !_repricingForTaxShopping;
  }

  void setRepricingForTaxShopping(bool val) { _repricingForTaxShopping = val; }

  const std::vector<Itin*>& allItins() const { return _allItins; }
  std::vector<Itin*>& allItins() { return _allItins; }

  const std::vector<int>& shoppingItinMapping() const { return _shoppingItinMapping; }
  std::vector<int>& shoppingItinMapping() { return _shoppingItinMapping; }

  bool isGroupItins() const { return _groupItins; }
  void setGroupItins(bool groupItins) { _groupItins = groupItins; }

private:
  LocCode _saleLoc;

  std::map<uint16_t, std::string> _warningMsg;
  std::string _errorMsg;

  std::ostringstream _response;

  TaxRequestType _requestType;

  PfcDisplayRequest* _pfcDisplayRequest = nullptr;

  TaxInfoRequest* _taxInfoRequest = nullptr;
  std::vector<TaxInfoResponse> _taxInfoResponseItems;

  std::string _otaRequestRootElementType;
  std::string _taxDisplayRequestRootElementType;

  std::string _otaXmlVersion;

  boost::mutex _mutexWarningMsg;
  bool _shoppingPath = false;
  bool _groupItins = false;
  bool _repricingForTaxShopping = false;
  std::vector<Itin*> _allItins;
  std::vector<int> _shoppingItinMapping;
}; // class TaxTrx
} // tse namespace
