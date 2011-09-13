#ifndef _object_details_h
#define _object_details_h
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

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QStringList>
#include <qmf/Data.h>
#include <sstream>
#include <string>

class ObjectDetailsModel : public QAbstractTableModel {
    Q_OBJECT

public:
    ObjectDetailsModel(QObject* parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public slots:
    void showObjectDetail(const qmf::Data&);
    void connectionChanged(bool isConnected);
    void clear();
signals:
    void detailReady();

protected:
    QStringList keys;
    QStringList values;
};

#endif

