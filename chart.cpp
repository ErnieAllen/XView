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

#include "chart.h"
#include "ui_chart.h"
#include <QPainter>
#include <math.h>

chart::chart(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::chart),
    properties(),
    oName()
{
    ui->setupUi(this);
    rate = false;
    samplesContainer = NULL;

    ui->graph->addAction(ui->actionShow_chart);
    ui->graph->addAction(ui->actionHide_chart);
    ui->actionSep->setSeparator(true);
    ui->graph->addAction(ui->actionSep);
    ui->graph->addAction(ui->actionContigure_chart);
}

chart::~chart()
{
    delete ui;
}

void chart::clear()
{
    properties.clear();
}

void chart::updateChart(bool isRate, ObjectListModel* samples, const QString& name, const QHash<QString, QColor>& props, int dur)
{
    // number of seconds on the x-axis
    duration = dur;

    // pointer to object that contains the samples
    samplesContainer = samples;

    // hash of sample's property names and line colors
    properties = props;

    // the object name needed by the samples container, ie. the queue name, or the binding key
    oName = name;

    // is this a rate chart
    rate = isRate;

    // force a paint
    update();
}

void chart::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    if (properties.isEmpty())
        return;

    // get the current min and max Y vales so we can draw the y-axis
    MinMax mm = samplesContainer->minMax(oName, properties.keys(), rate);
    mm.max = (qint32)(mm.max * 1.1 + 1.0);

    if (mm.min < 0)
        mm.min = (qint32)(mm.min * 1.1 - 1.0);

    int yIntervals = 6;
    int yStep = 2;

    if ((mm.max - mm.min) < 3)
        yStep = 3;

    QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing);

    drawXAxis(painter, 10, 2, duration);
    drawYAxis(painter, yIntervals, yStep, mm);

    QPen pen = QPen(Qt::SolidLine);
    pen.setColor(QColor(226, 226, 226));
    pen.setWidth(3);
    painter.setPen(pen);
    painter.setOpacity(0.5);

    if (rate)
        paintRate(painter, mm);
    else
        paintValue(painter, mm);
}

void chart::paintValue(QPainter &painter, MinMax &mm)
{
    int w = ui->graph->width();
    int h = ui->graph->height();

    QDateTime tnow(QDateTime::currentDateTime());
    const ObjectListModel::Samples& samples(samplesContainer->samples());

    ObjectListModel::const_iterSamples iterHash = samples.constFind(oName);
    // shallow reference to the list
    ObjectListModel::SampleList sampleList = iterHash.value();
    ObjectListModel::const_iterSampleList iterSamples;

    QPointF p1, p2;
    bool tick;
    QPen pen(painter.pen());
    Sample prevSample;

    // for each property line
    QHash<QString, QColor>::const_iterator iter = properties.constBegin();
    while (iter != properties.constEnd()) {

        QColor lineColor = QColor(iter.value());
        lineColor.setAlpha(127);
        pen.setColor(lineColor);
        painter.setPen(pen);
        painter.setBrush(QBrush(lineColor));
        QString prop = iter.key();

        tick = true;
        iterSamples = sampleList.constEnd();
        if (iterSamples == sampleList.constBegin())
            break;

        // backup past the place holder since we are going backwards
        --iterSamples;

        // get the 1st point for a line segment
        Sample sample = *iterSamples;
        p1 = xy(sample, prop, tnow, w, h, duration, mm);

        // loop backwards through the samples
        while (iterSamples != sampleList.constBegin()) {
            --iterSamples;

            prevSample = sample;
            sample = *iterSamples;

            int diff = sample.dateTime().secsTo(prevSample.dateTime());
            if (diff > 0) {
                if (tick) {
                    p2 = xy(sample, prop, tnow, w, h, duration, mm);
                } else {
                    p1 = xy(sample, prop, tnow, w, h, duration, mm);
                }
                painter.drawLine(p1, p2);
                tick = !tick;
            }
        }
        // next property line
        ++iter;
    }
}

void chart::paintRate(QPainter &painter, MinMax &mm)
{

    int w = ui->graph->width();
    int h = ui->graph->height();

    QDateTime tnow(QDateTime::currentDateTime());
    const ObjectListModel::Samples& samples(samplesContainer->samples());

    ObjectListModel::const_iterSamples iterHash = samples.constFind(oName);
    // shallow reference to the list
    ObjectListModel::SampleList sampleList = iterHash.value();
    ObjectListModel::const_iterSampleList iterSamples;

    QPointF p1, p2;
    bool tick;
    QPen pen(painter.pen());
    bool rateSkippedFirst = false;
    Sample prevSample;

    // for each property line
    QHash<QString, QColor>::const_iterator iter = properties.constBegin();
    while (iter != properties.constEnd()) {

        rateSkippedFirst = false;

        QColor lineColor = QColor(iter.value());
        lineColor.setAlpha(127);
        pen.setColor(lineColor);
        painter.setPen(pen);
        painter.setBrush(QBrush(lineColor));
        QString prop = iter.key();

        tick = true;
        iterSamples = sampleList.constEnd();
        if (iterSamples == sampleList.constBegin())
            break;

        // backup past the place holder since we are going backwards
        --iterSamples;

        // get the 1st point for a line segment
        Sample sample = *iterSamples;
        p1 = xy(sample, prop, tnow, w, h, duration, mm);

        // loop backwards through the samples
        while (iterSamples != sampleList.constBegin()) {
            --iterSamples;

            prevSample = sample;
            sample = *iterSamples;

            int diff = sample.dateTime().secsTo(prevSample.dateTime());
            if (diff > 0) {
                if (tick) {
                    p2 = xyRate(prevSample, sample, prop, tnow, w, h, duration, mm);
                } else {
                    p1 = xyRate(prevSample, sample, prop, tnow, w, h, duration, mm);
                }
                if (!rateSkippedFirst)
                    rateSkippedFirst = true;
                else {
                    painter.drawLine(p1, p2);
                }
                tick = !tick;

            }
        }
        // next property line
        ++iter;
    }
}

