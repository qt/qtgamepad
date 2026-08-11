// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGamepad/QGamepad>
#include <QtUniversalInput/QUniversalInput>

QT_USE_NAMESPACE

using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;

// Input is injected through the singleton on an unmapped device. The test
// devices are reconnected before each function to reset their state.
class tst_QGamepad : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void buttonProjection_data();
    void buttonProjection();
    void axisProjection_data();
    void axisProjection();

    void ignoresOtherDevices();
    void deviceSwitchingFollowsNewDevice();
    void connectionAndNameSync();
};

void tst_QGamepad::init()
{
    auto *input = QUniversalInput::instance();
    input->updateJoyConnection(0, true, QStringLiteral("Pad 0"), QString());
    input->updateJoyConnection(1, true, QStringLiteral("Pad 1"), QString());
}

void tst_QGamepad::cleanup()
{
    auto *input = QUniversalInput::instance();
    for (int i = 0; i < 6; ++i)
        input->updateJoyConnection(i, false, QString(), QString());
}

void tst_QGamepad::buttonProjection_data()
{
    QTest::addColumn<int>("button");
    QTest::addColumn<QByteArray>("property");
    QTest::addColumn<QByteArray>("signal");

    auto row = [](const char *name, JoyButton button, const char *prop, const char *sig) {
        QTest::newRow(name) << int(button) << QByteArray(prop) << QByteArray(sig);
    };

    row("A", JoyButton::A, "buttonA", "buttonAChanged()");
    row("B", JoyButton::B, "buttonB", "buttonBChanged()");
    row("X", JoyButton::X, "buttonX", "buttonXChanged()");
    row("Y", JoyButton::Y, "buttonY", "buttonYChanged()");
    row("L1", JoyButton::LeftShoulder, "buttonL1", "buttonL1Changed()");
    row("R1", JoyButton::RightShoulder, "buttonR1", "buttonR1Changed()");
    row("L3", JoyButton::LeftStick, "buttonL3", "buttonL3Changed()");
    row("R3", JoyButton::RightStick, "buttonR3", "buttonR3Changed()");
    row("Select", JoyButton::Back, "buttonSelect", "buttonSelectChanged()");
    row("Start", JoyButton::Start, "buttonStart", "buttonStartChanged()");
    row("Up", JoyButton::DpadUp, "buttonUp", "buttonUpChanged()");
    row("Down", JoyButton::DpadDown, "buttonDown", "buttonDownChanged()");
    row("Left", JoyButton::DpadLeft, "buttonLeft", "buttonLeftChanged()");
    row("Right", JoyButton::DpadRight, "buttonRight", "buttonRightChanged()");
    row("Guide", JoyButton::Guide, "buttonGuide", "buttonGuideChanged()");
}

void tst_QGamepad::buttonProjection()
{
    QFETCH(int, button);
    QFETCH(QByteArray, property);
    QFETCH(QByteArray, signal);

    QGamepad gamepad(0);
    QSignalSpy spy(&gamepad, (QByteArray("2") + signal).constData());
    QVERIFY(spy.isValid());

    QUniversalInput::instance()->joyButton(0, JoyButton(button), true);

    QCOMPARE(gamepad.property(property.constData()).toBool(), true);
    QCOMPARE(spy.count(), 1);
}

void tst_QGamepad::axisProjection_data()
{
    QTest::addColumn<int>("axis");
    QTest::addColumn<QByteArray>("property");
    QTest::addColumn<QByteArray>("signal");

    auto row = [](const char *name, JoyAxis axis, const char *prop, const char *sig) {
        QTest::newRow(name) << int(axis) << QByteArray(prop) << QByteArray(sig);
    };

    row("LeftX", JoyAxis::LeftX, "axisLeftX", "axisLeftXChanged()");
    row("LeftY", JoyAxis::LeftY, "axisLeftY", "axisLeftYChanged()");
    row("RightX", JoyAxis::RightX, "axisRightX", "axisRightXChanged()");
    row("RightY", JoyAxis::RightY, "axisRightY", "axisRightYChanged()");
    row("TriggerLeft", JoyAxis::TriggerLeft, "buttonL2", "buttonL2Changed()");
    row("TriggerRight", JoyAxis::TriggerRight, "buttonR2", "buttonR2Changed()");
}

void tst_QGamepad::axisProjection()
{
    QFETCH(int, axis);
    QFETCH(QByteArray, property);
    QFETCH(QByteArray, signal);

    QGamepad gamepad(0);
    QSignalSpy spy(&gamepad, (QByteArray("2") + signal).constData());
    QVERIFY(spy.isValid());

    QUniversalInput::instance()->joyAxis(0, JoyAxis(axis), 0.5f);

    QCOMPARE(gamepad.property(property.constData()).toFloat(), 0.5f);
    QCOMPARE(spy.count(), 1);
}

void tst_QGamepad::ignoresOtherDevices()
{
    QGamepad gamepad(0);
    QUniversalInput::instance()->joyButton(1, JoyButton::A, true);
    QCOMPARE(gamepad.buttonA(), false);
}

void tst_QGamepad::deviceSwitchingFollowsNewDevice()
{
    QGamepad gamepad(0);
    QSignalSpy deviceIdSpy(&gamepad, &QGamepad::deviceIdChanged);

    gamepad.setDeviceId(1);
    QCOMPARE(gamepad.deviceId(), 1);
    QCOMPARE(deviceIdSpy.count(), 1);

    QUniversalInput::instance()->joyButton(1, JoyButton::B, true);
    QCOMPARE(gamepad.buttonB(), true);

    QUniversalInput::instance()->joyButton(0, JoyButton::X, true);
    QCOMPARE(gamepad.buttonX(), false);
}

void tst_QGamepad::connectionAndNameSync()
{
    auto *input = QUniversalInput::instance();
    // A device that init() did not connect.
    input->updateJoyConnection(5, true, QStringLiteral("My Pad"), QString());

    QGamepad gamepad(5);
    QVERIFY(gamepad.isConnected());
    QCOMPARE(gamepad.name(), QStringLiteral("My Pad"));

    QSignalSpy connectedSpy(&gamepad, &QGamepad::connectedChanged);
    input->updateJoyConnection(5, false, QString(), QString());
    QCOMPARE(connectedSpy.count(), 1);
    QVERIFY(!gamepad.isConnected());
}

QTEST_MAIN(tst_QGamepad)

#include "tst_qgamepad.moc"
