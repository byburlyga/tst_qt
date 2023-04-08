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

#include "qcommandlineparser.h"

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

#if defined(_WIN32)
#include <windows.h>
#endif

//QT_BEGIN_NAMESPACE

typedef std::unordered_map<std::string, size_t> NameHash_t;

class QCommandLineParserPrivate
{
public:
    inline QCommandLineParserPrivate()
        : singleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions),
          optionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions),
          builtinVersionOption(false),
          builtinHelpOption(false),
          needsParsing(true)
    { }

    bool parse(const std::vector<std::string> &args);
    void checkParsed(const char *method);
    std::vector<std::string> aliases(const std::string &name) const;
    std::string helpText() const;
    bool registerFoundOption(const std::string &optionName);
    bool parseOptionValue(const std::string &optionName, const std::string &argument,
                          std::vector<std::string>::const_iterator *argumentIterator,
                          std::vector<std::string>::const_iterator argsEnd);

    std::string errorText;

    std::vector<QCommandLineOption> commandLineOptionList;
    NameHash_t nameHash;

    std::unordered_map<size_t, std::vector<std::string>> optionValuesHash;

    std::vector<std::string> optionNames;

    std::vector<std::string> positionalArgumentList;

    std::vector<std::string> unknownOptionNames;

    std::string description;

    struct PositionalArgumentDefinition
    {
        std::string name;
        std::string description;
        std::string syntax;
    };
    std::vector<PositionalArgumentDefinition> positionalArgumentDefinitions;

    QCommandLineParser::SingleDashWordOptionMode singleDashWordOptionMode;

    QCommandLineParser::OptionsAfterPositionalArgumentsMode optionsAfterPositionalArgumentsMode;

    bool builtinVersionOption;

    bool builtinHelpOption;

    bool needsParsing;
};

std::vector<std::string> QCommandLineParserPrivate::aliases(const std::string &optionName) const
{
    const NameHash_t::const_iterator it = nameHash.find(optionName);
    if (it == nameHash.cend()) {
        std::cerr << "QCommandLineParser: option not defined: \"" << optionName << "\"" << std::endl;
        return std::vector<std::string>();
    }
    return commandLineOptionList.at(it->second).names();
}

QCommandLineParser::QCommandLineParser()
    : d(new QCommandLineParserPrivate)
{
}

QCommandLineParser::~QCommandLineParser()
{
    delete d;
}

void QCommandLineParser::setSingleDashWordOptionMode(QCommandLineParser::SingleDashWordOptionMode singleDashWordOptionMode)
{
    d->singleDashWordOptionMode = singleDashWordOptionMode;
}

void QCommandLineParser::setOptionsAfterPositionalArgumentsMode(QCommandLineParser::OptionsAfterPositionalArgumentsMode parsingMode)
{
    d->optionsAfterPositionalArgumentsMode = parsingMode;
}

bool QCommandLineParser::addOption(const QCommandLineOption &option)
{
    const std::vector<std::string> optionNames = option.names();

    if (!optionNames.empty()) {
        for (const std::string &name : optionNames) {
            auto search = d->nameHash.find(name);
            if (search != d->nameHash.end())
                return false;
        }


        d->commandLineOptionList.push_back(option);

        const size_t offset = d->commandLineOptionList.size() - 1;
        for (const std::string &name : optionNames)
            d->nameHash.insert({name, offset});

        return true;
    }

    return false;
}

bool QCommandLineParser::addOptions(const std::vector<QCommandLineOption> &options)
{
    // should be optimized (but it's no worse than what was possible before)
    bool result = true;
    for (auto it = options.begin(), end = options.end(); it != end; ++it)
        result &= addOption(*it);
    return result;
}

QCommandLineOption QCommandLineParser::addVersionOption()
{
    std::vector<std::string> list;
    list.push_back("v");
    list.push_back("version");
    QCommandLineOption opt(list, "Displays version information.");
    addOption(opt);
    d->builtinVersionOption = true;
    return opt;
}

