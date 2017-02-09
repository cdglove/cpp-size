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
#include "cpp_dep/inferred_include_visitor.hpp"

// -----------------------------------------------------------------------------
//
template<typename Derived>
struct tree_view_builder_base : cpp_dep::inferred_include_visitor<tree_view_builder_base<Derived>>
{
    struct option
    {
        enum
        {
            none        = 0,
            checkbox    = 1 << 0,
        };
    };

    tree_view_builder_base(std::uint32_t options)
        : options_(options)
    {}

    void operator()(cpp_dep::include_graph_t const& g, QTreeWidget* root)
    {
        current_item_ = nullptr;
        current_order_ = 0;
        total_size_ = 0;
        root_ = root;

        this->visit(g);
    }

private:

    bool filter(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
    {
        return true;
    }

    friend class cpp_dep::inferred_include_visitor<tree_view_builder_base<Derived>>;

    void root_file(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
    {
        cpp_dep::include_vertex_t const& file = g[v];

        total_size_ = file.size + file.size_dependencies;
        current_item_ = new IncludeTreeWidgetItem(root_);
        current_item_->setColumnFile(file.name.c_str());
        current_item_->setColumnSize(total_size_, total_size_);
        current_item_->setColumnOrder(current_order_++);
        current_item_->setColumnOccurence(this->get_include_count(v));

        if(wants_checkboxes())
        {
            current_item_->showCheckbox();
        }
    }

    void include_file(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
    {
        if(!derived().filter(v, g))            return;

        cpp_dep::include_vertex_t const& file = g[v];
        
        std::size_t show_size = file.size + file.size_dependencies;
        current_item_ = new IncludeTreeWidgetItem(current_item_);
        current_item_->setColumnFile(file.name.c_str());
        current_item_->setColumnSize(show_size, total_size_);
        current_item_->setColumnOrder(current_order_++);
        current_item_->setColumnOccurence(this->get_include_count(v));

        if(wants_checkboxes())
        {
            current_item_->showCheckbox();
        }
    }

    void finish_file(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
    {
        if(!derived().filter(v, g))
            return;

        current_item_ = current_item_->parent();
    }

    Derived& derived()
    {
        return *static_cast<Derived*>(this);
    }

    bool wants_checkboxes() const
    {
        return (options_ & option::checkbox);
    }

    QTreeWidget* root_;
    IncludeTreeWidgetItem* current_item_;
    int current_order_;
    std::size_t total_size_;
    std::uint32_t options_;
};

// -----------------------------------------------------------------------------
//
struct tree_view_builder : tree_view_builder_base<tree_view_builder>
{
    tree_view_builder(std::uint32_t options)
        : tree_view_builder_base(options)
    {}
};



#endif // CPPSIZE_UI_TREEVIEWBUILDER_HPP_
