// *****************************************************************************
//
// ui/dialog.hpp
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
#ifndef _UI_DIALOG_HPP_
#define _UI_DIALOG_HPP_

#include <QDialog>
#include <QFutureWatcher>
#include <memory>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/tag.hpp>
#include "cpp_dep/cpp_dep.hpp"
#include "async_ui_task.hpp"

// -----------------------------------------------------------------------------
//
namespace Ui {
class Dialog;
}

class QTreeWidget;
class QTreeWidgetItem;
class IncludeTreeWidgetItem;

// -----------------------------------------------------------------------------
//
namespace cpp_dep {
    class include_graph_t;
};

// -----------------------------------------------------------------------------
//
class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:

    void filterTextChanged(QString const& filter_text);
    void includeTreeItemChanged(QTreeWidgetItem* item, int column);

private:

    // -------------------------------------------------------------------------
    // Event overrides
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;

    // -------------------------------------------------------------------------
    // private helpers.
    void populateTrees();
    void filterTreeBuilt(QTreeWidget* new_widget);

    Ui::Dialog *ui;
    std::unique_ptr<cpp_dep::include_graph_t> include_graph_;
    std::unique_ptr<cpp_dep::include_graph_t> filesystem_graph_;
    async_ui_task<QTreeWidget*> update_tree_widget_;
    bool building_include_tree_;

    struct include_node
    {
        cpp_dep::include_vertex_descriptor_t vert;
        IncludeTreeWidgetItem* item;
        int occurence;
    };

    struct by_item {};
    struct by_ordinal {};

    typedef boost::multi_index_container<
      include_node,
      boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
          boost::multi_index::tag<by_item>,
          boost::multi_index::member<include_node, IncludeTreeWidgetItem*, &include_node::item>
        >,
        boost::multi_index::ordered_unique<
          boost::multi_index::tag<by_ordinal>,
          boost::multi_index::composite_key<
            include_node,
            boost::multi_index::member<include_node, cpp_dep::include_vertex_descriptor_t, &include_node::vert>,
            boost::multi_index::member<include_node, int, &include_node::occurence>
          >
        >
      >
    > include_item_database_t;

    include_item_database_t include_item_db;

    struct include_tree_view_builder;
};

#endif // _UI_DIALOG_H_
