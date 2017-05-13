//-------------------------------------------------------------------
//
//  Authors:     Michal Mlynek
//
//  Copyright Sabre 2013
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

#pragma once

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/Logger.h"
#include "Common/TseStlTypes.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

namespace tse
{
  struct BrandedFaresComparator
  {
    typedef boost::property<boost::vertex_name_t, BrandCode> VertexProperty;
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProperty > Graph;
    typedef std::pair<BrandCode, BrandCode> BrandPair;

    BrandedFaresComparator(const std::vector<QualifiedBrand>& brandProgramVec, Logger& logger)
      : _logger(logger)
    {
      std::set<BrandPair> edges; // relationships between brands
      std::vector<BrandCode> vertices;  // brands themselves

      scanTrxForBrandsAndRelationships(brandProgramVec, vertices, edges);

      Graph graph(vertices.size());
      putBrandsIntoGraph(graph, vertices, edges);

      // Perform a topological sort.
      try
      {
        std::deque<int> topo_order;
        boost::topological_sort(graph, std::front_inserter(topo_order));
        for(std::deque<int>::const_iterator i = topo_order.begin(); i != topo_order.end(); ++i)
        {
          _orderedBrands.push_back(boost::get(boost::vertex_name_t(), graph, (*i)));
        }
      }
      // unable to order brands, fallback to the default algorithm
      // ( using order of brands in which they were processed and put into vertces )
      catch(...)
      {
        _orderedBrands = vertices;
      }
    }

    bool operator()(const BrandCode& brand1, const BrandCode& brand2)
    {
      if (brand1 == brand2)
        return false;

      for (auto& elem : _orderedBrands)
      {
        if (brand1 == elem)
          return true;
        if (brand2 == elem)
          return false;
      }

      // If for some reason none of the brands have been found in the trx
      // vector (which should "never happen" ) then we need to fallback to the
      // default alphabetic ordering
      LOG4CXX_ERROR(_logger, "Sorting brands in a set, brands "
          << brand1 << " and " << brand2 << " not found in trx.brandProgramVec()!!!");

      return (brand1 < brand2);
    }

    const std::vector<BrandCode>& getOrderedBrands() const
    {
      return _orderedBrands;
    }

private:

    void putBrandsIntoGraph(Graph& graph, const std::vector<BrandCode>& vertices, const std::set<BrandPair>& edges)
    {
      // map that stores the inverted mapping
      std::map<std::string, Graph::vertex_descriptor> indexes;
      // Putting all the vertices ( brands ) into the graph
      for(unsigned int i = 0; i < vertices.size(); i++)
      {
        boost::put(boost::vertex_name_t(), graph, i, vertices[i]); // set the property of a vertex
        indexes[vertices[i]] = boost::vertex(i, graph);     // retrives the associated vertex descriptor
      }

      // Put all the edges into the graph
      for (const auto& edge : edges)
      {
        boost::add_edge(indexes[edge.first], indexes[edge.second], graph);
      }
    }

    void scanTrxForBrandsAndRelationships(const std::vector<QualifiedBrand>& brandProgramVec,
        std::vector<BrandCode>& vertices,
        std::set<BrandPair>& edges)
    {
      std::set<BrandCode> uniqueBrands;
      BrandProgram* previousProgram = nullptr;
      BrandCode previousBrand;

      // Get unique brands and all the relationshops between them within programs
      for (auto& elem : brandProgramVec)
      {
        BrandProgram* currentProgram = elem.first;
        const BrandCode& currentBrand = elem.second->brandCode();

        if (uniqueBrands.insert(currentBrand).second)
        {
          // vertices hold unique brands in a default order ( in which they are processed
          // Later, if we're not able to perform a topological sort we'll use this vector instead
          vertices.push_back(currentBrand);
        }

        if (currentProgram == previousProgram)
        {
          edges.insert(std::make_pair(previousBrand, currentBrand));
        }
        previousProgram = currentProgram;
        previousBrand = currentBrand;
      }
    }

private:
    Logger& _logger;
    std::vector<BrandCode> _orderedBrands;
  };
} // namespace tse
