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

#ifndef XVIEW_H
#define XVIEW_H

#include <QtGui>
#include "qmf-thread.h"
#include "dialogopen.h"
#include "dialogabout.h"
#include "dialogobjects.h"
#include "dialogexchanges.h"
#include "widgetqmfobject.h"
#include "fisheyelayout.h"

namespace Ui {
    class XView;
}

Q_DECLARE_METATYPE(qmf::Data);
Q_DECLARE_METATYPE(QItemSelection);

class XView : public QMainWindow
{
    Q_OBJECT

public:
    explicit XView(QWidget *parent = 0);
    ~XView();

    void init(int argc, char *argv[]);

private:
    Ui::XView *ui;

    DialogOpen*      openDialog;
    DialogAbout*     aboutDialog;
    DialogObjects*   bindingsDialog;
    DialogExchanges* exchangesDialog;
    DialogObjects*   queuesDialog;
    DialogObjects*   subscriptionsDialog;
    DialogObjects*   sessionsDialog;
    DialogObjects*   connectionsDialog;
    QActionGroup*    actionGroup;
    QToolBar *       modeToolBar;

    QmfThread* qmf;
    QLabel *label_connection_prompt;
    QLabel *label_connection_status;

    void setupStatusBar();
    void queryObjects(const std::string& qmf_class, DialogObjects* dialog);

    void setMode(WidgetQmfObject::StatMode mode);

private slots:
    void queryExchanges();
    void queryBindings();
    void queryQueues();
    void querySubscriptions();
    void querySessions();
    void queryConnections();
    void dispatchResponse(QObject *target, const qmf::ConsoleEvent& event);
    void queryCurrent();
    void setMessageMode();
    void setByteMode();
    void setMessageRateMode();
    void setByteRateMode();
    void forceLayout();

};

#endif // XVIEW_H
