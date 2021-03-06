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

#include "widgetsessions.h"
#include <QPainter>

WidgetSessions::WidgetSessions(QWidget *parent) :
    WidgetQmfObject(parent)
{
    this->setSectionName(QString("Sessions"));
    summaryColumns.append(Column("unackedMessages", "unAcked", Qt::AlignRight, "N", modeMessages, true));
    summaryColumns.append(Column("TxnCount", "transactions", Qt::AlignRight, "N", modeMessages, true, QColor(Qt::green)));
    summaryColumns.append(Column("TxnStarts", "starts", Qt::AlignRight, "N", modeMessages));
    summaryColumns.append(Column("TxnCommits", "commits", Qt::AlignRight, "N", modeMessages));
    summaryColumns.append(Column("TxnRejects", "rejects", Qt::AlignRight, "N", modeMessages));

    summaryColumns.append(Column("unackedMessages", "unAcked", Qt::AlignRight, "N", modeMessageRate, true));
    summaryColumns.append(Column("TxnCount", "transactions", Qt::AlignRight, "N", modeMessageRate, true, QColor(Qt::green)));
    summaryColumns.append(Column("TxnStarts", "starts", Qt::AlignRight, "N", modeMessageRate));
    summaryColumns.append(Column("TxnCommits", "commits", Qt::AlignRight, "N", modeMessageRate));
    summaryColumns.append(Column("TxnRejects", "rejects", Qt::AlignRight, "N", modeMessageRate));

    setRelatedText("Related sessions");
}

WidgetSessions::~WidgetSessions()
{
}

void WidgetSessions::showRelated(const qmf::Data& object, const QString &widget_type, ArrowDirection a)
{
    if (!updateAll)
        if (this->hasData() && (arrow() != arrowNone)) {
            //qDebug("showRelated: %s needs an update", this->objectName().toStdString().c_str());
            emit needUpdate();
            return;
        }

    setArrow(a);

    if (widget_type == "widgetConnections") {
        qpid::types::Variant value = object.getProperty("address");
        std::string name = value.asString();
        related->setRelatedData("connectionRef", name);
        related->clearFilter();
        emit needData();
        return;
    }
    // the object is a subscription
    qpid::types::Variant value = object.getProperty("sessionRef");
    QString name(value.asMap()["_object_name"].asString().c_str());
    QString session = name.section(':', -1);

    related->setRelatedData("name", session.toStdString());
    related->clearFilter();
    //qDebug("showRelated: %s needs new data", this->objectName().toStdString().c_str());
    emit needData();

}
