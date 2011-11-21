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

#include "widgetbindings.h"
#include <QPainter>

WidgetBindings::WidgetBindings(QWidget *parent) :
    WidgetQmfObject(parent)
{
    this->setSectionName(QString("Bindings"));
    summaryColumns.append(Column("msgMatched", "matched", Qt::AlignRight, "N", modeMessages, true));
    summaryColumns.append(Column("msgMatched", "matched / sec", Qt::AlignRight, "N", modeMessageRate, true));

    setRelatedText("Related bindings");
}

WidgetBindings::~WidgetBindings()
{
}

void WidgetBindings::paintEvent(QPaintEvent *e)
 {
    QRectF link1(0.0, 0.0, 20.0, 10.0);
    QRectF link2(18.0, 3.0, 20.0, 5.0);

    WidgetQmfObject::paintEvent(e);
return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor linkColor = getLineColor();
    QColor linkColor2 = getFillColor();
    QPen   linkPen(linkColor);
    QPen   linkPen2(linkColor2);
    linkPen.setWidth(3);
    linkPen2.setWidth(1);


    int mid = mid_paint();
    painter.translate(-6, mid);

    painter.save();

    painter.setPen(linkPen);
    painter.setBrush(Qt::NoBrush);
    painter.translate(2, 0);

    for (int x=0; x<width(); x+=(link2.width() - 4)) {
        painter.drawRoundedRect(link1, 30.0, 30.0, Qt::RelativeSize);
        painter.translate(link1.width()+link2.width()-12, 0);
    }
    painter.restore();

    painter.save();

    painter.setPen(linkPen2);
    painter.setBrush(linkColor);

    painter.translate(link1.width()/2-12, 0);
    for (int x=0; x<width(); x+=(link2.width() - 4)) {
        painter.drawRoundedRect(link2, 30.0, 30.0, Qt::RelativeSize);
        painter.translate(link1.width()+link2.width()-12, 0);
    }

    painter.restore();

 }
