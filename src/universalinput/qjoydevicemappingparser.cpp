// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qjoydevicemappingparser_p.h"

#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcUniversalInput, "qt.universalinput")

QJoyDeviceMappingParser::QJoyDeviceMappingParser(const QString &filepath)
    : m_filepath(filepath), m_file(filepath), m_stream(&m_file)
{
    if (!m_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(lcUniversalInput) << "QJoyDeviceMappingParser could not open file" << m_filepath;
        return;
    }
}

struct Axis
{
    JoyAxis axis;
    QUniversalInput::JoyAxisRange range;
    bool invert;
};

static Axis axisFromString(const QString &axisStr)
{
    auto axis = axisStr;

    auto joyAxis = JoyAxis::Invalid;
    auto axisRange = QUniversalInput::JoyAxisRange::FullAxis;

    // if first character is a minus, it's a negative axis
    if (axis[0] == u"-"_s) {
        axisRange = QUniversalInput::JoyAxisRange::NegativeHalfAxis;
        // remove the minus sign
        // qstring
        axis = axis.remove(0, 1);
    } else if (axis[0] == u"+"_s) {
        // if first character is a plus, it's a positive axis
        axisRange = QUniversalInput::JoyAxisRange::PositiveHalfAxis;
        // remove the plus sign
        axis = axis.remove(0, 1);
    }

    if (axis[0] == u"a"_s) {
        // remove the a
        axis = axis.remove(0, 1);
        int axisNumber = axis.toInt();
        joyAxis = static_cast<JoyAxis>(axisNumber);
    } else if (axis == u"rightx"_s) {
        joyAxis = JoyAxis::RightX;
    } else if (axis == u"righty"_s) {
        joyAxis = JoyAxis::RightY;
    } else if (axis == u"leftx"_s) {
        joyAxis = JoyAxis::LeftX;
    } else if (axis == u"lefty"_s) {
        joyAxis = JoyAxis::LeftY;
    } else if (axis == u"lefttrigger"_s) {
        joyAxis = JoyAxis::TriggerLeft;
    } else if (axis == u"righttrigger"_s) {
        joyAxis = JoyAxis::TriggerRight;
    } else {
    }

    return {joyAxis, axisRange, false};
}

static JoyButton buttonFromString(const QString &button)
{
    if (button == u"a"_s)
        return JoyButton::A;
    if (button == u"b"_s)
        return JoyButton::B;
    if (button == u"x"_s)
        return JoyButton::X;
    if (button == u"y"_s)
        return JoyButton::Y;
    if (button == u"back"_s)
        return JoyButton::Back;
    if (button == u"guide"_s)
        return JoyButton::Guide;
    if (button == u"start"_s)
        return JoyButton::Start;
    if (button == u"leftstick"_s)
        return JoyButton::LeftStick;
    if (button == u"rightstick"_s)
        return JoyButton::RightStick;
    if (button == u"leftshoulder"_s)
        return JoyButton::LeftShoulder;
    if (button == u"rightshoulder"_s)
        return JoyButton::RightShoulder;
    if (button == u"dpadup"_s || button == u"dpup"_s)
        return JoyButton::DpadUp;
    if (button == u"dpaddown"_s || button == u"dpdown"_s)
        return JoyButton::DpadDown;
    if (button == u"dpadleft"_s || button == u"dpleft"_s)
        return JoyButton::DpadLeft;
    if (button == u"dpadright"_s || button == u"dpright"_s)
        return JoyButton::DpadRight;
    if (button == u"misc1"_s)
        return JoyButton::Misc1;
    if (button == u"paddle1"_s)
        return JoyButton::Paddle1;
    if (button == u"paddle2"_s)
        return JoyButton::Paddle2;
    if (button == u"paddle3"_s)
        return JoyButton::Paddle3;
    if (button == u"paddle4"_s)
        return JoyButton::Paddle4;
    if (button == u"touchpad"_s)
        return JoyButton::Touchpad;
    if (button.startsWith(u"b"_s)) {
        bool ok;
        int index = button.mid(1).toInt(&ok);
        if (ok)
            return static_cast<JoyButton>(index);
    }
    qCWarning(lcUniversalInput) << "QJoyDeviceMappingParser::buttonFromString: Unknown button" << button;
    return JoyButton::Invalid;
}

