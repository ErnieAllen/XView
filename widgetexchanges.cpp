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
    summaryColumns.append(Column("msgRoutes", "routed", Qt::AlignRight, "B", modeMessages, true));
    summaryColumns.append(Column("msgReceives", "received", Qt::AlignRight, "B", modeMessages, true, QColor(Qt::green)));
    summaryColumns.append(Column("msgDrops", "dropped", Qt::AlignRight, "B", modeMessages, true, QColor(Qt::blue)));

    summaryColumns.append(Column("byteRoutes", "routed", Qt::AlignRight, "B", modeBytes, true));
    summaryColumns.append(Column("byteReceives", "received", Qt::AlignRight, "B", modeBytes, true, QColor(Qt::green)));
    summaryColumns.append(Column("byteDrops", "dropped", Qt::AlignRight, "B", modeBytes, true, QColor(Qt::blue)));

    summaryColumns.append(Column("msgRoutes", "routed / sec", Qt::AlignRight, "N", modeMessageRate, true));
    summaryColumns.append(Column("msgReceives", "received / sec", Qt::AlignRight, "N", modeMessageRate, true, QColor(Qt::green)));
    summaryColumns.append(Column("msgDrops", "dropped / sec", Qt::AlignRight, "N", modeMessageRate, true, QColor(Qt::blue)));

    summaryColumns.append(Column("byteRoutes", "routed / sec", Qt::AlignRight, "N", modeByteRate, true));
    summaryColumns.append(Column("byteReceives", "received / sec", Qt::AlignRight, "N", modeByteRate, true, QColor(Qt::green)));
    summaryColumns.append(Column("byteDrops", "dropped / sec", Qt::AlignRight, "N", modeByteRate, true, QColor(Qt::blue)));

    setRelatedText("Related exchanges");

}

WidgetExchanges::~WidgetExchanges()
{
}

QString WidgetExchanges::unique_property(bool translate)
{
    QString name = WidgetQmfObject::unique_property();
    if (translate)
        if (data.isValid())
            if (name.isEmpty())
                name = QString("Default");
    return name;
}

void WidgetExchanges::showRelated(const qmf::Data& object, const QString &, ArrowDirection a)
{
    if (!updateAll)
        if (this->hasData() && (arrow() != arrowNone)) {
            //qDebug("showRelated: %s needs an update", this->objectName().toStdString().c_str());
            emit needUpdate();
            return;
        }

    setArrow(a);

    // for exchanges, we should only be asked to show records related to a binding
    // so assume the object is a binding
    qpid::types::Variant value = object.getProperty("exchangeRef");
    QString name(value.asMap()["_object_name"].asString().c_str());
    QString exchange = name.section(':', -1);

    related->setRelatedData("name", exchange.toStdString());
    related->clearFilter();
    //qDebug("showRelated: %s needs new data", this->objectName().toStdString().c_str());
    emit needData();

}
