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

#include "related-model.h"

RelatedFilterProxyModel::RelatedFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

void RelatedFilterProxyModel::setRelatedData( const std::string& f, const std::string& v)
{
    field = f;
    value = v;
}

// Override the virtual filterAcceptsRow to provide custom filtering
// Asks the model to get the value of this->field and compares
// it to this->value.
bool RelatedFilterProxyModel::filterAcceptsRow(int sourceRow,
         const QModelIndex &) const
{
    if (field == "")
        return false;

    ObjectListModel *model = (ObjectListModel *)sourceModel();

    QString row_val(model->fieldValue(sourceRow, field).c_str());
    if (value == "")
        return row_val.isEmpty();
    bool accept = row_val.endsWith(value.c_str());
    return accept;
}
