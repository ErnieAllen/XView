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
    qRegisterMetaType<qmf::ConsoleEvent>();
    qRegisterMetaType<QItemSelection>();

    //
    // Restore the window size and location
    //
    QSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());

    //
    // Create the thread object that maintains communication with the messaging plane.
    //
    qmf = new QmfThread(this);
    qmf->start();

    setupStatusBar();
    connect(qmf, SIGNAL(connectionStatusChanged(QString)), label_connection_status, SLOT(setText(QString)));

    connect(qmf, SIGNAL(receivedResponse(QObject*,qmf::ConsoleEvent)), this, SLOT(dispatchResponse(QObject*,qmf::ConsoleEvent)));
    connect(qmf, SIGNAL(qmfTimer()), this, SLOT(queryCurrent()));

    // menu actions to open and close the broker connection
    connect(ui->actionOpen_localhost, SIGNAL(triggered()), qmf, SLOT(connect_localhost()));
    connect(ui->actionClose, SIGNAL(triggered()), qmf, SLOT(disconnect()));

    //
    // Create the dialog boxes
    //
    openDialog = new DialogOpen(this);
    // when the menu item is selected, show the dialog
    connect(ui->actionOpen_URL, SIGNAL(triggered()), openDialog, SLOT(show()));
    // when the dialog is accepted, open the URL
    connect(openDialog, SIGNAL(dialogOpenAccepted(QString,QString,QString)), qmf, SLOT(connect_url(QString,QString,QString)));

    aboutDialog = new DialogAbout(this);
    // when the menu item is selected, show the dialog
    connect(ui->actionAbout, SIGNAL(triggered()), aboutDialog, SLOT(show()));

    // make direct connections between the sections to simplify
    // the logistics of showing related objects
    ui->widgetExchanges->rightBuddy = ui->widgetBindings;
    ui->widgetExchanges->peers.append(ui->widgetQueues);
    ui->widgetExchanges->peers.append(ui->widgetSubscriptions);

    ui->widgetBindings->leftBuddy= ui->widgetExchanges;
    ui->widgetBindings->rightBuddy = ui->widgetQueues;
    ui->widgetBindings->peers.append(ui->widgetSubscriptions);

    ui->widgetQueues->leftBuddy = ui->widgetBindings;
    ui->widgetQueues->rightBuddy = ui->widgetSubscriptions;
    ui->widgetQueues->peers.append(ui->widgetExchanges);

    ui->widgetSubscriptions->leftBuddy = ui->widgetQueues;
    ui->widgetSubscriptions->peers.append(ui->widgetBindings);
    ui->widgetSubscriptions->peers.append(ui->widgetExchanges);

    // when the sections request data, send the request
    // over to qmf
    connect(ui->widgetBindings, SIGNAL(needData()),
            this, SLOT(queryBindings()));
    connect(ui->widgetExchanges, SIGNAL(needData()),
            this, SLOT(queryExchanges()));
    connect(ui->widgetQueues, SIGNAL(needData()),
            this, SLOT(queryQueues()));
    connect(ui->widgetSubscriptions, SIGNAL(needData()),
            this, SLOT(querySubscriptions()));


    // The dialog boxes share the same data-model with the widgets
    // Create the dialog boxes and pass their model to the widgets
    exchangesDialog = new DialogExchanges(this, "exchanges");
    exchangesDialog->initModels("name");
    ui->widgetExchanges->setRelatedModel(exchangesDialog->listModel(), this);
    connect(exchangesDialog, SIGNAL(setCurrentObject(qmf::Data,QString)),
            ui->widgetExchanges, SLOT(setCurrentObject(qmf::Data)));
    connect(exchangesDialog, SIGNAL(objectRefreshed(qmf::Data,QString)),
            ui->widgetExchanges, SLOT(showData(qmf::Data)));
    connect(ui->widgetExchanges->pushButton(), SIGNAL(clicked()), this, SLOT(queryExchanges()));
    connect(ui->widgetExchanges->pushButton(), SIGNAL(clicked()), exchangesDialog, SLOT(exec()));
    connect(exchangesDialog, SIGNAL(finalAdded()), ui->widgetExchanges, SLOT(initRelated()));

    bindingsDialog = new DialogObjects(this, "bindings");
    bindingsDialog->initModels("bindingKey");
    ui->widgetBindings->setRelatedModel(bindingsDialog->listModel(), this);
    connect(bindingsDialog, SIGNAL(setCurrentObject(qmf::Data,QString)),
            ui->widgetBindings, SLOT(setCurrentObject(qmf::Data)));
    connect(bindingsDialog, SIGNAL(objectRefreshed(qmf::Data,QString)),
            ui->widgetBindings, SLOT(showData(qmf::Data)));
    connect(ui->widgetBindings->pushButton(), SIGNAL(clicked()), this, SLOT(queryBindings()));
    connect(ui->widgetBindings->pushButton(), SIGNAL(clicked()), bindingsDialog, SLOT(exec()));
    connect(bindingsDialog, SIGNAL(finalAdded()), ui->widgetBindings, SLOT(initRelated()));

    queuesDialog = new DialogObjects(this, "queues");
    queuesDialog->initModels("name");
    ui->widgetQueues->setRelatedModel(queuesDialog->listModel(), this);
    connect(queuesDialog, SIGNAL(setCurrentObject(qmf::Data,QString)),
            ui->widgetQueues, SLOT(setCurrentObject(qmf::Data)));
    connect(queuesDialog, SIGNAL(objectRefreshed(qmf::Data,QString)),
            ui->widgetQueues, SLOT(showData(qmf::Data)));
    connect(ui->widgetQueues->pushButton(), SIGNAL(clicked()), this, SLOT(queryQueues()));
    connect(ui->widgetQueues->pushButton(), SIGNAL(clicked()), queuesDialog, SLOT(exec()));
    connect(queuesDialog, SIGNAL(finalAdded()), ui->widgetQueues, SLOT(initRelated()));

    subscriptionsDialog = new DialogObjects(this, "subscriptions");
    subscriptionsDialog->initModels("name");
    ui->widgetSubscriptions->setRelatedModel(subscriptionsDialog->listModel(), this);
    connect(subscriptionsDialog, SIGNAL(setCurrentObject(qmf::Data,QString)),
            ui->widgetSubscriptions, SLOT(setCurrentObject(qmf::Data)));
    connect(subscriptionsDialog, SIGNAL(objectRefreshed(qmf::Data,QString)),
            ui->widgetSubscriptions, SLOT(showData(qmf::Data)));
    connect(ui->widgetSubscriptions->pushButton(), SIGNAL(clicked()), this, SLOT(querySubscriptions()));
    connect(ui->widgetSubscriptions->pushButton(), SIGNAL(clicked()), subscriptionsDialog, SLOT(exec()));
    connect(subscriptionsDialog, SIGNAL(finalAdded()), ui->widgetSubscriptions, SLOT(initRelated()));

    //
    // Create linkages to enable and disable main-window components based on the connection status.
    //
    connect(qmf, SIGNAL(isConnected(bool)), ui->actionOpen_localhost,    SLOT(setDisabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), ui->actionOpen_URL,          SLOT(setDisabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), ui->actionClose,             SLOT(setEnabled(bool)));

    connect(qmf, SIGNAL(isConnected(bool)), ui->widgetExchanges,         SLOT(setEnabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), ui->widgetBindings,          SLOT(setEnabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), ui->widgetQueues,            SLOT(setEnabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), ui->widgetSubscriptions,     SLOT(setEnabled(bool)));

    connect(qmf, SIGNAL(isConnected(bool)), exchangesDialog,             SLOT(connectionChanged(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), bindingsDialog,              SLOT(connectionChanged(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), queuesDialog,                SLOT(connectionChanged(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), subscriptionsDialog,         SLOT(connectionChanged(bool)));
}

// Display the connection status in the status bar
void XView::setupStatusBar() {
    label_connection_status = new QLabel();
    label_connection_prompt = new QLabel();
    label_connection_prompt->setText("Connection status: ");
    statusBar()->addWidget(label_connection_prompt);
    statusBar()->addWidget(label_connection_status);

    ui->actionMessages->setIcon(QIcon(":/images/messages.png"));
    ui->actionBytes->setIcon(QIcon(":/images/bytes.png"));
    actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->actionMessages);
    actionGroup->addAction(ui->actionBytes);
    actionGroup->addAction(ui->actionMessage_rate);
    actionGroup->addAction(ui->actionByte_rate);

    modeToolBar = addToolBar(tr("Modes"));
    modeToolBar->setObjectName("Mode");
    modeToolBar->addActions(actionGroup->actions());
    modeToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

// SLOT Triggered when the qmf thread has been idle for 2 seconds
// If there is a current object in a widget, refresh it
void XView::queryCurrent()
{
    if (ui->widgetBindings->current() || bindingsDialog->isVisible())
        queryBindings();
    else if (ui->widgetExchanges->current() || exchangesDialog->isVisible())
        queryExchanges();
    else if (ui->widgetQueues->current() || queuesDialog->isVisible())
        queryQueues();
    else if (ui->widgetSubscriptions->current() || subscriptionsDialog->isVisible())
        querySubscriptions();
}

// Send an async query to get the list of objects
// When the response is received, send an event to the object's dialog
void XView::queryObjects(const std::string& qmf_class, DialogObjects* dialog)
{
    qmf->queryBroker(qmf_class, dialog, dialog->eventType);
}

// SLOT: triggered when Exchange Dialog is displayed
// Send an async query to get the list of exchanges
// When the response is received, send an event to the exchange dialog
void XView::queryExchanges()
{
    queryObjects("exchange", exchangesDialog);
}

// SLOT: triggered when Bindings Dialog is displayed
// Send an async query to get the list of bindings
// When the response is received, send an event to the bindings dialog
void XView::queryBindings()
{
    queryObjects("binding", bindingsDialog);
}

// SLOT: triggered when Queues Dialog is displayed
// Send an async query to get the list of queues
// When the response is received, send an event to the queues dialog
void XView::queryQueues()
{
    queryObjects("queue", queuesDialog);
}

// SLOT: triggered when Subscriptions Dialog is displayed
// Send an async query to get the list of Subscriptions
// When the response is received, send an event to the Subscriptions dialog
void XView::querySubscriptions()
{
    queryObjects("subscription", subscriptionsDialog);
}

// SLOT: Triggered when a qmf query response is received
// Send the received event over to the appropriate dialog box
void XView::dispatchResponse(QObject *target, const qmf::ConsoleEvent& event)
{
    DialogObjects *dialog = (DialogObjects *)target;
    dialog->gotDataEvent(event);
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

    delete openDialog;
    delete aboutDialog;
    delete bindingsDialog;
    delete exchangesDialog;
    delete queuesDialog;
    delete subscriptionsDialog;

    delete label_connection_status;
    delete label_connection_prompt;

    delete actionGroup;
    delete modeToolBar;

    qmf->cancel();
    qmf->wait();
    delete qmf;

    delete ui;
}
