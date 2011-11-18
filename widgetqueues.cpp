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

#include "widgetqueues.h"
#include <QPainter>

WidgetQueues::WidgetQueues(QWidget *parent) :
    WidgetQmfObject(parent)
{
    this->setSectionName(QString("Queues"));
    summaryColumns.append(Column("msgDepth", "deep", Qt::AlignRight, "N", modeMessages, true));
    summaryColumns.append(Column("msgPersistDequeues", "persist dequeues", Qt::AlignRight, "N", modeMessages));
    summaryColumns.append(Column("msgPersistEnqueues", "persist enqueues", Qt::AlignRight, "N", modeMessages));
    summaryColumns.append(Column("msgTotalDequeues", "total dequeues", Qt::AlignRight, "N", modeMessages, true, QColor(Qt::green)));
    summaryColumns.append(Column("msgTotalEnqueues", "total enqueues", Qt::AlignRight, "N", modeMessages, true, QColor(Qt::blue)));
    summaryColumns.append(Column("msgTxnDequeues", "transaction dequeues", Qt::AlignRight, "N", modeMessages));
    summaryColumns.append(Column("msgTxnEnqueues", "transaction enqueues", Qt::AlignRight, "N", modeMessages));

    summaryColumns.append(Column("byteDepth", "deep", Qt::AlignRight, "N", modeBytes, true));
    summaryColumns.append(Column("bytePersistDequeues", "persist dequeues", Qt::AlignRight, "N", modeBytes));
    summaryColumns.append(Column("bytePersistEnqueues", "persist enqueues", Qt::AlignRight, "N", modeBytes));
    summaryColumns.append(Column("byteTotalDequeues", "total dequeues", Qt::AlignRight, "B", modeBytes, true, QColor(Qt::green)));
    summaryColumns.append(Column("byteTotalEnqueues", "total enqueues", Qt::AlignRight, "B", modeBytes, true, QColor(Qt::blue)));
    summaryColumns.append(Column("byteTxnDequeues", "transaction dequeues", Qt::AlignRight, "N", modeBytes));
    summaryColumns.append(Column("byteTxnEnqueues", "transaction enqueues", Qt::AlignRight, "N", modeBytes));

    summaryColumns.append(Column("msgPersistDequeues", "persist dequeues / sec", Qt::AlignRight, "N", modeMessageRate));
    summaryColumns.append(Column("msgPersistEnqueues", "persist enqueues / sec", Qt::AlignRight, "N", modeMessageRate));
    summaryColumns.append(Column("msgTotalDequeues", "total dequeues / sec", Qt::AlignRight, "N", modeMessageRate, true, QColor(Qt::green)));
    summaryColumns.append(Column("msgTotalEnqueues", "total enqueues / sec", Qt::AlignRight, "N", modeMessageRate, true, QColor(Qt::blue)));
    summaryColumns.append(Column("msgTxnDequeues", "transaction dequeues / sec", Qt::AlignRight, "N", modeMessageRate));
    summaryColumns.append(Column("msgTxnEnqueues", "transaction enqueues / sec", Qt::AlignRight, "N", modeMessageRate));

    summaryColumns.append(Column("bytePersistDequeues", "persist dequeues / sec", Qt::AlignRight, "N", modeByteRate));
    summaryColumns.append(Column("bytePersistEnqueues", "persist enqueues / sec", Qt::AlignRight, "N", modeByteRate));
    summaryColumns.append(Column("byteTotalDequeues", "total dequeues / sec", Qt::AlignRight, "N", modeByteRate, true, QColor(Qt::green)));
    summaryColumns.append(Column("byteTotalEnqueues", "total enqueues / sec", Qt::AlignRight, "N", modeByteRate, true, QColor(Qt::blue)));
    summaryColumns.append(Column("byteTxnDequeues", "transaction dequeues / sec", Qt::AlignRight, "N", modeByteRate));
    summaryColumns.append(Column("byteTxnEnqueues", "transaction enqueues / sec", Qt::AlignRight, "N", modeByteRate));

    setRelatedText("Related queues");
}

WidgetQueues::~WidgetQueues()
{
}

void WidgetQueues::paintEvent(QPaintEvent *e)
{
    QRectF queueRects[] = {
        QRect(0.0, 0.0, width()-12*5, 30.0),
        QRect(width()-12*5, 0.0, 12, 30.0),
        QRect(width()-12*4, 0.0, 12, 30.0),
        QRect(width()-12*3, 0.0, 12, 30.0),
        QRect(width()-12*2, 0.0, 12, 30.0),
        QRect(width()-12,   0.0, 10, 30.0)
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
    painter.translate(1, mid - 4);

    painter.scale(1.0, 0.6);
    painter.drawRects(queueRects, 6);

}

void WidgetQueues::showRelated(const qmf::Data& object, const QString &, ArrowDirection a)
{
    if (!updateAll)
        if (this->hasData() && (arrow() != arrowNone)) {
            //qDebug("showRelated: %s needs an update", this->objectName().toStdString().c_str());
            emit needUpdate();
            return;
        }

    setArrow(a);
    // for queues, we could be asked to show records related to a binding
    // or a subscription

    qpid::types::Variant value = object.getProperty("queueRef");
    QString name(value.asMap()["_object_name"].asString().c_str());
    QString queue = name.section(':', -1);

    related->setRelatedData("name", queue.toStdString());
    related->clearFilter();
    //qDebug("showRelated: %s needs new data", this->objectName().toStdString().c_str());
    emit needData();

}
