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

    static const QColor colors[];

    bool current();
    void setCurrent(bool b=true);
    void setSectionName(const QString&);
    const QString &sectionName() {return sectionTitle; }
    void initRelatedButtons();
    void setAction(QAction *myAction);
    QAction * getAction() { return action; }
    void setRelatedModel(ObjectListModel *, QWidget *);
    void reset();
    void setArrow(ArrowDirection a) {_arrow = a;}
    QStringList getSampleProperties();

    // expose the pushbutton publically so signal/slots can be connected
    QPushButton* pushButton();

    // the other widgets in the ui
    typedef QList<WidgetQmfObject *> qmfWidgetList;
    qmfWidgetList peers;
    WidgetQmfObject *leftBuddy;
    WidgetQmfObject *rightBuddy;

    const qmf::DataAddr& getDataAddr();
    bool hasData();

public slots:
    void setCurrentObject(const qmf::Data& object);
    void setCurrentMode(StatMode);
    void objectRefreshed();
    void setEnabled(bool enabled);
    void initRelated();
    void showChart(bool b);
    void setDrawAsRect(bool b);
    void showRelatedButtons(bool);
    void setUpdateStrategy(bool);
    void setChartType(bool);

protected:
    QString sectionTitle;
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void focusInEvent ( QFocusEvent * event );
    void keyPressEvent ( QKeyEvent * event );
    virtual void showRelated(const qmf::Data& object, const QString& widget_name, ArrowDirection a);
    void showChart(const qmf::Data& object, ObjectListModel *model);


    int reservedY(); // amount reserved at top of widget for button and label
    int mid_paint(); // use this as a common mid point for painting
    virtual QString unique_property(bool useKey=false); // allow derived classes to override object name
    void setRelatedText(const std::string&);
    ArrowDirection arrow() { return _arrow; }
    QColor getLineColor(); // the color to draw the section's background icon (based on focus)
    QColor getFillColor(); // the color to draw the section's background icon (based on focus)

    // the columns that are to be displayed in the summary box
    struct Column {
        std::string     name;
        QString         header;
        Qt::Alignment   alignment;
        std::string     format;
        StatMode        mode;
        bool            chart;
        QColor          color;

        Column(const std::string& _n, const char* _h, Qt::Alignment _a, const std::string& _f, StatMode _m, bool _c=false, QColor _co=QColor(Qt::red)):
            name(_n), header(_h), alignment(_a), format(_f), mode(_m), chart(_c), color(_co) {}
    };
    typedef QList<Column> ObjectColumnList;
    ObjectColumnList summaryColumns;

    RelatedFilterProxyModel *related;

    QColor backgroundColor;
    StatMode currentMode;
    QAction *action;
    bool updateAll;  // current update strategy (all or just the current object)
    bool chartType;  // true->area chart

    // the currently displayed object
    qmf::Data data;

signals:
    void needData();    // send query that gets all objects of this type
    void needUpdate();  // send query that updated only the current object
    void pivotTo(const QModelIndex&);  // select this row in the associated dialog box

private slots:
    void relatedIndexChanged(int index);
    void pivot(); // make the current related object the base/root object


private:
    void setLabelName();
    void fillTableWidget();
    void resetOthers();
    void updateComboboxIndex(int i, bool all);
    bool reallyHasFocus();
    void showData(const qmf::Data& object);
    QString fmtBytes(const qpid::types::Variant& v) const;

    QString value(const qpid::types::Variant::Map::const_iterator& iter, const QString& uname, const std::string &);

    bool _current; // this section is the primary one, others show related objects to this one
    bool chart;     // should we show the chart
    ArrowDirection _arrow;   // direction to draw a background arrow 0->none 1->left 2->right
    Ui::WidgetQmfObject *ui;
    int duration;

    QIcon redIcon;
    QIcon greenIcon;
    QIcon blueIcon;
    bool drawAsRect; // add a white background and border when the layout is cascading
};

#endif // WIDGETQMFOBJECT_H
