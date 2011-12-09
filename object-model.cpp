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

ObjectListModel::ObjectListModel(QObject* parent, std::string unique, const QStringList& columnList) :
        QAbstractTableModel(parent), uniqueProperty(unique),
        sampleProperties(),
        invalid()
{
    sampleLife = 600;
    sampleProperties = columnList;
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
    //dataHash[name.asString()] = o;
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

int ObjectListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return sampleProperties.size() + 1;
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

// Return the min and max for this column
MinMax ObjectListModel::minMax(const std::string & name)
{
    MinMax mm = MinMax();
    for (int idx=0; idx<dataList.size(); idx++) {

        const qmf::Data& object(dataList[idx]);

        const qpid::types::Variant::Map& props(object.getProperties());
        qpid::types::Variant::Map::const_iterator iter;

        iter = props.find(name);
        if (iter != props.end()) {
            qreal val = (qreal)(*iter).second.asInt64();
            mm.max = qMax(val, mm.max);
            mm.min = qMin(val, mm.min);
        }
    }
    return mm;
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

    // we want the entire data object at this row
    if (role == Qt::UserRole) {
        return QVariant(dataList.at(index.row()));
    }

    if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
        return QVariant();

    const qmf::Data& object(dataList[index.row()]);
    //const qmf::Data& object = dataList.at(index.row());

    const qpid::types::Variant::Map& props(object.getProperties());
    qpid::types::Variant::Map::const_iterator iter;

    // for the "name" column, we will show either the key (prefered) or the unique field
    if (index.column() == 0) {
        iter = props.find(dataKey);
        if (iter == props.end()) {
            iter = props.find(uniqueProperty);
        }

        if (iter != props.end())
            return QString((*iter).second.asString().c_str());

        return QString();
    }
    // for the value columns (shown in the related table) return the numeric value
    int col = index.column() - 1;
    std::string prop = sampleProperties.at(col).toStdString();
    iter = props.find(prop);
    if (iter != props.end()) {
        if (role == Qt::DisplayRole)
            return QVariant((qreal)(*iter).second.asInt64());
        else {
            return QString("%1 %2").arg(prop.c_str()).arg((*iter).second.asInt64());
        }
    }
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
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == 0) {
                if (dataKey == "")
                    return QString(uniqueProperty.c_str());
                else
                    return QString(dataKey.c_str());
            }
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

void ObjectListModel::clearSamples()
{
    samplesData.clear();
}
