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
#ifndef PATIENTBASE_H
#define PATIENTBASE_H

#include <utils/database.h>

#include <coreplugin/isettings.h>

#include <patientbaseplugin/patientbase_exporter.h>

#include <QObject>
#include <QString>
#include <QDate>

/**
 * \file patientbase.h
 * \author Eric MAEKER <eric.maeker@gmail.com>
 * \version 0.6.0
 * \date 06 Aug 2011
*/

/** \todo create a sqlite db in memory to improve speed ? */

namespace Patients {
namespace Internal {
class PatientBasePrivate;
class PatientData;

class PATIENT_EXPORT PatientBase : public QObject, public Utils::Database
{
    Q_OBJECT
protected:
    PatientBase(QObject *parent = 0);
    bool init();

public:
    // Constructor
    static PatientBase *instance();
    virtual ~PatientBase();

//    // initialize
//    bool initialize(Core::ISettings *settings);
    bool isInitialized() const {return m_initialized;}

    void createVirtualPatient(const QString &name, const QString &secondname, const QString &firstname,
                              const QString &gender, const int title, const QDate &dob,
                              const QString &country, const QString &note,
                              const QString &street, const QString &zip, const QString &city,
                              QString uuid, const int lkid,
                              const QString &photoFile = QString(), const QDate &death = QDate());

    //private:
    bool createDatabase(const QString &connectionName, const QString &dbName,
                          const QString &pathOrHostName,
                          TypeOfAccess access, AvailableDrivers driver,
                          const QString &login, const QString &pass,
                          const int port,
                          CreationOption createOption
                         );

    void toTreeWidget(QTreeWidget *tree);

private Q_SLOTS:
    void onCoreDatabaseServerChanged();

private:
    static bool m_initialized;
    static PatientBase *m_Instance;
//    QList<PatientData *> m_OrderedHistory;
//    bool m_IsNewlyCreated;
};

}  // End namespace Internal
}  // End namespace Patients

#endif // PATIENTBASE_H
