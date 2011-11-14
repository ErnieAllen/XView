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

#include "object-model.h"
#include <iostream>

using std::cout;
using std::endl;

ObjectListModel::ObjectListModel(QObject* parent, std::string unique) :
        QAbstractListModel(parent), uniqueProperty(unique),
        sampleProperties(),
        invalid()
{
    sampleLife = 600;
}

void ObjectListModel::addObject(const qmf::Data& object, uint correlator)
{
    if (!object.isValid())
        return;

    // get the uniqueu property for this object
    const qpid::types::Variant& name = object.getProperty(uniqueProperty);

    // create a new sample
    addSample(object, name);

    // see if the object exists in the list
    for (int idx=0; idx<dataList.size(); idx++) {
        qmf::Data existing = dataList.at(idx);
        if (name.isEqualTo(existing.getProperty(uniqueProperty))) {

            qpid::types::Variant::Map map = qpid::types::Variant::Map(object.getProperties());
            map["correlator"] = correlator;
            existing.overwriteProperties(map);
            return;
        }
    }

    qmf::Data o = qmf::Data(object);
    qpid::types::Variant corr =  qpid::types::Variant(correlator);
    o.setProperty("correlator", corr);

    // this is a new queue
    int last = dataList.size();
    beginInsertRows(QModelIndex(), last, last);
    dataList.append(o);
    endInsertRows();
}

void ObjectListModel::refresh(uint correlator)
{
    // remove any old queues that were not added/updated with this correlator
    for (int idx=0; idx<dataList.size(); idx++) {
        uint corr = dataList.at(idx).getProperty("correlator").asUint32();
        if (corr != correlator) {
            // clear out the old samples
            QString name(dataList.at(idx).getProperty(uniqueProperty).asString().c_str());
            if (samplesData.contains(name)) {
                samplesData.remove(name);
            }
            beginRemoveRows( QModelIndex(), idx, idx );
            dataList.removeAt(idx--);
            endRemoveRows();
        }
    }

    // force a refresh of the display
    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(dataList.size() - 1, 2);
    emit dataChanged ( topLeft, bottomRight );
}


void ObjectListModel::connectionChanged(bool isConnected)
{
    if (!isConnected)
        clear();
}

void ObjectListModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, dataList.count() - 1);
    dataList.clear();
    endRemoveRows();
}


int ObjectListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return (int) dataList.size();
}


void ObjectListModel::setKey(const std::string &altKey)
{
    dataKey = altKey;
}

const std::string &ObjectListModel::unique(bool useKey)
{
    if (useKey) {
        if (dataKey != "") {
            return dataKey;
        }
    }

    return uniqueProperty;
}

const qmf::Data& ObjectListModel::find(const qmf::Data& existing)
{
    const qpid::types::Variant& name = existing.getProperty(uniqueProperty);
    for (int idx=0; idx<dataList.size(); idx++) {
        if (name.isEqualTo(dataList.at(idx).getProperty(uniqueProperty))) {
            return dataList.at(idx);
        }
    }
    return invalid;
}

QVariant ObjectListModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    if (role == Qt::UserRole) {
        return QVariant(dataList.at(index.row()));
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    const qmf::Data& object= dataList.at(index.row());

    const qpid::types::Variant::Map& props(object.getProperties());
    qpid::types::Variant::Map::const_iterator iter;

    iter = props.find(dataKey);
    if (iter == props.end()) {
        iter = props.find(uniqueProperty);
    }

    if (iter != props.end())
        return QString((*iter).second.asString().c_str());

    return QString();
}

std::string ObjectListModel::fieldValue(int row, const std::string& field)
{
    const qmf::Data& object= dataList.at(row);
    qpid::types::Variant value = object.getProperty(field);
    if ((field == "queueRef") ||
        (field == "exchangeRef") ||
        (field == "sessionRef") ||
        (field == "connectionRef") ||
        (field == "vhostRef")) {
        return value.asMap()["_object_name"].asString();
    }
    return value.asString();
}

const qmf::Data& ObjectListModel::qmfData(int row)
{
    return dataList.at(row);
}

QVariant ObjectListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0) {
        if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
            return QString("List");
        }
    }
    return QVariant();
}

// SLOT
void ObjectListModel::selected(const QModelIndex &index)
{
    if (index.isValid()) {
        const qmf::Data& object = dataList.at(index.row());
        // show the details for this object in the dialog's object table
        emit objectSelected(object);
    }
}

const qmf::Data& ObjectListModel::getSelected(const QModelIndex &index)
{
    return dataList.at(index.row());
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

void ObjectListModel::setSampleProperties(const QStringList& list)
{
    sampleProperties = list;
}

void ObjectListModel::addSample(const qmf::Data& object, const qpid::types::Variant& name)
{
    QString key = QString(name.asString().c_str());
    samplesData[key].append(Sample(object, sampleProperties));
}

void ObjectListModel::expireSamples()
{
    QDateTime tnow(QDateTime::currentDateTime());
    int secs;

    QStringList keys = samplesData.keys();
    QStringList::const_iterator iter = keys.constBegin();
    while (iter != keys.constEnd()) {
        SampleList &sampleList(samplesData[*iter]);

        iterSampleList iterList = sampleList.begin();
        while (iterList != sampleList.end()) {
            secs = (*iterList).dateTime().secsTo(tnow);
            if (secs > sampleLife) {
                // erase increments iterList
                iterList = sampleList.erase(iterList);
            } else
                break;
        }

        ++iter;
    }
}

// get the min and max for the given properties in the Sample list for name
MinMax ObjectListModel::minMax(const QString& name, const QStringList& props, bool isRate)
{
    MinMax mm = MinMax();
    SampleList& list(samplesData[name]);

    QStringList::const_iterator prop = props.constBegin();
    while (prop != props.constEnd()) {
        QString p = *prop;
        ObjectListModel::const_iterSampleList iter = list.constBegin();
        while (iter != list.constEnd()) {
            Sample sample1 = *iter;
            qint64 v1 = sample1.data(p);
            if (!isRate) {
                mm.min = qMin(mm.min, (qreal)v1);
                mm.max = qMax(mm.max, (qreal)v1);
            } else {
                QDateTime t1 = sample1.dateTime();
                ++iter;
                if (iter != list.constEnd()) {
                    Sample sample2 = *iter;
                    qint64 v2 = sample2.data(p);
                    QDateTime t2 = sample2.dateTime();
                    int secs = t1.secsTo(t2);
                    if (secs) {
                        mm.min = qMin(mm.min, (v2 - v1) / (qreal)secs);
                        mm.max = qMax(mm.max, (v2 - v1) / (qreal)secs);
                    }

                }
                // we don't want to increment iter twice
                --iter;
            }
            ++iter;
        }
        ++prop;
    }

    return mm;
}

void ObjectListModel::clearSamples()
{
    samplesData.clear();
}
