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

#include "qmf-thread.h"
#include <qpid/messaging/exceptions.h>
#include <qmf/Query.h>
#include <qmf/engine/Value.h>
#include <qmf/DataAddr.h>

#include <iostream>
#include <string>
#include <QtGui/QApplication>

using std::cout;
using std::endl;

QmfThread::QmfThread(QObject* parent) :
    QThread(parent), cancelled(false), connected(false), disconnecting(false)
{
    // Intentionally Left Blank
}


void QmfThread::cancel()
{
    cancelled = true;
}


void QmfThread::connect_localhost()
{
    QMutexLocker locker(&lock);
    command_queue.push_back(Command(true, "localhost", "", "{strict-security:False}"));
    cond.wakeOne();
}

void QmfThread::connect_url(const QString& url, const QString& conn_options, const QString& qmf_options)
{
    QMutexLocker locker(&lock);
    command_queue.push_back((Command(true, url.toStdString(),
                                     conn_options.toStdString(),
                                     qmf_options.toStdString())));
    cond.wakeOne();
}

void QmfThread::disconnect()
{
    QMutexLocker locker(&lock);
    command_queue.push_back(Command(false, "", "", ""));
    cond.wakeOne();
}

void QmfThread::run()
{
    emit connectionStatusChanged("Closed");

    while(true) {
        if (connected) {
            qmf::ConsoleEvent event;
            qmf::Data data;
            uint32_t pcount;
            std::string s;
            qpid::types::Variant::Map args;

            if (sess.nextEvent(event, qpid::messaging::Duration::SECOND * 3)) {
                //
                // Process the event
                //
                qmf::Agent agent = event.getAgent();

                switch (event.getType()) {
                case qmf::CONSOLE_AGENT_ADD :
                    if (agent.getName() == sess.getConnectedBrokerAgent().getName()) {
                        // we just got the broker agent
                        // get the broker object so we can make calls
                        event = agent.query(qmf::Query(qmf::QUERY_OBJECT, "broker", "org.apache.qpid.broker"));
                        pcount = event.getDataCount();
                        if (pcount == 1) {
                            brokerData = event.getData(0);
                            emit isConnected(true);
                        }
                    }
                    break;

                case qmf::CONSOLE_AGENT_DEL :
                    //emit delAgent(agent);
                    break;

                case qmf::CONSOLE_QUERY_RESPONSE :
                    dispatchQueryResults(event);
                    break;

                case qmf::CONSOLE_METHOD_RESPONSE :
                   break;
                case qmf::CONSOLE_EXCEPTION :
                   if (event.getDataCount() > 0) {
                       data = event.getData(0);

                       s = data.getProperty("error_text").asString();
                       emit qmfError(QString(s.c_str()));
                   }
                   break;

                default :
                    break;
                }

            }
            else {
                if (connected)
                    emit qmfTimer();
            }

            {
                QMutexLocker locker(&lock);
                if (command_queue.size() > 0) {
                    Command command(command_queue.front());
                    disconnecting = true;

                    // make sure there are no pending queries before disconnecting
                    if (query_queue.size() == 0) {
                        command_queue.pop_front();
                        if (!command.connect) {
                            emit connectionStatusChanged("QMF Session Closing...");
                            sess.close();
                            emit connectionStatusChanged("Closing...");
                            conn.close();
                            emit connectionStatusChanged("Closed");
                            connected = false;
                            emit isConnected(false);
                        }
                    }
                }
            }
        } else {
            QMutexLocker locker(&lock);
            if (command_queue.size() == 0) {
                cond.wait(&lock, 1000);
            }
            if (command_queue.size() > 0) {
                Command command(command_queue.front());
                command_queue.pop_front();
                if (command.connect & !connected)
                    try {
                        emit connectionStatusChanged("QMF connection opening...");

                        conn = qpid::messaging::Connection(command.url, command.conn_options);
                        conn.open();

                        emit connectionStatusChanged("QMF session opening...");
                        sess = qmf::ConsoleSession(conn, command.qmf_options);
                        sess.open();
                        try {
                            sess.setAgentFilter("[eq, _product, [quote, 'qpidd']]");
                        } catch (std::exception&) {}
                        connected = true;
                        disconnecting = false;
                        //emit isConnected(true);

                        std::stringstream line;
                        line << "Operational (URL: " << command.url << ")";
                        emit connectionStatusChanged(line.str().c_str());
                    } catch(qpid::messaging::MessagingException& ex) {
                        std::stringstream line;
                        line << "QMF Session Failed: " << ex.what();
                        emit connectionStatusChanged(line.str().c_str());
                    }
            }
        }

        if (cancelled) {
            if (connected) {
                sess.close();
                conn.close();
            }
            break;
        }
    }
}

// Send a query
// Remember the correlator for the call and associate it
// with the args used to make the call and an object that
// will be notified when the call completes.
void QmfThread::queryBroker(const std::string& qmf_class,
                            QObject* object,
                            QEvent::Type event_type)
{
    // don't try to send a query if we are connecting or disconnecting
    if ((command_queue.size() > 0) || (!connected) || (disconnecting))
        return;

    QMutexLocker locker(&lock);

    query_queue.push_back(Query(object, event_type));
    qmf::Agent agent = sess.getConnectedBrokerAgent();
    query_queue.back().correlator = agent.queryAsync(
                qmf::Query(qmf::QUERY_OBJECT, qmf_class, "org.apache.qpid.broker"));

    cond.wakeOne();
}

// Called when a qmf::CONSOLE_METHOD_RESPONSE type event comes in.
// Find the event correlator and send a custom event to the
// ossociated QOBJECT
void QmfThread::dispatchQueryResults(qmf::ConsoleEvent& event)
{
    QMutexLocker locker(&lock);

    uint32_t correlator = event.getCorrelator();

    for (query_queue_t::iterator iter=query_queue.begin();
                            iter != query_queue.end(); iter++) {
        const Query& qq(*iter);
        if (qq.correlator == correlator) {
            emit receivedResponse(qq.object, event);

            // this was causing intermittent problems
            //QmfEvent qmfe(qq.t, event);
            //QApplication::sendEvent(qq.object, &qmfe);
            if (event.isFinal())
                query_queue.erase(iter);
            break;
        }
    }
    cond.wakeOne();
}