QCommandLineOption QCommandLineParser::addHelpOption()
{
    std::vector<std::string> list;
#if defined(_WIN32)
    list.push_back("?");
#endif
    list.push_back("h");
    list.push_back("help");
    QCommandLineOption opt(list, "Displays this help.");
    addOption(opt);
    d->builtinHelpOption = true;
    return opt;
}

void QCommandLineParser::setApplicationDescription(const std::__cxx11::string &description)
{
    d->description = description;
}

std::__cxx11::string QCommandLineParser::applicationDescription() const
{
    return d->description;
}

void QCommandLineParser::addPositionalArgument(const std::__cxx11::string &name, const std::__cxx11::string &description, const std::__cxx11::string &syntax)
{
    QCommandLineParserPrivate::PositionalArgumentDefinition arg;
    arg.name = name;
    arg.description = description;
    arg.syntax = syntax.empty() ? name : syntax;
    d->positionalArgumentDefinitions.push_back(arg);
}

void QCommandLineParser::clearPositionalArguments()
{
    d->positionalArgumentDefinitions.clear();
}

bool QCommandLineParser::parse(const std::vector<std::__cxx11::string> &arguments)
{
    return d->parse(arguments);
}

std::string QCommandLineParser::errorText() const
{
    if (!d->errorText.empty())
        return d->errorText;
    if (d->unknownOptionNames.size() == 1)
        return "Unknown option '" + d->unknownOptionNames.front() + "'.";
    if (d->unknownOptionNames.size() > 1) {
        std::string result;
        for (auto it = d->unknownOptionNames.begin(), end = d->unknownOptionNames.end(); it != end; ++it)
            result += (*it);
        return "Unknown options: " + result + ".";
    }
    return std::string();
}

enum MessageType { UsageMessage, ErrorMessage };

#if defined(_WIN32) && !defined(QT_BOOTSTRAPPED) && !defined(Q_OS_WINCE) && !defined(Q_OS_WINRT)
// Return whether to use a message box. Use handles if a console can be obtained
// or we are run with redirected handles (for example, by QProcess).
static inline bool displayMessageBox()
{
    if (GetConsoleWindow())
        return false;
    STARTUPINFO startupInfo;
    startupInfo.cb = sizeof(STARTUPINFO);
    GetStartupInfo(&startupInfo);
    return !(startupInfo.dwFlags & STARTF_USESTDHANDLES);
}
#endif // _WIN32 && !QT_BOOTSTRAPPED && !Q_OS_WINCE && !Q_OS_WINRT

static void showParserMessage(const std::string &message, MessageType type) {
#if defined(Q_OS_WINRT)
    if (type == UsageMessage)
        qInfo(qPrintable(message));
    else
        qCritical(qPrintable(message));
    return;
#elif defined(_WIN32) && !defined(QT_BOOTSTRAPPED) && !defined(Q_OS_WINCE)
    if (displayMessageBox()) {
        const UINT flags = MB_OK | MB_TOPMOST | MB_SETFOREGROUND | (type == UsageMessage ? MB_ICONINFORMATION : MB_ICONERROR);
        char buffer[MAX_PATH];
        GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wmessage = converter.from_bytes(message);
        std::wstring wtitle = converter.from_bytes(buffer);
        MessageBoxW(0, wmessage.c_str(), wtitle.c_str(), flags);
        return;
    }
#endif // _WIN32 && !QT_BOOTSTRAPPED && !Q_OS_WINCE
    fputs(message.c_str(), type == UsageMessage ? stdout : stderr);
}

void QCommandLineParser::process(const std::vector<std::__cxx11::string> &arguments)
{
    if (!d->parse(arguments)) {
        showParserMessage(errorText() + '\n', ErrorMessage);
        ::exit(EXIT_FAILURE);
    }

    if (d->builtinVersionOption && isSet("version"))
        showVersion();

    if (d->builtinHelpOption && isSet("help"))
        showHelp(EXIT_SUCCESS);
}

