// *****************************************************************************
//
// ui/tree_view_filter.hpp
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
#ifndef CPPSIZE_UI_TREEVIEWFILTER_HPP_
#define CPPSIZE_UI_TREEVIEWFILTER_HPP_

#include <boost/graph/depth_first_search.hpp>
#include "cpp_dep/inferred_include_visitor.hpp"
#include "cpp_dep/cpp_dep.hpp"
#include <vector>

// -----------------------------------------------------------------------------
//
struct tree_view_filter
{
    template<typename FilterFunc>
    void operator()(
        cpp_dep::include_graph_t const& g, 
        std::vector<IncludeTreeWidgetItem*> const& items_by_index, 
        FilterFunc&& filter_func)
    {
        filter_builder<std::decay_t<FilterFunc>> filter(std::move(filter_func), items_by_index.size());
        filter(g);

        for(int i = 0; i < items_by_index.size(); ++i)
            items_by_index[i]->setHidden(!filter.visible_by_index_[i]);     
    }

private:

    template<typename FilterFunc>
    struct filter_builder : cpp_dep::inferred_include_visitor<filter_builder<FilterFunc>>
    {
        void operator()(cpp_dep::include_graph_t const& g)
        {
            this->visit(g);
        }

        filter_builder(FilterFunc filter_func, std::size_t num_items)
            : filter_func_(std::move(filter_func))
            , visible_by_index_(num_items, false)
        {}

        void root_file(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
        {
            index_stack_.push_back(this->get_current_include_index());
        }

        void include_file(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
        {
            if(filter_func_(v, g))
            {
                visible_by_index_[this->get_current_include_index()] = true;
                for(auto&& idx : index_stack_)
                {
                    visible_by_index_[idx] = true;
                }
            }
            
            index_stack_.push_back(this->get_current_include_index());
        }

        void finish_file(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
        {
            index_stack_.pop_back();
        }

        FilterFunc filter_func_;
        std::vector<int> index_stack_;
        std::vector<bool> visible_by_index_;
    };
};

#endif // CPPSIZE_UTIL_FILTEREDSUBGRAPHBUILDER_HPP_
