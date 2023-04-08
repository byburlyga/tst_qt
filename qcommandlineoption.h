/****************************************************************************
**
** Copyright (C) 2013 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QCOMMANDLINEOPTION_H
#define QCOMMANDLINEOPTION_H

#include <memory>
#include <string>
#include <vector>

class QCommandLineOptionPrivate;

class QCommandLineOption
{
public:
    explicit QCommandLineOption(const std::string &name);
    explicit QCommandLineOption(const std::vector<std::string> &names);
    /*implicit*/ QCommandLineOption(const std::string &name, const std::string &description,
                                const std::string &valueName = std::string(),
                                const std::string &defaultValue = std::string());
    /*implicit*/ QCommandLineOption(const std::vector<std::string> &names, const std::string &description,
                                const std::string &valueName = std::string(),
                                const std::string &defaultValue = std::string());
    QCommandLineOption(const QCommandLineOption &other);

    ~QCommandLineOption();

    QCommandLineOption &operator=(const QCommandLineOption &other);
#ifdef Q_COMPILER_RVALUE_REFS
    QCommandLineOption &operator=(QCommandLineOption &&other) Q_DECL_NOTHROW { swap(other); return *this; }
#endif

    void swap(QCommandLineOption &other)
    { std::swap(d, other.d); }

    std::vector<std::string> names() const;

    void setValueName(const std::string &name);
    std::string valueName() const;

    void setDescription(const std::string &description);
    std::string description() const;

    void setDefaultValue(const std::string &defaultValue);
    void setDefaultValues(const std::vector<std::string> &defaultValues);
    std::vector<std::string> defaultValues() const;

    void setHidden(bool hidden);
    bool isHidden() const;

private:
    std::shared_ptr<QCommandLineOptionPrivate> d;
};

#endif // QCOMMANDLINEOPTION_H
