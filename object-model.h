#ifndef _object_model_h
#define _object_model_h
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

#include <QAbstractListModel>
#include <QModelIndex>
#include <QLinkedList>
#include <QHash>
#include <QDateTime>
#include <qmf/Data.h>
#include <sstream>
#include <string>
#include "sample.h"

class ObjectListModel : public QAbstractTableModel {
    Q_OBJECT

public:
    ObjectListModel(QObject* parent, std::string unique, const QStringList& columnList);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    std::string fieldValue(int row, const std::string& field);
    const qmf::Data& qmfData(int row);
    const qmf::Data& find(const qmf::Data& existing);
    void refresh(uint correlator);
    void expireSamples();
    void setDuration(int duration) { sampleLife = duration; }
    void clearSamples();
    void setKey(const std::string &altKey);
    const std::string &unique(bool useKey);

    // list of values for an individual object
    typedef QLinkedList<Sample> SampleList;
    typedef QLinkedList<Sample>::const_iterator const_iterSampleList;

    // hash of lists keyed by object name
    typedef QHash<QString, SampleList> Samples;
    typedef QHash<QString, SampleList>::const_iterator const_iterSamples;

    const Samples& samples() { return samplesData; }
    const qmf::Data& getSelected(const QModelIndex &index);

public slots:
    void addObject(const qmf::Data&, uint);
    void connectionChanged(bool isConnected);
    void clear();
    void selected(const QModelIndex &index);

signals:
    void objectSelected(const qmf::Data&);

protected:
    typedef QLinkedList<Sample>::iterator iterSampleList;
    typedef QHash<QString, SampleList>::iterator iterSamples;

    // the data for the objects in the display listbox
    // This is the list of Queues or Exchanges or whatever the object happens to be.
    typedef QList<qmf::Data> DataList;
    DataList    dataList;

    std::string dataKey; // field name used in list if "uniqueProperty" is absent
    std::string uniqueProperty;

    int sampleLife;
    void addSample(const qmf::Data& object, const qpid::types::Variant& name);

private:
    // historical data for rates and charting
    Samples samplesData;

    // list of properties to save for charting
    QStringList sampleProperties;
    qmf::Data invalid;

    QHash<std::string, qmf::Data&> dataHash;
};

std::ostream& operator<<(std::ostream& out, const qmf::Data& queue);

#endif

