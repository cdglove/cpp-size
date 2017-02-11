// *****************************************************************************
//
// ui/filtered_tree_view_builder.hpp
//
// Boost.Graph filter that filters the graph in such a way as to keep all
// vertices that pass the filter, plus all vertices that path to and from
// that vertex from the specified start vertex.
//
// Copyright Chris Glover 2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// *****************************************************************************
#ifndef CPPSIZE_UTIL_FILTEREDSUBGRAPHBUILDER_HPP_
#define CPPSIZE_UTIL_FILTEREDSUBGRAPHBUILDER_HPP_

#include <boost/graph/depth_first_search.hpp>
#include "ui/tree_view_builder.hpp"
#include "cpp_dep/cpp_dep.hpp"
#include <vector>

// -----------------------------------------------------------------------------
//
struct filtered_tree_view_builder
{
    template<typename FilterFunc>
    void operator()(cpp_dep::include_graph_t const& g, QTreeWidget* root, FilterFunc&& filter_func)
    {
        std::vector<bool> keepers(boost::num_vertices(g), false);
        filter_builder<std::decay_t<FilterFunc>> build(std::move(filter_func), keepers);
        build(g);

        filter_applier apply(keepers);
        apply(g, root);   
    }

private:

    template<typename FilterFunc>
    struct filter_builder : cpp_dep::inferred_include_visitor<filter_builder<FilterFunc>>
    {
        void operator()(cpp_dep::include_graph_t const& g)
        {
            visit(g);
        }

        filter_builder(FilterFunc filter_func, std::vector<bool>& keepers)
            : filter_func_(std::move(filter_func))
            , keepers_(keepers)
        {}

        void root_file(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
        {
            visited_.resize(boost::num_vertices(g), false);
            vertex_stack_.push_back(v);
        }

        void include_file(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
        {
            visited_[v] = true;

            if(filter_func_(v, g))
            {
                keepers_[v] = true;
                // If we keep one in the tree, we keep all of the ones in the stack
                // But we only need to traverse the stack if the top isn't already true.
                // cglover-note: feb9th_2017
                // This doesn't work for some reason so I 've removed the check for now.
                //if(!keepers_[vertex_stack_.back()])
                {
                    for(auto&& vert : vertex_stack_)
                    {
                        keepers_[vert] = true;
                    }
                }
            }
            
            vertex_stack_.push_back(v);
        }

        void finish_file(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
        {
            vertex_stack_.pop_back();
        }

        FilterFunc filter_func_;
        std::vector<cpp_dep::include_vertex_descriptor_t> vertex_stack_;
        std::vector<bool>& keepers_;
        std::vector<bool> visited_;
    };

    struct filter_applier : tree_view_builder_base<filter_applier>
    {
        filter_applier(std::vector<bool>& keepers)
            : tree_view_builder_base(tree_view_builder_base::option::none)
            , keepers_(keepers)
        {}

        bool filter(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const&)
        {
            return keepers_[v];
        }

        std::vector<bool>& keepers_;
    };
};

#endif // CPPSIZE_UTIL_FILTEREDSUBGRAPHBUILDER_HPP_
