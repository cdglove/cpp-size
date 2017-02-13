// *****************************************************************************
//
// ui/dialog.cpp
//
// Main dialog for cpp-size
//
// Copyright Chris Glover 2015-2017
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// *****************************************************************************
#include "ui/dialog.hpp"
#include "ui/include_tree_widget_item.hpp"
#include "ui/tree_view_builder.hpp"
#include "ui/filtered_tree_view_builder.hpp"
#include "ui/tree_view_include_size_calculator.hpp"
#include "ui_dialog.h"
#include "cpp_dep/cpp_dep.hpp"
#include <boost/graph/depth_first_search.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/scope_exit.hpp>
#include <QFileInfo>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMessageBox>
#include <algorithm>

// -----------------------------------------------------------------------------
//
struct Dialog::include_tree_view_builder : tree_view_builder_base<include_tree_view_builder>
{
    include_tree_view_builder(std::vector<IncludeTreeWidgetItem*>& destination_items)
        : tree_view_builder_base(tree_view_builder_base::option::checkbox)
        , item_list_(destination_items)
    {}

    void item_created(cpp_dep::include_vertex_descriptor_t const& v, IncludeTreeWidgetItem* new_item)
    {
        int current_idx = get_current_include_index();
        if(item_list_.size() <= current_idx)
            item_list_.resize(current_idx+1, nullptr);
        item_list_[current_idx] = new_item;
    }

    std::vector<IncludeTreeWidgetItem*>& item_list_;
};

// -----------------------------------------------------------------------------
//
template<typename Fn>
void for_all_children(IncludeTreeWidgetItem* item, Fn fn)
{
    fn(item);
    for(int i = 0; i < item->childCount(); ++i)
    {
        for_all_children(static_cast<IncludeTreeWidgetItem*>(item->child(i)), fn);
    }
}

template<typename Fn>
void for_all_ancestors(IncludeTreeWidgetItem* item, Fn fn)
{
    do 
    {
        fn(item);
        item = item->parent();
    }
    while(item);    
}

// -----------------------------------------------------------------------------
//
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , update_tree_widget_(std::bind(&Dialog::filterTreeBuilt, this, std::placeholders::_1))
    , building_include_tree_(false)
{
    ui->setupUi(this);
    ui->filesystem_tree->header()->resizeSection(0, 400);
    ui->include_tree->header()->resizeSection(0, 400);
}

Dialog::~Dialog()
{
    delete ui;
}

// -----------------------------------------------------------------------------
//
void Dialog::filterTextChanged(QString const& filter_text)
{

}

// -----------------------------------------------------------------------------
//
void Dialog::includeTreeItemChanged(QTreeWidgetItem* changed_item_, int column)
{
    if(building_include_tree_)
        return;

    if(column != IncludeTreeWidgetItem::Column::File)
        return;

    tree_view_include_size_calculator calculate_sizes(all_include_tree_items_);
    calculate_sizes(*include_graph_);
}

// -----------------------------------------------------------------------------
//
void Dialog::dropEvent(QDropEvent* event)
{
    QMimeData const* mime_data = event->mimeData();
    // check for our needed mime type, here a file or a list of files
    if (mime_data->hasUrls())
    {
        QList<QUrl> urls = mime_data->urls();
        if(urls.size() > 0)
        {
            // Just take the first one in case the user dragged multiple.
            QString file = urls.at(0).toLocalFile();
            try
            {
                cpp_dep::include_graph_t includes =
                    cpp_dep::read_deps_file(file.toStdString().c_str());

                cpp_dep::include_graph_t paths =
                    cpp_dep::invert_to_paths(includes);

                // Save the two graphs.
                include_graph_ = std::make_unique<
                    cpp_dep::include_graph_t
                >(std::move(includes));

                filesystem_graph_ = std::make_unique<
                    cpp_dep::include_graph_t
                >(std::move(paths));

                populateTrees();
            }
            catch(std::exception& e)
            {
                QString msg;
                msg += "Failed to load \"" + file + "\"\n"
                    +  "Error: " + e.what();

                QMessageBox msg_box;
                msg_box.setText(msg);
                msg_box.setIcon(QMessageBox::Critical);
                msg_box.exec();
            }
        }
    }

    // Reapply the filter text.
    filterTextChanged(ui->filter_text->text());
}

// -----------------------------------------------------------------------------
//
void Dialog::dragEnterEvent(QDragEnterEvent* event)
{
    // if some actions should not be usable, like move, this code must be adopted
    event->acceptProposedAction();
}

// -----------------------------------------------------------------------------
//
void Dialog::dragMoveEvent(QDragMoveEvent* event)
{
    // if some actions should not be usable, like move, this code must be adopted
    event->acceptProposedAction();
}

// -----------------------------------------------------------------------------
//
void Dialog::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
}

// -----------------------------------------------------------------------------
//
void Dialog::populateTrees()
{
    // Clear both trees first so that if parsing fails
    // both are empty instead of just one.
    ui->include_tree->clear();
    ui->filesystem_tree->clear();
    all_include_tree_items_.clear();

    // Populate the include tree
    {
        BOOST_SCOPE_EXIT_ALL(this)
        {
            building_include_tree_ = false;
        };

        building_include_tree_ = true;
        include_tree_view_builder build_tree(all_include_tree_items_);
        build_tree(*include_graph_, ui->include_tree);

        tree_view_include_size_calculator calculate_sizes(all_include_tree_items_);
        calculate_sizes(*include_graph_);
    }

    // Populate the filesystem tree
    {
        tree_view_builder build_tree(tree_view_builder::option::none);
        build_tree(*filesystem_graph_, ui->filesystem_tree);
    }
}

// -----------------------------------------------------------------------------
//
void Dialog::filterTreeBuilt(QTreeWidget* new_widget)
{
    assert(new_widget != ui->include_tree);

    ui->include_tree->clear();

    for(int i = 0; i < new_widget->topLevelItemCount(); ++i)
    {
        ui->include_tree->insertTopLevelItem(
            i, new_widget->takeTopLevelItem(i)
        );
    }

    delete new_widget;
}
