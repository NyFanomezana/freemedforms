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
 *   Main Developer: Christian A. Reiter <christian.a.reiter@gmail.com     *
 *   Contributors:                                                         *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#include "genericzipcodesstep.h"

#include <coreplugin/icore.h>
#include <coreplugin/imainwindow.h>
#include <coreplugin/ftb_constants.h>
#include <coreplugin/globaltools.h>
#include <coreplugin/isettings.h>
#include <coreplugin/ftb_constants.h>

#include <utils/global.h>
#include <utils/log.h>
#include <utils/database.h>
#include <utils/httpdownloader.h>
#include <quazip/global.h>

#include <extensionsystem/pluginmanager.h>

#include <QFileInfo>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QDomDocument>
#include <QtXml>
#include <QStandardItemModel>

#include <QDebug>

using namespace ZipCodes;

namespace {
const char* const  DB_NAME     = "ZIPCODES";

static inline Core::ISettings *settings()  { return Core::ICore::instance()->settings(); }

static inline QString databaseAbsPath() {
    return QDir::cleanPath(settings()->value(
                               Core::Constants::S_DBOUTPUT_PATH).toString() +"/zipcodes/zipcodes.db");
}

static inline QString workingPath() {
    return QDir::cleanPath(settings()->value(
                               Core::Constants::S_TMP_PATH).toString() + "/ZipCodes/") + QDir::separator();
}

static inline QString sqlMasterFileAbsPath() {
    return QDir::cleanPath(settings()->value(
                               Core::Constants::S_GITFILES_PATH).toString() + "/global_resources/sql/zipcodes/zipcodes.sql");
}

static inline QString sqlImportFileAbsPath()
{
    return QDir::cleanPath(settings()->value(Core::Constants::S_GITFILES_PATH).toString() +
                           "/global_resources/sql/zipcodes/zipcodes-fr-import.sql");
}

} // end namespace


// ########################## GenericZipCodesStep ##########################

GenericZipCodesStep::GenericZipCodesStep(QObject *parent) :
    Core::IFullReleaseStep(parent),
    m_WithProgress(false),
    m_availableCountriesModel(new QStandardItemModel()),
    m_selectedCountriesModel(new QStandardItemModel())
{
    setObjectName("GenericZipCodesStep");
}

GenericZipCodesStep::~GenericZipCodesStep()
{
}

/*! Downloads the list of available countries. */
bool GenericZipCodesStep::downloadFiles(QProgressBar *bar)
{
    Q_UNUSED(bar);

    //TODO: what to do if download fails? set timeout!
    QNetworkAccessManager *netAccessManager = new QNetworkAccessManager(this);

//    get list of countries that GeoNames has informations for
    QNetworkRequest request;
    request.setUrl(QUrl("http://api.geonames.org/postalCodeCountryInfo?username=freemedforms"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");

    /*QNetworkReply* reply =*/ netAccessManager->get(request);
//    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(slotSetProgress(qint64,qint64)));
    connect(netAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onAvailableCountriesDownloaded(QNetworkReply*)));
    connect(netAccessManager, SIGNAL(finished(QNetworkReply*)), netAccessManager, SLOT(deleteLater()));

    return true;

}

bool GenericZipCodesStep::process()
{
    createDatabaseScheme();
    populateDatabase();
    Q_EMIT processFinished();
    return true;
}


bool GenericZipCodesStep::createDatabaseScheme()
{
    LOG(tr("Creating Generic zipcodes database"));
    Q_EMIT progressLabelChanged(tr("Creating Generic zipcodes database"));
    Q_EMIT progressRangeChanged(0, 1);
    Q_EMIT progress(0);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", DB_NAME);
    db.setDatabaseName(databaseAbsPath());
    if (!db.open()) {
        LOG_ERROR("Unable to open database.");
        return false;
    }

    if (db.tables().contains("ZIPS_IMPORT")) {
        Utils::Database::executeSQL("DROP TABLE ZIPS_IMPORT;", db);
    }
    if (!db.tables().contains("ZIPS")) {
        Utils::Database::executeSqlFile(DB_NAME, sqlMasterFileAbsPath());
    }

    LOG(QString("Database schema created"));
    Q_EMIT progress(1);
    return true;
}

bool GenericZipCodesStep::populateDatabase()
{
    if (!Core::Tools::connectDatabase(DB_NAME, databaseAbsPath()))
        return false;

    Q_EMIT progressLabelChanged(tr("Reading raw sources..."));
    Q_EMIT progressRangeChanged(0, 1);
    Q_EMIT progress(0);

    // import the raw source in memory

    QSqlDatabase db = QSqlDatabase::database(DB_NAME);

    if (!db.tables().contains("ZIPS_IMPORT")) {
        Utils::Database::executeSqlFile(DB_NAME, sqlImportFileAbsPath());
    }

//    if (!Utils::Database::importCsvToDatabase(DB_NAME, csvFileAbsPath(), "ZIPS_IMPORT", ";", true))
//        return false;

    Q_EMIT progressLabelChanged(tr("Reading raw sources..."));
    Q_EMIT progress(1);

    // create the new database
    QString req;

    // remove alread recorded codes for that country
    req = "DELETE FROM `ZIPS` WHERE `ZIPS`.`COUNTRY`='fr';";
    Utils::Database::executeSQL(req, db);

    // import new Generic codes
    // 4 chars
//    select * from zips where length(ZIP)>5;
//    select substr('00000' || ZIP, -5, 5) from ZIPS where zip like'14%';
    req = "INSERT INTO `ZIPS` (`ZIP`,`CITY`,`EXTRACODE`, `COUNTRY`) \n"
          "SELECT substr('00000' || ZIP, -5, 5), `COMMUNE`, `CODEINSEE`, 'fr' FROM `ZIPS_IMPORT` WHERE LENGTH(ZIP)=4 ORDER BY `ZIP`;";
    Utils::Database::executeSQL(req, db);
    // 5 chars
    req = "INSERT INTO `ZIPS` (`ZIP`,`CITY`,`EXTRACODE`, `COUNTRY`) \n"
          "SELECT substr(`ZIP`, 1, 5) , `COMMUNE`, `CODEINSEE`, 'fr' FROM `ZIPS_IMPORT` WHERE LENGTH(ZIP)>=5 ORDER BY `ZIP`;";
    Utils::Database::executeSQL(req, db);

    // clean database
    req = "DROP TABLE ZIPS_IMPORT;";
    Utils::Database::executeSQL(req, db);
    req = "VACUUM;";
    Utils::Database::executeSQL(req, db);

    LOG(QString("Database processed"));

    return true;
}

void GenericZipCodesStep::slotSetProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_UNUSED(bytesReceived)
    Q_UNUSED(bytesTotal)

    //TODO: implementation
}

