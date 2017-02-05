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
private:

    enum Col
    {
        ColFile,
        ColSize,
        ColPercent,
        ColOrder,
        ColOccurence
    };

public:

    IncludeTreeWidgetItem(QTreeWidget* parent)
        : QTreeWidgetItem(parent)
        , size_(0)
        , order_(0)
    {
        init();
    }

    IncludeTreeWidgetItem(IncludeTreeWidgetItem* parent)
        : QTreeWidgetItem(parent)
        , size_(0)
        , order_(0)
    {}

    void setColumnFile(QString file)
    {
        setText(ColFile, file);
    }

    void setColumnOrder(int order)
    {
        order_ = order;
        setText(ColOrder, QString::number(order));
        setTextAlignment(ColOrder, Qt::AlignRight);
    }

    void setColumnSize(qint64 size, qint64 total_size)
    {
        size_ = size;
        setText(ColSize, QString::number((size+1023)/1024) + "kb");

        qint64 this_size = total_size ? (size * 100) / total_size : 0;
        setText(ColPercent, QString::number(this_size) + "%");
        setTextAlignment(ColSize, Qt::AlignRight);
        setTextAlignment(ColPercent, Qt::AlignRight);
    }

    void setColumnOccurence(int occurence)
    {
        occurence_ = occurence;
        setText(ColOccurence, QString::number(occurence));
        setTextAlignment(ColOccurence, Qt::AlignRight);
    }

    IncludeTreeWidgetItem* parent()
    {
        return static_cast<IncludeTreeWidgetItem*>(QTreeWidgetItem::parent());
    }

private:

    void init()
    {
        setTextAlignment(ColSize, Qt::AlignRight);
        setTextAlignment(ColPercent, Qt::AlignRight);
        setTextAlignment(ColOrder, Qt::AlignRight);
        setTextAlignment(ColOccurence, Qt::AlignRight);
    }

    bool operator<(QTreeWidgetItem const& o) const override
    {
        IncludeTreeWidgetItem const& other = *static_cast<IncludeTreeWidgetItem const*>(&o);

        int column = treeWidget()->sortColumn();
        switch(column)
        {
        case ColFile:
            return text(ColFile) < other.text(ColFile);
            break;
        case ColSize:
        case ColPercent:
            return size_ < other.size_;
        case ColOrder:
            return order_ < other.order_;
        case ColOccurence:
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
