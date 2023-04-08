/****************************************************************************
**
** Copyright (C) 2013 Laszlo Papp <lpapp@kde.org>
** Copyright (C) 2013 David Faure <faure@kde.org>
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

#include "qcommandlineoption.h"

#include <algorithm>
#include <iostream>
#include <set>

class QCommandLineOptionPrivate
{
public:
    explicit QCommandLineOptionPrivate(const std::string &name)
        : hidden(false)
    {
        names.push_back(name);
        names = removeInvalidNames(names);
    }

    explicit QCommandLineOptionPrivate(const std::vector<std::string> &names)
        : names(removeInvalidNames(names)),
          hidden(false)
    { }

    static std::vector<std::string> removeInvalidNames(std::vector<std::string> nameList);

    //! The list of names used for this option.
    std::vector<std::string> names;

    //! The documentation name for the value, if one is expected
    //! Example: "-o <file>" means valueName == "file"
    std::string valueName;

    //! The description used for this option.
    std::string description;

    //! The list of default values used for this option.
    std::vector<std::string> defaultValues;

    //! Show or hide in --help
    bool hidden;
};

QCommandLineOption::QCommandLineOption(const std::string &name)
    : d(new QCommandLineOptionPrivate(name))
{
}

QCommandLineOption::QCommandLineOption(const std::vector<std::string> &names)
    : d(new QCommandLineOptionPrivate(names))
{
}

QCommandLineOption::QCommandLineOption(const std::string &name, const std::string &description,
                                       const std::string &valueName,
                                       const std::string &defaultValue)
    : d(new QCommandLineOptionPrivate(name))
{
    setValueName(valueName);
    setDescription(description);
    setDefaultValue(defaultValue);
}

QCommandLineOption::QCommandLineOption(const std::vector<std::string> &names, const std::string &description,
                                       const std::string &valueName,
                                       const std::string &defaultValue)
    : d(new QCommandLineOptionPrivate(names))
{
    setValueName(valueName);
    setDescription(description);
    setDefaultValue(defaultValue);
}

QCommandLineOption::QCommandLineOption(const QCommandLineOption &other)
    : d(other.d)
{
}

QCommandLineOption::~QCommandLineOption()
{
}

QCommandLineOption &QCommandLineOption::operator=(const QCommandLineOption &other)
{
    d = other.d;
    return *this;
}

std::vector<std::string> QCommandLineOption::names() const
{
    return d->names;
}

namespace {
    struct IsInvalidName
    {
        typedef bool result_type;
        typedef std::string argument_type;

        result_type operator()(const std::string &name) const
        {
            if (name.empty())
                return warn("be empty");

            const char c = name.at(0);
            if (c == '-')
                return warn("start with a '-'");
            if (c == '/')
                return warn("start with a '/'");
            if (name.find('=') != std::string::npos)
                return warn("contain a '='");

            return false;
        }

        static bool warn(const char *what)
        {
            std::cerr << "QCommandLineOption: Option names cannot " << what << std::endl;
            return true;
        }
    };
} // unnamed namespace

// static
std::vector<std::string> QCommandLineOptionPrivate::removeInvalidNames(std::vector<std::string> nameList)
{
    if (nameList.empty())
        std::cerr << "QCommandLineOption: Options must have at least one name" << std::endl;
    else
        nameList.erase(std::remove_if(nameList.begin(), nameList.end(), IsInvalidName()),
                       nameList.end());
    return nameList;
}

void QCommandLineOption::setValueName(const std::string &valueName)
{
    d->valueName = valueName;
}

std::string QCommandLineOption::valueName() const
{
    return d->valueName;
}

void QCommandLineOption::setDescription(const std::string &description)
{
    d->description = description;
}

std::string QCommandLineOption::description() const
{
    return d->description;
}

void QCommandLineOption::setDefaultValue(const std::string &defaultValue)
{
    std::vector<std::string> newDefaultValues;
    if (!defaultValue.empty()) {
        newDefaultValues.reserve(1);
        newDefaultValues.push_back(defaultValue);
    }
    // commit:
    d->defaultValues.swap(newDefaultValues);
}

void QCommandLineOption::setDefaultValues(const std::vector<std::string> &defaultValues)
{
    d->defaultValues = defaultValues;
}

std::vector<std::string> QCommandLineOption::defaultValues() const
{
    return d->defaultValues;
}

void QCommandLineOption::setHidden(bool hide)
{
    d->hidden = hide;
}

bool QCommandLineOption::isHidden() const
{
    return d->hidden;
}