/*! \brief Called when downloading of countries from GeoNames is finished. */
void GenericZipCodesStep::onAvailableCountriesDownloaded(QNetworkReply *reply)
{
    if (reply->error() > 0) {
        qDebug() << reply->errorString();
    } else {

        // get XML structure of reply into a parseable format

        QDomDocument doc;
        QXmlInputSource src;
        QString countryIso3166Code/*, countryName*/;
        QMap<QLocale::Country, QString> countryIsoMap; // for e.g. Andorra: <5, "AD">
        QLocale::Country country;
        bool success = false;
        QString flagPath = Core::ICore::instance()->settings()->path(Core::ISettings::SmallPixmapPath) + "/flags/";
        LOG(flagPath);

        src.setData(reply->readAll());
        doc.setContent(src.data());

        // iter over countries
        QDomNodeList countries = doc.elementsByTagName("country");
        for (uint i = 0; i < countries.length(); i++) {

            // get a list of country info subitems
            // (countryCode, countryName, numPostalCodes, minPostalCode, maxPostalCode)
            countryIso3166Code = countries.item(i).firstChildElement("countryCode").toElement().text();
//            countryName = countries.item(i).firstChildElement("countryName").toElement().text();

            if (countryIso3166Code.isEmpty() || (countryIso3166Code.size() != 2) /*|| countryName.isEmpty()*/) {
                qWarning() << "Country conversion error: wrong countryIso3166Code:" << countryIso3166Code;
                continue;
            }

            // transform ISO3166 to QLocale::Country
            country = Utils::countryIsoToCountry(countryIso3166Code);
            if (country == QLocale::AnyCountry) {
                // error: just skip this unknown country
                qWarning() << "Country conversion error: country" << countryIso3166Code << "has no Qt equivalent.";
                continue;
            }
            // transform ISO-3166-1 2 char countryCode to Qt's internal integer country codes
            countryIsoMap.insert(country, countryIso3166Code);
            qDebug() << country << countryIso3166Code;


            QStandardItem *item = new QStandardItem(
                        QIcon(QString("%1/%2.png").arg(flagPath, countryIso3166Code)),
                        QLocale::countryToString(country));
//            item->setData(Qt::UserRole, country);
            m_availableCountriesModel->appendRow(item);
            success = true;
        }
        Q_EMIT countryListDownloaded(success);

        // find default system country and add it to the selected list
        QList<QStandardItem*> items = m_availableCountriesModel->findItems(
                    QLocale::countryToString(QLocale::system().country()));
        if (!items.isEmpty()) {
            QStandardItem *defaultCountry = items.first()->clone();
            m_availableCountriesModel->removeRow(defaultCountry->index().row());
            m_selectedCountriesModel->appendRow(defaultCountry);
            m_selectedCountriesModel->sort(0);
        }
    }
}
