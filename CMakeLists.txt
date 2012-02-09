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

PROJECT(qpid-xbroker)
FIND_PACKAGE(Qt4 REQUIRED)
FIND_LIBRARY(qmf2 qmf2)

SET(xview_FORMS
    xview.ui
    chart.ui
    dialogopen.ui
    dialogabout.ui
    dialogobjects.ui
    widgetqmfobject.ui
    )

SET(xview_HEADERS
    chart.h
    commandlinkbutton.h
    dialogabout.h
    dialogexchanges.h
    dialogobjects.h
    dialogopen.h
    exchange-details.h
    exchange-model.h
    fisheyelayout.h
    object-details.h
    object-model.h
    propertydelegate.h
    qmf-thread.h
    related-model.h
    relatedheaderview.h
    sample.h
    widgetbindings.h
    widgetconnections.h
    widgetexchanges.h
    widgetqmfobject.h
    widgetqueues.h
    widgetsessions.h
    widgetsubscriptions.h
    xview.h
    )

SET(xview_SOURCES
    chart.cpp
    commandlinkbutton.cpp
    dialogabout.cpp
    dialogexchanges.cpp
    dialogobjects.cpp
    dialogopen.cpp
    exchange-details.cpp
    exchange-model.cpp
    fisheyelayout.cpp
    main.cpp
    object-details.cpp
    object-model.cpp
    propertydelegate.cpp
    qmf-thread.cpp
    related-model.cpp
    relatedheaderview.cpp
    sample.cpp
    widgetbindings.cpp
    widgetconnections.cpp
    widgetexchanges.cpp
    widgetqmfobject.cpp
    widgetqueues.cpp
    widgetsessions.cpp
    widgetsubscriptions.cpp
    xview.cpp
    )

SET(xview_RESOURCES xview.qrc)

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

QT4_WRAP_UI(xview_FORMS_HEADERS ${xview_FORMS})
QT4_WRAP_CPP(xview_HEADERS_MOC ${xview_HEADERS})
QT4_ADD_RESOURCES(xview_RESOURCES_RCC ${xview_RESOURCES})

ADD_EXECUTABLE(qpid-xbroker ${xview_SOURCES} ${xview_HEADERS_MOC} ${xview_FORMS_HEADERS} ${xview_RESOURCES_RCC})
TARGET_LINK_LIBRARIES(qpid-xbroker ${QT_LIBRARIES} qmf2 qpidmessaging qpidtypes)

INSTALL (TARGETS qpid-xbroker RUNTIME DESTINATION bin)
