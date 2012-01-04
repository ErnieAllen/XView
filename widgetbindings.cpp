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

#include "widgetbindings.h"
#include <QPainter>

WidgetBindings::WidgetBindings(QWidget *parent) :
    WidgetQmfObject(parent)
{
    this->setSectionName(QString("Bindings"));
    summaryColumns.append(Column("msgMatched", "matched", Qt::AlignRight, "N", modeMessages, true));
    summaryColumns.append(Column("msgMatched", "matched / sec", Qt::AlignRight, "N", modeMessageRate, true));

    setRelatedText("Related bindings");
}
