// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtUniversalInput/QUniversalInput>

QT_USE_NAMESPACE

using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;
using JoyBinding = QUniversalInput::JoyBinding;
using JoyDeviceMapping = QUniversalInput::JoyDeviceMapping;
using JoyEvent = QUniversalInput::JoyEvent;
using HatDirection = QUniversalInput::HatDirection;
using HatMask = QUniversalInput::HatMask;
using HatFlag = QUniversalInput::HatFlag;

// A friend of QUniversalInput on internal builds, so that the private mapping
// transforms can be called directly with hand-built mappings.
class tst_QMappingTransform : public QObject
{
    Q_OBJECT

    static JoyDeviceMapping single(const JoyBinding &binding)
    {
        return { QStringLiteral("uid"), QStringLiteral("name"), { binding } };
    }

    static JoyBinding buttonToButton(JoyButton in, JoyButton out)
    {
        JoyBinding b = {};
        b.inputType = QUniversalInput::TypeButton;
        b.input.button = in;
        b.outputType = QUniversalInput::TypeButton;
        b.output.button = out;
        return b;
    }

    static JoyBinding buttonToAxis(JoyButton in, JoyAxis out, QUniversalInput::JoyAxisRange range)
    {
        JoyBinding b = {};
        b.inputType = QUniversalInput::TypeButton;
        b.input.button = in;
        b.outputType = QUniversalInput::TypeAxis;
        b.output.axis.axis = out;
        b.output.axis.range = range;
        return b;
    }

    static JoyBinding axisToAxis(JoyAxis in, QUniversalInput::JoyAxisRange inRange, bool invert,
                                 JoyAxis out, QUniversalInput::JoyAxisRange outRange)
    {
        JoyBinding b = {};
        b.inputType = QUniversalInput::TypeAxis;
        b.input.axis.axis = in;
        b.input.axis.range = inRange;
        b.input.axis.invert = invert;
        b.outputType = QUniversalInput::TypeAxis;
        b.output.axis.axis = out;
        b.output.axis.range = outRange;
        return b;
    }

private slots:
    void buttonMapsToButton();
    void buttonMapsToHalfAxis();
    void unmatchedButtonYieldsNoEvent();

    void fullAxisPassesThrough();
    void invertedAxisNegates();
    void negativeHalfAxisConvertsToFull();
    void positiveHalfAxisConvertsToFull();
    void axisOutsideHalfRangeYieldsNoEvent();
    void fullAxisMapsToButton();

    void hatMapsToButton();
};

void tst_QMappingTransform::buttonMapsToButton()
{
    auto *input = QUniversalInput::instance();
    const auto mapping = single(buttonToButton(JoyButton(6), JoyButton::Back));

    const JoyEvent event = input->mappedButtonEvent(mapping, JoyButton(6));
    QCOMPARE(event.type, int(QUniversalInput::TypeButton));
    QCOMPARE(event.index, int(JoyButton::Back));
}

void tst_QMappingTransform::buttonMapsToHalfAxis()
{
    auto *input = QUniversalInput::instance();

    const JoyEvent positive = input->mappedButtonEvent(
            single(buttonToAxis(JoyButton(10), JoyAxis::TriggerLeft,
                                QUniversalInput::PositiveHalfAxis)),
            JoyButton(10));
    QCOMPARE(positive.type, int(QUniversalInput::TypeAxis));
    QCOMPARE(positive.index, int(JoyAxis::TriggerLeft));
    QCOMPARE(positive.value, 1.0f);

    const JoyEvent negative = input->mappedButtonEvent(
            single(buttonToAxis(JoyButton(11), JoyAxis::TriggerRight,
                                QUniversalInput::NegativeHalfAxis)),
            JoyButton(11));
    QCOMPARE(negative.value, -1.0f);
}

void tst_QMappingTransform::unmatchedButtonYieldsNoEvent()
{
    auto *input = QUniversalInput::instance();
    const auto mapping = single(buttonToButton(JoyButton(0), JoyButton::A));

    const JoyEvent event = input->mappedButtonEvent(mapping, JoyButton(20));
    QCOMPARE(event.type, int(QUniversalInput::TypeMax));
    QCOMPARE(event.index, -1);
}

void tst_QMappingTransform::fullAxisPassesThrough()
{
    auto *input = QUniversalInput::instance();
    const auto mapping = single(axisToAxis(JoyAxis::LeftX, QUniversalInput::FullAxis, false,
                                           JoyAxis::RightX, QUniversalInput::FullAxis));

    const JoyEvent event = input->mappedAxisEvent(mapping, JoyAxis::LeftX, 0.5f);
    QCOMPARE(event.type, int(QUniversalInput::TypeAxis));
    QCOMPARE(event.index, int(JoyAxis::RightX));
    QCOMPARE(event.value, 0.5f);
}

