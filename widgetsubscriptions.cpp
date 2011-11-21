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

#include "widgetsubscriptions.h"
#include <QPainter>

WidgetSubscriptions::WidgetSubscriptions(QWidget *parent) :
    WidgetQmfObject(parent)
{
    this->setSectionName(QString("Subscriptions"));
    summaryColumns.append(Column("delivered", "delivered", Qt::AlignRight, "N", modeMessages, true));
    summaryColumns.append(Column("delivered", "delivered / sec", Qt::AlignRight, "N", modeMessageRate, true));
    setRelatedText("Related subscriptions");
}

WidgetSubscriptions::~WidgetSubscriptions()
{
}

void WidgetSubscriptions::paintEvent(QPaintEvent *e)
{
    QPointF points[6] = {
        QPointF(0.0, 0.0),
        QPointF(10.0, -10.0),
        QPointF(width()-10.0, -10.0),
        QPointF(width()-2, 0.0),
        QPointF(width()-10.0, 10.0),
        QPointF(10.0, 10.0)
    };

    WidgetQmfObject::paintEvent(e);
    return;

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
    painter.translate(1, mid + 6);

    painter.drawPolygon(points, 6);
}
