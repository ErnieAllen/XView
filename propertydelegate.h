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

#ifndef PROPERTYDELEGATE_H
#define PROPERTYDELEGATE_H

#include <QItemDelegate>
#include "sample.h" // for MinMax

class PropertyDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit PropertyDelegate(QObject *parent, const QStringList & cols);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setColumnInfo(const QList<QColor> & colorList, const QList<MinMax> & mmList, const QStringList & nl);

signals:

public slots:

protected:

private:
    QList<MinMax> mmList;
    QList<QColor> colorList;
    QStringList   nameList;
    QStringList allColumns;
};

#endif // PROPERTYDELEGATE_H
