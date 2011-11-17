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

#include "fisheyelayout.h"
#include "widgetqmfobject.h"

FisheyeLayout::FisheyeLayout(QWidget *parent, bool tile) :
        QLayout(parent), tiled(tile)
{
    setMargin(0);
    setSpacing(0);
}
void FisheyeLayout::setCascade(bool cascade)
{
    tiled = !cascade;
}

int FisheyeLayout::count() const
{
        // QList::size() returns the number of QLayoutItems in the list
    return list.size();
}

QLayoutItem *FisheyeLayout::itemAt(int idx) const
{
    // QList::value() performs index checking, and returns 0 if we are
    // outside the valid range
    return list.value(idx);
}

QLayoutItem *FisheyeLayout::takeAt(int idx)
{
    // QList::take does not do index checking
    return idx >= 0 && idx < list.size() ? list.takeAt(idx) : 0;
}

void FisheyeLayout::addItem(QLayoutItem *item)
{
    list.append(item);
}

QSize FisheyeLayout::sizeHint() const
{
    QSize s(0,0);
    int n = list.count();
    if (n > 0)
        s = QSize(100,70); //start with a nice default size
    int i = 0;
    while (i < n) {
        QLayoutItem *o = list.at(i);
        s = s.expandedTo(o->sizeHint());
        ++i;
    }
    return s + n*QSize(spacing(), spacing());
}

QSize FisheyeLayout::minimumSize() const
{
    QSize s(0,0);
    int n = list.count();
    int i = 0;
    while (i < n) {
        QLayoutItem *o = list.at(i);
        s = s.expandedTo(o->minimumSize());
        ++i;
    }
    return s + n*QSize(spacing(), spacing());
}

void FisheyeLayout::setTiledGeometry(const QRect &r)
{
    if (list.size() == 0)
        return;

    setToolTips(0, false);

    int w = r.width() / list.size();
    int i = 0;
    while (i < list.size()) {
        QRect geom(w * i, 0, w, r.height());

        startGeometryAnimation(list.at(i), geom);
        //list.at(i)->setGeometry(geom);
        ++i;
    }
}

void FisheyeLayout::setGeometry(const QRect &r)
{
    QLayout::setGeometry(r);

    if (list.size() == 0)
        return;

    if (tiled)
        return setTiledGeometry(r);

    int focusedItem = getFocusedItem();
    if (focusedItem < 0)
        focusedItem = getCurrentItem();
    if (focusedItem == -1)
        return;
    setToolTips(focusedItem, true);

    //
    // Setup the corrent z-order
    //
    // stack the siblings to the left of the focus widget
    int i = focusedItem - 1;
    while (i >= 0) {
        list.at(i)->widget()->lower();
        --i;
    }
    // stack the siblings to the right of the focus widget
    i = focusedItem + 1;
    while (i < list.size()) {
        list.at(i)->widget()->lower();
        ++i;
    }
    list.at(focusedItem)->widget()->raise();

    if (list.size() > 1) {
        QRect geom = QRect();
        int xGap = (r.width() * 0.1) / (list.size() - 1);
        int yGap = (r.height() * 0.1) / (list.size() - 1);
        int diff;
        i = 0;
        while (i < list.size()) {
            QLayoutItem *o = list.at(i);

            diff = qAbs(i - focusedItem);
            geom.setLeft(i * xGap);
            geom.setTop(diff * yGap);
            geom.setWidth(r.width() * 0.9);
            geom.setHeight(r.height() - diff * yGap * 2);

            startGeometryAnimation(o, geom);
            //o->setGeometry(geom);
            ++i;
        }
    } else {
        list.at(0)->setGeometry(QRect(0, 0, r.width(), r.height()));
    }
}

void FisheyeLayout::startGeometryAnimation(QLayoutItem *o, const QRect& geom)
{
    QPropertyAnimation *animation;
    if (!animationHash.contains(o)) {
        animation = new QPropertyAnimation(o->widget(), "geometry");
        animationHash[o] = animation;
    } else {
        animation = animationHash[o];
    }
    animation->setDuration(500);
    animation->setStartValue(o->widget()->geometry());
    animation->setEndValue(geom);
    animation->setEasingCurve(QEasingCurve::OutBack);

    animation->start();
}


int FisheyeLayout::getFocusedItem()
{
    int i = 0;
    while (i < list.size()) {
        QWidget *w = list.at(i)->widget();
        if (w->hasFocus())
            return i;
        ++i;
    }
    return -1;
}

int FisheyeLayout::getCurrentItem()
{
    int i = 0;
    while (i < list.size()) {
        WidgetQmfObject *w = (WidgetQmfObject *)(list.at(i)->widget());
        if (w->current())
            return i;
        ++i;
    }
    return -1;
}

void FisheyeLayout::setToolTips(int current, bool set)
{
    int i = 0;
    while (i < list.size()) {
        WidgetQmfObject *w = (WidgetQmfObject *)(list.at(i)->widget());
        if (set) {
            if (i == current)
                w->setToolTip("");
            else
                w->setToolTip(w->sectionName());
        } else {
            w->setToolTip("");
        }
        ++i;
    }
}

FisheyeLayout::~FisheyeLayout()
{
     QLayoutItem *item;
     while ((item = takeAt(0))) {
         if (animationHash.contains(item))
             delete animationHash[item];
         delete item;
    }
}
