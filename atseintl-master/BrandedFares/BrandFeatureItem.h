//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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
#include "Common/TsePrimitiveTypes.h"

#include <boost/noncopyable.hpp>

namespace tse
{

// ATPCO Features Table 166 item
class BrandFeatureItem : boost::noncopyable
{
public:
  void setSequenceNumber(const SequenceNumberLong sequenceNumber)
  { _sequenceNumber = sequenceNumber; }
  SequenceNumberLong getSequenceNumber() const { return _sequenceNumber; }

  // RFISC
  void setSubCode(const ServiceSubTypeCode& subCode) { _subCode = subCode; }
  const ServiceSubTypeCode& getSubCode() const { return _subCode; }

  void setCommercialName(const std::string& name) { _commercialName = name; }
  const std::string& getCommercialName() const { return _commercialName; }

  void setServiceType(const char serviceType) { _serviceType = serviceType; }
  char getServiceType() const { return _serviceType; }

  void setApplication(const char application) { _application = application; }
  char getApplication() const { return _application; }

private:
  SequenceNumberLong _sequenceNumber = 0;
  ServiceSubTypeCode _subCode;
  std::string _commercialName;
  char _serviceType = ' ';
  char _application = ' ';
};

};
