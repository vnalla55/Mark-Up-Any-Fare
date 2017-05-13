#include "Common/TseConsts.h"
#include "FreeBagService/test/S5Builder.h"
namespace tse
{
S5Builder&
S5Builder::withAllowanceMatched()
{
  _s5->serviceTypeCode() = "OC";
  _s5->serviceGroup() = "BG";
  _s5->serviceSubTypeCode() = "0DF";
  _s5->fltTktMerchInd() = BAGGAGE_ALLOWANCE;
  _s5->concur() = 'X';
  _s5->rfiCode() = 'C';
  _s5->ssimCode() = ' ';
  _s5->ssrCode() = "";
  _s5->emdType() = '4';
  _s5->bookingInd() = "";
  return *this;
}

S5Builder&
S5Builder::withCodes(const Indicator& concur,
                     const Indicator& ssim,
                     const Indicator& rfic,
                     const Indicator& emd,
                     const ServiceBookingInd& booking,
                     const SubCodeSSR& ssr)
{
  _s5->concur() = concur;
  _s5->ssimCode() = ssim;
  _s5->ssrCode() = ssr;
  _s5->rfiCode() = rfic;
  _s5->emdType() = emd;
  _s5->bookingInd() = booking;
  return *this;
}
}
