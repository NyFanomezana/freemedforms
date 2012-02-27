/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2012 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *   Main Developer : Eric MAEKER, <eric.maeker@gmail.com>                *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/

/**
  \class Core::ITheme
  \brief this is the theme manager. Actually manages only the icons of the app.
  You first need to instanciate it, inform it of path to use. Then ask the needed QIcon.\n
  There is a cache of asked icons. Limit of the cache can be dynamycally set using setCacheMaxCost().

  \sa constants_theme
  \todo write full documentation
*/

#include "theme.h"

#include <translationutils/constanttranslations.h>
#include <utils/log.h>

#include <QCache>
#include <QString>
#include <QApplication>
#include <QDir>
#include <QDesktopWidget>

using namespace Core;
using namespace Core::Internal;

class ThemePrivatePrivate
{
public:
    QIcon icon(const QString &fileName, ThemePrivate::IconSize size);
    QString iconFullPath(const QString &fileName, ThemePrivate::IconSize size);
    QString transformFileName(const QString &fileName, ThemePrivate::IconSize size);

};

static QString transformFileName(const QString &fileName, ThemePrivate::IconSize size)
{
    switch (size) {
        case ThemePrivate::SmallIcon : return fileName+"__S__";
        case ThemePrivate::MediumIcon : return fileName+"__M__";
        case ThemePrivate::BigIcon : return fileName+"__B__";
    }
    return QString::null;
}


ThemePrivate::ThemePrivate(QObject *parent, const int cacheSize)
    : ITheme(parent), m_Splash(0)
{
    if (!parent)
        setParent(qApp);
    setObjectName("ThemePrivate");
//    Log::instance()->addObjectWatcher(this);
    m_IconCache.setMaxCost(cacheSize);
}

ThemePrivate::~ThemePrivate()
{
    if (m_Splash)
        delete m_Splash;
    m_Splash = 0;
}

void ThemePrivate::setThemeRootPath(const QString &absPath)
{
    // guesses all pixmap size paths
    if (QDir(absPath).exists()) {
        m_AbsolutePath = QDir::cleanPath(absPath);
        LOG(QString("Setting theme path to : %1").arg(m_AbsolutePath));
    }
    else
        LOG_ERROR(Trans::ConstantTranslations::tkTr(Trans::Constants::PATH_1_DOESNOT_EXISTS).arg(absPath));
}

void ThemePrivate::setThemeRelativeRootPath(const QString &relPathFromAppBinary)
{
    // guesses all pixmap size paths
    QString path = QDir::cleanPath(qApp->applicationDirPath() + QDir::separator() + relPathFromAppBinary);
    if (QDir(path).exists()) {
        m_AbsolutePath = path;
        Utils::Log::addMessage(this, QString("INFO : theme path set to : %1").arg(path));
    }
    else
        LOG_ERROR(Trans::ConstantTranslations::tkTr(Trans::Constants::PATH_1_DOESNOT_EXISTS).arg(relPathFromAppBinary));
}

void ThemePrivate::refreshCache()
{
    /** \todo */
}

/** \brief Defines the max number of icons in the cache */
void ThemePrivate::setCacheMaxCost(const int max)
{
    m_IconCache.setMaxCost(max);
}

/** \brief Set the specific small icon path - should not be used */
void ThemePrivate::setSmallIconPath(const QString &absPath)
{
    if (QDir(absPath).exists())
        m_SmallIconPath = absPath;
    else
        LOG_ERROR(Trans::ConstantTranslations::tkTr(Trans::Constants::PATH_1_DOESNOT_EXISTS).arg("SmallIcon : "+absPath));
}

/** \brief Set the specific medium icon path - should not be used */
void ThemePrivate::setMediumIconPath(const QString &absPath)
{
    if (QDir(absPath).exists())
        m_MediumIconPath = absPath;
    else
        LOG_ERROR(Trans::ConstantTranslations::tkTr(Trans::Constants::PATH_1_DOESNOT_EXISTS).arg("MediumIcon : "+absPath));
}

/** \brief Set the specific big icon path - should not be used */
void ThemePrivate::setBigIconPath(const QString &absPath)
{
    if (QDir(absPath).exists())
        m_BigIconPath = absPath;
    else
        LOG_ERROR(Trans::ConstantTranslations::tkTr(Trans::Constants::PATH_1_DOESNOT_EXISTS).arg("BigIcon : "+absPath));
}

