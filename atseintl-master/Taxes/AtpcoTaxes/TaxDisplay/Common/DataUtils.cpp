#include "TaxDisplay/Common/DataUtils.h"

#include "ServiceInterfaces/LocService.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"

namespace tax
{

namespace display
{

bool getNationCode(const TaxDisplayRequest& request,
                   const LocService& locService,
                   type::Nation& nationCode)
{
  if (request.nationCode.empty())
  {
    if (!request.nationName.empty())
    {
      nationCode = locService.getNationByName(request.nationName);
    }
    else if (!request.airportCode.empty())
    {
      nationCode = locService.getNation(request.airportCode);
    }
    else
    {
      return false;
    }
  }
  else
  {
    nationCode = request.nationCode;
  }

  return !nationCode.empty();
}

bool getNationName(const type::Nation& requestNationCode,
                   const type::NationName& requestNationName,
                   const LocService& locService,
                   type::NationName& nationName)
{
  if (requestNationName.empty())
  {
    if (!requestNationCode.empty())
    {
      nationName = locService.getNationName(requestNationCode);
    }
    else
    {
      return false;
    }
  }
  else
  {
    nationName = requestNationName;
  }

  return !nationName.empty();
}

} // namespace display
} // namespace tax
