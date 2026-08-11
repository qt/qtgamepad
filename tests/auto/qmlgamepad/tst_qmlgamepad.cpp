// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlComponent>

#include <QtUniversalInput/QUniversalInput>

QT_USE_NAMESPACE

using JoyButton = QUniversalInput::JoyButton;

// Drives the QML layer from C++: input is injected through the singleton and
// observed on inline QML objects.
class tst_QmlGamepad : public QObject
{
    Q_OBJECT

    QObject *create(const QByteArray &qml)
    {
        auto *component = new QQmlComponent(&m_engine, this);
        component->setData(qml, QUrl(QStringLiteral("inline.qml")));
        if (!component->isReady())
            qWarning() << component->errorString();
        QObject *root = component->create();
        return root;
    }

private slots:
    void init();
    void cleanup();

    void modulesImport();
    void gamepadReactsToInjectedInput();
    void actionStoreReactsToDeclaredAction();
    void qmlInjectionForwardsToSingleton();

private:
    QQmlEngine m_engine;
};

void tst_QmlGamepad::init()
{
    QUniversalInput::instance()->updateJoyConnection(0, true, QStringLiteral("Pad"), QString());
}

void tst_QmlGamepad::cleanup()
{
    QUniversalInput::instance()->updateJoyConnection(0, false, QString(), QString());
}

void tst_QmlGamepad::modulesImport()
{
    std::unique_ptr<QObject> gamepad(create("import QtGamepad\nGamepad { deviceId: 0 }\n"));
    QVERIFY(gamepad);

    std::unique_ptr<QObject> store(create(
            "import QtActionStore\n"
            "ActionStore { property int a: UniversalInput.JoyButton.A }\n"));
    QVERIFY(store);
    QCOMPARE(store->property("a").toInt(), int(JoyButton::A));

    std::unique_ptr<QObject> universal(create("import QtUniversalInput\nUniversalInput {}\n"));
    QVERIFY(universal);
}

void tst_QmlGamepad::gamepadReactsToInjectedInput()
{
    std::unique_ptr<QObject> gamepad(create("import QtGamepad\nGamepad { deviceId: 0 }\n"));
    QVERIFY(gamepad);
    QCOMPARE(gamepad->property("buttonA").toBool(), false);

    QUniversalInput::instance()->joyButton(0, JoyButton::A, true);

    QCOMPARE(gamepad->property("buttonA").toBool(), true);
}

void tst_QmlGamepad::actionStoreReactsToDeclaredAction()
{
    std::unique_ptr<QObject> store(create(
            "import QtActionStore\n"
            "ActionStore {\n"
            "    InputAction {\n"
            "        title: \"Jump\"\n"
            "        JoyButtonEvent { device: 0; button: UniversalInput.JoyButton.A; isPressed: true }\n"
            "    }\n"
            "}\n"));
    QVERIFY(store);

    QSignalSpy actionSpy(store.get(), SIGNAL(actionEvent(QString)));
    QVERIFY(actionSpy.isValid());

    QUniversalInput::instance()->joyButton(0, JoyButton::A, true);

    QCOMPARE(actionSpy.count(), 1);
    QCOMPARE(actionSpy.at(0).at(0).toString(), QStringLiteral("Jump"));
}

void tst_QmlGamepad::qmlInjectionForwardsToSingleton()
{
    std::unique_ptr<QObject> universal(create("import QtUniversalInput\nUniversalInput {}\n"));
    QVERIFY(universal);

    QSignalSpy spy(QUniversalInput::instance(), &QUniversalInput::joyButtonEvent);

    // The QML element's injection slot forwards to the singleton.
    QVERIFY(QMetaObject::invokeMethod(universal.get(), "joyButton",
                                      Q_ARG(int, 0),
                                      Q_ARG(QUniversalInput::JoyButton, JoyButton::B),
                                      Q_ARG(bool, true)));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<JoyButton>(), JoyButton::B);
}

QTEST_MAIN(tst_QmlGamepad)

#include "tst_qmlgamepad.moc"
