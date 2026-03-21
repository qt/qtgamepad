// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QCoreApplication>
#include <QtUniversalInput/QUniversalInput>

#include <QtCore/QDebug>
#include <QVector2D>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    auto input = QUniversalInput::instance();

    QObject::connect(input, &QUniversalInput::joyConnectionChanged, [](int joyId, bool connected) {
        qDebug() << "joyConnectionChanged!";
        qDebug() << "joy" << joyId << "connected:" << connected;
    });
    QObject::connect(input, &QUniversalInput::joyButtonEvent, [&input](int device, JoyButton button, bool pressed) {
        qDebug() << "Device: " << device << "button: " << int(button) << ( pressed ? "pressed" : "released");
        input->addForce(0, QVector2D(1, 1), 1.0f);
    });
    QObject::connect(input, &QUniversalInput::joyAxisEvent, [](int device, JoyAxis axis, float value) {
        qDebug() << "Device: " << device << "axis: " << int(axis) << "value: " << value;
    });

    qDebug() << "hello, this is a console joystick monitor";

    return app.exec();
}
