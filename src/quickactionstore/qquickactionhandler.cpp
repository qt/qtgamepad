// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickactionhandler_p.h"

QT_BEGIN_NAMESPACE

// Bring the QUniversalInput input enums into scope for this file.
using HatMask = QUniversalInput::HatMask;
using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;

// ActionHandler

QQuickActionHandler::QQuickActionHandler(QObject *parent)
    : QObject(parent), m_actionStore(nullptr)
{
}

void QQuickActionHandler::setActionStore(QQuickActionStore *actionStore)
{
    if (m_actionStore == actionStore)
        return;

    if (m_actionStore) {
        disconnect(m_actionStore, &QQuickActionStore::actionEvent, this, nullptr);
        disconnect(m_actionStore, &QQuickActionStore::actionKeyEvent, this, nullptr);
    }
    m_actionStore = actionStore;
    connect(m_actionStore, &QQuickActionStore::actionKeyEvent, this, [this](const QString &action, Qt::Key key, bool isPressed)
            {
                Q_UNUSED(key)
                Q_UNUSED(isPressed)
                if (action == m_actionTitle) {
                    setSource(Source::Key);
                    setValue(1.0f);
                    emit triggered();
                } });
    connect(m_actionStore, &QQuickActionStore::actionMouseButtonEvent, this, [this](const QString &action, Qt::MouseButton button, bool isPressed)
            {
                Q_UNUSED(button)
                Q_UNUSED(isPressed)
                if (action == m_actionTitle) {
                    setSource(Source::MouseButton);
                    setValue(1.0f);
                    emit triggered();
                } });
    connect(m_actionStore, &QQuickActionStore::actionJoyAxisEvent, this, [this](const QString &action, int device, JoyAxis axis, float value)
            {
                Q_UNUSED(device)
                Q_UNUSED(axis)
                if (action == m_actionTitle)
                {
                    setSource(Source::JoyAxis);
                    setValue(value);
                    emit triggered();
                }
            });
    connect(m_actionStore, &QQuickActionStore::actionJoyButtonEvent, this, [this](const QString &action, int device, JoyButton button, bool isPressed)
            {
                Q_UNUSED(device)
                Q_UNUSED(button)
                Q_UNUSED(isPressed)
                if (action == m_actionTitle) {
                    setSource(Source::JoyButton);
                    setValue(1.0f);
                    emit triggered();
                } });

    emit actionStoreChanged();
}

QQuickActionStore *QQuickActionHandler::actionStore() const
{
    return m_actionStore;
}

void QQuickActionHandler::setActionTitle(const QString &actionTitle)
{
    if (m_actionTitle == actionTitle)
        return;

    m_actionTitle = actionTitle;
    emit actionTitleChanged();
}

QString QQuickActionHandler::actionTitle() const
{
    return m_actionTitle;
}

void QQuickActionHandler::setSource(Source source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged();
}

QQuickActionHandler::Source QQuickActionHandler::source() const
{
    return m_source;
}

void QQuickActionHandler::setValue(float value)
{
    if (m_value == value)
        return;

    m_value = value;
    emit valueChanged();
}

float QQuickActionHandler::value() const
{
    return m_value;
}

// ActionHandler
// ActionDispatch

QQuickActionDispatch::QQuickActionDispatch(QObject *parent)
    : QObject(parent), m_actionStore(nullptr)
{
}

void QQuickActionDispatch::setActionStore(QQuickActionStore *actionStore)
{
    if (m_actionStore == actionStore)
        return;

    m_actionStore = actionStore;
    for (QQuickActionHandler *handler : m_handlers)
        handler->setActionStore(m_actionStore);

    emit actionStoreChanged();
}

QQuickActionStore *QQuickActionDispatch::actionStore() const
{
    return m_actionStore;
}

QQmlListProperty<QQuickActionHandler> QQuickActionDispatch::handlers()
{
    return QQmlListProperty<QQuickActionHandler>(this, nullptr, &QQuickActionDispatch::appendActionHandler,
                                                 &QQuickActionDispatch::countActionHandler,
                                                 &QQuickActionDispatch::atActionHandler,
                                                 &QQuickActionDispatch::clearActionHandler);
}

void QQuickActionDispatch::appendActionHandler(QQmlListProperty<QQuickActionHandler> *list, QQuickActionHandler *action)
{
    auto *dispatch = static_cast<QQuickActionDispatch *>(list->object);
    dispatch->m_handlers.append(action);
    action->setActionStore(dispatch->m_actionStore);
    emit dispatch->handlersChanged();
}

qsizetype QQuickActionDispatch::countActionHandler(QQmlListProperty<QQuickActionHandler> *list)
{
    auto *dispatch = static_cast<QQuickActionDispatch *>(list->object);
    return dispatch->m_handlers.count();
}

QQuickActionHandler *QQuickActionDispatch::atActionHandler(QQmlListProperty<QQuickActionHandler> *list, qsizetype index)
{
    auto *dispatch = static_cast<QQuickActionDispatch *>(list->object);
    return dispatch->m_handlers.at(index);
}

void QQuickActionDispatch::clearActionHandler(QQmlListProperty<QQuickActionHandler> *list)
{
    auto *dispatch = static_cast<QQuickActionDispatch *>(list->object);
    for (QQuickActionHandler *handler : dispatch->m_handlers)
        handler->setActionStore(nullptr);
    dispatch->m_handlers.clear();
    emit dispatch->handlersChanged();
}

QT_END_NAMESPACE
