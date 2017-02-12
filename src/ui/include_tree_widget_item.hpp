// *****************************************************************************
// 
// ui/include_tree_widget_item.cpp
//
// Special include tre widget that corres some data with it so we can sort 
// correctly, etc.
//
// Copyright Chris Glover 2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// *****************************************************************************
#ifndef _UI_INCLUDETREEWIDGET_HPP_
#define _UI_INCLUDETREEWIDGET_HPP_

#include <QTreeWidgetItem>

// -----------------------------------------------------------------------------
//
class IncludeTreeWidgetItem : public QTreeWidgetItem 
{
public:

    struct Column
    {
        enum
        {
            File,
            Size,
            Percent,
            Order,
            Occurence,
        };
    };

    IncludeTreeWidgetItem(QTreeWidget* parent)
        : QTreeWidgetItem(parent)
        , size_(0)
        , order_(0)
        , occurence_(0)
    {
        init();
    }

    IncludeTreeWidgetItem(IncludeTreeWidgetItem* parent)
        : QTreeWidgetItem(parent)
        , size_(0)
        , order_(0)
        , occurence_(0)
    {
        init();
    }

    void setColumnFile(QString file)
    {
        setText(Column::File, file);
    }

    void setColumnOrder(int order)
    {
        order_ = order;
        setText(Column::Order, QString::number(order));
    }

    void setColumnSize(qint64 size, qint64 total_size)
    {
        size_ = size;
        setText(Column::Size, QString::number((size+1023)/1024) + "kb");

        qint64 this_size = total_size ? (size * 100) / total_size : 0;
        setText(Column::Percent, QString::number(this_size) + "%");
    }

    qint64 getColumnSize() const
    {
        return size_;
    }

    void setColumnOccurence(int occurence)
    {
        occurence_ = occurence;
        setText(Column::Occurence, QString::number(occurence));
    }

    int getColumnOccurence() const
    {
        return occurence_;
    }

    void showCheckbox()
    {
        setFlags(flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        setCheckState(Column::File, Qt::Checked);
    }

    bool isChecked() const
    {
        return checkState(Column::File) == Qt::Checked;
    }

    IncludeTreeWidgetItem* parent()
    {
        return static_cast<IncludeTreeWidgetItem*>(QTreeWidgetItem::parent());
    }

private:

    void init()
    {
        setTextAlignment(Column::Size, Qt::AlignRight);
        setTextAlignment(Column::Percent, Qt::AlignRight);
        setTextAlignment(Column::Order, Qt::AlignRight);
        setTextAlignment(Column::Occurence, Qt::AlignRight);
    }

    bool operator<(QTreeWidgetItem const& o) const override
    {
        IncludeTreeWidgetItem const& other = *static_cast<IncludeTreeWidgetItem const*>(&o);

        int column = treeWidget()->sortColumn();
        switch(column)
        {
        case Column::File:
            return text(Column::File) < other.text(Column::File);
            break;
        case Column::Size:
        case Column::Percent:
            return size_ < other.size_;
        case Column::Order:
            return order_ < other.order_;
        case Column::Occurence:
            return occurence_ < other.occurence_;
        default:
            Q_ASSERT(false && "Invalid column");
            return order_ < other.order_;
        }
    }

    qint64 size_;
    int order_;
    int occurence_;
};

#endif // _UI_INCLUDETREEWIDGET_HPP_
