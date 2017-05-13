//-------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "DataModel/NoPNRPricingTrx.h"

#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/NoPNRFareTypeGroup.h"
#include "DBAccess/NoPNROptions.h"
#include "Xform/NoPNRPricingResponseFormatter.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/XMLConvertUtils.h"


using namespace std;
namespace tse
{
static Logger
logger("atseintl.DataModel.NoPNRPricingTrx.h");

const char* NoPNRPricingTrx::FareTypes::FTGC_NONE = "NONE";
const char* NoPNRPricingTrx::FareTypes::FTGC_BUSINESS = "BUSINESS";
const char* NoPNRPricingTrx::FareTypes::FTGC_ECONOMY = "ECONOMY";
const char* NoPNRPricingTrx::FareTypes::FTGC_ECONOMY_EXCURSION = "ECONOMY EXCURSION";
const char* NoPNRPricingTrx::FareTypes::FTGC_ECONOMY_ADVANCE_PURCHASE = "ECONOMY ADVANCE PURCHASE";
const char* NoPNRPricingTrx::FareTypes::FTGC_ECONOMY_INSTANT_PURCHASE = "ECONOMY INSTANT PURCHASE";
const char* NoPNRPricingTrx::FareTypes::FTGC_FIRST = "FIRST";
const char* NoPNRPricingTrx::FareTypes::FTGC_PREMIUM_ECONOMY = "PREMIUM ECONOMY";
const char* NoPNRPricingTrx::FareTypes::FTGC_PREMIUM_FIRST = "PREMIUM FIRST";
const char* NoPNRPricingTrx::FareTypes::FTGC_PROMOTIONAL = "PROMOTIONAL";
const char* NoPNRPricingTrx::FareTypes::FTGC_SPECIAL = "SPECIAL";

const char* NoPNRPricingTrx::FareTypes::FTGC[] = {
  NoPNRPricingTrx::FareTypes::FTGC_NONE,
  NoPNRPricingTrx::FareTypes::FTGC_BUSINESS,
  NoPNRPricingTrx::FareTypes::FTGC_ECONOMY,
  NoPNRPricingTrx::FareTypes::FTGC_ECONOMY_EXCURSION,
  NoPNRPricingTrx::FareTypes::FTGC_ECONOMY_ADVANCE_PURCHASE,
  NoPNRPricingTrx::FareTypes::FTGC_ECONOMY_INSTANT_PURCHASE,
  NoPNRPricingTrx::FareTypes::FTGC_FIRST,
  NoPNRPricingTrx::FareTypes::FTGC_PREMIUM_ECONOMY,
  NoPNRPricingTrx::FareTypes::FTGC_PREMIUM_FIRST,
  NoPNRPricingTrx::FareTypes::FTGC_PROMOTIONAL,
  NoPNRPricingTrx::FareTypes::FTGC_SPECIAL
};

size_t
NoPNRPricingTrx::FareTypes::hashFT::
operator()(const FareType& ftp) const
{
  union
  {
    char chr[sizeof(size_t)];
    size_t integral;
  } data;
  strncpy(data.chr, ftp.c_str(), sizeof(size_t));
  return data.integral;
}

void
NoPNRPricingTrx::FareTypes::loadFareTypes()
{
  if (_trx.isFullFBCItin())
    return;

  if (_trx.noPNROptions() != nullptr)
  {
    const std::vector<NoPNROptionsSeg*>& segments = _trx.noPNROptions()->segs();
    // unsigned int k = 0;
    if (segments.empty())
      throw ErrorResponseException(ErrorResponseException::NO_NOPNROPTIONS_FOR_TRX);
    vector<NoPNROptionsSeg*>::const_iterator segIt;
    for (segIt = segments.begin(); segIt != segments.end(); segIt++)
    {
      const NoPNRFareTypeGroup* ftg =
          _trx.dataHandle().getNoPNRFareTypeGroup((*segIt)->fareTypeGroup());
      if (ftg == nullptr)
        throw ErrorResponseException(ErrorResponseException::NO_NOPNROPTIONS_FOR_TRX);
      const vector<FareTypeMatrix*>& segs = (*ftg).segs();
      vector<FareTypeMatrix*>::const_iterator ftmIt;
      for (ftmIt = segs.begin(); ftmIt != segs.end(); ftmIt++) //, k++)
      {
        _fareTypes.insert(
            std::pair<FareType, FareTypeGroup>((*ftmIt)->fareType(), ftg->fareTypeGroup()));
      }
    } // end loop
  }
  else
    throw ErrorResponseException(ErrorResponseException::NO_NOPNROPTIONS_FOR_TRX);
}

NoPNRPricingTrx::FareTypes::FTGroup
NoPNRPricingTrx::FareTypes::getFareTypeGroup(const FareType& ftp)
{
  HashMap::iterator it = _fareTypes.find(ftp);
  if (it != _fareTypes.end())
    return (FTGroup)it->second;

  return NoPNRPricingTrx::FareTypes::FTG_NONE;
}

NoPNRPricingTrx::Solutions::PaxTypeInfo*
NoPNRPricingTrx::Solutions::element(PaxType* paxType)
{
  std::map<PaxType*, PaxTypeInfo*>::iterator iter = _info.find(paxType);

  if (iter != _info.end())
    return iter->second;

  throw std::out_of_range("invalid paxType");
}

void
NoPNRPricingTrx::Solutions::limit(int solutions)
{
  const std::vector<PaxType*>& paxTypes = _trx.paxType();

  std::vector<PaxType*>::const_iterator iter = paxTypes.begin();
  std::vector<PaxType*>::const_iterator iterEnd = paxTypes.end();
  while (iter != iterEnd)
  {
    PaxTypeInfo* info = element(*iter);

    int limit = solutions;
    int found = info->_found;
    if (found + limit > _max)
      limit = std::max(_max - found, 0);
    info->_limit = limit;

    ++iter;
  }
}

void
NoPNRPricingTrx::Solutions::found(PaxType* paxType, int solutions, bool add /* = false */)
{
  int& count = element(paxType)->_found;
  if (add)
    count += solutions;
  else
    count = solutions;
}

int
NoPNRPricingTrx::Solutions::count() const
{
  int count = 0;
  std::map<PaxType*, PaxTypeInfo*>::const_iterator iter = _info.begin();
  std::map<PaxType*, PaxTypeInfo*>::const_iterator iterEnd = _info.end();
  while (iter != iterEnd)
  {
    count += iter->second->_found;
    ++iter;
  }
  return count;
}

void
NoPNRPricingTrx::Solutions::initialize()
{
  const std::vector<PaxType*>& paxTypes = _trx.paxType();
  std::vector<PaxType*>::const_iterator iter = paxTypes.begin();
  std::vector<PaxType*>::const_iterator iterEnd = paxTypes.end();
  while (iter != iterEnd)
  {
    PaxTypeInfo* info;
    _trx.dataHandle().get(info);

    info->_process = true;
    info->_found = 0;
    info->_limit = 0;

    _info[*iter] = info;
    ++iter;
  }
  _max = _trx.noPNROptions()->maxNoOptions();
  _maxAll = static_cast<int>(_max * paxTypes.size());
}

void
NoPNRPricingTrx::Solutions::clear()
{
  std::map<PaxType*, PaxTypeInfo*>::const_iterator iter = _info.begin();
  std::map<PaxType*, PaxTypeInfo*>::const_iterator iterEnd = _info.end();
  while (iter != iterEnd)
  {
    PaxTypeInfo* info = iter->second;
    if (info)
    {
      info->_process = true;
      info->_found = 0;
      info->_limit = 0;
    }
    ++iter;
  }
}

void
NoPNRPricingTrx::loadNoPNROptions()
{
  _noPNROptions = getNoPNROptions(getRequest()->ticketingAgent(), dataHandle());
  if (_noPNROptions == nullptr)
    throw ErrorResponseException(ErrorResponseException::NO_NOPNROPTIONS_FOR_TRX);
}

NoPNROptions*
NoPNRPricingTrx::getNoPNROptions(const Agent* agent, DataHandle& dataHandle)
{
  if (!agent)
  {
    return nullptr;
  }

  const Loc* loc = dataHandle.getLoc(agent->agentCity(), time(nullptr));
  if (!loc)
  {
    return nullptr;
  }

  Indicator userApplType = CRS_USER_APPL;
  UserApplCode userAppl = "";

  std::string csr = agent->vendorCrsCode();
  if (csr.empty())
  {
    if (agent->hostCarrier().empty() || agent->hostCarrier() == "AA")
    {
      csr = agent->cxrCode();
    }
    else
    {
      userAppl = agent->hostCarrier();
      userApplType = MULTIHOST_USER_APPL;
    }
  }
  if (!csr.empty())
  {
    if (csr == INFINI_MULTIHOST_ID)
    {
      userAppl = INFINI_USER;
    }
    else if (csr == AXESS_MULTIHOST_ID)
    {
      userAppl = AXESS_USER;
    }
    else if (csr == ABACUS_MULTIHOST_ID)
    {
      userAppl = ABACUS_USER;
    }
    else if (csr == SABRE_MULTIHOST_ID)
    {
      userAppl = SABRE_USER;
    }
  }

  const vector<NoPNROptions*>& opt = dataHandle.getNoPNROptions(userApplType, userAppl);

  vector<NoPNROptions*>::const_iterator iter = opt.begin();
  vector<NoPNROptions*>::const_iterator end = opt.end();
  int optPriority = 100;
  NoPNROptions* opts = nullptr;
  for (; iter != end; ++iter)
  {
    NoPNROptions* npro = *iter;
    if (!npro->userAppl().empty() &&
        ((npro->loc1().loc().empty() && npro->loc1().locType() == ' ') ||
         (LocUtil::isInLoc(
             *loc, npro->loc1().locType(), npro->loc1().loc(), Vendor::SABRE, MANUAL))))
    {
      switch (npro->loc1().locType())
      {
      case LOCTYPE_CITY:
        opts = npro;
        optPriority = 0;
        break;
      case LOCTYPE_STATE:
        if (optPriority > 1)
        {
          opts = npro;
          optPriority = 1;
        }
        break;
      case LOCTYPE_NATION:
        if (optPriority > 2)
        {
          opts = npro;
          optPriority = 2;
        }
        break;
      case LOCTYPE_ZONE:
        if (optPriority > 3)
        {
          opts = npro;
          optPriority = 3;
        }
        break;
      case LOCTYPE_SUBAREA:
        if (optPriority > 4)
        {
          opts = npro;
          optPriority = 4;
        }
        break;
      case LOCTYPE_AREA:
        if (optPriority > 5)
        {
          opts = npro;
          optPriority = 5;
        }
        break;
      case ' ':
        if (optPriority > 6)
        {
          opts = npro;
          optPriority = 6;
        }
        break;
      }
    }
    else if (npro->userAppl().empty() &&
             ((npro->loc1().loc().empty() && npro->loc1().locType() == ' ') ||
              (LocUtil::isInLoc(
                  *loc, npro->loc1().locType(), npro->loc1().loc(), Vendor::SABRE, MANUAL))))
    {
      switch (npro->loc1().locType())
      {
      case LOCTYPE_CITY:
        if (optPriority > 7)
        {
          opts = npro;
          optPriority = 7;
        }
        break;
      case LOCTYPE_STATE:
        if (optPriority > 8)
        {
          opts = npro;
          optPriority = 8;
        }
        break;
      case LOCTYPE_NATION:
        if (optPriority > 9)
        {
          opts = npro;
          optPriority = 9;
        }
        break;
      case LOCTYPE_ZONE:
        if (optPriority > 10)
        {
          opts = npro;
          optPriority = 10;
        }
        break;
      case LOCTYPE_SUBAREA:
        if (optPriority > 11)
        {
          opts = npro;
          optPriority = 11;
        }
        break;
      case LOCTYPE_AREA:
        if (optPriority > 12)
        {
          opts = npro;
          optPriority = 12;
        }
        break;
      case ' ':
        if (optPriority > 13)
        {
          opts = npro;
          optPriority = 13;
        }
        break;
      }
    }
  }
  return opts;
}

NoPNRPricingTrx::FareTypes::FTGroup
NoPNRPricingTrx::mapFTtoFTG(const std::string& diagQlf) const
{
  if (!diagQlf.compare("PF"))
    return NoPNRPricingTrx::FareTypes::FTG_PREMIUM_FIRST;
  else if (!diagQlf.compare("F"))
    return NoPNRPricingTrx::FareTypes::FTG_FIRST;
  else if (!diagQlf.compare("B"))
    return NoPNRPricingTrx::FareTypes::FTG_BUSINESS;
  else if (!diagQlf.compare("PE"))
    return NoPNRPricingTrx::FareTypes::FTG_PREMIUM_ECONOMY;
  else if (!diagQlf.compare("E"))
    return NoPNRPricingTrx::FareTypes::FTG_ECONOMY;
  else if (!diagQlf.compare("EE"))
    return NoPNRPricingTrx::FareTypes::FTG_ECONOMY_EXCURSION;
  else if (!diagQlf.compare("EA"))
    return NoPNRPricingTrx::FareTypes::FTG_ECONOMY_ADVANCE_PURCHASE;
  else if (!diagQlf.compare("EI"))
    return NoPNRPricingTrx::FareTypes::FTG_ECONOMY_INSTANT_PURCHASE;
  else if (!diagQlf.compare("S"))
    return NoPNRPricingTrx::FareTypes::FTG_SPECIAL;
  else if (!diagQlf.compare("P"))
    return NoPNRPricingTrx::FareTypes::FTG_PROMOTIONAL;
  else
    return NoPNRPricingTrx::FareTypes::FTG_NONE;
}

void
NoPNRPricingTrx::prepareNoMatchItin()
{
  const BookingCode NO_RBD_BOOKINGCODE('1');

  _entryInfos.clear();

  std::vector<TravelSeg*>::iterator iter = _travelSeg.begin();
  std::vector<TravelSeg*>::iterator iterEnd = _travelSeg.end();
  while (iter != iterEnd)
  {
    NoPNRPricingTrx::SegmentInfo* info;
    dataHandle().get(info);
    _entryInfos.push_back(info);

    TravelSeg* seg = *iter;
    if (seg)
    {
      CabinType& cabin = seg->bookedCabin();

      info->_bookedCabin = cabin;
      cabin.setEconomyClass(); // set to lowest cabin

      info->_bookingCode = seg->getBookingCode();
      seg->setBookingCode(NO_RBD_BOOKINGCODE);
    }
    ++iter;
  }
}

void
NoPNRPricingTrx::restoreOldItin()
{
  std::vector<SegmentInfo*>::iterator infos = _entryInfos.begin();
  std::vector<TravelSeg*>::iterator iter = _travelSeg.begin();
  std::vector<TravelSeg*>::iterator iterEnd = _travelSeg.end();
  while (iter != iterEnd)
  {
    NoPNRPricingTrx::SegmentInfo* info = *infos;
    TravelSeg* seg = *iter;
    if (seg)
    {
      seg->bookedCabin() = info->_bookedCabin;
      seg->setBookingCode(info->_bookingCode);
    }
    ++iter;
    ++infos;
  }
}

void
NoPNRPricingTrx::setFullFBCItin()
{
  if (!TrxUtil::isDnataEnabled(*this))
    return;

  if (std::none_of(itin().front()->travelSeg().cbegin(),
                   itin().front()->travelSeg().cend(),
                   [](const TravelSeg* ts)
                   { return ts->fareBasisCode().empty(); }))
    _fullFBCItin = true;
}

void
NoPNRPricingTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  std::string tmpResponse(ere.message());
  if (ere.code() > 0 && ere.message().empty())
  {
    tmpResponse = "UNKNOWN EXCEPTION";
  }

