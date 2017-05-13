#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <string>

class SimpleIniMap
{
public: // types
  typedef std::map<std::string, std::string> Param;
  typedef std::map<std::string, Param> SectionMap;
  typedef SectionMap::iterator iterator;

private: // members
  SectionMap m_sections;

public: // member functions
  bool GetParams(std::string fileName);
  Param operator[](std::string p) { return m_sections[p]; }
  iterator find(std::string p) { return m_sections.find(p); }
  SectionMap::iterator begin() { return m_sections.begin(); }
  SectionMap::iterator end() { return m_sections.end(); }

  bool exist(std::string section, std::string key);
};