struct Hat
{
    HatDirection direction;
    HatMask mask;
};

static Hat hatFromString(const QString &hat)
{
    int hatMask = hat[hat.length() - 1].digitValue();
    int log2hatMask = qFloor(qLn(hatMask) / qLn(2));
    return {HatDirection(log2hatMask), HatMask(hatMask)};
}

QUniversalInput::JoyType QJoyDeviceMappingParser::toJoyType(const QString &type) const
{
    if (type == u"a"_s || type == u"b"_s) // these messes with the rest of the capturing
        return QUniversalInput::JoyType::TypeButton;
    if (auto match = m_buttonRegex.match(type); match.hasMatch() && match.capturedLength() == type.length())
        return QUniversalInput::JoyType::TypeButton;
    if (m_axisRegex.match(type).hasMatch())
        return QUniversalInput::JoyType::TypeAxis;
    if (m_hatRegex.match(type).hasMatch())
        return QUniversalInput::JoyType::TypeHat;

    qCWarning(lcUniversalInput) << "QJoyDeviceMappingParser::toJoyType: Unknown type" << type;
    return QUniversalInput::JoyType::TypeMax;
}

QUniversalInput::JoyBinding QJoyDeviceMappingParser::parseBinding(const QString &token) const
{
    auto actions = token.split(u':');
    auto output = actions[0];
    auto input = actions[0];
    if (actions.size() == 2)
        input = actions[1];

    QUniversalInput::JoyBinding binding = {};
    binding.inputType = toJoyType(input);
    binding.outputType = toJoyType(output);

    // input
    switch (binding.inputType) {
    case QUniversalInput::JoyType::TypeButton:
        binding.input.button = buttonFromString(input);
        break;
    case QUniversalInput::JoyType::TypeAxis:
    {
        auto axis = axisFromString(input);
        binding.input.axis.axis = axis.axis;
        binding.input.axis.range = axis.range;
        binding.input.axis.invert = axis.invert;
        break;
    }
    case QUniversalInput::JoyType::TypeHat:
    {
        auto hat = hatFromString(input);
        binding.input.hat.hat = hat.direction;
        binding.input.hat.hat_mask = hat.mask;
        break;
    }
    default:
        qCWarning(lcUniversalInput) << "QJoyDeviceMappingParser::parseBinding: Unknown input type" << input;
        break;
    }

    // output
    switch (binding.outputType) {
    case QUniversalInput::JoyType::TypeHat:
        binding.outputType = QUniversalInput::JoyType::TypeButton;
        break;
    case QUniversalInput::JoyType::TypeButton:
        binding.output.button = buttonFromString(output);
        break;
    case QUniversalInput::JoyType::TypeAxis:
    {
        auto axis = axisFromString(output);
        binding.output.axis.axis = axis.axis;
        binding.output.axis.range = axis.range;
        break;
    }
    default:
        qCWarning(lcUniversalInput) << "QJoyDeviceMappingParser::parseBinding: Unknown output type" << output;
        break;
    }

    return binding;
}

std::optional<QUniversalInput::JoyDeviceMapping> QJoyDeviceMappingParser::next()
{
    if (m_stream.atEnd())
        return {};

    auto line = m_stream.readLine().trimmed();

    // skip comments and empty lines
    while (line.startsWith(u"#"_s) || line.isEmpty()) {
        line = m_stream.readLine().trimmed();
        if (m_stream.atEnd())
            return {};
    }

    auto tokens = line.split(u',');

    // last , makes an empty token
    if (tokens.back().isEmpty())
        tokens.pop_back();

    auto uid = tokens.takeFirst();
    auto name = tokens.takeFirst();
    auto platform = tokens.takeLast();
    Q_UNUSED(platform); // just had to remove platform from the tokens

    QVector<QUniversalInput::JoyBinding> bindings;
    for (const auto &token : tokens) {
        auto bindingToken = token.trimmed();
        auto binding = parseBinding(bindingToken);
        bindings.push_back(binding);
    }

    QUniversalInput::JoyDeviceMapping mapping = {uid, name, bindings};
    return mapping;
}

QT_END_NAMESPACE
