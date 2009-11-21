#include "globaldrugsmodel.h"

#include <drugsbaseplugin/drugsbase.h>
#include <drugsbaseplugin/constants.h>

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>
#include <coreplugin/itheme.h>
#include <coreplugin/constants.h>

#include <QList>
#include <QColor>

#include <QDebug>


static inline Core::ISettings *settings() {return Core::ICore::instance()->settings();}
static inline Core::ITheme *theme()  { return Core::ICore::instance()->theme(); }
static inline DrugsDB::Internal::DrugsBase *drugsBase() {return DrugsDB::Internal::DrugsBase::instance();}

namespace DrugsDB {
namespace Internal {

class GlobalDrugsModelPrivate
{
public:
    GlobalDrugsModelPrivate::GlobalDrugsModelPrivate()
    {
        m_CachedAvailableDosageForCIS = drugsBase()->getAllCISThatHaveRecordedDosages();
        m_CachedAvailableDosageForINN = drugsBase()->getAllINNThatHaveRecordedDosages();
    }

    bool CISHasRecordedDosage(const int CIS)
    {
        return m_CachedAvailableDosageForCIS.contains(CIS);
    }

    bool INNHasRecordedDosage(const int INN)
    {
        return m_CachedAvailableDosageForINN.contains(INN);
    }

private:
    QList<int> m_CachedAvailableDosageForCIS, m_CachedAvailableDosageForINN;

};
}  // End Internal
}  // End DrugsDB

using namespace DrugsDB;

GlobalDrugsModel::GlobalDrugsModel(QObject *parent) :
        QSqlTableModel(parent, QSqlDatabase::database(Constants::DRUGS_DATABASE_NAME)),
        d(0)
{
    d = new Internal::GlobalDrugsModelPrivate();
    setTable(drugsBase()->table(Constants::Table_CIS));
    setEditStrategy( QSqlTableModel::OnManualSubmit );
    QHashWhere where;
    where.insert(Constants::CIS_COMMERCIALISATION, "='O'");
    setFilter(drugsBase()->getWhereClause(Constants::Table_CIS, where));
    select();
}

QVariant GlobalDrugsModel::data(const QModelIndex &item, int role) const
{
    if (!item.isValid())
        return QVariant();

    if (role == Qt::BackgroundRole) {
        if (item.column() == Constants::CIS_DENOMINATION) {
            if (settings()->value(DrugsDB::Constants::S_MARKDRUGSWITHAVAILABLEDOSAGES).toBool()) {
                QModelIndex cis = index(item.row(), Constants::CIS_CIS);
                if (d->CISHasRecordedDosage(cis.data().toInt())) {
                    QColor c = QColor(settings()->value(Constants::S_AVAILABLEDOSAGESBACKGROUNGCOLOR).toString());
                    c.setAlpha(125);
                    return c;
                }
                //            if (d->INNHasRecordedDosage())
                //                ;
            }
        }
    }
    return QSqlTableModel::data(item,role);
}

Qt::ItemFlags GlobalDrugsModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

//bool GlobalDrugsModel::select()
//{
//    qWarning() << "lkjlkjlkjlkjlkjlkjlkjl SELECT";
//    bool r = QSqlTableModel::select();
//        qWarning() << this->tableName() << rowCount() << columnCount() << filter();
//return r;
//}
