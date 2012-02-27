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
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "mainwindowplugin.h"
#include "mainwindow.h"
#include "mainwindowpreferences.h"

#include <coreplugin/dialogs/pluginaboutpage.h>
#include <coreplugin/icore.h>
#include <coreplugin/itheme.h>
#include <coreplugin/translators.h>

#include <utils/log.h>

#include <QtCore/QtPlugin>

#include <QDebug>

using namespace MainWin;

static inline Core::ITheme *theme()  { return Core::ICore::instance()->theme(); }
static inline void messageSplash(const QString &s) {theme()->messageSplashScreen(s); }

MainWinPlugin::MainWinPlugin() :
        m_MainWindow(0), prefPage(0), virtualBasePage(0)
{
    if (Utils::Log::warnPluginsCreation())
        qWarning() << "creating FREEMEDFORMS::MainWinPlugin";

    // Add Translator to the Application
    Core::ICore::instance()->translators()->addNewTranslator("fmfmainwindowplugin");

    // Declare MainWindow to the Core
    m_MainWindow = new MainWindow;
    Core::ICore::instance()->setMainWindow(m_MainWindow);
    m_MainWindow->init();

    virtualBasePage = new Internal::VirtualPatientBasePage();
    addObject(virtualBasePage);
}

MainWinPlugin::~MainWinPlugin()
{
    qWarning() << "MainWinPlugin::~MainWinPlugin()";
    if (virtualBasePage) {
        removeObject(virtualBasePage);
        delete virtualBasePage; virtualBasePage=0;
    }
    // m_MainWindow is deleted by Core
}

bool MainWinPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    if (Utils::Log::warnPluginsCreation())
        qWarning() << "FREEMEDFORMS::MainWinPlugin::initialize";

    Q_UNUSED(arguments);
    Q_UNUSED(errorString);

    messageSplash(tr("Initializing main window plugin..."));

    m_MainWindow->initialize(arguments, errorString);
    return true;
}

void MainWinPlugin::extensionsInitialized()
{
    if (Utils::Log::warnPluginsCreation())
        qWarning() << "FREEMEDFORMS::MainWinPlugin::extensionsInitialized";

    messageSplash(tr("Initializing main window plugin..."));
    addAutoReleasedObject(new Core::PluginAboutPage(pluginSpec(), this));

    // Add preferences pages
    virtualBasePage->checkSettingsValidity();

    m_MainWindow->extensionsInitialized();
}

Q_EXPORT_PLUGIN(MainWinPlugin)
