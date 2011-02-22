/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2010 by Eric MAEKER, MD (France) <eric.maeker@free.fr>        *
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
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef CATEGORYPLUGIN_CATEGORYMODELHELPER_H
#define CATEGORYPLUGIN_CATEGORYMODELHELPER_H

#include <categoryplugin/category_exporter.h>
#include <QAbstractItemModel>
#include <QAbstractProxyModel>


namespace Category {
class CategoryItem;
class CategoryOnlyProxyModel;


class CATEGORY_EXPORT ICategoryModelHelper : public QAbstractItemModel
{
public:
    ICategoryModelHelper(QObject *parent) : QAbstractItemModel(parent), m_Proxy(0) {}
    virtual ~ICategoryModelHelper() {}

    virtual CategoryOnlyProxyModel *categoryOnlyModel();

    virtual bool isCategory(const QModelIndex &item) const = 0;
    virtual CategoryItem *categoryForIndex(const QModelIndex &item) const = 0;
    virtual QModelIndex indexForCategory(const Category::CategoryItem *category) const = 0;

    virtual void addCategory(CategoryItem *category) = 0;
    virtual void updateCategory(CategoryItem *category) = 0;

    virtual QString mime() const = 0;


private:
    CategoryOnlyProxyModel *m_Proxy;
};


} // End namespace Category

#endif // CATEGORYPLUGIN_CATEGORYMODELHELPER_H
