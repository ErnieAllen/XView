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

using std::cout;
using std::endl;

QmfThread::QmfThread(QObject* parent) :
    QThread(parent), cancelled(false), connected(false)
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

            if (sess.nextEvent(event, qpid::messaging::Duration::SECOND * 2)) {
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
                        if (pcount == 1)
                            brokerData = event.getData(0);

                        // get the exchanges for this broker
                        agent.queryAsync(qmf::Query(qmf::QUERY_OBJECT, "exchange", "org.apache.qpid.broker"));
                    }
                    break;

                case qmf::CONSOLE_AGENT_DEL :
                    //emit delAgent(agent);
                    break;

                case qmf::CONSOLE_QUERY_RESPONSE :
                    // Handle the query response from the QUERY_OBJECT for exchanges
                    // Currently, this is the only async query called
                    pcount = event.getDataCount();
                    for (uint32_t idx = 0; idx < pcount; idx++) {
                        emit addExchange(event.getData(idx), event.getCorrelator());
                    }
                    if (event.isFinal())
                        emit doneAddingExchanges(event.getCorrelator());
                    break;

                case qmf::CONSOLE_METHOD_RESPONSE :
                    callCallback(event);
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

            {
                QMutexLocker locker(&lock);
                if (command_queue.size() > 0) {
                    Command command(command_queue.front());
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
        } else {
            QMutexLocker locker(&lock);
            if (command_queue.size() == 0)
                cond.wait(&lock, 1000);
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
                        emit isConnected(true);

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

// Call an async method
// Remember the correlator for the call and associate it
// with the args used to make the call and a signal that
// will be emitted when the call completes.
void QmfThread::addCallback(qmf::Agent agent, const std::string& method,
                            const qpid::types::Variant::Map& args,
                            const qmf::DataAddr& dataAddr,
                            const std::string &signal,
                            const QModelIndex& index)
{
    QMutexLocker locker(&lock);

    callback_queue.push_back(Callback(signal, args, index));
    callback_queue.back().correlator = agent.callMethodAsync(method, args, dataAddr);

    cond.wakeOne();
}

// Called when a qmf::CONSOLE_METHOD_RESPONSE type event comes in.
// Find the event correlator and call the associated function
void QmfThread::callCallback(const qmf::ConsoleEvent& event)
{
    QMutexLocker locker(&lock);

    uint32_t correlator = event.getCorrelator();

    for (callback_queue_t::iterator iter=callback_queue.begin();
                            iter != callback_queue.end(); iter++) {
        const Callback& cb(*iter);
        if (cb.correlator == correlator) {
            emitCallback(cb, event);
            callback_queue.erase(iter);
            break;
        }
    }
    cond.wakeOne();
}

// Resolve the association between the method string stored in the callback_queue
// and a signal function.
void QmfThread::emitCallback(const Callback& cb, const qmf::ConsoleEvent& event)
{
    if (cb.method == SIGNAL(gotMessageHeaders())) {
        emit gotMessageHeaders(event, cb.args);
    } else if (cb.method == SIGNAL(gotMessageBody())) {
        emit gotMessageBody(event, cb.args, cb.index);
    } else if (cb.method == SIGNAL(removedMessage())) {
        emit removedMessage(event, cb.args);
    }
}

void QmfThread::getQueueHeaders(const QString& name)
{
    qpid::types::Variant::Map map;
    map["name"] = name.toStdString();
    // get the list of message header ids
    qmf::Agent agent = brokerData.getAgent();
    qmf::ConsoleEvent event = agent.callMethod("queueGetIdList", map, brokerData.getAddr());

    if (event.getType() == qmf::CONSOLE_METHOD_RESPONSE) {

        uint messageId = 0;
        qpid::types::Variant::Map callMap;

        callMap["name"] = name.toStdString();

        // get the list of ids
        const qpid::types::Variant::Map& args(event.getArguments());
        qpid::types::Variant::Map::const_iterator iter = args.begin();
        if (iter != args.end()) {
           qpid::types::Variant::List sublist = (iter->second).asList();
           // for each header id, get the message header
           for (qpid::types::Variant::List::const_iterator subIter = sublist.begin();
                 subIter != sublist.end(); subIter++) {
                messageId = *subIter;

                callMap["id"] = messageId;
                // submit an asyncronous call to get the header
                // and request that the gotMessageHeaders signal be emitted when ready
                addCallback(agent, "queueGetMessageHeader", callMap, brokerData.getAddr(),
                            SIGNAL(gotMessageHeaders()));
            }
        }
    }
}

void QmfThread::queueRemoveMessage(const QString& name, const qpid::types::Variant::Map& args)
{
    Q_UNUSED(name);
    qmf::Agent agent = brokerData.getAgent();

    // submit an asyncronous call to remove the message
    // and request that the removedMessage signal be emitted when ready
    addCallback(agent, "queueRemoveMessage", args, brokerData.getAddr(),
                SIGNAL(removedMessage()));
}

// SLOT: Show the current message body
void QmfThread::showBody(const QModelIndex& index, const qmf::ConsoleEvent &event, const qpid::types::Variant::Map &args)
{
    qmf::Agent agent = brokerData.getAgent();

    // get the expected body content type
    const qpid::types::Variant::Map& headerMap(event.getArguments());
    qpid::types::Variant::Map::const_iterator iter = headerMap.begin();
    const qpid::types::Variant::Map& headerAttributes(iter->second.asMap());
    iter = headerAttributes.find("ContentType");

    std::string contentType;

    if (iter != headerAttributes.end())
        contentType = iter->second.asString();

    qpid::types::Variant::Map map(args);
    // remember the content type so we can decode the response properly
    map["ContentType"] = contentType;
    // make the call
    addCallback(agent, "queueGetMessageBody", map, brokerData.getAddr(),
                SIGNAL(gotMessageBody()), index);
}

qmf::ConsoleEvent QmfThread::fetchBody(const qpid::types::Variant::Map& args)
{
    qmf::Agent agent = brokerData.getAgent();
    return agent.callMethod("queueGetMessageBody", args, brokerData.getAddr());
}

void QmfThread::pauseRefreshes(bool checked)
{
    pausedRefreshes = checked;
}
