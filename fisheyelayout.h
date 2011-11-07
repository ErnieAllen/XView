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

#ifndef FISHEYELAYOUT_H
#define FISHEYELAYOUT_H

#include <QtGui>

class FisheyeLayout : public QLayout
{
    Q_OBJECT
public:
    FisheyeLayout(): QLayout() {}
    FisheyeLayout(QWidget *parent, bool tile=false);
    ~FisheyeLayout();

    void addItem(QLayoutItem *item);
    QSize sizeHint() const;
    QSize minimumSize() const;
    int count() const;
    QLayoutItem *itemAt(int) const;
    QLayoutItem *takeAt(int);
    void setGeometry(const QRect &rect);

public slots:
    void setCascade(bool cascade);

protected:
    int getFocusedItem();
    void setTiledGeometry(const QRect &r);

private:
    QList<QLayoutItem*> list;
    bool tiled;

};

#endif // FISHEYELAYOUT_H
