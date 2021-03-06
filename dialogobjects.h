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

#ifndef DIALOGOBJECTS_H
#define DIALOGOBJECTS_H

#include <QDialog>
#include <QSettings>
#include <QSortFilterProxyModel>
#include "object-model.h"
#include "object-details.h"
#include <qmf/ConsoleEvent.h>

namespace Ui {
    class DialogObjects;
}

class DialogObjects : public QDialog
{
    Q_OBJECT

public:
    explicit DialogObjects(QWidget *parent = 0, const std::string &s = "");
    ~DialogObjects();

    void initModels(std::string unique, const QStringList &columnList);
    void setKey(const QString &altKey);
    ObjectListModel *listModel() { return objectModel; }

    void gotDataEvent(const qmf::ConsoleEvent& event, bool all);
public slots:
    void connectionChanged(bool isConnected);
    void accept();
    void dataRefreshed();
    void selected(const QModelIndex &index);
    void setCurrentRow(const QModelIndex& row);

protected:
    void initConnections();

    ObjectListModel *objectModel;
    ObjectDetailsModel *objectDetailsModel;

signals:
    // user selected an object
    void setCurrentObject(const qmf::Data&, const QString &);
    // automatic data refresh
    void objectRefreshed();
    void finalAdded();

private:
    Ui::DialogObjects *ui;
    void saveSettings();
    void restoreSettings();
    QSortFilterProxyModel *proxyModel;

private slots:
    void resizeDetail();
};

#endif // DialogObjects_H
