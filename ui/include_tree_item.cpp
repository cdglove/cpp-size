/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*
    IncludeTreeItem.cpp

    A container for items of data supplied by the simple tree model.
*/

#include <QStringList>

#include "include_tree_item.hpp"

IncludeTreeItem::IncludeTreeItem(const QList<QVariant> &data, IncludeTreeItem *parent)
{
    m_parentItem = parent;
    m_itemData = data;
}

IncludeTreeItem::~IncludeTreeItem()
{
    qDeleteAll(m_childItems);
}

void IncludeTreeItem::appendChild(IncludeTreeItem *item)
{
    m_childItems.append(item);
}

IncludeTreeItem *IncludeTreeItem::child(int row)
{
    return m_childItems.value(row);
}

int IncludeTreeItem::childCount() const
{
    return m_childItems.count();
}

int IncludeTreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant IncludeTreeItem::data(int column) const
{
    return m_itemData.value(column);
}

IncludeTreeItem *IncludeTreeItem::parentItem()
{
    return m_parentItem;
}

int IncludeTreeItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<IncludeTreeItem*>(this));

    return 0;
}