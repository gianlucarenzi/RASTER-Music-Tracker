// RmtQtSettings.cpp - the configuration of the Qt frontend in QSettings
//
// CRmtView (RmtView.cpp) reads and writes the configuration and the tuning
// as the "NAME = value" lines of rmt.ini / tuning.ini; here each line is a
// key of QSettings, in the group named after the file ("rmt", "tuning"). The
// settings belong to the user on the host system, not to the program folder,
// so an update or a new installation of RMT keeps them:
//   Linux    ~/.config/raster-atari.org/rmt.conf ($XDG_CONFIG_HOME)
//   Windows  registry, HKEY_CURRENT_USER\Software\raster-atari.org\rmt
//   macOS    ~/Library/Preferences/org.raster-atari.rmt.plist
// With nothing saved yet, an rmt.ini / tuning.ini next to the program (the
// earlier versions kept them there) is taken over once.

#include "StdAfx.h"
#include "Global.h"

#include <QFileInfo>
#include <QSettings>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

static QSettings& Settings()
{
    static QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "raster-atari.org", "rmt");
    return settings;
}

static QString Group(const char* fileName)
{
    return QFileInfo(QString::fromLocal8Bit(fileName)).completeBaseName();
}

bool RmtLoadConfigText(const char* fileName, std::string& text)
{
    QSettings& settings = Settings();
    settings.beginGroup(Group(fileName));
    const QStringList keys = settings.childKeys();
    std::ostringstream lines;
    for (const QString& key : keys)
        lines << key.toLocal8Bit().constData() << " = " << settings.value(key).toString().toLocal8Bit().constData() << "\n";
    settings.endGroup();
    if (!keys.isEmpty()) {
        text = lines.str();
        return true;
    }

    // First start: the file of an earlier version, if there is one
    std::ifstream in(GetResourceFilePath(std::filesystem::path(""), fileName));
    if (!in) return false;
    std::ostringstream all;
    all << in.rdbuf();
    text = all.str();
    return true;
}

bool RmtSaveConfigText(const char* fileName, const std::string& text)
{
    QSettings& settings = Settings();
    settings.remove(Group(fileName));
    settings.beginGroup(Group(fileName));
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        // "NAME = value", as CRmtView reads it: the value starts after "= "
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos || eq == 0) continue;
        std::string name = line.substr(0, eq - 1);
        std::string value = eq + 2 <= line.size() ? line.substr(eq + 2) : "";
        settings.setValue(QString::fromLocal8Bit(name.c_str()), QString::fromLocal8Bit(value.c_str()));
    }
    settings.endGroup();
    settings.sync();
    return settings.status() == QSettings::NoError;
}

CString RmtConfigTextLocation(const char* fileName)
{
    return CString((Settings().fileName() + " [" + Group(fileName) + "]").toLocal8Bit().constData());
}
