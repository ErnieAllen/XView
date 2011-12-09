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

#ifndef RELATEDHEADERVIEW_H
#define RELATEDHEADERVIEW_H

#include <QHeaderView>
#include "sample.h"

class RelatedHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit RelatedHeaderView(Qt::Orientation orientation, QWidget *parent = 0);

    void setColumnInfo(const QList<QColor> & colorList, const QStringList & nl);
    void setAllColumns(const QStringList & cols);
    void moveNameColumn();

signals:

public slots:

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    int actual2Logical(int col) const;

private:
    QList<QColor> colorList;
    QStringList   nameList;
    QStringList allColumns;
    bool moved;
};

#endif // RELATEDHEADERVIEW_H
