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
    void init();
    void cleanup();

    void buttonActionEmitsEvent();
    void unregisteredButtonDoesNotEmit();
    void clearActionsStopsEvents();
    void clearActionsRemovesAllActions();

    void deadzoneFiltersAxis();
    void axisDirectionMatching_data();
    void axisDirectionMatching();

    void wildcardControllerMatchesAnyDevice();
    void specificControllerIgnoresOtherDevices();

    void keyActionEmitsEvent();
    void mouseButtonActionEmitsEvent();

    void builderAssemblesAction();
};

// Reconnecting with an empty GUID resets the singleton's per-device state and
// leaves the device unmapped, so raw enums reach the store unchanged.
void tst_QActionStore::init()
{
    auto *input = QUniversalInput::instance();
    input->updateJoyConnection(0, true, QStringLiteral("Test Pad 0"), QString());
    input->updateJoyConnection(1, true, QStringLiteral("Test Pad 1"), QString());
}

void tst_QActionStore::cleanup()
{
    auto *input = QUniversalInput::instance();
    input->updateJoyConnection(0, false, QString(), QString());
    input->updateJoyConnection(1, false, QString(), QString());
}

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

void tst_QActionStore::clearActionsRemovesAllActions()
{
    QActionStore store;
    store.registerAction(QActionStore::ActionBuilder(QStringLiteral("Jump"))
                                 .addButton(QActionStore::Controller::Device0,
                                            QUniversalInput::JoyButton::A, true)
                                 .build());
    store.registerAction(QActionStore::ActionBuilder(QStringLiteral("Fire"))
                                 .addButton(QActionStore::Controller::Device0,
                                            QUniversalInput::JoyButton::B, true)
                                 .build());
    store.clearActions();

    QSignalSpy actionSpy(&store, &QActionStore::actionEvent);
    QUniversalInput::instance()->joyButton(0, QUniversalInput::JoyButton::A, true);
    QUniversalInput::instance()->joyButton(0, QUniversalInput::JoyButton::B, true);

    QCOMPARE(actionSpy.count(), 0);
}

void tst_QActionStore::deadzoneFiltersAxis()
{
    QActionStore store;
    store.registerAction(QActionStore::ActionBuilder(QStringLiteral("Move"))
                                 .addAxis(QActionStore::Controller::Device0,
                                          QUniversalInput::JoyAxis::LeftX,
                                          QActionStore::AxisDirection::All, 0.5f)
                                 .build());

    QSignalSpy actionSpy(&store, &QActionStore::actionEvent);

    QUniversalInput::instance()->joyAxis(0, QUniversalInput::JoyAxis::LeftX, 0.3f);
    QCOMPARE(actionSpy.count(), 0);

    QUniversalInput::instance()->joyAxis(0, QUniversalInput::JoyAxis::LeftX, 0.6f);
    QCOMPARE(actionSpy.count(), 1);
}

void tst_QActionStore::axisDirectionMatching_data()
{
    QTest::addColumn<int>("direction");
    QTest::addColumn<double>("value");
    QTest::addColumn<bool>("shouldFire");

    QTest::newRow("right-positive") << int(QActionStore::AxisDirection::Right) << 0.8 << true;
    QTest::newRow("right-negative") << int(QActionStore::AxisDirection::Right) << -0.8 << false;
    QTest::newRow("left-negative") << int(QActionStore::AxisDirection::Left) << -0.8 << true;
    QTest::newRow("left-positive") << int(QActionStore::AxisDirection::Left) << 0.8 << false;
    QTest::newRow("up-negative") << int(QActionStore::AxisDirection::Up) << -0.8 << true;
    QTest::newRow("down-positive") << int(QActionStore::AxisDirection::Down) << 0.8 << true;
    QTest::newRow("all-positive") << int(QActionStore::AxisDirection::All) << 0.8 << true;
    QTest::newRow("all-negative") << int(QActionStore::AxisDirection::All) << -0.8 << true;
}

void tst_QActionStore::axisDirectionMatching()
{
    QFETCH(int, direction);
    QFETCH(double, value);
    QFETCH(bool, shouldFire);

    QActionStore store;
    store.registerAction(QActionStore::ActionBuilder(QStringLiteral("Move"))
                                 .addAxis(QActionStore::Controller::Device0,
                                          QUniversalInput::JoyAxis::LeftX,
                                          QActionStore::AxisDirection(direction), 0.5f)
                                 .build());

    QSignalSpy actionSpy(&store, &QActionStore::actionEvent);
    QUniversalInput::instance()->joyAxis(0, QUniversalInput::JoyAxis::LeftX, float(value));

    QCOMPARE(actionSpy.count(), shouldFire ? 1 : 0);
}

