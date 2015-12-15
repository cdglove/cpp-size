// *****************************************************************************
// 
// ui/dialog.cpp
//
// Main dialog for cpp-size
//
// Copyright Chris Glover 2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// *****************************************************************************
#include "ui/dialog.hpp"
#include "ui/include_tree_widget_item.hpp"
#include "ui_dialog.h"
#include "cpp_dep/cpp_dep.hpp"
#include <boost/graph/depth_first_search.hpp>
#include <QFileInfo>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMessageBox>

// -----------------------------------------------------------------------------
//
namespace cpp_dep
{
    // To be supplied by the application;
    std::size_t get_file_size(char const* path)
    {
        QFileInfo finfo(path);
        return finfo.size();
    }
}

// -----------------------------------------------------------------------------
//
struct tree_view_builder : public boost::default_dfs_visitor
{
    tree_view_builder(QTreeWidget* tree)
        : tree_(tree)
        , current_item_(nullptr)
        , current_order_(1)
    { }

    template <typename Vertex, typename Graph>
    void start_vertex(Vertex const& v, Graph const& g)
    {
        cpp_dep::include_vertex_t const& file = g[v];

        current_item_ = new IncludeTreeWidgetItem(tree_);
        current_item_->setColumnFile(file.name.c_str());
        current_item_->setColumnSize(file.size + file.size_dependencies);
        current_item_->setColumnOrder(0);
        size_stack_.push_back(0);
    }

    template <typename Edge, typename Graph>
    void examine_edge(Edge const& e, Graph const& g)
    {
        cpp_dep::include_vertex_t const& source = g[e.m_source];
        cpp_dep::include_vertex_t const& file = g[e.m_target];

        current_item_ = new IncludeTreeWidgetItem(current_item_);
        current_item_->setColumnFile(file.name.c_str());
        current_item_->setColumnSize(file.size + file.size_dependencies);
        current_item_->setColumnOrder(current_order_++);
    }

    template <typename Vertex, typename Graph>
    void finish_vertex(Vertex const&, Graph const&)
    {
        current_item_ = current_item_->parent();
    }

    QTreeWidget* tree_;
    IncludeTreeWidgetItem* current_item_;
    int current_order_;
    std::vector<qint64> size_stack_;
};

// -----------------------------------------------------------------------------
//
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
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

                ui->include_hierarchy->clear();
                tree_view_builder build_tree(ui->include_hierarchy);
                boost::depth_first_search(includes, boost::visitor(build_tree));
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
