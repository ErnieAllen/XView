/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "relatedheaderview.h"
#include <QPainter>

RelatedHeaderView::RelatedHeaderView(Qt::Orientation orientation, QWidget *parent) :
    QHeaderView(orientation, parent),
    colorList(),
    nameList(),
    allColumns()
{
    moved = false;
}

void RelatedHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    static const QPoint sortArrow[] = {
        QPoint(0,  0),
        QPoint(10, 0),
        QPoint(5,  7)
    };

    if (logicalIndex == 0) {
        QHeaderView::paintSection(painter, rect, logicalIndex);
        return;
    }
    painter->setRenderHint(QPainter::Antialiasing);

    int infoIndex = logicalIndex - 1;

    QString colName = allColumns.at(infoIndex);
    int nameIndex = nameList.indexOf(colName);
    if (nameIndex >= 0 && nameIndex < colorList.size()) {
        QColor color(colorList.at(nameIndex));

        QPen pen(QColor(Qt::black));
        QBrush brush(color);
        pen.setWidth(1);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawRect(rect.x() + 2, rect.y() + rect.height() /2 - 6, 12, 12);
        if (sortIndicatorSection() == logicalIndex) {
            QBrush whiteBrush(Qt::white);
            QPen whitePen(QColor(Qt::white));
            painter->setBrush(whiteBrush);
            painter->setPen(whitePen);
            if (sortIndicatorOrder() == Qt::AscendingOrder) {
                painter->translate(rect.x() + 13, 17);
                painter->rotate(180.0);
            } else {
                painter->translate(rect.x() + 3, 10);
            }
            painter->drawPolygon(sortArrow, 3);
        }

    }
}

void RelatedHeaderView::setColumnInfo(const QList<QColor> & c, const QStringList & n)
{
    colorList = c;
    nameList = n;
}

void RelatedHeaderView::setAllColumns(const QStringList & cols)
{
    allColumns = cols;
}

void RelatedHeaderView::moveNameColumn()
{
    if (!moved) {
        moved = true;
        this->moveSection(0, this->count() - 1);
    }
}
