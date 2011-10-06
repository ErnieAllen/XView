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

#ifndef SAMPLE_H
#define SAMPLE_H

#include <QDateTime>
#include <QVariant>
#include <QStringList>
#include <qmf/Data.h>

class Sample
{
public:
    Sample(const qmf::Data& d, const QStringList& l);

    const QDateTime& dateTime() { return _dateTime; }
    qint64 data(const QString& key);
    qint64 data(const std::string& key);

private:
    QDateTime                _dateTime;
    QHash<QString, qint64>   _data;

    bool _valid;
};

#endif // SAMPLE_H
