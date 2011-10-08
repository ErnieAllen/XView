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

#include <QSharedData>
#include <QDateTime>
#include <QVariant>
#include <QStringList>
#include <qmf/Data.h>
#include <float.h> // for DBL_MAX

class MinMax {
public:

    MinMax () {min = DBL_MAX; max=DBL_MAX * -1.0; }
    qreal min;
    qreal max;
};

class SampleData : public QSharedData
{
public:
    SampleData()  { }
    SampleData(const SampleData& other)
        : QSharedData(other), dateTime(other.dateTime), data(other.data) { }
    ~SampleData() { }

    QDateTime               dateTime;
    QHash<QString, qint64>  data;
};

class Sample
{
public:
    Sample() { d = new SampleData; }
    Sample(const qmf::Data& data, const QStringList& list, QDateTime dt=QDateTime::currentDateTime());
    Sample(const Sample& other) : d (other.d) { }

    void setDateTime(const QDateTime & dt) { d->dateTime = dt; }
    void setProperty(const QString& key, qint64 value) { d->data.insert(key, value); }

    QDateTime dateTime() const { return d->dateTime; }
    qint64 data(const QString& key) const { return d->data.value(key, 0); }
    qint64 data(const std::string& key) const { return data(QString(key.c_str())); }

private:
    QSharedDataPointer<SampleData> d;

};

#endif // SAMPLE_H