void QCommandLineParser::process()
{
#if defined(_WIN32)
int argc = 0;
LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
std::vector<std::string> args;
for (int i = 0; i < argc; ++i) {
const int size_needed = WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, NULL, 0, NULL, NULL);
std::string arg(size_needed, 0);
WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, &arg[0], size_needed, NULL, NULL);
args.push_back(arg);
}
LocalFree(argv);
process(args);
#endif
}

void QCommandLineParserPrivate::checkParsed(const char *method)
{
if (needsParsing)
std::cerr << "QCommandLineParser: call process() or parse() before " << method << std::endl;
}

bool QCommandLineParserPrivate::registerFoundOption(const std::string &optionName)
{
auto search = nameHash.find(optionName);
if (search != nameHash.end()) {
optionNames.push_back(optionName);
return true;
} else {
unknownOptionNames.push_back(optionName);
return false;
}
}

bool QCommandLineParserPrivate::parseOptionValue(const std::string &optionName, const std::string &argument,
std::vector<std::string>::const_iterator *argumentIterator, std::vector<std::string>::const_iterator argsEnd)
{
const char assignChar('=');
const NameHash_t::const_iterator nameHashIt = nameHash.find(optionName);
if (nameHashIt!=nameHash.end()) {
const size_t assignPos = argument.find(assignChar);
const NameHash_t::mapped_type optionOffset = (*nameHashIt).second;
const bool withValue = !commandLineOptionList.at(optionOffset).valueName().empty();
if (withValue) {
if (assignPos==std::string::npos) {
++(*argumentIterator);
if (*argumentIterator == argsEnd) {
errorText = "Missing value after '" + argument + "%1" + "'.";
return false;
}
optionValuesHash[optionOffset].push_back(*(*argumentIterator));
}else{
optionValuesHash[optionOffset].push_back(argument.substr(assignPos + 1, argument.size()));
}
}else{if(assignPos != std::string::npos) { errorText = "Unexpected value after '" + argument.substr(assignPos) + "%1" + "'.";
return false;
}
}
}
return true;
}

bool QCommandLineParserPrivate::parse(const std::vector<std::string> &args)
{
    needsParsing = false;
    bool error = false;

    const std::string doubleDashString("--");
    const char dashChar('-');
    const char assignChar('=');

    bool forcePositional = false;
    errorText.clear();
    positionalArgumentList.clear();
    optionNames.clear();
    unknownOptionNames.clear();
    optionValuesHash.clear();

    if (args.empty()) {
        std::cerr << "QCommandLineParser: argument list cannot be empty, it should contain at least the executable name" << std::endl;
        return false;
    }

    std::vector<std::string>::const_iterator argumentIterator = args.begin();
    ++argumentIterator; // skip executable name

    for (; argumentIterator != args.end() ; ++argumentIterator) {
        std::string argument = *argumentIterator;

        if (forcePositional) {
            positionalArgumentList.push_back(argument);
        } else if (argument.rfind(doubleDashString, 0) == 0) {
            if (argument.length() > 2) {
                std::string optionName = argument.substr(2, argument.size());
                optionName = optionName.substr(0, optionName.find(assignChar));
                if (registerFoundOption(optionName)) {
                    if (!parseOptionValue(optionName, argument, &argumentIterator, args.end()))
                        error = true;
                } else {
                    error = true;
                }
            } else {
                forcePositional = true;
            }
        } else if (argument.rfind(dashChar, 0) == 0) {
            if (argument.size() == 1) { // single dash ("stdin")
                positionalArgumentList.push_back(argument);
                continue;
            }
            switch (singleDashWordOptionMode) {
            case QCommandLineParser::ParseAsCompactedShortOptions:
            {
                std::string optionName;
                bool valueFound = false;
for (size_t pos = 1 ; pos < argument.size(); ++pos) {
optionName = argument.substr(pos, 1);
if (!registerFoundOption(optionName)) {
error = true;
} else {
const NameHash_t::const_iterator nameHashIt = nameHash.find(optionName);
const NameHash_t::mapped_type optionOffset = (*nameHashIt).second;
const bool withValue = !commandLineOptionList.at(optionOffset).valueName().empty();
if (withValue) {
if (pos + 1 < argument.size()) {
if (argument.at(pos + 1) == assignChar)
++pos;
optionValuesHash[optionOffset].push_back(argument.substr(pos + 1, argument.size()));
valueFound = true;
}
break;
}
if (pos + 1 < argument.size() && argument.at(pos + 1) == assignChar)
break;
                    }
                }
                if (!valueFound && !parseOptionValue(optionName, argument, &argumentIterator, args.end()))
                    error = true;
                break;
            }
            case QCommandLineParser::ParseAsLongOptions:
            {
                std::string optionName = argument.substr(1, argument.size());
                optionName = optionName.substr(0, optionName.find(assignChar));
                if (registerFoundOption(optionName)) {
                    if (!parseOptionValue(optionName, argument, &argumentIterator, args.end()))
                        error = true;
                } else {
                    error = true;
                }
                break;
            }
            }
        } else {
            positionalArgumentList.push_back(argument);
            if (optionsAfterPositionalArgumentsMode == QCommandLineParser::ParseAsPositionalArguments)
                forcePositional = true;
        }
        if (argumentIterator == args.end())
            break;
    }
    return !error;
}

