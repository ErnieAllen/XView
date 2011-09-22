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
    summaryColumns.append(Column("msgDepth", "deep", Qt::AlignRight, "N"));
    setRelatedText("Related queues");
}

WidgetQueues::~WidgetQueues()
{
}

void WidgetQueues::paintEvent(QPaintEvent *e)
{

    QRectF queueRects[4] = {
        QRect(0.0, 0.0, width()-12*3, 30.0),
        QRect(width()-12*3, 0.0, 12, 30.0),
        QRect(width()-12*2, 0.0, 12, 30.0),
        QRect(width()-12,   0.0, 10, 30.0)
    };

    WidgetQmfObject::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor queueColor(backgroundColor);
    QPen   queuePen(queueColor);
    QBrush queueBrush(Qt::white);
    queuePen.setWidth(2);

    int mid = mid_paint();
    painter.translate(1, mid - 15);

    painter.setPen(queuePen);
    painter.setBrush(queueBrush);

    painter.drawRects(queueRects, 4);

}

void WidgetQueues::showRelated(const qmf::Data& object, const QString &, ArrowDirection a)
{

    setArrow(a);

    // for queues, we could be asked to show records related to a binding
    // or a subscription

    qpid::types::Variant value = object.getProperty("queueRef");
    QString name(value.asMap()["_object_name"].asString().c_str());
    QString queue = name.section(':', -1);

    related->setRelatedData("name", queue.toStdString());
    related->clearFilter();
    emit needData();

}
