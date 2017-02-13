// *****************************************************************************
//
// ui/tree_view_include_size_calculator.hpp
//
// Copyright Chris Glover 2017
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// *****************************************************************************
#ifndef CPPSIZE_UI_TREEVIEWINCLUDESIZECALCULATOR_HPP_
#define CPPSIZE_UI_TREEVIEWINCLUDESIZECALCULATOR_HPP_

#include <boost/graph/depth_first_search.hpp>
#include "ui/include_tree_widget_item.hpp"
#include "cpp_dep/inferred_include_visitor.hpp"
#include <vector>

// -----------------------------------------------------------------------------
//
class tree_view_include_size_calculator
    : private cpp_dep::inferred_include_visitor<tree_view_include_size_calculator>
{
public:

    tree_view_include_size_calculator(std::vector<IncludeTreeWidgetItem*> const& items_by_index)
        : items_by_index_(items_by_index)
        , default_bg_(QPalette().base().color())
    {}

    void operator()(cpp_dep::include_graph_t const& g)
    {
        size_by_index_.clear();
        skip_count_by_vertex_.resize(num_vertices(g), 0);

        unchecked_counter_ = 0;
        if(boost::num_vertices(g) > 0)
        {
            size_by_index_.resize(items_by_index_.size(), 0);
            cpp_dep::include_vertex_t const& file = g[0];
            size_stack_.push_back(0);
            size_stack_.push_back(file.size);
            
            if(!items_by_index_[0]->isChecked())
                ++unchecked_counter_;

            this->visit(g);

            for(int i = 0; i < size_by_index_.size(); ++i)
                items_by_index_[i]->setColumnSize(size_by_index_[i], size_by_index_[0]);
        }
    }

private:

    friend class cpp_dep::inferred_include_visitor<tree_view_include_size_calculator>;
    
    void include_file(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
    {
        cpp_dep::include_vertex_t const& file = g[v];

        IncludeTreeWidgetItem* current_widget = items_by_index_[get_current_include_index()];

        if(!current_widget->isChecked())
            ++unchecked_counter_;
        
        if(unchecked_counter_ > 0)
            ++skip_count_by_vertex_[v];

        current_widget->setBackgroundColor(
            IncludeTreeWidgetItem::Column::Size, default_bg_
        );

        if(expanded_counter_ > 0)
            ++expanded_counter_;

        if(get_current_include_count(v) - skip_count_by_vertex_[v] == 1)
        {
            size_stack_.push_back(file.size);
            if(get_current_include_count(v) != 1)
            {
                current_widget->setBackgroundColor(
                    IncludeTreeWidgetItem::Column::Size, QColor::fromRgb(0xFF, 0x99, 0x33)
                );

                if(expanded_counter_ == 0)
                {
                    IncludeTreeWidgetItem* expand = current_widget->parent();
                    while(expand)
                    {
                        expand->setExpanded(true);
                        expand = expand->parent();
                    }

                    ++expanded_counter_;
                }
            }
        }
        else
        {
            size_stack_.push_back(0);
        }
    }

    void finish_file(cpp_dep::include_vertex_descriptor_t const& v, cpp_dep::include_graph_t const& g)
    {
        std::size_t back = size_stack_.back();
        size_stack_.pop_back();
        size_stack_.back() += back;
        size_by_index_[get_current_include_index()] = back;

        if(!items_by_index_[get_current_include_index()]->isChecked())
            --unchecked_counter_;

        if(expanded_counter_ > 0)
            --expanded_counter_;
    }

    std::vector<std::size_t> size_stack_;
    std::vector<std::size_t> skip_count_by_vertex_;
    int expanded_counter_ = 0;
    int unchecked_counter_ = 0;
    std::vector<std::size_t> size_by_index_;
    std::vector<IncludeTreeWidgetItem*> const& items_by_index_;
    QColor default_bg_;
};

#endif // CPPSIZE_UI_TREEVIEWINCLUDESIZECALCULATOR_HPP_