bool QCommandLineParser::isSet(const std::string &name) const
{
    d->checkParsed("isSet");
    auto result1 = std::find(d->optionNames.begin(), d->optionNames.end(), name);
    if (result1 != d->optionNames.end())
        return true;
    const std::vector<std::string> aliases = d->aliases(name);
    for (const std::string &optionName : static_cast<const std::vector<std::string>&>(d->optionNames)) {
        auto result2 = std::find(aliases.begin(), aliases.end(), optionName);
        if (result2 != aliases.end())
            return true;
    }
    return false;
}

std::string QCommandLineParser::value(const std::string &optionName) const
{
    d->checkParsed("value");
    const std::vector<std::string> valueList = values(optionName);

    if (!valueList.empty())
        return valueList.back();

    return std::string();
}

std::vector<std::string> QCommandLineParser::values(const std::string &optionName) const
{
    d->checkParsed("values");
    const NameHash_t::const_iterator it = d->nameHash.find(optionName);
    if (it != d->nameHash.cend()) {
        const size_t optionOffset = (*it).second;
        std::vector<std::string> values;
        auto it = d->optionValuesHash.find(optionOffset);
        if (it != d->optionValuesHash.cend())
            values = it->second;
        if (values.empty())
            values = d->commandLineOptionList.at(optionOffset).defaultValues();
        return values;
    }

    std::cerr << "QCommandLineParser: option not defined: \"" << optionName << "\"" << std::endl;
    return std::vector<std::string>();
}

bool QCommandLineParser::isSet(const QCommandLineOption &option) const
{
    // option.names() might be empty if the constructor failed
    const auto names = option.names();
    return !names.empty() && isSet(names.front());
}

std::string QCommandLineParser::value(const QCommandLineOption &option) const
{
    return value(option.names().front());
}

std::vector<std::string> QCommandLineParser::values(const QCommandLineOption &option) const
{
    return values(option.names().front());
}

std::vector<std::string> QCommandLineParser::positionalArguments() const
{
    d->checkParsed("positionalArguments");
    return d->positionalArgumentList;
}

std::vector<std::string> QCommandLineParser::optionNames() const
{
    d->checkParsed("optionNames");
    return d->optionNames;
}

std::vector<std::string> QCommandLineParser::unknownOptionNames() const
{
    d->checkParsed("unknownOptionNames");
    return d->unknownOptionNames;
}

void QCommandLineParser::showVersion()
{
    ::exit(EXIT_SUCCESS);
}

void QCommandLineParser::showHelp(int exitCode)
{
    showParserMessage(d->helpText(), UsageMessage);
    ::exit(exitCode);
}

