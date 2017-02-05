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
#ifndef CPPSIZE_UI_TREEVIEWBUILDER_HPP_
#define CPPSIZE_UI_TREEVIEWBUILDER_HPP_

#include <boost/graph/depth_first_search.hpp>
#include "ui/include_tree_widget_item.hpp"
#include "cpp_dep/cpp_dep.hpp"

// -----------------------------------------------------------------------------
//
template<typename Derived>
struct tree_view_builder_base
{
    void operator()(cpp_dep::include_graph_t const& g, QTreeWidget* root)
    {
        current_item_ = nullptr;
        current_order_ = 0;
        total_size_ = 0;
        root_ = root;
        include_count_.resize(boost::num_vertices(g), 0);

        dfs_visitor<VisitPolicy::Initial> visit(this);
        boost::depth_first_search(g, boost::visitor(visit));
    }

private:

    Derived& derived()
    {
        return *static_cast<Derived*>(this);
    }

    struct terminator {};
    enum class VisitPolicy
    {
        Initial,
        Recursing,
    };

    template<VisitPolicy Policy>
    struct dfs_visitor : boost::default_dfs_visitor
    {

        dfs_visitor(tree_view_builder_base* owner)
            : owner_(owner)
        {}

        void start_vertex(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
        {
            if(!v && is_recursing())
                throw terminator();

            if(!owner_->derived().filter(v, g))
                return;

            cpp_dep::include_vertex_t const& file = g[v];


            if(is_recursing())
            {
                owner_->current_item_ = new IncludeTreeWidgetItem(owner_->current_item_);
            }
            else
            {
                owner_->total_size_ = file.size + file.size_dependencies;
                owner_->current_item_ = new IncludeTreeWidgetItem(owner_->root_);
            }

            owner_->current_item_->setColumnFile(file.name.c_str());
            owner_->current_item_->setColumnSize(is_recursing() ? 0 : owner_->total_size_, owner_->total_size_);
            owner_->current_item_->setColumnOrder(owner_->current_order_++);
            owner_->current_item_->setColumnOccurence(owner_->include_count_[v]);
        }

        void examine_edge(cpp_dep::include_edge_descriptor_t const& e, cpp_dep::include_graph_t const& g)
        {
            if(!owner_->derived().filter(e.m_target, g))
                return;

            if(owner_->include_count_[e.m_target]++ && !is_recursing())
            {
                try 
                {
                    dfs_visitor<VisitPolicy::Recursing> recurse(owner_);
                    boost::depth_first_search(g, boost::visitor(recurse).root_vertex(e.m_target));
                }
                catch(terminator)
                {}
            }
            else
            {
                cpp_dep::include_vertex_t const& file = g[e.m_target];
                cpp_dep::include_edge_t edge = g[e];

                std::size_t show_size = is_recursing() ? 0 : file.size + file.size_dependencies;
                owner_->current_item_ = new IncludeTreeWidgetItem(owner_->current_item_);
                owner_->current_item_->setColumnFile(file.name.c_str());
                owner_->current_item_->setColumnSize(show_size, owner_->total_size_);
                owner_->current_item_->setColumnOrder(owner_->current_order_++);
                owner_->current_item_->setColumnOccurence(owner_->include_count_[e.m_target]);
            }
        }

        void finish_vertex(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
        {
            if(!owner_->derived().filter(v, g))
                return;

            owner_->current_item_ = owner_->current_item_->parent();
        }

    private:

        bool is_recursing() const
        {
            return Policy == VisitPolicy::Recursing;
        }

        tree_view_builder_base* owner_;
    };

    std::vector<int> include_count_;
    QTreeWidget* root_;
    IncludeTreeWidgetItem* current_item_;
    int current_order_;
    std::size_t total_size_;
};

// -----------------------------------------------------------------------------
//
struct tree_view_builder : tree_view_builder_base<tree_view_builder>
{
    bool filter(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
    {
        return true;
    }
};

#endif // CPPSIZE_UI_TREEVIEWBUILDER_HPP_
