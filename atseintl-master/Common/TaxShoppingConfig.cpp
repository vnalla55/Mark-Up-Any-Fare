#include "Common/TaxShoppingConfig.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace tse
{
namespace
{
ConfigurableValue<ConfigSet<std::string>>
tvlDateDepTaxNations("TAX_SVC", "TVL_DEP_TAX_NATIONS");
ConfigurableValue<ConfigSet<std::string>>
fltNoDepTaxNations("TAX_SVC", "FLTNO_DEP_TAX_NATIONS");
ConfigurableValue<ConfigSet<std::string>>
sameDayDepTaxNations("TAX_SVC", "SAME_DAY_DEP_TAX_NATIONS");
ConfigurableValue<ConfigSet<std::string>>
taxTransitMinutesRounding("TAX_SVC", "TAX_TRANSIT_MINUTES_ROUNDING");

template <class Container>
Container
initListFromConfig(ConfigMan& config, const char* listName)
{
  Container container;
  std::string valuesString;

  config.getValue(listName, valuesString, "TAX_SVC");

  std::stringstream stream(valuesString);
  std::string value;
  while (std::getline(stream, value, '|'))
    container.insert(
        std::upper_bound(
            container.begin(), container.end(), value, std::less<typename Container::value_type>()),
        value);

  container.erase(std::unique(container.begin(), container.end()), container.end());
  return container;
}

template <class Container>
Container
initListFromCV(ConfigurableValue<ConfigSet<std::string>>& cv)
{
  Container container;
  for (const auto value : cv.getValue())
  {
    container.push_back(value);
  }
  return container;
}
}

void
TaxShoppingConfig::init(ConfigMan& config)
{
  _tvlDateDepTaxNations = initListFromCV<NationCodesVec>(tvlDateDepTaxNations);
  _fltNoDepTaxNations = initListFromCV<NationCodesVec>(fltNoDepTaxNations);
  _sameDayDepTaxNations = initListFromCV<NationCodesVec>(sameDayDepTaxNations);
  _roundTransitMinutesTaxCodes = initListFromCV<TaxCodesVec>(taxTransitMinutesRounding);
}
}
