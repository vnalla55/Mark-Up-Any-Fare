#pragma once

#include "Common/FcConfig.h"
#include "DBAccess/NoPNROptions.h"

namespace tse
{

class FareCalcConfig;

namespace // string literals
{
std::string RO_RBD_TICKETING_OVERRIDE = "* RO - RBD TICKETING OVERRIDE";
std::string NO_FARES_FOR_CLASS_USED = "*NO FARES FOR CLASS USED";
std::string NO_FARES_RBD_CARRIER = "*NO FARES/RBD/CARRIER";
std::string VERIFY_BOOKING_CLASS = "VERIFY BOOKING CLASS";
std::string ATTN = "ATTN*";
std::string BOOK_OPTION_OF_CHOICE_IN_APPL_BOOK_CLASS =
    "BOOK OPTION OF CHOICE IN APPLICABLE BOOKING CLASS";
std::string NO_MATCH_BOOKING_CLASS = "WPA NO MATCH BOOKING CLASS";
}

class NoPNRFcConfigWrapper : public FcConfig
{
public:
  NoPNRFcConfigWrapper(const FcConfig* fcConf,
                       const FcConfig** pointerToUpdateAtDestruction,
                       const NoPNROptions* nopOptions)
    : FcConfig()
  {
    _pointerToUpdateAtDestruction = pointerToUpdateAtDestruction;
    if (fcConf)
    {
      _wrapped = fcConf;
      _fcc = fcConf->_fcc;
      _trxType = fcConf->_trxType;
      _fccText = fcConf->_fccText;
    }
    else
    {
      _wrapped = nullptr;
      _fcc = nullptr;
      _fccText = nullptr;
    }
    _noPnrOptions = nopOptions;
    // update the pointer, it will be changed back in the destruction
    if (pointerToUpdateAtDestruction && *pointerToUpdateAtDestruction)
      *pointerToUpdateAtDestruction = this;
  }
  ~NoPNRFcConfigWrapper()
  {
    if (nullptr != _pointerToUpdateAtDestruction && nullptr != _wrapped)
      (*_pointerToUpdateAtDestruction) = _wrapped;
  }

  const FcConfig* _wrapped;
  const FcConfig** _pointerToUpdateAtDestruction;
  const NoPNROptions* _noPnrOptions;

  virtual bool getMsgAppl(FareCalcConfigText::TextAppl appl, std::string& msg) const override
  {
    switch (appl)
    {
    case FareCalcConfigText::WPA_RO_INDICATOR:
      if (_noPnrOptions->applyROInFareDisplay() == 'N')
        return false;

      msg = RO_RBD_TICKETING_OVERRIDE;
      break;

    case FareCalcConfigText::WPA_NO_MATCH_NO_FARE:
      if (_noPnrOptions->noMatchNoFaresErrorMsg() == '1')
      {
        msg = NO_FARES_FOR_CLASS_USED;
      }
      else
      {
        msg = NO_FARES_RBD_CARRIER;
      }
      break;

    case FareCalcConfigText::WPA_NO_MATCH_VERIFY_BOOKING_CLASS:
      msg = VERIFY_BOOKING_CLASS;
      break;

    case FareCalcConfigText::WPA_NO_MATCH_REBOOK:
      if (_noPnrOptions->rbdNoMatchTrailerMsg() == '3')
        return false; // no message
      if (_noPnrOptions->rbdNoMatchTrailerMsg() == '1')
      {
        msg = ATTN;
      }
      msg += BOOK_OPTION_OF_CHOICE_IN_APPL_BOOK_CLASS;
      break;

    case FareCalcConfigText::WPA_NO_MATCH_BOOKING_CLASS:
      msg = NO_MATCH_BOOKING_CLASS;
      break;

    default:
      return false;
    }
    return true;
  }
};

} // namespace tse

