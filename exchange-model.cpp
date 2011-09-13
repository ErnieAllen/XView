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

ExchangeListModel::ExchangeListModel(QObject* parent, std::string unique) :
        ObjectListModel(parent, unique)
{
}

QVariant ExchangeListModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    const qmf::Data& exchange = dataList.at(index.row());
    const qpid::types::Variant& name = exchange.getProperty(uniqueProperty);

    QString n = QString(name.asString().c_str());
    if (n.isEmpty())
        n = QString("Default");
    return n;
}