/** \brief Returns the icon corresponding to the themed file name \e fileName and the size \e size */
QIcon ThemePrivate::icon(const QString &fileName, IconSize size)
{
    Q_ASSERT_X(!m_AbsolutePath.isEmpty(), "ThemePrivate::icon", "No path set");
    // retreive transformed FileName (manage size)
    QString transformedFileName = transformFileName(fileName, size);

    // 0. get icon from cache is possible
    if (m_IconCache.contains(transformedFileName))
        return QIcon(*m_IconCache[transformedFileName]);

    // 1. test size by size if a path was set else use the absolutePath of the theme
    QIcon *i = 0;

    // 2. get icom from file
    QString fullName = iconFullPath(fileName,size);
    if (QFile(fullName).exists()) {
        i = new QIcon(fullName);
        if (!i->isNull()) {
            m_IconCache.insert(transformedFileName, i);
            return QIcon(*i);
        } else {
            LOG_ERROR(QCoreApplication::translate("ThemePrivate", "ERROR - Theme : Unable to load icon file %1").arg(fileName));
        }
    }
    return QIcon();
}

/** \brief Returns the full icon's file name absolute path corresponding to the themed file name \e fileName and the size \e size */
QString ThemePrivate::iconFullPath(const QString &fileName, IconSize size)
{
    QString path = m_AbsolutePath + QDir::separator() + "pixmap" + QDir::separator();
    if (size == ThemePrivate::SmallIcon) {
        if (!m_SmallIconPath.isEmpty())
            path = m_SmallIconPath;
        else
            path += "16x16";
    }
    else if (size == ThemePrivate::MediumIcon) {
        if (!m_MediumIconPath.isEmpty())
            path = m_MediumIconPath;
        else
            path += "32x32";
    }
    else if (size == ThemePrivate::BigIcon) {
        if (!m_BigIconPath.isEmpty())
            path = m_BigIconPath;
        else
            path += "64x64";
    }
    path = QDir::cleanPath(path) + QDir::separator() + fileName;
    return path;
}

/** \brief Returns the themed splashscreen \e fileName. */
QPixmap ThemePrivate::splashScreenPixmap(const QString &fileName, const IconSize size)
{
    QString file = fileName;
    QString extra;
    switch (QDate::currentDate().month()) {
    case 1: extra = "-birthday"; break;
    case 7:
    case 8: extra = "-summer"; break;
    case 10 : extra = "-halloween"; break;
    case 12 : extra = "-christmas"; break;
    }
    // Append size
    QFileInfo fi(file);
    if (size==BigIcon) {
        QString tmp = fi.baseName() + extra + "-600." + fi.completeSuffix();
        if (QFile(m_AbsolutePath + "/pixmap/splashscreens/" + tmp).exists()) {
            file = tmp;
        } else {
            tmp = fi.baseName() + "-600." + fi.completeSuffix();
            if (QFile(m_AbsolutePath + "/pixmap/splashscreens/" + tmp).exists()) {
                file = tmp;
            } else {
                file = fileName;
            }
        }
    }

    // return themed splashscreen
    if (QFile(m_AbsolutePath + "/pixmap/splashscreens/" + file).exists())
        return QPixmap(m_AbsolutePath + "/pixmap/splashscreens/" + file);
    else
        Utils::Log::addError("ThemePrivate", QString("SplashScreen file does not exists %1").arg(m_AbsolutePath + "/pixmap/splashscreens/" + fileName),
                              __FILE__, __LINE__);
    return QPixmap();
}

void ThemePrivate::createSplashScreen(const QString &fileName)
{
    if (!m_Splash) {
        if (qApp->desktop()->screenGeometry().width() > 1024) {
            m_Splash = new QSplashScreen(splashScreenPixmap(fileName, BigIcon));
        } else {
            m_Splash = new QSplashScreen(splashScreenPixmap(fileName));
        }
        QFont ft(m_Splash->font());
        ft.setPointSize(ft.pointSize() - 2);
        ft.setBold(true);
        m_Splash->setFont(ft);
        m_Splash->show();
    }
}

void ThemePrivate::messageSplashScreen(const QString &msg)
{
    Q_ASSERT(m_Splash);
    if (m_Splash)
        m_Splash->showMessage(msg, Qt::AlignLeft | Qt::AlignBottom, Qt::black);
}

void ThemePrivate::finishSplashScreen(QWidget *widget)
{
    Q_ASSERT(m_Splash);
    if (m_Splash) {
        m_Splash->finish(widget);
        delete m_Splash;
        m_Splash = 0;
    }
}
