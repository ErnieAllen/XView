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

#include "widgetexchanges.h"
#include <QPainter>

WidgetExchanges::WidgetExchanges(QWidget *parent) :
    WidgetQmfObject(parent)
{
    this->setSectionName(QString("Exchanges"));
    summaryColumns.append(Column("msgRoutes", "routed", Qt::AlignRight, "N", modeMessages));
    summaryColumns.append(Column("msgReceives", "received", Qt::AlignRight, "N", modeMessages));
    summaryColumns.append(Column("msgDrops", "dropped", Qt::AlignRight, "N", modeMessages));

    summaryColumns.append(Column("byteRoutes", "routed", Qt::AlignRight, "N", modeBytes));
    summaryColumns.append(Column("byteReceives", "received", Qt::AlignRight, "N", modeBytes));
    summaryColumns.append(Column("byteDrops", "dropped", Qt::AlignRight, "N", modeBytes));

    summaryColumns.append(Column("msgRoutes", "routed / sec", Qt::AlignRight, "N", modeMessageRate));
    summaryColumns.append(Column("msgReceives", "received / sec", Qt::AlignRight, "N", modeMessageRate));
    summaryColumns.append(Column("msgDrops", "dropped / sec", Qt::AlignRight, "N", modeMessageRate));

    summaryColumns.append(Column("byteRoutes", "routed / sec", Qt::AlignRight, "N", modeByteRate));
    summaryColumns.append(Column("byteReceives", "received / sec", Qt::AlignRight, "N", modeByteRate));
    summaryColumns.append(Column("byteDrops", "dropped / sec", Qt::AlignRight, "N", modeByteRate));

    setRelatedText("Related exchanges");

}

WidgetExchanges::~WidgetExchanges()
{
}

void WidgetExchanges::paintEvent(QPaintEvent *e)
{

    // points that define an arrow
    static const QPoint arrowPoints[] = {
        QPoint(10,  -10),
        QPoint(30, -10),
        QPoint(30, -20),
        QPoint(40,  0),
        QPoint(30,  20),
        QPoint(30,  10),
        QPoint(10,   10)
    };

    WidgetQmfObject::paintEvent(e);

    QColor exchangeColor(200, 200, 200);
    QPen exchangePen(exchangeColor);
    exchangePen.setWidth(2);
    QBrush exchangeBrush(Qt::white);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(exchangePen);
    painter.setBrush(exchangeBrush);

    //int reserved = reservedY();
    //int h = height() - reserved;
    //int side = qMin(width(), h);

    int mid = this->mid_paint();
    //painter.drawEllipse(width() - side + 2, h / 2 - side / 2 + reserved + 2, side-3, side-3);

    // or
    painter.drawRect(0, mid - 5, width(), 10);

    painter.save();
    painter.translate(width() / 2, mid);
    painter.rotate(45);
    for (int i=0; i<4; i++) {
        painter.drawPolygon(arrowPoints,  sizeof( arrowPoints ) / sizeof( arrowPoints[0] ));
        painter.rotate(90);
    }
    painter.restore();
    //painter.setPen(QColor(Qt::white));
    int radius = 18;
    painter.drawEllipse(width()/2 - radius, mid - radius, radius * 2, radius * 2);

}

QString WidgetExchanges::unique_property()
{
    QString name = WidgetQmfObject::unique_property();
    if (data.isValid())
        if (name.isEmpty())
            name = QString("Default");
    return name;
}

void WidgetExchanges::showRelated(const qmf::Data& object, const QString &, ArrowDirection a)
{
    setArrow(a);

    // for exchanges, we should only be asked to show records related to a binding
    // so assume the object is a binding
    qpid::types::Variant value = object.getProperty("exchangeRef");
    QString name(value.asMap()["_object_name"].asString().c_str());
    QString exchange = name.section(':', -1);

    related->setRelatedData("name", exchange.toStdString());
    related->clearFilter();
    emit needData();

}
