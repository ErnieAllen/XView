#ifndef _qe_qmf_thread_h
#define _qe_qmf_thread_h
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

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QLineEdit>
#include <QStringList>
#include <QModelIndex>
#include <QEvent>

#include <qpid/messaging/Connection.h>
#include <qmf/ConsoleSession.h>
#include <qmf/ConsoleEvent.h>
#include "qpid/types/Variant.h"
#include <qmf/Data.h>
#include <sstream>
#include <deque>
#include "qmf-event.h"

static QModelIndex defaultIndex;

Q_DECLARE_METATYPE(qmf::ConsoleEvent);

class QmfThread : public QThread {
    Q_OBJECT

public:
    QmfThread(QObject* parent);
    void cancel();

    void queryBroker(const std::string& qmf_class,
                                QObject* object,
                                QEvent::Type event_type);
public slots:
    void connect_localhost();
    void disconnect();
    void connect_url(const QString&, const QString&, const QString&);


signals:
    void connectionStatusChanged(const QString&);
    void isConnected(bool);
    void addExchange(const qmf::Data&, uint);
    void doneAddingExchanges(uint);

    void qmfError(const QString&);
    void receivedResponse(QObject *target, const qmf::ConsoleEvent& event);
    void qmfTimer();

protected:
    void run();

private:
    struct Command {
        bool connect;
        std::string url;
        std::string conn_options;
        std::string qmf_options;

        Command(bool _c, const std::string& _u, const std::string& _co, const std::string& _qo) :
            connect(_c), url(_u), conn_options(_co), qmf_options(_qo) {}
    };
    typedef std::deque<Command> command_queue_t;

    mutable QMutex lock;
    QWaitCondition cond;
    qpid::messaging::Connection conn;
    qmf::ConsoleSession sess;
    bool cancelled;
    bool connected;
    bool disconnecting;
    bool pausedRefreshes;
    command_queue_t command_queue;

    // support for async queries
    struct Query {
        uint32_t correlator;
        QObject* object;
        QEvent::Type t;

        Query(QObject* _o, QEvent::Type _t) : correlator(0),
            object(_o), t(_t) {}
    };
    typedef std::deque<Query> query_queue_t;
    query_queue_t query_queue;
    void dispatchQueryResults(qmf::ConsoleEvent& event);


    // remember the broker object so we can make qmf calls
    qmf::Data brokerData;
};

#endif

