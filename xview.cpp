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
    setupStatusBar();

    // allow qmf types to be passed in signals
    qRegisterMetaType<qmf::Data>();
    qRegisterMetaType<qmf::ConsoleEvent>();
    qRegisterMetaType<QItemSelection>();

    //
    // Restore the window size and location and menu check boxes
    //
    QSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    ui->actionCharts->setChecked( settings.value("mainWindowChecks/Charts", false).toBool());
    if (settings.value("mainWindowChecks/Layout", false).toBool())
        ui->action_Cascading->setChecked(true);
    else
        ui->action_Horizontal->setChecked(true);

    // restore the checkboxes for the update menu items
    if (settings.value("mainWindowChecks/Update", true).toBool())
        ui->actionUpdate_all->setChecked(true);
    else
        ui->actionUpdate_visible->setChecked(true);

    // initialize the widgets to know which update strategy to use
    toggleUpdate();

    // connect the menu items to the widgets so the widgets know when the update
    // strategy has changed
    connect(ui->actionUpdate_all, SIGNAL(changed()), this, SLOT(toggleUpdate()));

    // restore the checkboxes for the chart type menu items
    if (settings.value("mainWindowChecks/Chart", true).toBool())
        ui->actionDraw_area_charts->setChecked(true);
    else
        ui->actionDraw_point_charts->setChecked(true);

    // initialize the widgets to know which chart type to use
    toggleChartType();

    // connect the menu items to the widgets so the widgets know when the update
    // strategy has changed
    connect(ui->actionDraw_area_charts, SIGNAL(changed()), this, SLOT(toggleChartType()));

    // setup the layout
    FisheyeLayout *fisheyeLayout;
    fisheyeLayout = new FisheyeLayout(ui->centralWidget, ui->action_Horizontal->isChecked());
    fisheyeLayout->addWidget(ui->widgetExchanges);
    fisheyeLayout->addWidget(ui->widgetBindings);
    fisheyeLayout->addWidget(ui->widgetQueues);
    fisheyeLayout->addWidget(ui->widgetSubscriptions);
    fisheyeLayout->addWidget(ui->widgetSessions);
    fisheyeLayout->addWidget(ui->widgetConnections);;
    fisheyeLayout->setCascade(ui->action_Cascading->isChecked());

    connect(ui->action_Cascading, SIGNAL(toggled(bool)), fisheyeLayout, SLOT(setCascade(bool)));
    connect(ui->action_Cascading, SIGNAL(toggled(bool)), ui->widgetExchanges, SLOT(showRelatedButtons(bool)));
    connect(ui->action_Horizontal, SIGNAL(changed()), this, SLOT(toggleLayout()));

    ui->widgetExchanges->setDrawAsRect(ui->action_Cascading->isChecked());
    ui->widgetBindings->setDrawAsRect(ui->action_Cascading->isChecked());
    ui->widgetQueues->setDrawAsRect(ui->action_Cascading->isChecked());
    ui->widgetSubscriptions->setDrawAsRect(ui->action_Cascading->isChecked());
    ui->widgetSessions->setDrawAsRect(ui->action_Cascading->isChecked());
    ui->widgetConnections->setDrawAsRect(ui->action_Cascading->isChecked());

    // connect the charts menu item to show/hide the charts
    ui->widgetExchanges->showChart(ui->actionCharts->isChecked());
    ui->widgetBindings->showChart(ui->actionCharts->isChecked());
    ui->widgetQueues->showChart(ui->actionCharts->isChecked());
    ui->widgetSubscriptions->showChart(ui->actionCharts->isChecked());
    ui->widgetSessions->showChart(ui->actionCharts->isChecked());
    ui->widgetConnections->showChart(ui->actionCharts->isChecked());

    connect(ui->actionCharts, SIGNAL(toggled(bool)), ui->widgetExchanges, SLOT(showChart(bool)));
    connect(ui->actionCharts, SIGNAL(toggled(bool)), ui->widgetBindings, SLOT(showChart(bool)));
    connect(ui->actionCharts, SIGNAL(toggled(bool)), ui->widgetQueues, SLOT(showChart(bool)));
    connect(ui->actionCharts, SIGNAL(toggled(bool)), ui->widgetSubscriptions, SLOT(showChart(bool)));
    connect(ui->actionCharts, SIGNAL(toggled(bool)), ui->widgetSessions, SLOT(showChart(bool)));
    connect(ui->actionCharts, SIGNAL(toggled(bool)), ui->widgetConnections, SLOT(showChart(bool)));

    connect(ui->actionExchanges,     SIGNAL(triggered()), ui->widgetExchanges, SLOT(setFocus()));
    connect(ui->actionBindings,      SIGNAL(triggered()), ui->widgetBindings, SLOT(setFocus()));
    connect(ui->actionQueues,        SIGNAL(triggered()), ui->widgetQueues, SLOT(setFocus()));
    connect(ui->actionSubscriptions, SIGNAL(triggered()), ui->widgetSubscriptions, SLOT(setFocus()));
    connect(ui->actionSessions,      SIGNAL(triggered()), ui->widgetSessions, SLOT(setFocus()));
    connect(ui->actionConnections,   SIGNAL(triggered()), ui->widgetConnections, SLOT(setFocus()));

    //
    // Create the thread object that maintains communication with the messaging plane.
    //
    qmf = new QmfThread(this);
    qmf->start();

    connect(qmf, SIGNAL(connectionStatusChanged(QString)), label_connection_status, SLOT(setText(QString)));

    connect(qmf, SIGNAL(receivedResponse(QObject*,qmf::ConsoleEvent,bool)), this, SLOT(dispatchResponse(QObject*,qmf::ConsoleEvent,bool)));
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
    ui->widgetExchanges->leftBuddy = 0;
    ui->widgetExchanges->rightBuddy = ui->widgetBindings;
    ui->widgetExchanges->peers.append(ui->widgetQueues);
    ui->widgetExchanges->peers.append(ui->widgetSubscriptions);
    ui->widgetExchanges->peers.append(ui->widgetSessions);
    ui->widgetExchanges->peers.append(ui->widgetConnections);

    ui->widgetBindings->leftBuddy= ui->widgetExchanges;
    ui->widgetBindings->rightBuddy = ui->widgetQueues;
    ui->widgetBindings->peers.append(ui->widgetSubscriptions);
    ui->widgetBindings->peers.append(ui->widgetSessions);
    ui->widgetBindings->peers.append(ui->widgetConnections);

    ui->widgetQueues->leftBuddy = ui->widgetBindings;
    ui->widgetQueues->rightBuddy = ui->widgetSubscriptions;
    ui->widgetQueues->peers.append(ui->widgetExchanges);
    ui->widgetQueues->peers.append(ui->widgetSessions);
    ui->widgetQueues->peers.append(ui->widgetConnections);

    ui->widgetSubscriptions->leftBuddy = ui->widgetQueues;
    ui->widgetSubscriptions->rightBuddy = ui->widgetSessions;
    ui->widgetSubscriptions->peers.append(ui->widgetBindings);
    ui->widgetSubscriptions->peers.append(ui->widgetExchanges);
    ui->widgetSubscriptions->peers.append(ui->widgetConnections);

    ui->widgetSessions->leftBuddy = ui->widgetSubscriptions;
    ui->widgetSessions->rightBuddy = ui->widgetConnections;
    ui->widgetSessions->peers.append(ui->widgetQueues);
    ui->widgetSessions->peers.append(ui->widgetBindings);
    ui->widgetSessions->peers.append(ui->widgetExchanges);

    ui->widgetConnections->leftBuddy = ui->widgetSessions;
    ui->widgetConnections->rightBuddy = 0;
    ui->widgetConnections->peers.append(ui->widgetQueues);
    ui->widgetConnections->peers.append(ui->widgetBindings);
    ui->widgetConnections->peers.append(ui->widgetExchanges);
    ui->widgetConnections->peers.append(ui->widgetSubscriptions);

    ui->widgetExchanges->setAction(ui->actionExchanges);
    ui->widgetBindings->setAction(ui->actionBindings);
    ui->widgetQueues->setAction(ui->actionQueues);
    ui->widgetSubscriptions->setAction(ui->actionSubscriptions);
    ui->widgetSessions->setAction(ui->actionSessions);
    ui->widgetConnections->setAction(ui->actionConnections);

    ui->widgetExchanges->initRelatedButtons();
    ui->widgetBindings->initRelatedButtons();
    ui->widgetQueues->initRelatedButtons();
    ui->widgetSubscriptions->initRelatedButtons();
    ui->widgetSessions->initRelatedButtons();
    ui->widgetConnections->initRelatedButtons();

    ui->widgetExchanges->showRelatedButtons(ui->action_Cascading->isChecked());

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
    connect(ui->widgetSessions, SIGNAL(needData()),
            this, SLOT(querySessions()));
    connect(ui->widgetConnections, SIGNAL(needData()),
            this, SLOT(queryConnections()));


    // when the sections request update for a single object, send it to qmf
    connect(ui->widgetExchanges, SIGNAL(needUpdate()), this, SLOT(updateExchange()));
    connect(ui->widgetBindings, SIGNAL(needUpdate()), this, SLOT(updateBinding()));
    connect(ui->widgetQueues, SIGNAL(needUpdate()), this, SLOT(updateQueue()));
    connect(ui->widgetSubscriptions, SIGNAL(needUpdate()), this, SLOT(updateSubscription()));
    connect(ui->widgetSessions, SIGNAL(needUpdate()), this, SLOT(updateSession()));
    connect(ui->widgetConnections, SIGNAL(needUpdate()), this, SLOT(updateConnection()));

    connect(ui->actionMessages,     SIGNAL(triggered()), this, SLOT(setMessageMode()));
    connect(ui->actionBytes,        SIGNAL(triggered()), this, SLOT(setByteMode()));
    connect(ui->actionMessage_rate, SIGNAL(triggered()), this, SLOT(setMessageRateMode()));
    connect(ui->actionByte_rate,    SIGNAL(triggered()), this, SLOT(setByteRateMode()));


    // The dialog boxes share the same data-model with the widgets
    // Create the dialog boxes and pass their model to the widgets
    exchangesDialog = new DialogExchanges(this, "exchanges");
    exchangesDialog->initModels("name", ui->widgetExchanges->getSampleProperties());
    ui->widgetExchanges->setRelatedModel(exchangesDialog->listModel(), this);
    // when the widget's button is clicked, get all the objects and show the dialog box
    connect(ui->widgetExchanges->pushButton(), SIGNAL(clicked()), this, SLOT(queryExchanges()));
    connect(ui->widgetExchanges->pushButton(), SIGNAL(clicked()), exchangesDialog, SLOT(exec()));
    // when the OK button is pressed on the dialog, set the widgets current object
    connect(exchangesDialog, SIGNAL(setCurrentObject(qmf::Data,QString)),
            ui->widgetExchanges, SLOT(setCurrentObject(qmf::Data)));
    connect(exchangesDialog, SIGNAL(objectRefreshed()),
            ui->widgetExchanges, SLOT(objectRefreshed()));
    // after a background update is complete, tell the widget to show related objects
    connect(exchangesDialog, SIGNAL(finalAdded()), ui->widgetExchanges, SLOT(initRelated()));
    // select the object that was just pivoted on
    connect(ui->widgetExchanges, SIGNAL(pivotTo(QModelIndex)), exchangesDialog, SLOT(setCurrentRow(QModelIndex)));

    bindingsDialog = new DialogObjects(this, "bindings");
    bindingsDialog->initModels("bindingKey", ui->widgetBindings->getSampleProperties());
    ui->widgetBindings->setRelatedModel(bindingsDialog->listModel(), this);
    connect(bindingsDialog, SIGNAL(setCurrentObject(qmf::Data,QString)),
            ui->widgetBindings, SLOT(setCurrentObject(qmf::Data)));
    connect(bindingsDialog, SIGNAL(objectRefreshed()),
            ui->widgetBindings, SLOT(objectRefreshed()));
    connect(ui->widgetBindings->pushButton(), SIGNAL(clicked()), this, SLOT(queryBindings()));
    connect(ui->widgetBindings->pushButton(), SIGNAL(clicked()), bindingsDialog, SLOT(exec()));
    connect(bindingsDialog, SIGNAL(finalAdded()), ui->widgetBindings, SLOT(initRelated()));
    connect(ui->widgetBindings, SIGNAL(pivotTo(QModelIndex)), bindingsDialog, SLOT(setCurrentRow(QModelIndex)));

    queuesDialog = new DialogObjects(this, "queues");
    queuesDialog->initModels("name", ui->widgetQueues->getSampleProperties());
    ui->widgetQueues->setRelatedModel(queuesDialog->listModel(), this);
    connect(queuesDialog, SIGNAL(setCurrentObject(qmf::Data,QString)),
            ui->widgetQueues, SLOT(setCurrentObject(qmf::Data)));
    connect(queuesDialog, SIGNAL(objectRefreshed()),
            ui->widgetQueues, SLOT(objectRefreshed()));
    connect(ui->widgetQueues->pushButton(), SIGNAL(clicked()), this, SLOT(queryQueues()));
    connect(ui->widgetQueues->pushButton(), SIGNAL(clicked()), queuesDialog, SLOT(exec()));
    connect(queuesDialog, SIGNAL(finalAdded()), ui->widgetQueues, SLOT(initRelated()));
    connect(ui->widgetQueues, SIGNAL(pivotTo(QModelIndex)), queuesDialog, SLOT(setCurrentRow(QModelIndex)));

    subscriptionsDialog = new DialogObjects(this, "subscriptions");
    subscriptionsDialog->initModels("name", ui->widgetSubscriptions->getSampleProperties());
    ui->widgetSubscriptions->setRelatedModel(subscriptionsDialog->listModel(), this);
    connect(subscriptionsDialog, SIGNAL(setCurrentObject(qmf::Data,QString)),
            ui->widgetSubscriptions, SLOT(setCurrentObject(qmf::Data)));
    connect(subscriptionsDialog, SIGNAL(objectRefreshed()),
            ui->widgetSubscriptions, SLOT(objectRefreshed()));
    connect(ui->widgetSubscriptions->pushButton(), SIGNAL(clicked()), this, SLOT(querySubscriptions()));
    connect(ui->widgetSubscriptions->pushButton(), SIGNAL(clicked()), subscriptionsDialog, SLOT(exec()));
    connect(subscriptionsDialog, SIGNAL(finalAdded()), ui->widgetSubscriptions, SLOT(initRelated()));
    connect(ui->widgetSubscriptions, SIGNAL(pivotTo(QModelIndex)), subscriptionsDialog, SLOT(setCurrentRow(QModelIndex)));

    sessionsDialog = new DialogObjects(this, "subscriptions");
    sessionsDialog->initModels("name", ui->widgetSessions->getSampleProperties());
    ui->widgetSessions->setRelatedModel(sessionsDialog->listModel(), this);
    connect(sessionsDialog, SIGNAL(setCurrentObject(qmf::Data,QString)),
            ui->widgetSessions, SLOT(setCurrentObject(qmf::Data)));
    connect(sessionsDialog, SIGNAL(objectRefreshed()),
            ui->widgetSessions, SLOT(objectRefreshed()));
    connect(ui->widgetSessions->pushButton(), SIGNAL(clicked()), this, SLOT(querySubscriptions()));
    connect(ui->widgetSessions->pushButton(), SIGNAL(clicked()), sessionsDialog, SLOT(exec()));
    connect(sessionsDialog, SIGNAL(finalAdded()), ui->widgetSessions, SLOT(initRelated()));
    connect(ui->widgetSessions, SIGNAL(pivotTo(QModelIndex)), sessionsDialog, SLOT(setCurrentRow(QModelIndex)));

    connectionsDialog = new DialogObjects(this, "connections");
    connectionsDialog->initModels("address", ui->widgetConnections->getSampleProperties());
    connectionsDialog->setKey("remoteProcessName"); // use this field in the object list
    ui->widgetConnections->setRelatedModel(connectionsDialog->listModel(), this);
    connect(connectionsDialog, SIGNAL(setCurrentObject(qmf::Data,QString)),
            ui->widgetConnections, SLOT(setCurrentObject(qmf::Data)));
    connect(connectionsDialog, SIGNAL(objectRefreshed()),
            ui->widgetConnections, SLOT(objectRefreshed()));
    connect(ui->widgetConnections->pushButton(), SIGNAL(clicked()), this, SLOT(queryConnections()));
    connect(ui->widgetConnections->pushButton(), SIGNAL(clicked()), connectionsDialog, SLOT(exec()));
    connect(connectionsDialog, SIGNAL(finalAdded()), ui->widgetConnections, SLOT(initRelated()));
    connect(ui->widgetConnections, SIGNAL(pivotTo(QModelIndex)), connectionsDialog, SLOT(setCurrentRow(QModelIndex)));

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
    connect(qmf, SIGNAL(isConnected(bool)), ui->widgetSessions,          SLOT(setEnabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), ui->widgetConnections,       SLOT(setEnabled(bool)));

    connect(qmf, SIGNAL(isConnected(bool)), exchangesDialog,             SLOT(connectionChanged(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), bindingsDialog,              SLOT(connectionChanged(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), queuesDialog,                SLOT(connectionChanged(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), subscriptionsDialog,         SLOT(connectionChanged(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), sessionsDialog,              SLOT(connectionChanged(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), connectionsDialog,           SLOT(connectionChanged(bool)));

    // always start on the message mode
    setMessageMode();

}

void XView::toggleChartType()
{
    ui->widgetExchanges->setChartType(ui->actionDraw_area_charts->isChecked());
    ui->widgetBindings->setChartType(ui->actionDraw_area_charts->isChecked());
    ui->widgetQueues->setChartType(ui->actionDraw_area_charts->isChecked());
    ui->widgetSubscriptions->setChartType(ui->actionDraw_area_charts->isChecked());
    ui->widgetSessions->setChartType(ui->actionDraw_area_charts->isChecked());
    ui->widgetConnections->setChartType(ui->actionDraw_area_charts->isChecked());
}

void XView::toggleUpdate()
{
    ui->widgetExchanges->setUpdateStrategy(ui->actionUpdate_all->isChecked());
    ui->widgetBindings->setUpdateStrategy(ui->actionUpdate_all->isChecked());
    ui->widgetQueues->setUpdateStrategy(ui->actionUpdate_all->isChecked());
    ui->widgetSubscriptions->setUpdateStrategy(ui->actionUpdate_all->isChecked());
    ui->widgetSessions->setUpdateStrategy(ui->actionUpdate_all->isChecked());
    ui->widgetConnections->setUpdateStrategy(ui->actionUpdate_all->isChecked());
}

void XView::toggleLayout()
{
    ui->widgetExchanges->setDrawAsRect(ui->action_Cascading->isChecked());
    ui->widgetBindings->setDrawAsRect(ui->action_Cascading->isChecked());
    ui->widgetQueues->setDrawAsRect(ui->action_Cascading->isChecked());
    ui->widgetSubscriptions->setDrawAsRect(ui->action_Cascading->isChecked());
    ui->widgetSessions->setDrawAsRect(ui->action_Cascading->isChecked());
    ui->widgetConnections->setDrawAsRect(ui->action_Cascading->isChecked());

    ui->centralWidget->layout()->update();
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
    ui->actionMessage_rate->setIcon(QIcon(":/images/msgrate.png"));
    ui->actionByte_rate->setIcon(QIcon(":/images/byterate.png"));

    actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->actionMessages);
    actionGroup->addAction(ui->actionBytes);
    actionGroup->addAction(ui->actionMessage_rate);
    actionGroup->addAction(ui->actionByte_rate);

    layoutGroup = new QActionGroup(ui->menu_Layout);
    layoutGroup->addAction(ui->action_Horizontal);
    layoutGroup->addAction(ui->action_Cascading);
    layoutGroup->setExclusive(true);

    updateGroup = new QActionGroup(ui->menu_Edit);
    updateGroup->addAction(ui->actionUpdate_all);
    updateGroup->addAction(ui->actionUpdate_visible);
    updateGroup->setExclusive(true);

    chartGroup = new QActionGroup(ui->menu_Edit);
    chartGroup->addAction(ui->actionDraw_area_charts);
    chartGroup->addAction(ui->actionDraw_point_charts);

    modeToolBar = addToolBar(tr("Modes"));
    modeToolBar->setObjectName("Mode");
    //modeToolBar->setIconSize(QSize(32,32));
    modeToolBar->addActions(actionGroup->actions());

    QList<QToolButton *> buttons = modeToolBar->findChildren<QToolButton *>();
    int i;
    QToolButton *button;
    for (i=1; i<buttons.size() && i<5; ++i) {
        button = buttons.at(i);
        QString background = WidgetQmfObject::colors[i-1].name();
        button->setStyleSheet(QString("background: %1; checked.background: %2").
                              arg(background).
                              arg(background));
    }

    modeToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

// SLOT Triggered when the qmf thread has been idle for 3 seconds
// If there is a current object in a widget, refresh it
void XView::queryCurrent()
{
    if (ui->widgetExchanges->current())
        qmf->queryObject(ui->widgetExchanges->getDataAddr(), exchangesDialog);
    else if (ui->widgetBindings->current())
        qmf->queryObject(ui->widgetBindings->getDataAddr(), bindingsDialog);
    else if (ui->widgetQueues->current())
        qmf->queryObject(ui->widgetQueues->getDataAddr(), queuesDialog);
    else if (ui->widgetSubscriptions->current())
        qmf->queryObject(ui->widgetSubscriptions->getDataAddr(), subscriptionsDialog);
    else if (ui->widgetSessions->current())
        qmf->queryObject(ui->widgetSessions->getDataAddr(), sessionsDialog);
    else if (ui->widgetConnections->current())
        qmf->queryObject(ui->widgetConnections->getDataAddr(), connectionsDialog);
}

// Send an async query to get the list of objects
// When the response is received, send an event to the object's dialog
void XView::queryObjects(const std::string& qmf_class, DialogObjects* dialog)
{
    qmf->queryBroker(qmf_class, dialog);
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

// SLOT: triggered when Sessions Dialog is displayed
// Send an async query to get the list of Sessions
// When the response is received, send an event to the Sessions dialog
void XView::querySessions()
{
    queryObjects("session", sessionsDialog);
}

// SLOT: triggered when Connections Dialog is displayed
// Send an async query to get the list of Connections
// When the response is received, send an event to the Connections dialog
void XView::queryConnections()
{
    queryObjects("connection", connectionsDialog);
}

// SLOT: Triggered when a qmf query response is received
// Send the received event over to the appropriate dialog box
void XView::dispatchResponse(QObject *target, const qmf::ConsoleEvent& event, bool all)
{
    DialogObjects *dialog = (DialogObjects *)target;
    dialog->gotDataEvent(event, all);
}

void XView::updateExchange()
{
    if (exchangesDialog->isHidden())
        qmf->queryObject(ui->widgetExchanges->getDataAddr(), exchangesDialog);
}

void XView::updateBinding()
{
    if (bindingsDialog->isHidden())
        qmf->queryObject(ui->widgetBindings->getDataAddr(), bindingsDialog);
}

void XView::updateQueue()
{
    if (queuesDialog->isHidden())
        qmf->queryObject(ui->widgetQueues->getDataAddr(), queuesDialog);
}

void XView::updateSubscription()
{
    if (subscriptionsDialog->isHidden())
        qmf->queryObject(ui->widgetSubscriptions->getDataAddr(), subscriptionsDialog);
}

void XView::updateSession()
{
    if (sessionsDialog->isHidden())
        qmf->queryObject(ui->widgetSessions->getDataAddr(), sessionsDialog);
}

void XView::updateConnection()
{
    if (connectionsDialog->isHidden())
        qmf->queryObject(ui->widgetConnections->getDataAddr(), connectionsDialog);
}

void XView::setMessageMode()
{
    setMode(WidgetQmfObject::modeMessages);
}
void XView::setByteMode()
{
    setMode(WidgetQmfObject::modeBytes);
}
void XView::setMessageRateMode()
{
    setMode(WidgetQmfObject::modeMessageRate);
}
void XView::setByteRateMode()
{
    setMode(WidgetQmfObject::modeByteRate);
}
void XView::setMode(WidgetQmfObject::StatMode mode)
{
    ui->widgetBindings->setCurrentMode(mode);
    ui->widgetExchanges->setCurrentMode(mode);
    ui->widgetQueues->setCurrentMode(mode);
    ui->widgetSubscriptions->setCurrentMode(mode);
    ui->widgetSessions->setCurrentMode(mode);
    ui->widgetConnections->setCurrentMode(mode);
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
    settings.setValue("mainWindowChecks/Charts", ui->actionCharts->isChecked());
    settings.setValue("mainWindowChecks/Layout", ui->action_Cascading->isChecked());
    settings.setValue("mainWindowChecks/Update", ui->actionUpdate_all->isChecked());
    settings.setValue("mainWindowChecks/Chart",   ui->actionDraw_area_charts->isChecked());

    delete openDialog;
    delete aboutDialog;
    delete bindingsDialog;
    delete exchangesDialog;
    delete queuesDialog;
    delete subscriptionsDialog;
    delete sessionsDialog;
    delete connectionsDialog;

    delete label_connection_status;
    delete label_connection_prompt;

    delete chartGroup;
    delete actionGroup;
    delete layoutGroup;
    delete updateGroup;
    delete modeToolBar;

    qmf->cancel();
    qmf->wait();
    delete qmf;

    delete ui;
}