/*!
    Returns a string containing the complete help information.

    \sa showHelp()
*/
std::string QCommandLineParser::helpText() const
{
    return d->helpText();
}

static std::string wrapText(const std::string &names, size_t longestOptionNameString, const std::string &description)
{
    const char nl('\n');
    std::ostringstream oss;
    oss << std::setw(longestOptionNameString) << std::left << names;
    std::string text = std::string("  ") + oss.str() + char(' ');
    const int indent = text.length();
    int lineStart = 0;
    int lastBreakable = -1;
    const int max = 79 - indent;
    int x = 0;
    const int len = description.length();

    for (int i = 0; i < len; ++i) {
        ++x;
        const char c = description.at(i);
        if (isspace(c))
            lastBreakable = i;

        int breakAt = -1;
        int nextLineStart = -1;
        if (x > max && lastBreakable != -1) {
            // time to break and we know where
            breakAt = lastBreakable;
            nextLineStart = lastBreakable + 1;
        } else if ((x > max - 1 && lastBreakable == -1) || i == len - 1) {
            // time to break but found nowhere [-> break here], or end of last line
            breakAt = i + 1;
            nextLineStart = breakAt;
        } else if (c == nl) {
            // forced break
            breakAt = i;
            nextLineStart = i + 1;
        }

        if (breakAt != -1) {
            const int numChars = breakAt - lineStart;
            //qDebug() << "breakAt=" << description.at(breakAt) << "breakAtSpace=" << breakAtSpace << lineStart << "to" << breakAt << description.mid(lineStart, numChars);
            if (lineStart > 0)
                text += std::string(indent, ' ');
            text += description.substr(lineStart, numChars) + nl;
            x = 0;
            lastBreakable = -1;
            lineStart = nextLineStart;
            if (lineStart < len && isspace(description.at(lineStart)))
                ++lineStart; // don't start a line with a space
            i = lineStart;
        }
    }

    return text;
}

std::string QCommandLineParserPrivate::helpText() const
{
const char nl('\n');
std::string text;
std::string usage;
usage += std::string("[executable name]");
if (!commandLineOptionList.empty())
usage += std::string(" ") + "[options]";
for (const PositionalArgumentDefinition &arg : positionalArgumentDefinitions)
usage += std::string(" ") + arg.syntax;
text += "Usage: " + usage + nl;
if (!description.empty())
text += description + nl;
text += nl;
if (!commandLineOptionList.empty())
text += std::string("Options:") + nl;
std::vector<std::string> optionNameList;
optionNameList.reserve(commandLineOptionList.size());
size_t longestOptionNameString = 0;
for (const QCommandLineOption &option : commandLineOptionList) {
if (option.isHidden())
 continue;
const std::vector<std::string> optionNames = option.names();
std::string optionNamesString;
for (const std::string &optionName : optionNames) {
const size_t numDashes = optionName.length() == 1 ? 1 : 2;
optionNamesString += std::string("--", numDashes) + optionName + std::string(", ");
}
if (!optionNames.empty()) {
optionNamesString.pop_back();
 optionNamesString.pop_back();
}
const auto valueName = option.valueName();
if (!valueName.empty())
optionNamesString += std::string(" <") + valueName + std::string(">");
 optionNameList.push_back(optionNamesString);
longestOptionNameString = std::max(longestOptionNameString, optionNamesString.length());
}
++longestOptionNameString;
auto optionNameIterator = optionNameList.cbegin();
for (const QCommandLineOption &option : commandLineOptionList) {
 if (option.isHidden())
continue;
text += wrapText(*optionNameIterator, longestOptionNameString, option.description());
++optionNameIterator;
}
if (!positionalArgumentDefinitions.empty()) {
if (!commandLineOptionList.empty())
text += nl;
text += std::string("Arguments:") + nl;
for (const PositionalArgumentDefinition &arg : positionalArgumentDefinitions)
text += wrapText(arg.name, longestOptionNameString, arg.description);
}
return text;
}
