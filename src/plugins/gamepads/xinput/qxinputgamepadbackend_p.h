/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Gamepad module
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/
#ifndef QXINPUTGAMEPADBACKEND_P_H
#define QXINPUTGAMEPADBACKEND_P_H

#include <QtGamepad/QGamepadManager>
#include <QtGamepad/private/qgamepadbackend_p.h>
#include <QtCore/QLibrary>

QT_BEGIN_NAMESPACE

class QXInputThread;

class QXInputGamepadBackend : public QGamepadBackend
{
    Q_OBJECT

public:
    QXInputGamepadBackend();
    bool start() override;
    void stop() override;

private:
    QXInputThread *m_thread;
    QLibrary m_lib;
};

QT_END_NAMESPACE

#endif // QXINPUTGAMEPADBACKEND_P_H
