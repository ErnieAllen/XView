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

#ifndef RELATEDMODEL_H
#define RELATEDMODEL_H

#include <QSortFilterProxyModel>
#include "object-model.h"

class RelatedFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit RelatedFilterProxyModel(QObject *parent = 0);

    void setRelatedData( const std::string& field, const std::string& value);
    MinMax minMax(int column);

    void clearFilter() { invalidateFilter(); }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

signals:

public slots:

private:
    std::string field;
    std::string value;

};

#endif // RELATEDMODEL_H
