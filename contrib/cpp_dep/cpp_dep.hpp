// *****************************************************************************
// 
// cpp_dep.hpp
//
// Represents the include file structure of a cpp source file.
//
// Copyright Chris Glover 2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// *****************************************************************************
#ifndef _CPPDEP_CPPDEP_HPP_
#define _CPPDEP_CPPDEP_HPP_

#include <string>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

// -----------------------------------------------------------------------------
//
namespace cpp_dep {

// -----------------------------------------------------------------------------
//
typedef std::string include_vertex_t;

typedef boost::adjacency_list<
	boost::vecS, 
	boost::vecS,
	boost::bidirectionalS, 
    include_vertex_t
> include_graph_t;

typedef boost::graph_traits<
	include_graph_t
>::vertex_descriptor include_vertex_descriptor_t;

// -----------------------------------------------------------------------------
// Builds the include deps graph from ain input file. The file must be in the 
// format of either gcc as output from the command;
//   g++ -H -E -o /dev/null source.cpp 2> includes.txt
// or from msvc as output from the command;
//   cl.exe /showIncludes /P source.cpp 1> nul 2> includes.txt
include_graph_t read_deps_file(char const* file);

}

#endif // _CPPDEP_CPPDEP_HPP_
