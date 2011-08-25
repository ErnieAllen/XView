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

#include "exchange-details.h"
#include <iostream>

using std::cout;
using std::endl;

ExchangeDetailsModel::ExchangeDetailsModel(QObject* parent) : QAbstractTableModel(parent)
{
    // Intentionally Left Blank
}


void ExchangeDetailsModel::showExchangeDetail(const qmf::Data& exchange)
{
    if (!exchange.isValid())
        return;

    clear();

    const qpid::types::Variant::Map& attrs(exchange.getProperties());

    beginInsertRows(QModelIndex(), 0, attrs.size() - 1);
    for (qpid::types::Variant::Map::const_iterator iter = attrs.begin();
         iter != attrs.end(); iter++) {
        keys << QString(iter->first.c_str());
        values << QString(iter->second.asString().c_str());
    }
    endInsertRows();
}


void ExchangeDetailsModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, keys.size() - 1);
    keys.clear();
    values.clear();
    endRemoveRows();
}


int ExchangeDetailsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return (int) keys.size();
}


int ExchangeDetailsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}


QVariant ExchangeDetailsModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (!index.isValid())
        return QVariant();

    QString name;
    switch (index.column()) {
    case 0: return keys.at(index.row());
    case 1:
        name = keys.at(index.row());
        if ((name == "name") && (values.at(index.row()).isEmpty()))
            return QString("Default");
        return values.at(index.row());
    }
    return 0;
}


QVariant ExchangeDetailsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    switch (section) {
    case 0: return QString("Key");
    case 1: return QString("Value");
    }

    return QVariant();
}

