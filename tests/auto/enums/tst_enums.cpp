// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtUniversalInput/QUniversalInput>

QT_USE_NAMESPACE

class tst_Enums : public QObject
{
    Q_OBJECT
private slots:
    void scopedEnumsAreRegistered();
    void hatMaskIsAFlag();
};

// The input enums moved into QUniversalInput as Q_ENUM members, so they must
// be visible to the meta-object system (and therefore to QML).
void tst_Enums::scopedEnumsAreRegistered()
{
    const QMetaEnum joyButton = QMetaEnum::fromType<QUniversalInput::JoyButton>();
    QVERIFY(joyButton.isValid());
    QCOMPARE(joyButton.keyToValue("A"), int(QUniversalInput::JoyButton::A));

    const QMetaEnum joyAxis = QMetaEnum::fromType<QUniversalInput::JoyAxis>();
    QVERIFY(joyAxis.isValid());
    QCOMPARE(joyAxis.keyToValue("LeftX"), int(QUniversalInput::JoyAxis::LeftX));

    const QMetaEnum hatDirection = QMetaEnum::fromType<QUniversalInput::HatDirection>();
    QVERIFY(hatDirection.isValid());
}

void tst_Enums::hatMaskIsAFlag()
{
    const QMetaEnum hatFlag = QMetaEnum::fromType<QUniversalInput::HatMask>();
    QVERIFY(hatFlag.isValid());
    QVERIFY(hatFlag.isFlag());

    // The QFlags combine as expected.
    const QUniversalInput::HatMask upRight =
            QUniversalInput::HatFlag::Up | QUniversalInput::HatFlag::Right;
    QVERIFY(upRight.testFlag(QUniversalInput::HatFlag::Up));
    QVERIFY(upRight.testFlag(QUniversalInput::HatFlag::Right));
    QVERIFY(!upRight.testFlag(QUniversalInput::HatFlag::Down));
}

QTEST_MAIN(tst_Enums)

#include "tst_enums.moc"
