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

#include "dialogobjects.h"
#include "ui_dialogobjects.h"

DialogObjects::DialogObjects(QWidget *parent, const std::string& name) :
    QDialog(parent),
    ui(new Ui::DialogObjects)
{
    ui->setupUi(this);
    QString qname(name.c_str());

    QString title("Select ");
    title += qname;
    ui->labelPrompt->setText(tr(title.toStdString().c_str()));

    eventType = QEvent::Type(QEvent::registerEventType());
    this->setObjectName(QString(name.c_str()));

    restoreSettings();
}

void DialogObjects::initModels(std::string unique)
{
    objectModel = new ObjectListModel(this, unique);
    objectDetailsModel = new ObjectDetailsModel(this);

    initConnections();
}

void DialogObjects::initConnections()
{
    ui->objectListView->setModel(objectModel);
    ui->objectTableView->setModel(objectDetailsModel);

    connect(ui->objectListView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            objectModel, SLOT(selected(QModelIndex)));
    connect(objectModel, SIGNAL(objectSelected(qmf::Data)),
            objectDetailsModel, SLOT(showObjectDetail(qmf::Data)));

    connect(objectDetailsModel, SIGNAL(detailReady()), this, SLOT(resizeDetail()));

}

DialogObjects::~DialogObjects()
{
    saveSettings();
    delete ui;
    if (objectModel)
        delete objectModel;
    if (objectDetailsModel)
        delete objectDetailsModel;
}

// SLOT triggered after the object details have been added
// This will cause the columns in the object details table
// to resize to the longest column contents
void DialogObjects::resizeDetail()
{
    // workaround for a QT bug https://bugreports.qt.nokia.com//browse/QTBUG-9352
    //ui->objectTableView->setVisible(false);
    //ui->objectTableView->resizeColumnsToContents();
    //ui->objectTableView->setVisible(true);
}

void DialogObjects::connectionChanged(bool isConnected)
{
    objectModel->connectionChanged(isConnected);
    objectDetailsModel->connectionChanged(isConnected);
}

void DialogObjects::clear()
{
    objectModel->clear();
    objectDetailsModel->clear();
}

void DialogObjects::selected(const QModelIndex &index)
{
    objectModel->selected(index);
}

// Handle the query response from the QUERY_OBJECT for qmf objects
bool DialogObjects::event(QEvent *e)
{
    QmfEvent* ce = (QmfEvent*)e;
    if (e->type() == eventType) {

        uint32_t pcount = ce->cevent.getDataCount();
        for (uint32_t idx = 0; idx < pcount; idx++) {
            objectModel->addObject(ce->cevent.getData(idx), ce->cevent.getCorrelator());
        }
        // select 1st item if none are selected
        if (ce->cevent.isFinal()) {
            if (!(ui->objectListView->selectionModel()->hasSelection()))
                ui->objectListView->setCurrentIndex(objectModel->index(0, 0));
            emit finalAdded();
        }
        return true;
    }
    return QDialog::event(e);
}

void DialogObjects::accept()
{
    // get the currently selected object in the list
    QModelIndex index = ui->objectListView->selectionModel()->currentIndex();
    emit setCurrentObject(objectModel->getSelected(index), objectName());
    close();
}

void DialogObjects::saveSettings() {
    QSettings settings;

    settings.beginGroup(objectName());
    settings.setValue("Geometry", saveGeometry());
    settings.endGroup();

}

void DialogObjects::restoreSettings() {
    QSettings settings;

    settings.beginGroup(this->objectName());
    restoreGeometry(settings.value("Geometry").toByteArray());
    settings.endGroup();
}
