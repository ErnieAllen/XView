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

#include "widgetconnections.h"
#include <QPainter>

WidgetConnections::WidgetConnections(QWidget *parent) :
    WidgetQmfObject(parent)
{
    this->setSectionName(QString("Connections"));
    summaryColumns.append(Column("msgsToClient", "To client", Qt::AlignRight, "N", modeMessages, true));
    summaryColumns.append(Column("msgsFromClient", "From client", Qt::AlignRight, "N", modeMessages, true, QColor(Qt::green)));

    summaryColumns.append(Column("msgsToClient", "To client / sec", Qt::AlignRight, "N", modeMessageRate, true));
    summaryColumns.append(Column("msgsFromClient", "From client / sec", Qt::AlignRight, "N", modeMessageRate, true, QColor(Qt::green)));

    summaryColumns.append(Column("bytesToClient", "To client", Qt::AlignRight, "B", modeBytes, true));
    summaryColumns.append(Column("bytesFromClient", "From client", Qt::AlignRight, "B", modeBytes, true, QColor(Qt::green)));

    summaryColumns.append(Column("bytesToClient", "To client / sec", Qt::AlignRight, "N", modeByteRate, true));
    summaryColumns.append(Column("bytesFromClient", "From client / sec", Qt::AlignRight, "N", modeByteRate, true, QColor(Qt::green)));

    setRelatedText("Related Connections");
}

WidgetConnections::~WidgetConnections()
{
}

void WidgetConnections::paintEvent(QPaintEvent *e)
{
    // points that define an arrow
    static const QPoint monitorPoints[] = {
        QPoint(4,  -10),
        QPoint(26, -10),
        QPoint(26,  10),
        QPoint(4,   10)
    };
    static const QPoint keyPoints[] = {
        QPoint(4,  -6),
        QPoint(26, -6),
        QPoint(30,  6),
        QPoint(0,   6)
    };

    WidgetQmfObject::paintEvent(e);

    QPainter painter(this);

    QColor lineColor = getLineColor();
    QColor fillColor = getFillColor();
    QPen pen(lineColor);
    pen.setWidth(2);
    QBrush brush(fillColor);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.setBrush(brush);

    int mid = mid_paint();

    painter.drawRect(0, mid, width(), 10);

    painter.save();

    painter.translate(width() / 2 - 15, mid);
    painter.drawPolygon(monitorPoints,  sizeof( monitorPoints ) / sizeof( monitorPoints[0] ));

    painter.translate(0, 10);
    painter.drawPolygon(keyPoints,  sizeof( keyPoints ) / sizeof( keyPoints[0] ));

    painter.restore();
}

void WidgetConnections::showRelated(const qmf::Data& object, const QString &, ArrowDirection a)
{
    if (!updateAll)
        if (this->hasData() && (arrow() != arrowNone)) {
            //qDebug("showRelated: %s needs an update", this->objectName().toStdString().c_str());
            emit needUpdate();
            return;
        }

    setArrow(a);

    // for connections, we should only be asked to show records related to a session
    // so assume the object is a session
    qpid::types::Variant value = object.getProperty("connectionRef");
    QString name(value.asMap()["_object_name"].asString().c_str());
    QString connection = name.section(':', -1);

    related->setRelatedData("address", connection.toStdString());
    related->clearFilter();
    //qDebug("showRelated: %s needs new data", this->objectName().toStdString().c_str());
    emit needData();

}

QString WidgetConnections::unique_property()
{
    QString prop = WidgetQmfObject::unique_property();
    // if the remoteProcessName is absent (it is optional after all)
    // then use the address as the object's "name"
    if (prop.isEmpty()) {
        if (data.isValid()) {
            const qpid::types::Variant::Map& props(data.getProperties());
            qpid::types::Variant::Map::const_iterator iter;

            iter = props.find("address");
            if (iter != props.end()) {
                prop = QString(iter->second.asString().c_str());
            }
        }
    }
    return prop;
}
