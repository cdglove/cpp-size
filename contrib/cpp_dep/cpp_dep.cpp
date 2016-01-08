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
#include <boost/unordered/unordered_set.hpp>
#include <boost/filesystem.hpp>

// -----------------------------------------------------------------------------
//
using namespace cpp_dep;

// -----------------------------------------------------------------------------
//
typedef boost::unordered::unordered_map<
    std::string, include_vertex_descriptor_t
> known_file_set_t;

// -----------------------------------------------------------------------------
//
struct subtree_cloner : boost::default_dfs_visitor
{

};

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
    known_file_set_t& known_files,
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
            std::transform(
                file.begin(),
                file.end(),
                file.begin(),
                [](char c)
                {
                    if(c == '\\' || c == '/')
                        return (char)boost::filesystem::path::preferred_separator;
                    else
                        return (char)std::tolower(c);
                }
            );

            if(file.empty())
                continue;

            std::size_t this_size = 0; 
            bool already_included = true;
            auto found_file = known_files.find(file); 
            if(found_file == known_files.end())
            {
                already_included = false;
                this_size = boost::filesystem::file_size(file.c_str());

                // Add subpath with 0 size.
                subtree_cloner clone_tree; 
                boost::depth_first_search(
                    deps, boost::visitor(clone_tree), found_file->second);
            }

            sub_tree_size += this_size;

            last_added = boost::add_vertex(include_vertex_t(file, this_size, already_included), deps);
            auto result = boost::add_edge(parent, last_added, deps);
            BOOST_ASSERT(result.second);

            if(!already_included)
            {
                known_files.insert(std::make_pair(file, last_added));
            }

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
                known_files,
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
    known_file_set_t known_files;
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
        known_files,
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
    known_file_set_t known_files;
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
        known_files,
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

// -----------------------------------------------------------------------------
//
include_graph_t cpp_dep::invert_to_paths(include_graph_t const& g)
{
    include_graph_t result;
    boost::graph_traits<
        include_graph_t
    >::vertex_iterator vi, vi_end;

    boost::unordered::unordered_map<
        std::string,
        include_vertex_descriptor_t
    > partial_path_map;

    boost::tie(vi, vi_end) = boost::vertices(g);
    auto root_vi = vi;

    // Skip the first item
    if(vi != vi_end)
        ++vi;

    for ( ; vi != vi_end; ++vi)
    {
        include_vertex_t const& file = g[*vi];
        boost::filesystem::path file_path(file.name);
        boost::filesystem::path partial_path;
        std::string partial_path_string;
        include_vertex_descriptor_t last_vert = *root_vi;

        for(auto&& component : file_path)
        {
            partial_path /= component;
            partial_path_string = partial_path.string();
            auto i = partial_path_map.find(partial_path_string);
            auto vert = include_vertex_descriptor_t();
            if(i != partial_path_map.end())
            {
                vert = i->second;
            }
            else
            {
                vert = boost::add_vertex(include_vertex_t(partial_path_string, 0, false), result);
                partial_path_map.insert(std::make_pair(partial_path_string, vert));
                boost::add_edge(last_vert, vert, result);
            }

            include_vertex_t& result_vert = result[vert];
            result_vert.size += file.size;
            last_vert = vert;
        }
    }

    return result;
}


