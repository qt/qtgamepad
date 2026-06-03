// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QJOYDEVICEMAPPINGPARSER_P_H
#define QJOYDEVICEMAPPINGPARSER_P_H

#include <QtUniversalInput/quniversalinput.h>

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

#include <optional>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

class Q_UNIVERSALINPUT_EXPORT QJoyDeviceMappingParser
{
public:
    QJoyDeviceMappingParser(const QString &filepath);

    std::optional<QUniversalInput::JoyDeviceMapping> next();

private:
    QUniversalInput::JoyBinding parseBinding(const QString &token) const;
    QUniversalInput::JoyType toJoyType(const QString &type) const;

private:
    QString m_filepath;
    QFile m_file;
    QTextStream m_stream;
    QString m_currentLine;

    QRegularExpression m_buttonRegex{u"b([0-9]+)|(x)|(y)|(back)|(guide)|(start)|(leftstick)|(rightstick)|(leftshoulder)|(rightshoulder)|(misc1)|(paddle1)|(paddle2)|(paddle3)|(paddle4)|(touchpad)"_s};
    QRegularExpression m_axisRegex{u"a[0-9]+|rightx|righty|leftx|lefty|lefttrigger|righttrigger"_s};
    QRegularExpression m_hatRegex{u"h[0-9]+|dpup|dpdown|dpleft|dpright|dpadup|dpaddown|dpadleft|dpadright"_s};
};

QT_END_NAMESPACE

#endif // QJOYDEVICEMAPPINGPARSER_P_H
