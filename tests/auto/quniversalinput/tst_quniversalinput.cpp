// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtUniversalInput/QUniversalInput>

QT_USE_NAMESPACE

using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;
using HatMask = QUniversalInput::HatMask;
using HatFlag = QUniversalInput::HatFlag;

// The input singleton keeps state across tests, so cleanup() disconnects the
// devices used here. High indices avoid colliding with a real controller.
class tst_QUniversalInput : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup();

    void connectionLifecycle();
    void unconnectedDeviceHasEmptyState();
    void unusedJoyIdAllocatesAndReuses();

    void buttonEventDeduplicates();
    void axisEventDeduplicates();
    void outOfRangeIndicesAreIgnored();

    void hatDecomposesToDpadButtons();

    void forceFeedbackStoresState();

    void mouseDisabledTogglesOnce();
    void mouseMoveEmitsDeltas();

    void mappingMatchedByGuid();
    void mappedButtonIsRemapped();
};

void tst_QUniversalInput::initTestCase()
{
    // The database is loaded from a queued call; flush it so mappings resolve.
    QCoreApplication::sendPostedEvents(QUniversalInput::instance(), QEvent::MetaCall);
}

void tst_QUniversalInput::cleanup()
{
    auto *input = QUniversalInput::instance();
    for (int i = 0; i < 16; ++i)
        input->updateJoyConnection(i, false, QString(), QString());
}

void tst_QUniversalInput::connectionLifecycle()
{
    auto *input = QUniversalInput::instance();
    constexpr int device = 10;

    QSignalSpy connectionSpy(input, &QUniversalInput::joyConnectionChanged);

    input->updateJoyConnection(device, true, QStringLiteral("Generic Pad"), QString());
    QCOMPARE(connectionSpy.count(), 1);
    QCOMPARE(connectionSpy.at(0).at(0).toInt(), device);
    QCOMPARE(connectionSpy.at(0).at(1).toBool(), true);
    QVERIFY(input->isJoyConnected(device));
    QCOMPARE(input->joyName(device), QStringLiteral("Generic Pad"));
    // An empty GUID never matches the database, so the device is not a gamepad.
    QVERIFY(!input->isGamepad(device));

    input->updateJoyConnection(device, false, QString(), QString());
    QCOMPARE(connectionSpy.count(), 2);
    QCOMPARE(connectionSpy.at(1).at(1).toBool(), false);
    QVERIFY(!input->isJoyConnected(device));
    QVERIFY(input->joyName(device).isEmpty());
}

void tst_QUniversalInput::unconnectedDeviceHasEmptyState()
{
    auto *input = QUniversalInput::instance();
    constexpr int device = 15; // never connected by these tests

    QVERIFY(!input->isJoyConnected(device));
    QVERIFY(!input->isGamepad(device));
    QVERIFY(input->joyName(device).isEmpty());
}

void tst_QUniversalInput::unusedJoyIdAllocatesAndReuses()
{
    auto *input = QUniversalInput::instance();

    // Work relative to the lowest free index, so that a real device connected
    // by a backend does not affect the test.
    const int first = input->unusedJoyId();
    input->updateJoyConnection(first, true, QStringLiteral("P0"), QString());

    const int second = input->unusedJoyId();
    QVERIFY(second > first);
    input->updateJoyConnection(second, true, QStringLiteral("P1"), QString());

    input->updateJoyConnection(first, false, QString(), QString());
    QCOMPARE(input->unusedJoyId(), first);
}

void tst_QUniversalInput::buttonEventDeduplicates()
{
    auto *input = QUniversalInput::instance();
    constexpr int device = 11;
    input->updateJoyConnection(device, true, QStringLiteral("Pad"), QString());

    QSignalSpy spy(input, &QUniversalInput::joyButtonEvent);

    input->joyButton(device, JoyButton::A, true);
    input->joyButton(device, JoyButton::A, true);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), device);
    QCOMPARE(spy.at(0).at(1).value<JoyButton>(), JoyButton::A);
    QCOMPARE(spy.at(0).at(2).toBool(), true);

    input->joyButton(device, JoyButton::A, false);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(2).toBool(), false);
}

void tst_QUniversalInput::axisEventDeduplicates()
{
    auto *input = QUniversalInput::instance();
    constexpr int device = 11;
    input->updateJoyConnection(device, true, QStringLiteral("Pad"), QString());

    QSignalSpy spy(input, &QUniversalInput::joyAxisEvent);

    input->joyAxis(device, JoyAxis::LeftX, 0.5f);
    input->joyAxis(device, JoyAxis::LeftX, 0.5f);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<JoyAxis>(), JoyAxis::LeftX);
    QCOMPARE(spy.at(0).at(2).toFloat(), 0.5f);

    input->joyAxis(device, JoyAxis::LeftX, -0.25f);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(2).toFloat(), -0.25f);
}

