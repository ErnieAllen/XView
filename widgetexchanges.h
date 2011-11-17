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

#ifndef WIDGETEXCHANGES_H
#define WIDGETEXCHANGES_H

#include "widgetqmfobject.h"

class WidgetExchanges : public WidgetQmfObject
{
    Q_OBJECT

public:
    explicit WidgetExchanges(QWidget *parent = 0);
    ~WidgetExchanges();


protected:
     void paintEvent(QPaintEvent *event);
     virtual QString unique_property(bool useKey=false); // allow derived classes to override object name
     void showRelated(const qmf::Data& object, const QString& widget_name, ArrowDirection a);

};

#endif // WIDGETEXCHANGES_H
