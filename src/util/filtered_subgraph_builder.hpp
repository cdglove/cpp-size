// *****************************************************************************
//
// util/filtered_subgraph_builder.hpp
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
#ifndef _CPPSIZE_UTIL_FILTEREDSUBGRAPHBUILDER_HPP_
#define _CPPSIZE_UTIL_FILTEREDSUBGRAPHBUILDER_HPP_

#include <boost/graph/depth_first_search.hpp>
#include "ui/include_tree_widget_item.hpp"
#include "cpp_dep/cpp_dep.hpp"

// -----------------------------------------------------------------------------
//
template<typename Graph>
struct filtered_subgraph_builder : boost::default_dfs_visitor
{
    typedef typename boost::graph_traits<
        Graph
    >::vertex_descriptor vertex_descriptor;

    typedef typename boost::graph_traits<
        Graph
    >::edge_descriptor edge_descriptor;

    typedef std::function<
        bool(vertex_descriptor const&, Graph const&)
    > filter_func_t;

    filtered_subgraph_builder(
        filter_func_t filter_func,
        Graph& result_graph)
        : filter_func_(std::move(filter_func))
        , result_graph_(result_graph)
        , match_count_(0)
    {}

    void start_vertex(vertex_descriptor const& v, Graph const& g)
    {
        resolved_path_.push_back(boost::add_vertex(g[v], result_graph_));
        current_path_.push_back(v);
    }

    void tree_edge(edge_descriptor const& e, Graph const& g)
    {
        current_path_.push_back(e.m_target);

        if(match_count_ > 0)
        {
            auto new_vert = boost::add_vertex(g[e.m_target], result_graph_);
            boost::add_edge(resolved_path_.back(), new_vert, result_graph_);
            resolved_path_.push_back(new_vert);
            match_stack_.push_back(false);
        }
        else
        {
            if(filter_func_(e.m_target, g))
            {
                resolve_current_path(g);

                match_stack_.push_back(true);
                ++match_count_;
            }
            else
            {
                match_stack_.push_back(false);
            }
        }        
    }

    void finish_vertex(vertex_descriptor const& v, Graph const&)
    {
        assert(current_path_.back() == v);
        current_path_.pop_back();
        if(match_count_ > 0)
        {
            resolved_path_.pop_back();
        }
        else if(resolved_path_.size() > current_path_.size())
        {
            resolved_path_.pop_back();
        }

        bool is_match = match_stack_.back();
        if(is_match)
        {
            --match_count_;
        }

        match_stack_.pop_back();
    }

protected:

    void resolve_current_path(Graph const& g)
    {
        auto last_added = resolved_path_.back();
        std::for_each(
            current_path_.begin() + resolved_path_.size(),
            current_path_.end(),
            [&](auto&& v)
            {
                auto new_vert = boost::add_vertex(g[v], result_graph_);
                boost::add_edge(last_added, new_vert, result_graph_);
                last_added = new_vert;
                resolved_path_.push_back(new_vert);
            }
        );
    }

    std::vector<vertex_descriptor> current_path_;
    std::vector<vertex_descriptor> resolved_path_;
    std::vector<bool> match_stack_;
    int match_count_;

    std::function<
        bool(vertex_descriptor const&, Graph const&)
    > filter_func_;

    Graph& result_graph_;
};

#endif // _CPPSIZE_UTIL_FILTEREDSUBGRAPHBUILDER_HPP_
