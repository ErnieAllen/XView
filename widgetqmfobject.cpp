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

#include "widgetqmfobject.h"
#include "ui_widgetqmfobject.h"
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>

WidgetQmfObject::WidgetQmfObject(QWidget *parent) :
    QWidget(parent),
    backgroundColor(200, 200, 200),
    ui(new Ui::WidgetQmfObject)
{
    ui->setupUi(this);

    _current = false;
    currentMode = modeMessages;
    ui->tableWidget->setColumnCount(2);
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(4);
    shadow->setColor(QColor(200, 200, 200, 180));
    shadow->setOffset(4);
    ui->tableWidget->setGraphicsEffect(shadow);

    // default to using the name property of the object
    // to display as the section name
    unique = "name";

    setFocusPolicy(Qt::StrongFocus);
    ui->comboBox->hide();
    ui->labelRelated->hide();
    ui->labelIndex->hide();
    _arrow = arrowNone;

}

WidgetQmfObject::~WidgetQmfObject()
{
    delete ui;
    if (related)
        delete related;
}

void WidgetQmfObject::setRelatedText(const std::string& name)
{
    ui->labelRelated->setText(QString(name.c_str()));
}


void WidgetQmfObject::setRelatedModel(ObjectListModel *model, QWidget *parent)
{
    related = new RelatedFilterProxyModel(parent);
    related->setDynamicSortFilter(true);
    related->setSourceModel(model);
    ui->comboBox->setModel(related);
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(relatedIndexChanged(int)));
}

void WidgetQmfObject::setEnabled(bool enabled)
{
    QWidget::setEnabled(enabled);
    ui->pushButton->setEnabled(enabled);

    if (!enabled)
        reset();
}

// This is exposed so the main window can setup a connect between the
// pushbutton and the dialog box
QPushButton* WidgetQmfObject::pushButton()
{
    return ui->pushButton;
}

int WidgetQmfObject::reservedY()
{
    return  ui->labelName->height() + ui->pushButton->height();
}

int WidgetQmfObject::mid_paint()
{
    return (reservedY() + height()) / 2;
}

void WidgetQmfObject::resizeEvent(QResizeEvent *)
{
    ui->pushButton->move(width() /2 - ui->pushButton->width() / 2, ui->pushButton->y());

    if (_current) {
        int y = ui->pushButton->height();
        ui->labelName->move(0, y);
        ui->tableWidget->move(width() / 2 - ui->tableWidget->width() / 2, reservedY() + 20);
    } else {
        ui->comboBox->move(0, reservedY());
        ui->tableWidget->move(width() / 2 - ui->tableWidget->width() / 2, ui->comboBox->y() + ui->comboBox->height() + 46);
    }
    ui->labelName->resize(width(), ui->labelName->height());
    ui->comboBox->resize(width(), ui->comboBox->height());

    // changing the size might change the elided text
    // so, reconstruct the text
    setLabelName();

    // move the Related label
    int x = qMax(0, width() / 2 - ui->labelRelated->width() / 2);
    ui->labelIndex->move(x, reservedY() + 40);

    // move the chart
    ui->widgetChart->resize(width() - 20, ui->widgetChart->height());
    ui->widgetChart->move(width() / 2 - ui->widgetChart->width() / 2, height() - ui->widgetChart->height() - 12);

}

void WidgetQmfObject::focusInEvent ( QFocusEvent * )
{
    //setCurrent(true);
    //emit madeCurrent(objectName());
    update();
}

