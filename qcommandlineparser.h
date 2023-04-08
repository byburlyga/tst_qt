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

#ifndef QCOMMANDLINEPARSER_H
#define QCOMMANDLINEPARSER_H

//#include "std::vector<std::string>.h"

//#include "qcoreapplication.h"
#include "qcommandlineoption.h"

//QT_BEGIN_NAMESPACE

class QCommandLineParserPrivate;
//class QCoreApplication;

class QCommandLineParser
{
//    Q_DECLARE_TR_FUNCTIONS(QCommandLineParser)
public:
    QCommandLineParser();
    ~QCommandLineParser();

    enum SingleDashWordOptionMode {
        ParseAsCompactedShortOptions,
        ParseAsLongOptions
    };
    void setSingleDashWordOptionMode(SingleDashWordOptionMode parsingMode);

    enum OptionsAfterPositionalArgumentsMode {
        ParseAsOptions,
        ParseAsPositionalArguments
    };
    void setOptionsAfterPositionalArgumentsMode(OptionsAfterPositionalArgumentsMode mode);

    bool addOption(const QCommandLineOption &commandLineOption);
    bool addOptions(const std::vector<QCommandLineOption> &options);

    QCommandLineOption addVersionOption();
    QCommandLineOption addHelpOption();
    void setApplicationDescription(const std::string &description);
    std::string applicationDescription() const;
    void addPositionalArgument(const std::string &name, const std::string &description, const std::string &syntax = "");
    void clearPositionalArguments();

    void process(const std::vector<std::string> &arguments);
    void process();
//    void process(const QCoreApplication &app);

    bool parse(const std::vector<std::string> &arguments);
    std::string errorText() const;

    bool isSet(const std::string &name) const;
    std::string value(const std::string &name) const;
    std::vector<std::string> values(const std::string &name) const;

    bool isSet(const QCommandLineOption &option) const;
    std::string value(const QCommandLineOption &option) const;
    std::vector<std::string> values(const QCommandLineOption &option) const;

    std::vector<std::string> positionalArguments() const;
    std::vector<std::string> optionNames() const;
    std::vector<std::string> unknownOptionNames() const;

    void showVersion();
    void showHelp(int exitCode = 0);
    std::string helpText() const;

private:
//    Q_DISABLE_COPY(QCommandLineParser)

    QCommandLineParserPrivate * const d;
};

#endif // QCOMMANDLINEPARSER_H
