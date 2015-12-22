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
#include "ui/tree_view_builder.hpp"
#include "util/filtered_subgraph_builder.hpp"
#include "ui_dialog.h"
#include "cpp_dep/cpp_dep.hpp"
#include <boost/graph/depth_first_search.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
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
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
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
    std::vector<std::string> match_list;
    std::string filter_text_std = filter_text.toStdString();
    boost::algorithm::split(
        match_list, filter_text_std,
        boost::algorithm::is_any_of(" \0\t\r"),
        boost::algorithm::token_compress_on);

    match_list.erase(
        std::remove_if(
            match_list.begin(),
            match_list.end(),
            [](std::string const& s)
            {
                return s.empty();
            }
        ),
        match_list.end()
    );

    ui->include_tree->clear();

    if(match_list.empty())
    {
        tree_view_builder build_tree(ui->include_tree);
        boost::depth_first_search(
            *include_graph_, boost::visitor(build_tree));
    }
    else
    {
        auto match_all_substrings = [&match_list](
            cpp_dep::include_vertex_descriptor_t const& v,
            cpp_dep::include_graph_t const& g)
        {
            cpp_dep::include_vertex_t const& file = g[v];
            return std::all_of(
                match_list.begin(),
                match_list.end(),
                [&file](std::string const& sub_str)
                {
                    return file.name.find(sub_str) != std::string::npos;
                }
            );
        };

        cpp_dep::include_graph_t result_graph;
        filtered_subgraph_builder<
            cpp_dep::include_graph_t
        >graph_filter(match_all_substrings, result_graph);

        boost::depth_first_search(
            *include_graph_, boost::visitor(graph_filter));

        tree_view_builder build_tree(ui->include_tree);
        boost::depth_first_search(
            result_graph, boost::visitor(build_tree));
    }
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

    // Populate the include tree
    {
        tree_view_builder build_tree(ui->include_tree);
        boost::depth_first_search(
            *include_graph_, boost::visitor(build_tree));
    }

    // Populate the filesystem tree
    {
        tree_view_builder build_tree(ui->filesystem_tree);
        boost::depth_first_search(
            *filesystem_graph_, boost::visitor(build_tree));
    }
}
