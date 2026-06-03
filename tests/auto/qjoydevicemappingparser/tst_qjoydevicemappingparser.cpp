// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtUniversalInput/private/qjoydevicemappingparser_p.h>

QT_USE_NAMESPACE

class tst_QJoyDeviceMappingParser : public QObject
{
    Q_OBJECT
private slots:
    void parsesBundledDatabase();
    void missingFileYieldsNoMappings();
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

QTEST_MAIN(tst_QJoyDeviceMappingParser)

#include "tst_qjoydevicemappingparser.moc"