void tst_QMappingTransform::invertedAxisNegates()
{
    auto *input = QUniversalInput::instance();
    const auto mapping = single(axisToAxis(JoyAxis::LeftX, QUniversalInput::FullAxis, true,
                                           JoyAxis::LeftX, QUniversalInput::FullAxis));

    const JoyEvent event = input->mappedAxisEvent(mapping, JoyAxis::LeftX, 0.5f);
    QCOMPARE(event.value, -0.5f);
}

void tst_QMappingTransform::negativeHalfAxisConvertsToFull()
{
    auto *input = QUniversalInput::instance();
    const auto mapping = single(axisToAxis(JoyAxis::LeftY, QUniversalInput::NegativeHalfAxis, false,
                                           JoyAxis::TriggerLeft, QUniversalInput::FullAxis));

    // The negative half axis [-1, 0] is stretched over the full [-1, 1] range.
    const JoyEvent extreme = input->mappedAxisEvent(mapping, JoyAxis::LeftY, -1.0f);
    QCOMPARE(extreme.type, int(QUniversalInput::TypeAxis));
    QCOMPARE(extreme.index, int(JoyAxis::TriggerLeft));
    QCOMPARE(extreme.value, -1.0f);

    const JoyEvent mid = input->mappedAxisEvent(mapping, JoyAxis::LeftY, -0.5f);
    QCOMPARE(mid.value, 0.0f);
}

void tst_QMappingTransform::positiveHalfAxisConvertsToFull()
{
    auto *input = QUniversalInput::instance();
    const auto mapping = single(axisToAxis(JoyAxis::RightX, QUniversalInput::PositiveHalfAxis, false,
                                           JoyAxis::TriggerRight, QUniversalInput::FullAxis));

    const JoyEvent rest = input->mappedAxisEvent(mapping, JoyAxis::RightX, 0.0f);
    QCOMPARE(rest.value, -1.0f);

    const JoyEvent full = input->mappedAxisEvent(mapping, JoyAxis::RightX, 1.0f);
    QCOMPARE(full.value, 1.0f);
}

void tst_QMappingTransform::axisOutsideHalfRangeYieldsNoEvent()
{
    auto *input = QUniversalInput::instance();
    const auto mapping = single(axisToAxis(JoyAxis::RightX, QUniversalInput::PositiveHalfAxis, false,
                                           JoyAxis::TriggerRight, QUniversalInput::FullAxis));

    const JoyEvent event = input->mappedAxisEvent(mapping, JoyAxis::RightX, -0.5f);
    QCOMPARE(event.type, int(QUniversalInput::TypeMax));
}

void tst_QMappingTransform::fullAxisMapsToButton()
{
    auto *input = QUniversalInput::instance();
    JoyBinding b = {};
    b.inputType = QUniversalInput::TypeAxis;
    b.input.axis.axis = JoyAxis::LeftX;
    b.input.axis.range = QUniversalInput::FullAxis;
    b.input.axis.invert = false;
    b.outputType = QUniversalInput::TypeButton;
    b.output.button = JoyButton::A;

    const JoyEvent event = input->mappedAxisEvent(single(b), JoyAxis::LeftX, 0.5f);
    QCOMPARE(event.type, int(QUniversalInput::TypeButton));
    QCOMPARE(event.index, int(JoyButton::A));
    QCOMPARE(event.value, 0.5f);
}

void tst_QMappingTransform::hatMapsToButton()
{
    auto *input = QUniversalInput::instance();

    JoyBinding b = {};
    b.inputType = QUniversalInput::TypeHat;
    b.input.hat.hat = HatDirection::Up;
    b.input.hat.hat_mask = HatMask(HatFlag::Up);
    b.outputType = QUniversalInput::TypeButton;
    b.output.button = JoyButton::DpadUp;
    const auto mapping = single(b);

    JoyEvent events[size_t(HatDirection::Max)] = {};
    input->mappedHatEvents(mapping, HatDirection::Up, events);

    QCOMPARE(events[size_t(HatDirection::Up)].type, int(QUniversalInput::TypeButton));
    QCOMPARE(events[size_t(HatDirection::Up)].index, int(JoyButton::DpadUp));
    QCOMPARE(events[size_t(HatDirection::Right)].type, int(QUniversalInput::TypeMax));
}

QTEST_MAIN(tst_QMappingTransform)

#include "tst_mappingtransform.moc"
