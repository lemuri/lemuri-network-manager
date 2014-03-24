/***************************************************************************
 *   Copyright (C) 2013 by Daniel Nicoletti                                *
 *   dantti12@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/
#include "AvailableConnectionsSortModel.h"

#include "AvailableConnectionsModel.h"

#include <QDebug>

AvailableConnectionsSortModel::AvailableConnectionsSortModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(0);
}

void AvailableConnectionsSortModel::setModel(QObject *model)
{
    if (model == sourceModel()) {
        return;
    }

    QAbstractItemModel *abstractItemModel = qobject_cast<QAbstractItemModel*>(model);
    if (abstractItemModel) {
        QSortFilterProxyModel::setSourceModel(abstractItemModel);
        emit sourceModelChanged(model);
    }
}

bool AvailableConnectionsSortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    bool leftActive = left.data(AvailableConnectionsModel::RoleActive).toBool();
    bool rightActive = right.data(AvailableConnectionsModel::RoleActive).toBool();

    if (leftActive != rightActive) {
        return leftActive;
    }

    int leftSignalStrength = left.data(AvailableConnectionsModel::RoleSignalStrength).toInt();
    int rightSignalStrength = right.data(AvailableConnectionsModel::RoleSignalStrength).toInt();
    if (leftSignalStrength != rightSignalStrength) {
        // If the left item is more than the right one, left should move right
        return leftSignalStrength > rightSignalStrength;
    }

    QString leftName = left.data(AvailableConnectionsModel::RoleNetworkID).toString();
    QString rightName = right.data(AvailableConnectionsModel::RoleNetworkID).toString();

    return QString::localeAwareCompare(leftName, rightName);
}
