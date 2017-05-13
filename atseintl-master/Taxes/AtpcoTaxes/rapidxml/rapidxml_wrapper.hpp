#pragma once

#include "rapidxml.hpp"

template class rapidxml::xml_node<>;
template class rapidxml::xml_attribute<>;
template class rapidxml::xml_document<>;
template void rapidxml::xml_document<>::parse<0>(char*);
