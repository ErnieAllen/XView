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

#include "sample.h"


Sample::Sample(const qmf::Data& data, const QStringList& list) :
        _dateTime(QDateTime::currentDateTime()),
        _data()
{

    QStringList::const_iterator iter = list.begin();
    while (iter != list.end()) {
        /*
        const QString& s(*iter);
        qpid::types::Variant qv = data.getProperty(s.toStdString());
        quint64 value = qv.asUint64();
        _data.insert(s, value);
        */
        _data.insert(*iter, data.getProperty((*iter).toStdString()).asInt64());
        ++iter;
    }
}

qint64 Sample::data(const QString &key)
{
    return _data.value(key, 0);
}

qint64 Sample::data(const std::string& key)
{
    return data(QString(key.c_str()));
}