void tst_QUniversalInput::outOfRangeIndicesAreIgnored()
{
    auto *input = QUniversalInput::instance();
    constexpr int device = 11;
    input->updateJoyConnection(device, true, QStringLiteral("Pad"), QString());

    QSignalSpy buttonSpy(input, &QUniversalInput::joyButtonEvent);
    QSignalSpy axisSpy(input, &QUniversalInput::joyAxisEvent);

    input->joyButton(device, JoyButton(QUniversalInput::JoyButtonsMax), true);
    input->joyButton(device, JoyButton(-5), true);
    input->joyAxis(device, JoyAxis(QUniversalInput::JoyAxesMax), 0.5f);
    input->joyAxis(device, JoyAxis(-2), 0.5f);

    QCOMPARE(buttonSpy.count(), 0);
    QCOMPARE(axisSpy.count(), 0);
}

// Only the directions whose bit changed since the previous hat state emit.
void tst_QUniversalInput::hatDecomposesToDpadButtons()
{
    auto *input = QUniversalInput::instance();
    constexpr int device = 11;
    input->updateJoyConnection(device, true, QStringLiteral("Pad"), QString());

    QSignalSpy spy(input, &QUniversalInput::joyButtonEvent);

    input->joyHat(device, HatMask(HatFlag::Up));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<JoyButton>(), JoyButton::DpadUp);
    QCOMPARE(spy.at(0).at(2).toBool(), true);

    input->joyHat(device, HatMask(HatFlag::Up) | HatFlag::Right);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(1).value<JoyButton>(), JoyButton::DpadRight);
    QCOMPARE(spy.at(1).at(2).toBool(), true);

    input->joyHat(device, HatMask(HatFlag::Center));
    QCOMPARE(spy.count(), 4);
    QList<JoyButton> released = { spy.at(2).at(1).value<JoyButton>(),
                                  spy.at(3).at(1).value<JoyButton>() };
    QVERIFY(released.contains(JoyButton::DpadUp));
    QVERIFY(released.contains(JoyButton::DpadRight));
    QCOMPARE(spy.at(2).at(2).toBool(), false);
    QCOMPARE(spy.at(3).at(2).toBool(), false);
}

void tst_QUniversalInput::forceFeedbackStoresState()
{
    auto *input = QUniversalInput::instance();
    constexpr int device = 12;
    constexpr int untouched = 13;

    const quint64 before = QDateTime::currentMSecsSinceEpoch();
    input->addForce(device, QVector2D(0.3f, 0.7f), 1.5f);

    QCOMPARE(input->joyVibrationStrength(device), QVector2D(0.3f, 0.7f));
    QCOMPARE(input->joyVibrationDuration(device), 1.5f);
    QVERIFY(input->joyVibrationTimestamp(device) >= before);

    QCOMPARE(input->joyVibrationStrength(untouched), QVector2D(0.0f, 0.0f));
    QCOMPARE(input->joyVibrationDuration(untouched), 0.0f);
    QCOMPARE(input->joyVibrationTimestamp(untouched), quint64(0));
}

void tst_QUniversalInput::mouseDisabledTogglesOnce()
{
    auto *input = QUniversalInput::instance();
    input->setMouseDisabled(false); // known starting state

    QSignalSpy spy(input, &QUniversalInput::mouseDisabledChanged);

    input->setMouseDisabled(true);
    QCOMPARE(spy.count(), 1);
    QVERIFY(input->isMouseDisabled());

    input->setMouseDisabled(true);
    QCOMPARE(spy.count(), 1);

    input->setMouseDisabled(false);
    QCOMPARE(spy.count(), 2);
    QVERIFY(!input->isMouseDisabled());
}

void tst_QUniversalInput::mouseMoveEmitsDeltas()
{
    auto *input = QUniversalInput::instance();
    QSignalSpy spy(input, &QUniversalInput::mouseMovedWithDeltas);

    input->mouseMove(QVector2D(3.0f, -2.0f));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<QVector2D>(), QVector2D(3.0f, -2.0f));
}

void tst_QUniversalInput::mappingMatchedByGuid()
{
    auto *input = QUniversalInput::instance();
    constexpr int device = 14;
    // Xbox 360 Controller entry from the bundled SDL database.
    const QString guid = QStringLiteral("030000002e160000efbe000000000000");

    input->updateJoyConnection(device, true, QStringLiteral("Raw Name"), guid);

    QVERIFY2(input->isGamepad(device),
             "the bundled database should contain the pinned Xbox 360 GUID");
    QCOMPARE(input->joyName(device), QStringLiteral("Xbox 360 Controller"));
}

// The Xbox entry maps "back:b6", so physical button 6 is reported as Back.
void tst_QUniversalInput::mappedButtonIsRemapped()
{
    auto *input = QUniversalInput::instance();
    constexpr int device = 14;
    const QString guid = QStringLiteral("030000002e160000efbe000000000000");
    input->updateJoyConnection(device, true, QStringLiteral("Raw Name"), guid);
    QVERIFY(input->isGamepad(device));

    QSignalSpy spy(input, &QUniversalInput::joyButtonEvent);

    input->joyButton(device, JoyButton(6), true);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<JoyButton>(), JoyButton::Back);
    QCOMPARE(spy.at(0).at(2).toBool(), true);
}

QTEST_MAIN(tst_QUniversalInput)

#include "tst_quniversalinput.moc"
