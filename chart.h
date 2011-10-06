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

#ifndef CHART_H
#define CHART_H

#include <QWidget>
#include "object-model.h"
#include <QPainterPath>

namespace Ui {
    class chart;
}

class chart : public QWidget
{
    Q_OBJECT

public:
    explicit chart(QWidget *parent = 0);
    ~chart();

    void clear();
    void updateChart(ObjectListModel *samples, const QString& name, const QStringList& props, int duration);

protected:

    void paintEvent(QPaintEvent *event);

    //void plot_values(const ObjectListModel::Samples& samples, const std::string& prop, const QString& name, const QPointF& mm, int duration);
    QPointF xy(Sample& sample, const QString& prop, const QDateTime& tnow, int width, int height, int duration, int maxy);

    void drawXAxis(QPainter& painter, int intervals, int step, int duration);
    void drawYAxis(QPainter& painter, int intervals, int step);

private:
    Ui::chart *ui;

    QString fmt_duration(int secs);

    ObjectListModel *samplesContainer;

    // each proprty gets it's own line on the chart
    QStringList properties;
    QString oName;
    MinMax mm;
    int duration;
};

#endif // CHART_H