QPointF chart::xy(Sample& sample, const QString& prop, const QDateTime& tnow, int width, int height, int duration, const MinMax& mm)
{
    float x, y;
    int secs;
    secs = sample.dateTime().secsTo(tnow);
    qint64 value = sample.data(prop);
    x = width - ((float)secs / (float)duration) * width;
    y = (float)height * (float)((mm.max - value) / (mm.max - mm.min)) + ui->topmargin->height();
    return QPointF(x, y);
}

QPointF chart::xyRate(Sample& prevSample, Sample& sample, const QString& prop, const QDateTime& tnow, int width, int height, int duration, const MinMax& mm)
{
    float x = 0.0, y = 0.0;
    float elapsed = sample.dateTime().secsTo(prevSample.dateTime());
    int secs = sample.dateTime().secsTo(tnow);

    qint64 value1 = sample.data(prop);
    qint64 value2 = prevSample.data(prop);
    qint64 diff = value2 - value1;

    if (elapsed) {
        x = width - ((float)secs / (float)duration) * width;
        float _rate = diff / elapsed;
        float _range = mm.max - mm.min;
        float prange = _rate / _range;
        if (prange < 0)
            prange = - prange;
        y = height - prange * height + ui->topmargin->height();
    }
    return QPointF(x, y);
}

// Draws x-axis text and vertical lines that separate the x axis
void chart::drawXAxis(QPainter& painter, int intervals, int step, int duration)
{
    int i;
    int x;

    int gWidth = ui->graph->width();
    int gHeight = ui->graph->height();

    int interval = (float)gWidth / (float)intervals;
    int gap = duration / intervals;
    QPen gridPen = QPen(QColor(220, 220, 220, 200));
    QPen gridPen1 = QPen(QColor(200, 200, 200, 200));
    QPen textPen = QPen(QColor(88, 88, 88));
    QFont textFont = QFont(painter.font());
    textFont.setPixelSize(11);
    painter.setFont(textFont);

    for (i=0; i<intervals + 1; ++i) {
        x = gWidth - (i * interval);
        if (i % step == 0) {
            painter.setPen(gridPen1);
            painter.drawLine(x, 4, x, gHeight+6);

            painter.setPen(textPen);
            painter.drawText(x, height() - 4, fmt_duration(i * gap));
        } else {
            painter.setPen(gridPen);
            painter.drawLine(x, 4, x, gHeight);
        }

        if (i % step == 0) {
        }
    }
}

// draw y-axis text and horizontal grid lines
void chart::drawYAxis(QPainter& painter, int intervals, int step, const MinMax& mm)
{
    int i;
    int y;

    int gHeight = ui->graph->height();
    int gWidth = ui->graph->width();
    int margin = ui->topmargin->height();

    float gap = (mm.max - mm.min) / (float)intervals;
    float value;
    QString sValue = QString();

    QPen gridPen = QPen(QColor(220, 220, 220, 200));
    QPen gridPen1 = QPen(QColor(200, 200, 200, 200));
    QPen textPen = QPen(QColor(88, 88, 88));
    QFont textFont = QFont(painter.font());
    textFont.setPixelSize(11);
    painter.setFont(textFont);

    for (i=0; i<intervals + 1; ++i) {
        value = mm.min + (i * gap);
        y = (float)gHeight * (float)((mm.max - value) / (mm.max - mm.min)) + margin;
        if (i % step == 0) {
            painter.setPen(gridPen1);
            painter.drawLine(6, y, gWidth + 3, y);

            painter.setPen(textPen);
            if (value >= 1000000)
                sValue.sprintf("%.1f1M", (value / 1000000.0));
            else if (value >= 1000) {
                sValue.sprintf("%.0fK", (value / 1000.0));
            } else if (gap < 1) {
                sValue.sprintf("%.1f", value);
            } else {
                sValue.sprintf("%.0f", value);

            }
            painter.drawText(gWidth + 6, y + 6, sValue);
        } else {
            painter.setPen(gridPen);
            painter.drawLine(6, y, gWidth, y);
        }
    }
    // draw a darker line at y=0
    if (mm.min <= 0 && mm.max >=0) {
        QPen zeroPen = QPen(QColor(Qt::darkGray));
        zeroPen.setStyle(Qt::DashLine);
        painter.setPen(zeroPen);
        y = (float)gHeight * (float)((mm.max - 0.0) / (mm.max - mm.min)) + margin;
        painter.drawLine(6, y, gWidth, y);
    }
}

// convert a number of seconds to a string containing the nearest d, h, m, or s
QString chart::fmt_duration(int secs)
{
    QString elems = QString();
    static const int periods[] = {86400, 3600, 60, 1};
    static const char units[] = {'d', 'h', 'm', 's'};
    uint i;
    int appended = 0;

    uint size = sizeof units / sizeof (units[0]);
    for (i=0; i < size; ++i) {
        if (secs >= periods[i]) {
            elems.append(QString::number(secs / periods[i]));
            elems.append(units[i]);
            if (++appended >= 2)
                break;
        }
        secs %= periods[i];
    }
    return elems;
}
