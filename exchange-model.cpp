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

#include "exchange-model.h"
#include <iostream>

using std::cout;
using std::endl;

ExchangeListModel::ExchangeListModel(QObject* parent) : QAbstractListModel(parent)
{
}


void ExchangeListModel::addExchange(const qmf::Data& exchange, uint correlator)
{
    if (!exchange.isValid())
        return;

    // see if the object already exists in the list
    const qpid::types::Variant& name = exchange.getProperty("name");

    for (int idx=0; idx<dataList.size(); idx++) {
        qmf::Data existing = dataList.at(idx);
        if (name.isEqualTo(existing.getProperty("name"))) {

            qpid::types::Variant::Map map = qpid::types::Variant::Map(exchange.getProperties());
            map["correlator"] = correlator;

            existing.overwriteProperties(map);
            return;
        }
    }

    qmf::Data x = qmf::Data(exchange);
    qpid::types::Variant corr =  qpid::types::Variant(correlator);
    x.setProperty("correlator", corr);

    // this is a new exchange
    int last = dataList.size();
    beginInsertRows(QModelIndex(), last, last);
    dataList.append(x);
    endInsertRows();
}


void ExchangeListModel::connectionChanged(bool isConnected)
{
    if (!isConnected)
        clear();
}

void ExchangeListModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, dataList.count() - 1);
    dataList.clear();
    endRemoveRows();
}


int ExchangeListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return (int) dataList.size();
}


QVariant ExchangeListModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    const qmf::Data& exchange = dataList.at(index.row());
    const qpid::types::Variant& name = exchange.getProperty("name");

    QString n = QString(name.asString().c_str());
    if (n.isEmpty())
        n = QString("Default");
    return n;
}

QVariant ExchangeListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0) {
        if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
            return QString("Exchange");
        }
    }
    return QVariant();
}

// SLOT
void ExchangeListModel::selected(const QModelIndex &index)
{
    if (index.isValid()) {
        const qmf::Data& exchange = dataList.at(index.row());
        emit exchangeSelected(exchange);
    }
}

std::ostream& operator<<(std::ostream& out, const qmf::Data& object)
{
    if (object.isValid()) {
        qpid::types::Variant::Map::const_iterator iter;
        const qpid::types::Variant::Map& attrs(object.getProperties());

        out << " <properties>\n";
        for (iter = attrs.begin(); iter != attrs.end(); iter++) {
            if (iter->first != "name") {
                out << "  <property>\n";
                out << "   <name>" << iter->first << "</name>\n";
                out << "   <value>" << iter->second << "</value>\n";
                out << "  </property>\n";
            }
        }
        out << " </properties>\n";
    }
    return out;
}
