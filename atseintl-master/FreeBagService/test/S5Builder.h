// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#ifndef S5BUILDER_H
#define S5BUILDER_H

#include "test/include/TestMemHandle.h"
#include "DBAccess/SubCodeInfo.h"

namespace tse
{

class S5Builder
{
  TestMemHandle* _memHandle;
  SubCodeInfo* _s5;

public:
  S5Builder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _s5 = _memHandle->create<SubCodeInfo>();
    _s5->fltTktMerchInd() = BAGGAGE_EMBARGO;
  }

  S5Builder& withFltTktMerchInd(const Indicator& ftmi)
  {
    _s5->fltTktMerchInd() = ftmi;
    return *this;
  }

  S5Builder& withFltTktMerchIndAndDescription(const Indicator& ftmi,
                                              const ServiceGroupDescription& description1,
                                              const ServiceGroupDescription& description2)
  {
    _s5->fltTktMerchInd() = ftmi;
    _s5->description1() = description1;
    _s5->description2() = description2;
    return *this;
  }

  S5Builder& withVendor(const VendorCode& vendor)
  {
    _s5->vendor() = vendor;
    return *this;
  }

  S5Builder& withCarrier(const CarrierCode& carrier)
  {
    _s5->carrier() = carrier;
    return *this;
  }

  S5Builder& withVendCarr(const VendorCode& vendor, const CarrierCode& carrier)
  {
    _s5->vendor() = vendor;
    _s5->carrier() = carrier;
    return *this;
  }

  S5Builder& withIndustryCarrier(const Indicator& industryCarrierInd = 'I')
  {
    _s5->industryCarrierInd() = industryCarrierInd;
    return *this;
  }


  S5Builder& withDesc(const ServiceGroupDescription& description1,
                             const ServiceGroupDescription& description2 = "")
  {
    _s5->description1() = description1;
    _s5->description2() = description2;
    return *this;
  }

  S5Builder& withTypeCode(const ServiceTypeCode& type)
  {
    _s5->serviceTypeCode() = type;
    return *this;
  }

  S5Builder& withSubCode(const ServiceSubTypeCode& subType)
  {
    _s5->serviceSubTypeCode() = subType;
    return *this;
  }

  S5Builder& withGroup(const ServiceGroup& group, const ServiceGroup& subGroup)
  {
    _s5->serviceGroup() = group;
    _s5->serviceSubGroup() = subGroup;
    return *this;
  }

  S5Builder& withGroup(const ServiceGroup& group)
  {
    _s5->serviceGroup() = group;
    return *this;
  }

  S5Builder& withSubGroup(const ServiceGroup& subGroup)
  {
    _s5->serviceSubGroup() = subGroup;
    return *this;
  }

  S5Builder& withCommercialName(const std::string& commercialName)
  {
    _s5->commercialName() = commercialName;
    return *this;
  }

  S5Builder& withConcur(const Indicator& concur)
  {
    _s5->concur() = concur;
    return *this;
  }

  S5Builder& withAllowanceMatched();
  S5Builder& withCodes(const Indicator& concur,
                       const Indicator& ssim,
                       const Indicator& rfic,
                       const Indicator& emd,
                       const ServiceBookingInd& booking,
                       const SubCodeSSR& ssr = "");

  SubCodeInfo* build()
  {
    const auto s5 = _s5;
    _s5 = _memHandle->create<SubCodeInfo>();
    _s5->fltTktMerchInd() = BAGGAGE_EMBARGO;
    return s5;
  }
};
}
#endif // S5BUILDER_H
