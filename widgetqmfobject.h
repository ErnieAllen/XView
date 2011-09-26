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

#ifndef WIDGETQMFOBJECT_H
#define WIDGETQMFOBJECT_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QVariant>
#include "qpid/types/Variant.h"
#include <qmf/Data.h>
#include "related-model.h"
#include "object-model.h"

namespace Ui {
    class WidgetQmfObject;
}

class WidgetQmfObject : public QWidget
{
    Q_OBJECT

public:
    enum ArrowDirection {
        arrowNone,
        arrowLeft,
        arrowRight
    };
    enum StatMode {
        modeMessages,
        modeBytes,
        modeMessageRate,
        modeByteRate
    };

    explicit WidgetQmfObject(QWidget *parent = 0);
    ~WidgetQmfObject();

    bool current();
    void setCurrent(bool b=true);
    void setSectionName(const QString&);
    void setRelatedModel(ObjectListModel *, QWidget *);
    void reset();
    void setArrow(ArrowDirection a) {_arrow = a;}

    // expose the pushbutton publically so signal/slots can be connected
    QPushButton* pushButton();

    // the other widgets in the ui
    typedef QList<WidgetQmfObject *> qmfWidgetList;
    qmfWidgetList peers;
    WidgetQmfObject *leftBuddy;
    WidgetQmfObject *rightBuddy;

public slots:
    void setCurrentObject(const qmf::Data& object);
    void setCurrentMode(StatMode);
    void showData(const qmf::Data& object);
    void setEnabled(bool enabled);
    void initRelated();


protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void focusInEvent ( QFocusEvent * event );
    void keyPressEvent ( QKeyEvent * event );
    virtual void showRelated(const qmf::Data& object, const QString& widget_name, ArrowDirection a);

    int reservedY(); // amount reserved at top of widget for button and label
    int mid_paint(); // use this as a common mid point for painting
    virtual QString unique_property(); // allow derived classes to override object name
    void setRelatedText(const std::string&);

    // the columns that are to be displayed in the summary box
    struct Column {
        std::string     name;
        QString         header;
        Qt::Alignment   alignment;
        std::string     format;
        StatMode        mode;

        Column(const std::string& _n, const char* _h, Qt::Alignment _a, const std::string& _f, StatMode _m) :
            name(_n), header(_h), alignment(_a), format(_f), mode(_m) {}
    };
    typedef QList<Column> ObjectColumnList;
    ObjectColumnList summaryColumns;
    std::string unique; // object property used for the bold section name

    qmf::Data data;
    RelatedFilterProxyModel *related;

    QColor backgroundColor;
    StatMode currentMode;

signals:
    void needData();

private slots:
    void relatedIndexChanged(int index);

private:
    void setLabelName();
    void fillTableWidget(const qmf::Data& object);
    void resetOthers();

    bool _current; // draw with highlighted background
    ArrowDirection _arrow;   // direction to draw a background arrow 0->none 1->left 2->right
    Ui::WidgetQmfObject *ui;

};

#endif // WIDGETQMFOBJECT_H