void WidgetQmfObject::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    static const QPoint arrowPoints[] = {
        QPoint(0,  0),
        QPoint(10, -5),
        QPoint(100, -5),
        QPoint(100,  5),
        QPoint(10,   5)
    };

    // if this section has the focus,
    // draw with a light background
    if (this->hasFocus()) {
        painter.save();
        QColor currentColor(248, 248, 248, 248);

        QPen currentPen(Qt::white);
        currentPen.setWidth(2);

        QBrush currentBrush(currentColor);

        painter.setBrush(currentBrush);
        painter.setPen(currentPen);
        painter.drawRect(1, reservedY() + 1, width()-2, height()-2 - reservedY());
        painter.restore();
    } else {
        painter.save();
        QPen currentPen(Qt::white);
        QColor currentColor(Qt::white);
        currentPen.setWidth(2);

        QBrush currentBrush(currentColor);

        painter.setBrush(currentBrush);
        painter.setPen(currentPen);
        painter.drawRect(1, reservedY() + 1, width()-2, height()-2 - reservedY());
        painter.restore();
    }


    // if this section has a current object
    // draw background with a diagonal lines
    // and a heavy border
    if (this->_current) {
        painter.save();

        QColor backColor(200, 200, 255);
        int border_width = 10;
        QPen currentPen(backColor);
        currentPen.setWidth(border_width);

        painter.setPen(currentPen);
        painter.drawRect(border_width / 2, border_width / 2 + reservedY(),
                         width() - border_width,
                         height() - border_width - reservedY());

        QBrush currentBrush(backColor, Qt::FDiagPattern);
        painter.fillRect(border_width / 2, border_width / 2 + reservedY(),
                         width() - border_width,
                         height() - border_width - reservedY(),
                         currentBrush);
        painter.restore();
    }

    // if this section is showing related objects
    // draw with the appropriate arrow
    if ((this->_arrow == arrowLeft) || (this->_arrow == arrowRight)) {
        painter.save();

        QColor arrowColor(200, 200, 200, 248);
        QBrush arrowBrush(arrowColor, Qt::SolidPattern);
        QPen currentPen(arrowColor);
        currentPen.setWidth(0);
        painter.setPen(currentPen);
        painter.setBrush(arrowBrush);

        // move the points down a bit
        painter.translate(0, reservedY() + ui->comboBox->height() + 20);

        // the points are defined as a right arrow
        // so if we want to show them as a left arrow,
        // move them over and rotate them
        if (this->_arrow == arrowLeft) {
            painter.translate(width(), 0);
            painter.rotate(180.0);
        }

        // the arrow points are defined to be 100 wide.
        // scale the viewport to show them as the width of the widget
        painter.scale(width() / 100.0, width() / 100.0);

        painter.drawPolygon(arrowPoints,  sizeof( arrowPoints ) / sizeof( arrowPoints[0] ));

        painter.restore();

    }
}