  if (altTrxType() == PricingTrx::WP)
  {
    PricingResponseFormatter formatter;
    response += formatter.formatResponse(tmpResponse, false, *this, nullptr, ere.code());
  }
  else
  {
    NoPNRPricingResponseFormatter formatter;
    response = formatter.formatResponse(tmpResponse, false, *this, nullptr, ere.code());
  }
}

bool
NoPNRPricingTrx::convert(std::string& response)
{
  XMLConvertUtils::tracking(*this);
  LOG4CXX_DEBUG(logger, "Doing PricingTrx response");
  if (!taxRequestToBeReturnedAsResponse().empty())
  {
    response = taxRequestToBeReturnedAsResponse();
    return true;
  }

  Diagnostic& diag = diagnostic();
  if (_fareCalcCollector.empty())
  {
    LOG4CXX_WARN(logger, "Pricing Response Items are Missing");
  }

  FareCalcCollector* fareCalcCollector = nullptr;
  if ((diag.diagnosticType() == DiagnosticNone || diag.diagnosticType() == Diagnostic855) &&
      !_fareCalcCollector.empty())
  {
    fareCalcCollector = _fareCalcCollector.front();
  }

  std::string tmpResponse;
  if (diagDisplay())
    tmpResponse = diag.toString();
  else
    tmpResponse.clear();

  if (diag.diagnosticType() != DiagnosticNone && tmpResponse.length() == 0)
  {
    char tmpBuf[512] = {};
    snprintf(tmpBuf, sizeof(tmpBuf), "DIAGNOSTIC %d RETURNED NO DATA", diag.diagnosticType());
    tmpResponse.insert(0, tmpBuf);
  }

  NoPNRPricingResponseFormatter formatter;

  if (getOptions()->returnAllData() == GDS)
    response = formatter.formatResponse(
        "NOFARES/RBD/CARRIER", false, *this, nullptr, ErrorResponseException::NO_FARES_RBD_CARRIER);
  else
    response = formatter.formatResponse(tmpResponse, displayOnly(), *this, fareCalcCollector);

  return true;
}
}
