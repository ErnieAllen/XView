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

#include "xview.h"
#include "ui_xview.h"
#include <QSettings>

XView::XView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::XView)
{
    //
    // Setup some global app vales to be used by the QSettings class
    //
    QCoreApplication::setOrganizationName("Red Hat");
    QCoreApplication::setOrganizationDomain("redhat.com");
    QCoreApplication::setApplicationName("XView");

    ui->setupUi(this);

    // allow qmf types to be passed in signals
    qRegisterMetaType<qmf::Data>();

    //
    // Create the thread object that maintains communication with the messaging plane.
    //
    qmf = new QmfThread(this);
    qmf->start();

    setupStatusBar();
    connect(qmf, SIGNAL(connectionStatusChanged(QString)), label_connection_status, SLOT(setText(QString)));

    exchangeModel = new ExchangeListModel(this);
    ui->exchangeListView->setModel(exchangeModel);

    connect(qmf, SIGNAL(addExchange(qmf::Data,uint)), exchangeModel, SLOT(addExchange(qmf::Data,uint)));

    exchangeDetailsModel = new ExchangeDetailsModel(this);
    ui->exchangeTableView->setModel(exchangeDetailsModel);


    // this is the internal object that controls the queue table selection
    QItemSelectionModel* itemSelector = ui->exchangeListView->selectionModel();
    connect(itemSelector, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), exchangeModel, SLOT(selected(QModelIndex)));
    connect(exchangeModel, SIGNAL(exchangeSelected(qmf::Data)), exchangeDetailsModel, SLOT(showExchangeDetail(qmf::Data)));
}

// Display the connection status in the status bar
void XView::setupStatusBar() {
    label_connection_status = new QLabel();
    label_connection_prompt = new QLabel();
    label_connection_prompt->setText("Connection status: ");
    statusBar()->addWidget(label_connection_prompt);
    statusBar()->addWidget(label_connection_status);
}


// process command line arguments
void XView::init(int argc, char *argv[])
{
    QString url;
    QString connectionOptions;
    QString sessionOptions;

    if (argc > 1) {
        url = QString(argv[1]);
        if (argc > 2) {
            connectionOptions = QString(argv[2]);
            if (argc > 3)
                sessionOptions = QString(argv[3]);
        }
    }
    // only connect if we have a url
    if (!url.isEmpty())
        qmf->connect_url(url, connectionOptions, sessionOptions);
}

XView::~XView()
{
    // save the window size and location
    QSettings settings;
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());

    qmf->cancel();
    qmf->wait();
    delete qmf;

    delete ui;
}
