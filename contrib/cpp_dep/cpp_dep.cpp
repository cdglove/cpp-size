// *****************************************************************************
// 
// cpp_dep.cpp
//
// Implementation of C++ dependency graph handling.
//
// Copyright Chris Glover 2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// *****************************************************************************
#include "cpp_dep/cpp_dep.hpp"
#include <fstream>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/breadth_first_search.hpp>

// -----------------------------------------------------------------------------
//
namespace cpp_dep
{
	// To be supplied by the application;
    extern std::size_t get_file_size(char const* path);
}

// -----------------------------------------------------------------------------
//
using namespace cpp_dep;

template<typename Iterator>
Iterator getline(Iterator cur, Iterator end, std::string& line)
{
    line.clear();
    while(cur != end && *cur != '\n')
        line.push_back(*cur++);

    if(cur != end && *cur == '\n')
        ++cur;

    if(cur != end && *cur == '\r')
        ++cur;

    return cur;
}

// -----------------------------------------------------------------------------
//
template<typename Iterator>
static void ReadDepsFileRecursive(
    char const* line_prefix,
    char depth_mark,
    include_vertex_descriptor_t parent,
	int depth, 
    include_graph_t& deps,
	int& line_number, 
    std::size_t& size,
    Iterator& current,
   	Iterator end)
{
    include_vertex_descriptor_t last_added = include_vertex_descriptor_t();
	std::string file;
    std::size_t sub_tree_size = 0;
	while(current != end)
	{
		int line_depth = 0;
		Iterator line_start_pos = current;

        for(int i = 0; current != end && *current == line_prefix[i]; ++i)
            ++current;

        while(current != end && *current == depth_mark)
		{
			++line_depth;
			++current;
		}

		if(line_depth == 0)
		{
            current = getline(current, end, file);
			++line_number;
			continue;
		}

		if(line_depth <= depth)
		{
            current = line_start_pos;
            break;
		}

		if(line_depth == (depth + 1))
		{
            // On gcc files, theres an extra space here.
            if(*current == ' ')
                ++current;

            current = getline(current, end, file);
			std::replace(
				file.begin(),
				file.end(),
				'\\',
				'/'
			);

            if(file.empty())
                continue;

            std::size_t this_size = get_file_size(file.c_str());
            sub_tree_size += this_size;
            last_added = boost::add_vertex(include_vertex_t(file, this_size), deps);
            auto result = boost::add_edge(parent, last_added, deps);
            BOOST_ASSERT(result.second);
			++line_number;
		}
		else
		{
            current = line_start_pos;
            ReadDepsFileRecursive(
                line_prefix,
                depth_mark,
                last_added,
                depth+1,
                deps,
                line_number,
                sub_tree_size,
                current,
                end);
		}
	}

    size += sub_tree_size;
    deps[parent].size_dependencies = sub_tree_size;
}

// -----------------------------------------------------------------------------
//
static include_graph_t ReadGccDepsFile(std::string const& deps)
{
    include_graph_t dep_graph;
    include_vertex_descriptor_t root = add_vertex(include_vertex_t(), dep_graph);
    int line_number = 0;
    std::size_t tree_size;
    auto begin = deps.begin();
    auto end = deps.end();
    ReadDepsFileRecursive(
        "",
        '.',
        root,
        0,
        dep_graph,
        line_number,
        tree_size,
        begin,
        end
    );

    return dep_graph;
}

// -----------------------------------------------------------------------------
//
static include_graph_t ReadMsvcDepsFile(std::string const& deps)
{
	include_graph_t dep_graph;
    include_vertex_descriptor_t root = add_vertex(include_vertex_t(), dep_graph);
	int line_number = 0;
    std::size_t tree_size;
    auto begin = deps.begin();
    auto end = deps.end();
    ReadDepsFileRecursive(
        "Note: including file:",
        ' ',
        root,
        0,
        dep_graph,
        line_number,
        tree_size,
        begin,
        end
    );

	return dep_graph;
}

// -----------------------------------------------------------------------------
//
include_graph_t cpp_dep::read_deps_file(char const* file)
{
	std::ifstream ins(file);
	if(!ins.is_open())
	{
        throw std::runtime_error(std::string("Failed to open ") + file + " for reading.");
	}

    std::stringstream sins;
    sins << ins.rdbuf();

    if(sins.peek() == '.')
	{
        return ReadGccDepsFile(sins.str());
	}
	else
	{
        return ReadMsvcDepsFile(sins.str());
	}
}
