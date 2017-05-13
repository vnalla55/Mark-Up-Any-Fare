#pragma once

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/VecMap.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"

#include <vector>

namespace tse
{

struct SOPInfo
{
  std::vector<PaxTypeFare*> ptfsToValidate;
  int _sopIndex;
  size_t _flightBitmapIndex;

  size_t queueRank() const { return _flightBitmapIndex; }

private:
  typedef VecMap<uint64_t, PaxTypeFare::FlightBitmap> DurationFltBitmap;
  std::map<uint64_t, char> _durationStatus;
  char _status;

public:
  char getStatus(const uint64_t duration = 0) const
  {
    if (duration == 0)
      return _status;
    else
    {
      std::map<uint64_t, char>::const_iterator it = _durationStatus.find(duration);
      if (LIKELY(it != _durationStatus.end()))
        return it->second;
      else
        return 'N';
    }
  }

  bool isStatusValid(const uint64_t duration = 0) const { return getStatus(duration) != 'N'; }

  int getSOPValidIndex() const
  {
    TSE_ASSERT(_sopIndex != -1);
    return _sopIndex;
  }

  void initialize(const ShoppingTrx& trx,
                  PaxTypeFare* ptf,
                  uint16_t legIndex,
                  size_t flbIdx,
                  uint32_t carrierKey,
                  const SOPUsages& sopUsages)
  {
    _flightBitmapIndex = flbIdx;

    if (trx.isAltDates())
    {
      _sopIndex = -1;
      _status = 'N';

      const DurationFltBitmap& durFltBitmap = ptf->durationFlightBitmapPerCarrier()[carrierKey];
      int sopID = sopUsages[flbIdx].origSopId_;

      for (const auto& elem : durFltBitmap)
      {
        const PaxTypeFare::FlightBitmap& fltBMap = elem.second;
        TSE_ASSERT(fltBMap[flbIdx]._flightBit != 0 || sopUsages[flbIdx].applicable_);

        const uint64_t duration = elem.first;

        switch (fltBMap[flbIdx]._flightBit)
        {
        case 0:
          if (_status != 'S')
            _status = 0;
          _durationStatus[duration] = 0;
          break;

        case 'S':
          _status = 'S';
          _durationStatus[duration] = 'S';
          break;

        default:
          _durationStatus[duration] = 'N';
          break;
        }
      }

      if (_status != 'N')
        _sopIndex = ShoppingUtil::findInternalSopId(trx, legIndex, sopID);
      if (_status == 'S')
        ptfsToValidate.push_back(ptf);
    }
    else
    {
      char flightBit = ptf->flightBitmap()[flbIdx]._flightBit;
      _sopIndex = -1;

      switch (flightBit)
      {
      case 0:
        _status = 0;
        break;

      case 'S':
        _status = 'S';
        ptfsToValidate.push_back(ptf);
        break;

      default:
        _status = 'N';
        break;
      }

      if (_status != 'N')
        _sopIndex =
            ShoppingUtil::findInternalSopId(trx, legIndex, sopUsages[flbIdx].origSopId_); // sopID);
    }
  }

  void update(ShoppingTrx& trx, PaxTypeFare* ptf, size_t flbIdx, uint32_t carrierKey)
  {
    if (trx.isAltDates())
    {
      if (_status == 'N')
        return;

      const DurationFltBitmap& durFltBitmap = ptf->durationFlightBitmapPerCarrier()[carrierKey];

      bool addPaxTypeFare(false);
      char status = 'N';

      for (const auto& elem : durFltBitmap)
      {
        char flightBit = elem.second[flbIdx]._flightBit;
        const uint64_t duration = elem.first;

        if (_durationStatus[duration] == 'N')
          continue;

        switch (flightBit)
        {
        case 0:
          if (status != 'S')
            status = 0;
          break;
        case 'S':
          _durationStatus[duration] = 'S';
          status = 'S';
          addPaxTypeFare = true;
          break;
        default:
          _durationStatus[duration] = 'N';
        }
      }
      _status = status;

      if (addPaxTypeFare)
        ptfsToValidate.push_back(ptf);
      if (_status == 'N')
        ptfsToValidate.clear();
    }
    else
    {
      char flightBit = ptf->flightBitmap()[flbIdx]._flightBit;
      // Nothing to do, leave status as is
      if (_status == 'N' || flightBit == 0)
        return;

      if (flightBit == 'S')
      {
        _status = 'S';
        ptfsToValidate.push_back(ptf);
      }
      else
      {
        _status = 'N';
        ptfsToValidate.clear();
      }
    }
  }
}; // class SOPInfo
}

