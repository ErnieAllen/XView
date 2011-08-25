#ifndef _qe_object_detail_model_h
#define _qe_object_detail_model_h
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

#include <QAbstractListModel>
#include <QModelIndex>
#include <QList>
#include <qmf/Data.h>
#include <sstream>
#include <string>


class ExchangeListModel : public QAbstractListModel {
    Q_OBJECT

public:
    ExchangeListModel(QObject* parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;


public slots:
    void addExchange(const qmf::Data&, uint);
    void connectionChanged(bool isConnected);
    void clear();
    void selected(const QModelIndex &index);

signals:
    void exchangeSelected(const qmf::Data& exchange);

protected:

private:

    // the data for the queues in the display list
    typedef QList<qmf::Data> DataList;
    DataList    dataList;
};

std::ostream& operator<<(std::ostream& out, const qmf::Data& queue);

#endif

