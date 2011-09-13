#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements. See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership. The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the License for the
# specific language governing permissions and limitations
# under the License.
#

cmake_minimum_required(VERSION 2.6)

SET(CMAKE_CXX_FLAGS "-g")

PROJECT(xview)
FIND_PACKAGE(Qt4 REQUIRED)

SET(xview_FORMS
    xview.ui
    dialogopen.ui
    dialogabout.ui
    dialogobjects.ui
    widgetqmfobject.ui
    )

SET(xview_HEADERS
    xview.h
    qmf-thread.h
    exchange-model.h
    exchange-details.h
    dialogopen.h
    dialogabout.h
    widgetbindings.h
    widgetexchanges.h
    widgetqueues.h
    widgetsubscriptions.h
    dialogexchanges.h
    qmf-event.h
    object-details.h
    object-model.h
    dialogobjects.h
    widgetqmfobject.h
    related-model.h
    )

SET(xview_SOURCES
    main.cpp
    xview.cpp
    qmf-thread.cpp
    exchange-model.cpp
    exchange-details.cpp
    dialogopen.cpp
    dialogabout.cpp
    widgetbindings.cpp
    widgetexchanges.cpp
    widgetqueues.cpp
    widgetsubscriptions.cpp
    dialogexchanges.cpp
    qmf-event.cpp
    object-details.cpp
    object-model.cpp
    dialogobjects.cpp
    widgetqmfobject.cpp
    related-model.cpp
    )

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

QT4_WRAP_UI(xview_FORMS_HEADERS ${xview_FORMS})
QT4_WRAP_CPP(xview_HEADERS_MOC ${xview_HEADERS})

ADD_EXECUTABLE(xview ${xview_SOURCES} ${xview_HEADERS_MOC} ${xview_FORMS_HEADERS})
TARGET_LINK_LIBRARIES(xview ${QT_LIBRARIES} qmf2 qpidmessaging qpidtypes)
