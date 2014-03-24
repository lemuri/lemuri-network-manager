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
#ifndef AVAILABLE_CONNECTIONS_SORT_MODEL_H
#define AVAILABLE_CONNECTIONS_SORT_MODEL_H

#include <QSortFilterProxyModel>

class AvailableConnectionsSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QObject *sourceModel READ sourceModel WRITE setModel NOTIFY sourceModelChanged)
public:
    explicit AvailableConnectionsSortModel(QObject *parent = 0);

    void setModel(QObject *model);

signals:
    void sourceModelChanged(QObject *);

private:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // AVAILABLE_CONNECTIONS_SORT_MODEL_H