void tst_QActionStore::wildcardControllerMatchesAnyDevice()
{
    QActionStore store;
    store.registerAction(QActionStore::ActionBuilder(QStringLiteral("Jump"))
                                 .addButton(QActionStore::Controller::All,
                                            QUniversalInput::JoyButton::A, true)
                                 .build());

    QSignalSpy buttonSpy(&store, &QActionStore::actionJoyButtonEvent);

    QUniversalInput::instance()->joyButton(1, QUniversalInput::JoyButton::A, true);

    QCOMPARE(buttonSpy.count(), 1);
    QCOMPARE(buttonSpy.at(0).at(1).toInt(), 1);
}

void tst_QActionStore::specificControllerIgnoresOtherDevices()
{
    QActionStore store;
    store.registerAction(QActionStore::ActionBuilder(QStringLiteral("Jump"))
                                 .addButton(QActionStore::Controller::Device1,
                                            QUniversalInput::JoyButton::A, true)
                                 .build());

    QSignalSpy actionSpy(&store, &QActionStore::actionEvent);

    QUniversalInput::instance()->joyButton(0, QUniversalInput::JoyButton::A, true);
    QCOMPARE(actionSpy.count(), 0);

    QUniversalInput::instance()->joyButton(1, QUniversalInput::JoyButton::A, true);
    QCOMPARE(actionSpy.count(), 1);
}

void tst_QActionStore::keyActionEmitsEvent()
{
    QActionStore store;
    store.registerAction(QActionStore::ActionBuilder(QStringLiteral("Jump"))
                                 .addKey(Qt::Key_Space, true)
                                 .build());

    QSignalSpy actionSpy(&store, &QActionStore::actionEvent);
    QSignalSpy keySpy(&store, &QActionStore::actionKeyEvent);

    store.sendKeyEvent(Qt::Key_A, true);
    QCOMPARE(actionSpy.count(), 0);

    store.sendKeyEvent(Qt::Key_Space, true);
    QCOMPARE(actionSpy.count(), 1);
    QCOMPARE(keySpy.count(), 1);
    QCOMPARE(keySpy.at(0).at(1).value<Qt::Key>(), Qt::Key_Space);
}

void tst_QActionStore::mouseButtonActionEmitsEvent()
{
    QActionStore store;
    store.registerAction(QActionStore::ActionBuilder(QStringLiteral("Fire"))
                                 .addMouseButton(Qt::LeftButton, true)
                                 .build());

    QSignalSpy actionSpy(&store, &QActionStore::actionEvent);
    QSignalSpy mouseSpy(&store, &QActionStore::actionMouseButtonEvent);

    store.sendMouseButtonEvent(Qt::RightButton, true);
    QCOMPARE(actionSpy.count(), 0);

    store.sendMouseButtonEvent(Qt::LeftButton, true);
    QCOMPARE(actionSpy.count(), 1);
    QCOMPARE(mouseSpy.count(), 1);
    QCOMPARE(mouseSpy.at(0).at(1).value<Qt::MouseButton>(), Qt::LeftButton);
}

void tst_QActionStore::builderAssemblesAction()
{
    const QActionStore::Action action =
            QActionStore::ActionBuilder(QStringLiteral("Combo"))
                    .addButton(QActionStore::Controller::Device2,
                               QUniversalInput::JoyButton::Y, true)
                    .addAxis(QActionStore::Controller::Device3,
                             QUniversalInput::JoyAxis::RightY,
                             QActionStore::AxisDirection::Down, 0.25f)
                    .addKey(Qt::Key_Return, false)
                    .addMouseButton(Qt::MiddleButton, true)
                    .build();

    QCOMPARE(action.name, QStringLiteral("Combo"));

    QCOMPARE(action.buttons.size(), 1);
    QCOMPARE(action.buttons.at(0).device, QActionStore::Controller::Device2);
    QCOMPARE(action.buttons.at(0).button, QUniversalInput::JoyButton::Y);
    QCOMPARE(action.buttons.at(0).isPressed, true);

    QCOMPARE(action.axes.size(), 1);
    QCOMPARE(action.axes.at(0).device, QActionStore::Controller::Device3);
    QCOMPARE(action.axes.at(0).axis, QUniversalInput::JoyAxis::RightY);
    QCOMPARE(action.axes.at(0).direction, QActionStore::AxisDirection::Down);
    QCOMPARE(action.axes.at(0).deadzone, 0.25f);

    QCOMPARE(action.keys.size(), 1);
    QCOMPARE(action.keys.at(0).key, Qt::Key_Return);
    QCOMPARE(action.keys.at(0).isPressed, false);

    QCOMPARE(action.mouseButtons.size(), 1);
    QCOMPARE(action.mouseButtons.at(0).button, Qt::MiddleButton);
    QCOMPARE(action.mouseButtons.at(0).isPressed, true);
}

QTEST_MAIN(tst_QActionStore)

#include "tst_qactionstore.moc"
