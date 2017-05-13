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
#ifndef MIRROR_IMAGE_H
#define MIRROR_IMAGE_H

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class DateTime;
class PricingTrx;
class TaxCodeReg;
class TaxResponse;
class TaxSpecConfigReg;
class TravelSeg;

// ----------------------------------------------------------------------------
// <PRE>
//
// @class MirrorImage
// Description:
//
// </PRE>
// ----------------------------------------------------------------------------

class MirrorImage
{
  friend class MirrorImageTest;

public:
  MirrorImage();
  MirrorImage(const DateTime& date, const std::vector<TaxSpecConfigReg*>* const taxSpecConfigRegs);
  virtual ~MirrorImage();

  virtual bool isMirrorImage(const PricingTrx& trx,
                             const TaxResponse& taxResponse,
                             const TaxCodeReg& taxCodeReg,
                             uint16_t startIndex);

  void setTravelSeg(const std::vector<TravelSeg*>* seg) { _travelSeg=seg; };

protected:
  const std::vector<TaxSpecConfigReg*>* const _taxSpecConfigRegs;
  const bool _shouldCheckCityMirror;

private:
  MirrorImage(const MirrorImage& mi);
  MirrorImage& operator=(const MirrorImage& mi);

  const std::vector<TravelSeg*>* _travelSeg = nullptr;

  const std::vector<TravelSeg*>& getTravelSeg(const TaxResponse& taxResponse) const;
};
}

#endif
