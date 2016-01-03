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
#include <deque>
#include <QtConcurrent/QtConcurrent>

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
    void filterTreeBuilt();

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

    Ui::Dialog *ui;
    std::unique_ptr<cpp_dep::include_graph_t> include_graph_;
    std::unique_ptr<cpp_dep::include_graph_t> filesystem_graph_;

    // Forced to use a raw ptr here because QFuture doesn't support
    // move only types and shared_ptr doesn't have a release function.
    QFutureWatcher<QTreeWidget*> filtered_tree_watcher_;

    template<typename Function, typename... Params>
    void run_or_enqueue(Function fun, Params... params)
    {
        if(work_queue_.size() > 1)
        {
            work_queue_.pop_back();
        }

        work_queue_.push_back(
            [=]()
            {
                return fun(params...);
            }
        );

        if(work_queue_.size() == 1)
        {
            filtered_tree_watcher_.setFuture(QtConcurrent::run(work_queue_.front()));
        }
    }

    void dequeue_and_run()
    {
        if(!work_queue_.empty())
        {
            work_queue_.pop_front();
            if(!work_queue_.empty())
            {
                filtered_tree_watcher_.setFuture(QtConcurrent::run(work_queue_.front()));
            }
        }
    }

    std::deque<std::function<QTreeWidget*()>> work_queue_;
};

#endif // _UI_DIALOG_H_
