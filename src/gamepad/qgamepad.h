// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGAMEPAD_H
#define QGAMEPAD_H

#include <QtCore/QObject>
#include <QtGamepad/qtgamepadexports.h>
#include <QtUniversalInput/quniversalinput.h>

QT_BEGIN_NAMESPACE

class QGamepadPrivate;

class Q_GAMEPAD_EXPORT QGamepad : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(float axisLeftX READ axisLeftX NOTIFY axisLeftXChanged)
    Q_PROPERTY(float axisLeftY READ axisLeftY NOTIFY axisLeftYChanged)
    Q_PROPERTY(float axisRightX READ axisRightX NOTIFY axisRightXChanged)
    Q_PROPERTY(float axisRightY READ axisRightY NOTIFY axisRightYChanged)
    Q_PROPERTY(bool buttonA READ buttonA NOTIFY buttonAChanged)
    Q_PROPERTY(bool buttonB READ buttonB NOTIFY buttonBChanged)
    Q_PROPERTY(bool buttonX READ buttonX NOTIFY buttonXChanged)
    Q_PROPERTY(bool buttonY READ buttonY NOTIFY buttonYChanged)
    Q_PROPERTY(bool buttonL1 READ buttonL1 NOTIFY buttonL1Changed)
    Q_PROPERTY(bool buttonR1 READ buttonR1 NOTIFY buttonR1Changed)
    Q_PROPERTY(float buttonL2 READ buttonL2 NOTIFY buttonL2Changed)
    Q_PROPERTY(float buttonR2 READ buttonR2 NOTIFY buttonR2Changed)
    Q_PROPERTY(bool buttonSelect READ buttonSelect NOTIFY buttonSelectChanged)
    Q_PROPERTY(bool buttonStart READ buttonStart NOTIFY buttonStartChanged)
    Q_PROPERTY(bool buttonL3 READ buttonL3 NOTIFY buttonL3Changed)
    Q_PROPERTY(bool buttonR3 READ buttonR3 NOTIFY buttonR3Changed)
    Q_PROPERTY(bool buttonUp READ buttonUp NOTIFY buttonUpChanged)
    Q_PROPERTY(bool buttonDown READ buttonDown NOTIFY buttonDownChanged)
    Q_PROPERTY(bool buttonLeft READ buttonLeft NOTIFY buttonLeftChanged)
    Q_PROPERTY(bool buttonRight READ buttonRight NOTIFY buttonRightChanged)
    Q_PROPERTY(bool buttonGuide READ buttonGuide NOTIFY buttonGuideChanged)
public:
    explicit QGamepad(int deviceId = 0, QObject *parent = nullptr);
    ~QGamepad();

    int deviceId() const;
    void setDeviceId(int number);

    bool isConnected() const;

    QString name() const;

    float axisLeftX() const;
    float axisLeftY() const;
    float axisRightX() const;
    float axisRightY() const;
    bool buttonA() const;
    bool buttonB() const;
    bool buttonX() const;
    bool buttonY() const;
    bool buttonL1() const;
    bool buttonR1() const;
    float buttonL2() const;
    float buttonR2() const;
    bool buttonSelect() const;
    bool buttonStart() const;
    bool buttonL3() const;
    bool buttonR3() const;
    bool buttonUp() const;
    bool buttonDown() const;
    bool buttonLeft() const;
    bool buttonRight() const;
    bool buttonGuide() const;

Q_SIGNALS:

    void deviceIdChanged();
    void connectedChanged();
    void nameChanged();
    void axisLeftXChanged();
    void axisLeftYChanged();
    void axisRightXChanged();
    void axisRightYChanged();
    void buttonAChanged();
    void buttonBChanged();
    void buttonXChanged();
    void buttonYChanged();
    void buttonL1Changed();
    void buttonR1Changed();
    void buttonL2Changed();
    void buttonR2Changed();
    void buttonSelectChanged();
    void buttonStartChanged();
    void buttonL3Changed();
    void buttonR3Changed();
    void buttonUpChanged();
    void buttonDownChanged();
    void buttonLeftChanged();
    void buttonRightChanged();
    void buttonGuideChanged();

private:
    Q_DECLARE_PRIVATE(QGamepad)
    Q_DISABLE_COPY(QGamepad)
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QGamepad*)

#endif // QGAMEPAD_H