void WidgetQmfObject::keyPressEvent ( QKeyEvent * event )
{
    int row;
    int rows;
    switch (event->key()) {
    case Qt::Key_Left:
        if (leftBuddy)
            leftBuddy->setFocus();
        break;
    case Qt::Key_Right:
        if (rightBuddy)
            rightBuddy->setFocus();
        break;
    case Qt::Key_Up:
        row = ui->comboBox->currentIndex();
        if ((ui->comboBox->isVisible()) && (row > 0)) {
            ui->comboBox->setCurrentIndex(row - 1);
        }
        break;
    case Qt::Key_Down:
        row = ui->comboBox->currentIndex();
        rows = ui->comboBox->model()->rowCount();
        if ((ui->comboBox->isVisible()) && (row >= 0) && (row < rows - 1)) {
            ui->comboBox->setCurrentIndex(row + 1);
        }
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        ui->pushButton->animateClick();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}


void WidgetQmfObject::setCurrent(bool _c)
{
    _current = _c;
    update();
}

bool WidgetQmfObject::current()
{
    return _current;
}

QString WidgetQmfObject::unique_property()
{
    QString prop;
    if (data.isValid()) {
        const qpid::types::Variant::Map& props(data.getProperties());
        qpid::types::Variant::Map::const_iterator iter;

        iter = props.find(unique);
        if (iter != props.end()) {
            prop = QString(iter->second.asString().c_str());
        }
    }
    return prop;
}

void WidgetQmfObject::setLabelName()
{
    QString full_text = unique_property();
    QFontMetrics label_fm(ui->labelName->font());
    QString elided_text = label_fm.elidedText(full_text, Qt::ElideRight, ui->labelName->width());
    ui->labelName->setText(elided_text);
    ui->labelName->setToolTip(full_text);
}

void WidgetQmfObject::reset()
{
    this->setCurrent(false);
    ui->tableWidget->hide();
    ui->labelName->hide();
    ui->labelRelated->hide();
    ui->labelIndex->hide();

    related->setRelatedData("", "");
    related->clearFilter();
    ui->comboBox->hide();
    //ui->comboBox->clear();
    _arrow = arrowNone;
    ui->widgetChart->clear();
}

void WidgetQmfObject::setCurrentMode(StatMode mode)
{
    currentMode = mode;
    fillTableWidget(data);
}

void WidgetQmfObject::setCurrentObject(const qmf::Data& object)
{
    if (!object.isValid())
        return;

    reset();
    resetOthers();
    setFocus();
    this->setCurrent(true);
    showData(object);

}

// SLOT triggered when an automatic background update
// has completed.
// Also called after an ohject is made current.
// If there is a current object, update it's data a
// refresh the related widgets
void WidgetQmfObject::showData(const qmf::Data& object)
{
    if (!object.isValid())
        return;

    if (!_current)
        return;

    ui->labelName->show();
    ui->tableWidget->show();
    ui->tableWidget->setRowCount(summaryColumns.size());

    fillTableWidget(object);

    if (leftBuddy) {
        leftBuddy->showRelated(object, objectName(), arrowLeft);
    }
    if (rightBuddy) {
        rightBuddy->showRelated(object, objectName(), arrowRight);
    }
    ObjectListModel *model = (ObjectListModel *)related->sourceModel();
    showChart(object, model);

}


void WidgetQmfObject::resetOthers()
{
    if (leftBuddy) {
        leftBuddy->reset();
    }
    if (rightBuddy) {
        rightBuddy->reset();
    }
    QList<WidgetQmfObject *>::const_iterator peers_iter = peers.constBegin();
    while (peers_iter != peers.constEnd()) {
        (*peers_iter)->reset();
        ++peers_iter;
    }
}

void WidgetQmfObject::fillTableWidget(const qmf::Data& object)
{
    if (!object.isValid())
        return;

    data = object;
    setLabelName();

    QTableWidgetItem *newItem;
    int row = 0;
    int maxValWidth = 0;
    int maxNameWidth = 0;
    int maxModeWidth = 0;
    QFontMetrics fm(ui->tableWidget->font());

    static const QColor colors[] = {
        QColor(255, 255, 220), // yellow  (messages)
        QColor(220, 255, 220), // green   (bytes)
        QColor(255, 220, 255), // magenta (message rate)
        QColor(220, 255, 255)};// cyan    (byte rate)

    const qpid::types::Variant::Map& props(object.getProperties());
    qpid::types::Variant::Map::const_iterator iter;

    ui->tableWidget->clear();
    QList<Column>::const_iterator column_iter = summaryColumns.constBegin();

    // loop through all the columns we might want to display
    while (column_iter != summaryColumns.constEnd()) {

        // find the column in the current data
        iter = props.find((*column_iter).name);
        bool show = (*column_iter).mode == currentMode;
        if ((iter != props.end()) && show) {
            newItem = new QTableWidgetItem(value(iter, props.find(unique)));
            newItem->setBackgroundColor(colors[currentMode]);
            newItem->setTextAlignment((*column_iter).alignment);
            maxValWidth = qMax(maxValWidth, fm.width(newItem->text()));
            ui->tableWidget->setItem(row, 0, newItem);

            newItem = new QTableWidgetItem((*column_iter).header);
            newItem->setBackgroundColor(colors[currentMode]);;
            maxNameWidth = qMax(maxNameWidth, fm.width(newItem->text()));
            ui->tableWidget->setItem(row, 1, newItem);

            ++row;
        }
        ++column_iter;
    }
    ui->tableWidget->resize(maxModeWidth + maxValWidth + maxNameWidth + 24, fm.height() * row - row/3);
    ui->tableWidget->setColumnWidth(0, maxValWidth + 8);
    ui->tableWidget->setColumnWidth(1, maxNameWidth + 8);

    // force a resize event so the table is drawn in the correct place
    QResizeEvent event(size(), size());
    QApplication::sendEvent(this, &event);

}

// Generate the value to display in the tableWidget
QString WidgetQmfObject::value(const qpid::types::Variant::Map::const_iterator& iter, const qpid::types::Variant::Map::const_iterator& uname)
{
    // if we aren't showing a rate, return the value directly
    if ((currentMode == this->modeMessages) || (currentMode == this->modeBytes))
        return QString(iter->second.asString().c_str());

    // we are showing a rate. get the two most recent values
    ObjectListModel *pModel = (ObjectListModel *)related->sourceModel();
    ObjectListModel::Samples samples = pModel->samples();

    // get the sample's hash entry for this object
    QString name(uname->second.asString().c_str());
    ObjectListModel::const_iterSamples iterSamples = samples.constFind(name);
    if (iterSamples != samples.constEnd()) {
        // get the list of samples
        ObjectListModel::SampleList sampleList = iterSamples.value();
        // get the last (most recent) sample from the list
        ObjectListModel::const_iterSampleList iList = sampleList.constEnd();
        if (iList != sampleList.constBegin()) {
            Sample sample1 = *iList;
            // if there is another sample
            --iList;
            if (iList != sampleList.begin()) {
                // get the previous sample
                Sample sample2 = *iList;
                // calculate the change / second
                int elapsedSecs = sample2.dateTime().secsTo(sample1.dateTime());
                if (elapsedSecs > 0) {
                    quint64 val1 = sample1.data(iter->first);
                    quint64 val2 = sample2.data(iter->first);
                    quint64 delta = val1 - val2;
                    float rate = delta / elapsedSecs;
                    return QString::number(rate);
                }
            }
        }
    }
    return QString("--");
}

void WidgetQmfObject::setSectionName(const QString &name)
{
    ui->pushButton->setText(name);
}

void WidgetQmfObject::showRelated(const qmf::Data& object, const QString &widget_type, ArrowDirection a)
{
    const qpid::types::Variant::Map& attrs(object.getProperties());

    setArrow(a);
    ui->labelName->hide();
    ui->labelIndex->show();

    std::string field = "name";
    if (widget_type == "widgetBindings")
        field = "bindingKey";
    qpid::types::Variant::Map::const_iterator iter = attrs.find(field);
    if (iter != attrs.end()) {
        std::string name = iter->second.asString();

        // find the object(s) whose queueRef["_object_name"] ends in :name
        if (widget_type == "widgetQueues") {
            std::string cname = ":" + name;
            related->setRelatedData("queueRef", cname);
        } else if (widget_type == "widgetExchanges") {
            std::string cname = ":" + name;
            related->setRelatedData("exchangeRef", cname);
        } else if (widget_type == "widgetBindings") {
            related->setRelatedData("name", name);
        }
        related->clearFilter();
        emit needData();
    }
}

// SLOT triggered when the dialog's finalAdded signal is fired
// All the rows in the data table are ready.
// If we are showing a related object, make sure a row in
// our comboBox is selected
void WidgetQmfObject::initRelated()
{
    if (!_current) {

        if (ui->comboBox->currentIndex() == -1) {
            if (ui->comboBox->model()->rowCount() > 0) {
                // setting the index should automatically trigger a relatedIndexChanged
                ui->comboBox->setCurrentIndex(0);
                return;
            }
        }
        relatedIndexChanged(ui->comboBox->currentIndex());
    }
}

// SLOT called when the current row in the ui->comboBox changes
void WidgetQmfObject::relatedIndexChanged(int i)
{
    update();

    if (i < 0) {
        // the combobox is empty, there is no related object
        ui->labelIndex->setText(QString("No %1").arg(ui->labelRelated->text().toLower()));
        ui->comboBox->hide();
        ui->tableWidget->hide();
        return;
    }
    int rows = ui->comboBox->model()->rowCount();
    int row = ui->comboBox->currentIndex() + 1;
    if (rows) {
        ui->comboBox->show();
        QString indexText = QString("%1 of %2 %3").arg(row).arg(rows).arg(ui->labelRelated->text().toLower());
        ui->labelIndex->setText(indexText);
        ui->labelIndex->show();
        ui->tableWidget->show();
    }

    QModelIndex source_row = related->mapToSource(related->index(i, 0));
    ObjectListModel *model = (ObjectListModel *)related->sourceModel();
    const qmf::Data& object = model->qmfData(source_row.row());

    // show the current stats for this object
    ui->tableWidget->setVisible(true);
    ui->tableWidget->setRowCount(summaryColumns.size());
    fillTableWidget(object);

    showChart(object, model);

    // cascade the related object
    if (_arrow == arrowLeft)
        if (leftBuddy)
            leftBuddy->showRelated(object, objectName(), arrowLeft);
    if (_arrow == arrowRight)
        if (rightBuddy)
            rightBuddy->showRelated(object, objectName(), arrowRight);

}

void WidgetQmfObject::showChart(const qmf::Data& object, ObjectListModel *model)
{
    //
    // show the chart for this object
    //
    QList<Column>::const_iterator column_iter = summaryColumns.constBegin();
    QStringList chartColumns;
    // loop through all the columns we might want to display
    while (column_iter != summaryColumns.constEnd()) {
        if ((*column_iter).mode == currentMode && (*column_iter).chart) {
            // accumulate all the columns for this object/chart mode
            chartColumns.append(QString((*column_iter).name.c_str()));
        }
        ++column_iter;
    }
    const qpid::types::Variant::Map& props(object.getProperties());
    QString name(props.find(unique)->second.asString().c_str());

    ui->widgetChart->updateChart(model, name, chartColumns, 600);
}

QStringList WidgetQmfObject::getSampleProperties()
{
    QSet<QString> set;
    QList<Column>::const_iterator iter = summaryColumns.constBegin();
    while (iter != summaryColumns.constEnd()) {
        if ((*iter).chart)
            set.insert(QString((*iter).name.c_str()));
        ++iter;
    }
    return set.toList();
}
