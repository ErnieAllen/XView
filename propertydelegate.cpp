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

#include "propertydelegate.h"
#include <QPainter>

PropertyDelegate::PropertyDelegate(QObject *parent, const QStringList &cols) :
    QItemDelegate(parent),
    mmList(),
    colorList(),
    nameList(),
    allColumns(cols)
{
}

void PropertyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->fillRect(option.rect, QBrush(Qt::white));

    int column = index.column();
    // draw the data for the name column
    if (column == 0) {
        QItemDelegate::paint(painter, option, index);
        return;
    }

    // draw a value column as a colored bar
    QString colName = allColumns.at(column - 1);

    int infoIndex = nameList.indexOf(colName);
    if (infoIndex >= 0) {
        MinMax mm = mmList.at(infoIndex);
        QColor color = colorList.at(infoIndex);

        qreal val = index.model()->data(index, Qt::DisplayRole).toReal();
        float percent;
        if (mm.max > 0)
            percent = val * 100.0 / mm.max;
        else
            percent = 100.0;

        //qDebug("property: colName:%s  val:%f  percent: %f max:%f  rect.width():%i", colName.toStdString().c_str(), (float)val, percent, (float)mm.max, option.rect.width());
        int width = percent * option.rect.width();
        if (percent > 0.0 && width < 1)
            width = 1;
        painter->fillRect(option.rect.x(), option.rect.y() + option.rect.height() / 2 - 8, width, 16, QBrush(color));
    } else {
        qDebug("*** can't find column %s", colName.toStdString().c_str());
    }
}

void PropertyDelegate::setColumnInfo(const QList<QColor> & c, const QList<MinMax> & mm, const QStringList & nl)
{
    mmList = mm;
    colorList = c;
    nameList = nl;
}
