// *****************************************************************************
// 
// ui/dialog.h
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

// -----------------------------------------------------------------------------
//
namespace Ui {
class Dialog;
}

class QTreeWidget;

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
    void onTreeFiltered();

    Ui::Dialog *ui;
    std::unique_ptr<cpp_dep::include_graph_t> include_graph_;
    std::unique_ptr<cpp_dep::include_graph_t> filesystem_graph_;

    // Forced to use a raw ptr here because QFuture doesn't support
    // move only types and shared_ptr doesn't have a release function.
    QFutureWatcher<QTreeWidget*> filtered_tree_watcher_;
};

#endif // _UI_DIALOG_H_
