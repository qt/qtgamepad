// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtUniversalInput/QUniversalInput>
#include <QtUniversalInput/private/qjoystickinputfactory_p.h>
#include <QtUniversalInput/private/qmouseinputfactory_p.h>

QT_USE_NAMESPACE

using JoyButton = QUniversalInput::JoyButton;

// The plugin factories and the fallback path must behave on a machine with no
// input backend, which is the normal state in CI.
class tst_Factory : public QObject
{
    Q_OBJECT
private slots:
    void factoriesReturnKeyLists();
    void unknownKeyYieldsNullBackend();
    void singletonUsableAfterInit();
};

// No specific keys are guaranteed, but every key must be a non-empty string.
void tst_Factory::factoriesReturnKeyLists()
{
    const QStringList joystickKeys = QJoystickInputFactory::keys();
    const QStringList mouseKeys = QMouseInputFactory::keys();

    for (const QString &key : joystickKeys)
        QVERIFY(!key.isEmpty());
    for (const QString &key : mouseKeys)
        QVERIFY(!key.isEmpty());
}

void tst_Factory::unknownKeyYieldsNullBackend()
{
    QCOMPARE(QJoystickInputFactory::create(QStringLiteral("no-such-backend"), {}), nullptr);
    QCOMPARE(QMouseInputFactory::create(QStringLiteral("no-such-backend"), {}), nullptr);
}

void tst_Factory::singletonUsableAfterInit()
{
    auto *input = QUniversalInput::instance();
    // Flush the deferred initialisation synchronously.
    QCoreApplication::sendPostedEvents(input, QEvent::MetaCall);

    constexpr int device = 9;
    input->updateJoyConnection(device, true, QStringLiteral("Pad"), QString());

    QSignalSpy spy(input, &QUniversalInput::joyButtonEvent);
    input->joyButton(device, JoyButton::A, true);
    QCOMPARE(spy.count(), 1);

    input->updateJoyConnection(device, false, QString(), QString());
}

QTEST_MAIN(tst_Factory)

#include "tst_factory.moc"
