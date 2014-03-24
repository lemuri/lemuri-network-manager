/***************************************************************************
 *   Copyright (C) 2013 by Daniel Nicoletti <dantti12@gmail.com>           *
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

#ifndef AVAILABLECONNECTIONSMODEL_H
#define AVAILABLECONNECTIONSMODEL_H

#include <QStandardItemModel>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/WirelessNetwork>
#include <NetworkManagerQt/WimaxNsp>

class AvailableConnectionsModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(ConnectionRoles)
    Q_ENUMS(Kind)
    Q_PROPERTY(QString device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString toolTip READ toolTip NOTIFY toolTipChanged)
public:
    enum ConnectionRoles {
        RoleConectionPath = Qt::UserRole + 1,
        RoleKinds,
        RoleNetworkID,
        RoleSsid,
        RoleBssid,
        RoleMacAddress,
        RoleSignalStrength,
        RoleSignalStrengthIcon,
        RoleSecurity,
        RoleSecurityType,
        RoleStatus,
        RoleActive
    };

    enum Kind {
        Connection      = 1 << 0,
        Network         = 1 << 1,
        NetworkWireless = 1 << 2,
        NetworkNsp      = 1 << 3
    };
    Q_DECLARE_FLAGS(Kinds, Kind)

    explicit AvailableConnectionsModel(QObject *parent = 0);

    QString device() const;
    void setDevice(const QString &deviceUni);

    void setDevicePtr(const NetworkManager::Device::Ptr &device);

    QString icon() const;
    QString toolTip() const;

Q_SIGNALS:
    void deviceChanged();
    void iconChanged();
    void toolTipChanged();

private slots:
    void availableConnectionChanged();
    void activeConnectionChanged();
    void connectionAdded(const QString &path);
    void connectionRemoved(const QString &path);
    void addConnection(const NetworkManager::Connection::Ptr &connection);
    void accessPointAppeared(const QString &uni);
    void accessPointDisappeared(const QString &uni);
    void signalStrengthChanged(int strength);
    void nspAppeared(const QString &uni);
    void nspDisappeared(const QString &name);
    void addNspNetwork(const NetworkManager::WimaxNsp::Ptr &nsp);
    void signalQualityChanged(uint quality);

private:
    void updateDeviceStatus();
    void updateAccessPointConnections(QStandardItem *stdItem);
    QStandardItem *findConnectionItem(const QString &path);
    QStandardItem *findNetworkItem(const QString &id);
    QStandardItem *findActiveItem();

    QString signalToString(int strength) const;
    NetworkManager::Device::Ptr m_device;
    QString m_deviceUni;
    QString m_icon;
    QString m_toolTip;
};

#endif // AVAILABLECONNECTIONSMODEL_H
