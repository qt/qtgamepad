// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtUniversalInput/QActionStore>
#include <QtUniversalInput/QUniversalInput>

QT_USE_NAMESPACE

class tst_QActionStore : public QObject
{
    Q_OBJECT
private slots:
    void buttonActionEmitsEvent();
    void unregisteredButtonDoesNotEmit();
    void clearActionsStopsEvents();
};

// A registered button action should fire when the matching joystick button
// event is delivered through QUniversalInput. This also exercises the
// QObjectPrivate signal/slot connection set up by QActionStore.
void tst_QActionStore::buttonActionEmitsEvent()
{
    QActionStore store;
    QActionStore::ActionBuilder builder(QStringLiteral("Jump"));
    builder.addButton(QActionStore::Controller::Device0, QUniversalInput::JoyButton::A, true);
    store.registerAction(builder.build());

    QSignalSpy actionSpy(&store, &QActionStore::actionEvent);
    QSignalSpy buttonSpy(&store, &QActionStore::actionJoyButtonEvent);

    QUniversalInput::instance()->joyButton(0, QUniversalInput::JoyButton::A, true);

    QCOMPARE(actionSpy.count(), 1);
    QCOMPARE(actionSpy.at(0).at(0).toString(), QStringLiteral("Jump"));
    QCOMPARE(buttonSpy.count(), 1);
}

void tst_QActionStore::unregisteredButtonDoesNotEmit()
{
    QActionStore store;
    QActionStore::ActionBuilder builder(QStringLiteral("Jump"));
    builder.addButton(QActionStore::Controller::Device0, QUniversalInput::JoyButton::A, true);
    store.registerAction(builder.build());

    QSignalSpy actionSpy(&store, &QActionStore::actionEvent);

    // A different button must not trigger the action.
    QUniversalInput::instance()->joyButton(0, QUniversalInput::JoyButton::B, true);

    QCOMPARE(actionSpy.count(), 0);
}

void tst_QActionStore::clearActionsStopsEvents()
{
    QActionStore store;
    QActionStore::ActionBuilder builder(QStringLiteral("Jump"));
    builder.addButton(QActionStore::Controller::Device0, QUniversalInput::JoyButton::X, true);
    store.registerAction(builder.build());
    store.clearActions();

    QSignalSpy actionSpy(&store, &QActionStore::actionEvent);
    QUniversalInput::instance()->joyButton(0, QUniversalInput::JoyButton::X, true);

    QCOMPARE(actionSpy.count(), 0);
}

QTEST_MAIN(tst_QActionStore)

#include "tst_qactionstore.moc"
