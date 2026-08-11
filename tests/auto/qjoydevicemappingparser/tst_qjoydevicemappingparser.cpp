// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtUniversalInput/private/qjoydevicemappingparser_p.h>

QT_USE_NAMESPACE

using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;
using JoyType = QUniversalInput::JoyType;
using JoyAxisRange = QUniversalInput::JoyAxisRange;
using HatDirection = QUniversalInput::HatDirection;
using HatFlag = QUniversalInput::HatFlag;

class tst_QJoyDeviceMappingParser : public QObject
{
    Q_OBJECT
private slots:
    void parsesBundledDatabase();
    void missingFileYieldsNoMappings();

    void skipsCommentsAndBlankLines();
    void parsesButtonBindings();
    void parsesAxisBindings();
    void parsesInvertedHalfAxis();
    void parsesHatBindings();
};

// The SDL game controller database is embedded as a resource by the
// QtUniversalInput module, which this test links against.
void tst_QJoyDeviceMappingParser::parsesBundledDatabase()
{
    QJoyDeviceMappingParser parser(
            QStringLiteral(":/qt-project.org/qtuniversalinput/gamecontrollerdb.txt"));

    int count = 0;
    for (auto mapping = parser.next(); mapping.has_value(); mapping = parser.next()) {
        QVERIFY(!mapping->uid.isEmpty());
        QVERIFY(!mapping->name.isEmpty());
        QVERIFY(!mapping->bindings.isEmpty());
        ++count;
    }

    QVERIFY2(count > 0, "expected at least one controller mapping in the bundled database");
}

void tst_QJoyDeviceMappingParser::missingFileYieldsNoMappings()
{
    QJoyDeviceMappingParser parser(QStringLiteral(":/does/not/exist.txt"));
    QVERIFY(!parser.next().has_value());
}

void tst_QJoyDeviceMappingParser::skipsCommentsAndBlankLines()
{
    QJoyDeviceMappingParser parser(QStringLiteral(":/gamecontrollerdb-fixture.txt"));

    QList<QUniversalInput::JoyDeviceMapping> mappings;
    for (auto mapping = parser.next(); mapping.has_value(); mapping = parser.next())
        mappings.push_back(*mapping);

    QCOMPARE(mappings.size(), 2);
    QCOMPARE(mappings.at(0).uid, QStringLiteral("03000000fixturedevice000000000001"));
    QCOMPARE(mappings.at(0).name, QStringLiteral("Fixture Pad"));
    QCOMPARE(mappings.at(0).bindings.size(), 6);
    QCOMPARE(mappings.at(1).uid, QStringLiteral("03000000fixturedevice000000000002"));
    QCOMPARE(mappings.at(1).name, QStringLiteral("Second Pad"));
    QCOMPARE(mappings.at(1).bindings.size(), 2);
}

void tst_QJoyDeviceMappingParser::parsesButtonBindings()
{
    QJoyDeviceMappingParser parser(QStringLiteral(":/gamecontrollerdb-fixture.txt"));
    const auto mapping = parser.next();
    QVERIFY(mapping.has_value());

    const auto binding = mapping->bindings.at(1); // leftshoulder:b4
    QCOMPARE(binding.outputType, JoyType::TypeButton);
    QCOMPARE(binding.output.button, JoyButton::LeftShoulder);
    QCOMPARE(binding.inputType, JoyType::TypeButton);
    QCOMPARE(binding.input.button, JoyButton(4));
}

void tst_QJoyDeviceMappingParser::parsesAxisBindings()
{
    QJoyDeviceMappingParser parser(QStringLiteral(":/gamecontrollerdb-fixture.txt"));
    const auto mapping = parser.next();
    QVERIFY(mapping.has_value());

    const auto fullAxis = mapping->bindings.at(2); // leftx:a0
    QCOMPARE(fullAxis.outputType, JoyType::TypeAxis);
    QCOMPARE(fullAxis.output.axis.axis, JoyAxis::LeftX);
    QCOMPARE(fullAxis.inputType, JoyType::TypeAxis);
    QCOMPARE(fullAxis.input.axis.axis, JoyAxis(0));
    QCOMPARE(fullAxis.input.axis.range, JoyAxisRange::FullAxis);
    QCOMPARE(fullAxis.input.axis.invert, false);

    const auto halfAxis = mapping->bindings.at(3); // lefttrigger:+a2
    QCOMPARE(halfAxis.outputType, JoyType::TypeAxis);
    QCOMPARE(halfAxis.output.axis.axis, JoyAxis::TriggerLeft);
    QCOMPARE(halfAxis.input.axis.axis, JoyAxis(2));
    QCOMPARE(halfAxis.input.axis.range, JoyAxisRange::PositiveHalfAxis);
    QCOMPARE(halfAxis.input.axis.invert, false);
}

void tst_QJoyDeviceMappingParser::parsesInvertedHalfAxis()
{
    QJoyDeviceMappingParser parser(QStringLiteral(":/gamecontrollerdb-fixture.txt"));
    const auto mapping = parser.next();
    QVERIFY(mapping.has_value());

    const auto binding = mapping->bindings.at(4); // righty:-a3~
    QCOMPARE(binding.outputType, JoyType::TypeAxis);
    QCOMPARE(binding.output.axis.axis, JoyAxis::RightY);
    QCOMPARE(binding.inputType, JoyType::TypeAxis);
    QCOMPARE(binding.input.axis.axis, JoyAxis(3));
    QCOMPARE(binding.input.axis.range, JoyAxisRange::NegativeHalfAxis);
    QCOMPARE(binding.input.axis.invert, true);
}

// The hat mask's set bit selects the direction through a log2 mapping.
void tst_QJoyDeviceMappingParser::parsesHatBindings()
{
    QJoyDeviceMappingParser parser(QStringLiteral(":/gamecontrollerdb-fixture.txt"));
    const auto mapping = parser.next();
    QVERIFY(mapping.has_value());

    const auto binding = mapping->bindings.at(5); // dpup:h0.1
    QCOMPARE(binding.inputType, JoyType::TypeHat);
    QCOMPARE(binding.input.hat.hat, HatDirection::Up);
    QCOMPARE(binding.input.hat.hat_mask, QUniversalInput::HatMask(HatFlag::Up));
}

QTEST_MAIN(tst_QJoyDeviceMappingParser)

#include "tst_qjoydevicemappingparser.moc"
