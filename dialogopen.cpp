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

#include "dialogopen.h"
#include "ui_dialogopen.h"
#include <QSettings>

DialogOpen::DialogOpen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOpen)
{
    ui->setupUi(this);
    restoreSettings();
}

DialogOpen::~DialogOpen()
{
    saveSettings();
    delete ui;
}

void DialogOpen::accept()
{
    emit dialogOpenAccepted(ui->lineEdit_url->text(),
                            ui->lineEdit_connect->text(),
                            ui->lineEdit_qmf->text());

    hide();;
}

void DialogOpen::saveSettings() {
    QSettings settings;

    settings.beginGroup("OpenConnection");
    settings.setValue("url",     ui->lineEdit_url->text());
    settings.setValue("connect", ui->lineEdit_connect->text());
    settings.setValue("qmf",     ui->lineEdit_qmf->text());
    settings.endGroup();

}

void DialogOpen::restoreSettings() {
    QSettings settings;

    settings.beginGroup("OpenConnection");
    ui->lineEdit_url->setText(QString(settings.value("url").toString()));
    ui->lineEdit_connect->setText(QString(settings.value("connect").toString()));
    ui->lineEdit_qmf->setText(QString(settings.value("qmf", "{strict-security:False}").toString()));
    settings.endGroup();
}
