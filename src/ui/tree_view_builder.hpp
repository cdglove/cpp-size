// *****************************************************************************
//
// ui/tree_view_builder.hpp
//
// Boost.Graph DFS visitor to construct a tree view from a boost graph.
//
// Copyright Chris Glover 2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// *****************************************************************************
#ifndef _CPPSIZE_UI_TREEVIEWBUILDER_HPP_
#define _CPPSIZE_UI_TREEVIEWBUILDER_HPP_

#include <boost/graph/depth_first_search.hpp>
#include "ui/include_tree_widget_item.hpp"
#include "cpp_dep/cpp_dep.hpp"

// -----------------------------------------------------------------------------
//
struct tree_view_builder : boost::default_dfs_visitor
{
    tree_view_builder(QTreeWidget* tree)
        : tree_(tree)
        , current_item_(nullptr)
        , current_order_(1)
        , total_size_(0)
    {}

    template <typename Vertex, typename Graph>
    void start_vertex(Vertex const& v, Graph const& g)
    {
        cpp_dep::include_vertex_t const& file = g[v];

        total_size_ = file.size + file.size_dependencies;

        current_item_ = new IncludeTreeWidgetItem(tree_);
        current_item_->setColumnFile(file.name.c_str());
        current_item_->setColumnSize(total_size_, total_size_);
        current_item_->setColumnOrder(0);
    }

    template <typename Edge, typename Graph>
    void tree_edge(Edge const& e, Graph const& g)
    {
        cpp_dep::include_vertex_t const& file = g[e.m_target];

        current_item_ = new IncludeTreeWidgetItem(current_item_);
        current_item_->setColumnFile(file.name.c_str());
        current_item_->setColumnSize(file.size + file.size_dependencies, total_size_);
        current_item_->setColumnOrder(current_order_++);
    }

    template <typename Vertex, typename Graph>
    void finish_vertex(Vertex const&, Graph const&)
    {
        current_item_ = current_item_->parent();
    }

protected:

    QTreeWidget* tree_;
    IncludeTreeWidgetItem* current_item_;
    int current_order_;
    std::size_t total_size_;
};

#endif // _CPPSIZE_UI_TREEVIEWBUILDER_HPP_
