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

#include "AvailableConnectionsModel.h"

#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WimaxDevice>
#include <NetworkManagerQt/WimaxNsp>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Utils>

#include <QDebug>

using namespace NetworkManager;

AvailableConnectionsModel::AvailableConnectionsModel(QObject *parent) :
    QStandardItemModel(parent)
{
    QHash<int,QByteArray> roles;
    roles[RoleConectionPath] = "connectionPath";
    roles[RoleKinds] = "kinds";
    roles[RoleNetworkID] = "networkId";
    roles[RoleSsid] = "ssid";
    roles[RoleMacAddress] = "macAddress";
    roles[RoleSignalStrength] = "signalStrength";
    roles[RoleSignalStrengthIcon] = "signalStrengthIcon";
    roles[RoleSecurity] = "security";
    roles[RoleSecurityType] = "securityType";
    roles[RoleStatus] = "status";
    roles[RoleActive] = "active";
    setItemRoleNames(roles);
}

QString AvailableConnectionsModel::device() const
{
    return m_deviceUni;
}

void AvailableConnectionsModel::setDevice(const QString &deviceUni)
{
    m_deviceUni = deviceUni;
    setDevicePtr(findNetworkInterface(deviceUni));
}

void AvailableConnectionsModel::setDevicePtr(const NetworkManager::Device::Ptr &device)
{
    if (m_device && m_device->uni() == device->uni()) {
        return;
    } else if (m_device) {
        m_device->disconnect(this);
    }
    m_device = device;
    removeRows(0, rowCount());
    qDebug() << Q_FUNC_INFO << device->uni() << device->type();

    connect(device.data(), SIGNAL(activeConnectionChanged()), SLOT(activeConnectionChanged()));
    connect(device.data(), SIGNAL(availableConnectionChanged()), SLOT(availableConnectionChanged()));
    if (device->type() == NetworkManager::Device::Wifi) {
        NetworkManager::WirelessDevice::Ptr wifi = device.dynamicCast<NetworkManager::WirelessDevice>();
        connect(wifi.data(), SIGNAL(accessPointAppeared(QString)), SLOT(accessPointAppeared(QString)));
        connect(wifi.data(), SIGNAL(accessPointDisappeared(QString)), SLOT(accessPointDisappeared(QString)));
        foreach (const QString &accessPoint, wifi->accessPoints()) {
            accessPointAppeared(accessPoint);
        }
    } else if (device->type() == NetworkManager::Device::Wimax) {
        NetworkManager::WimaxDevice::Ptr wiMax = device.dynamicCast<NetworkManager::WimaxDevice>();
        connect(wiMax.data(), SIGNAL(nspAppeared(QString)), SLOT(nspAppeared(QString)));
        connect(wiMax.data(), SIGNAL(nspDisappeared(QString)), SLOT(nspDisappeared(QString)));
        foreach (const QString &nsp, wiMax->nsps()) {
            NetworkManager::WimaxNsp::Ptr nspPtr = wiMax->findNsp(nsp);
            if (nspPtr) {
                addNspNetwork(nspPtr);
            }
        }
    } else {
        foreach (const Connection::Ptr &connection, device->availableConnections()) {
            addConnection(connection);
        }
    }

    // Make sure the current active connection is set
    activeConnectionChanged();
}

QString AvailableConnectionsModel::icon() const
{
    return m_icon;
}

QString AvailableConnectionsModel::toolTip() const
{
    return m_toolTip;
}

void AvailableConnectionsModel::availableConnectionChanged()
{
    for (int i = 0; i < rowCount(); ++i) {
        updateAccessPointConnections(item(i));
    }
}

void AvailableConnectionsModel::activeConnectionChanged()
{
    ActiveConnection::Ptr activeConnection = m_device->activeConnection();
    qDebug() << Q_FUNC_INFO << activeConnection.isNull();
    if (activeConnection) {
//        QString connectionPath = activeConnection->connection()->path();

        if (m_device->type() == NetworkManager::Device::Wifi) {
            NetworkManager::WirelessDevice::Ptr wifi = m_device.dynamicCast<NetworkManager::WirelessDevice>();
            AccessPoint::Ptr activeAP = wifi->activeAccessPoint();
            QString bssid;
            if (activeAP) {
                bssid = activeAP->hardwareAddress();
            }

            for (int i = 0; i < rowCount(); ++i) {
                QStandardItem *stdItem = item(i);
                qDebug() << "model" << i << bssid << stdItem->data(RoleBssid).toString() << stdItem->data(RoleNetworkID).toString();
                bool active = bssid == stdItem->data(RoleBssid).toString();
                if (stdItem->data(RoleActive).toBool() != active) {
                    stdItem->setData(active, RoleActive);
                    if (active) {
                        updateDeviceStatus();
                    }
                }
            }
        }
//        ConnectionSettings::Ptr settings = activeConnection->connection()->settings();
//        WirelessSetting::Ptr wifiSetting = settings->setting(Setting::Wireless).dynamicCast<WirelessSetting>();
//        if (!wifiSetting) {
//            return;
//        }

//        QStringList seenBssid = wifiSetting->seenBssids();
//        qDebug() << "Active" << connectionPath << seenBssid << WirelessDevice::Ptr->bssid();
//        for (int i = 0; i < rowCount(); ++i) {
//            QStandardItem *stdItem = item(i);
//            qDebug() << "model" << i << stdItem->data(RoleBssid).toString() << stdItem->data(RoleNetworkID).toString();;
//            bool active = seenBssid.contains(stdItem->data(RoleBssid).toString());
//            if (stdItem->data(RoleActive).toBool() != active) {
//                stdItem->setData(active, RoleActive);
//                if (active) {
//                    updateDeviceStatus();
//                }
//            }
//        }
    } else {
        updateDeviceStatus();
    }
}

void AvailableConnectionsModel::connectionAdded(const QString &path)
{
    Connection::Ptr connection = findConnection(path);
    if (connection) {
        addConnection(connection);
    }
}

void AvailableConnectionsModel::connectionRemoved(const QString &path)
{
    QStandardItem *stdItem = findConnectionItem(path);
    if (stdItem) {
        removeRow(stdItem->row());
    }
}

void AvailableConnectionsModel::addConnection(const NetworkManager::Connection::Ptr &connection)
{
    QStandardItem *stdItem = findConnectionItem(connection->path());
    if (stdItem) {
        return;
    }

    stdItem = new QStandardItem;
    stdItem->setData(Connection, RoleKinds);
    stdItem->setData(connection->path(), RoleConectionPath);

    ConnectionSettings::Ptr settings = connection->settings();
    if (settings->connectionType() == ConnectionSettings::Wireless) {
        WirelessSetting::Ptr wirelessSetting;
        wirelessSetting = settings->setting(Setting::Wireless).dynamicCast<WirelessSetting>();
        stdItem->setData(wirelessSetting->ssid(), RoleNetworkID);
        stdItem->setData(wirelessSetting->macAddress(), RoleMacAddress);
        stdItem->setData(false, RoleActive);
    } else {
        stdItem->setText(connection->name());
    }
qDebug() << stdItem->text();
    appendRow(stdItem);
}

void AvailableConnectionsModel::accessPointAppeared(const QString &uni)
{
    NetworkManager::WirelessDevice::Ptr wifi = m_device.dynamicCast<NetworkManager::WirelessDevice>();
    AccessPoint::Ptr accessPoint = wifi->findAccessPoint(uni);
    if (!accessPoint) {
        return;
    }

    QStandardItem *stdItem = findNetworkItem(uni);
    if (stdItem) {
        qWarning() << "Access Point known" << uni;
        return;
    }

    connect(accessPoint.data(), SIGNAL(signalStrengthChanged(int)),
            SLOT(signalStrengthChanged(int)), Qt::UniqueConnection);
    stdItem = new QStandardItem;
    stdItem->setData(uni, RoleNetworkID);
    stdItem->setData(QString(), RoleStatus);
    stdItem->setData(accessPoint->ssid(), RoleSsid);
    stdItem->setData(Network | NetworkWireless, RoleKinds);

    QString securityType;
    bool isSecure = accessPoint->capabilities() & AccessPoint::Privacy;
    if (isSecure) {
        Utils::WirelessSecurityType security;
        security = Utils::findBestWirelessSecurity(wifi->wirelessCapabilities(),
                                                   true,
                                                   (accessPoint->mode() == AccessPoint::Adhoc),
                                                   accessPoint->capabilities(),
                                                   accessPoint->wpaFlags(),
                                                   accessPoint->rsnFlags());
        switch (security) {
        case Utils::StaticWep:
        case Utils::DynamicWep:
            securityType = "WEP";
            break;
        case Utils::WpaPsk:
            securityType = "WPA PSK";
            break;
        case Utils::WpaEap:
            securityType = "WPA EAP";
            break;
        case Utils::Wpa2Psk:
            securityType = "WPA2 PSK";
            break;
        case Utils::Wpa2Eap:
            securityType = "WPA2 EAP";
            break;
        }
    }
//    qDebug() << accessPoint->ssid() << accessPoint->wpaFlags() << accessPoint->rsnFlags();

    stdItem->setData(securityType, RoleSecurityType);
    stdItem->setData(isSecure, RoleSecurity);
    stdItem->setData(false, RoleActive);
    stdItem->setData(accessPoint->signalStrength(), RoleSignalStrength);
    stdItem->setData(signalToString(accessPoint->signalStrength()), RoleSignalStrengthIcon);
    stdItem->setData(accessPoint->hardwareAddress(), RoleBssid);
    // Requires Bssid set
    updateAccessPointConnections(stdItem);

    appendRow(stdItem);
}

void AvailableConnectionsModel::accessPointDisappeared(const QString &uni)
{
    QStandardItem *stdItem = findNetworkItem(uni);
    if (stdItem) {
        removeRow(stdItem->row());
    }
}

void AvailableConnectionsModel::signalStrengthChanged(int strength)
{
    AccessPoint *accessPoint = qobject_cast<AccessPoint*>(sender());
    QStandardItem *stdItem = findNetworkItem(accessPoint->uni());
    if (stdItem) {
        stdItem->setData(strength, RoleSignalStrength);
        stdItem->setData(signalToString(strength), RoleSignalStrengthIcon);
        if (stdItem->data(RoleActive).toBool()) {
            updateDeviceStatus();
        }
    }
}

void AvailableConnectionsModel::nspAppeared(const QString &uni)
{
    NetworkManager::WimaxDevice::Ptr wimax = m_device.dynamicCast<NetworkManager::WimaxDevice>();
    if (wimax) {
        WimaxNsp::Ptr nsp = wimax->findNsp(uni);
        addNspNetwork(nsp);
    }
}

void AvailableConnectionsModel::nspDisappeared(const QString &name)
{
    QStandardItem *stdItem = findNetworkItem(name);
    if (stdItem) {
        removeRow(stdItem->row());
    }
}

void AvailableConnectionsModel::addNspNetwork(const WimaxNsp::Ptr &nsp)
{
    QStandardItem *stdItem = findNetworkItem(nsp->name());
    connect(nsp.data(), SIGNAL(signalQualityChanged(uint)), SLOT(signalQualityChanged(int)), Qt::UniqueConnection);
    if (!stdItem) {
        stdItem = new QStandardItem;
        stdItem->setData(nsp->uni(), RoleNetworkID);
        stdItem->setData(Network | NetworkNsp, RoleKinds);
        stdItem->setText(nsp->name());
        appendRow(stdItem);
    } else {
        stdItem->setData(Network | Connection, RoleKinds);
    }
    stdItem->setData(true, RoleSecurity);
    stdItem->setData(signalToString(nsp->signalQuality()), RoleSignalStrength);
}

void AvailableConnectionsModel::signalQualityChanged(uint quality)
{
    WimaxNsp *nsp = qobject_cast<WimaxNsp*>(sender());
    QStandardItem *stdItem = findNetworkItem(nsp->name());
    if (stdItem) {
        stdItem->setData(signalToString(quality), RoleSignalStrength);
    }
}

void AvailableConnectionsModel::updateDeviceStatus()
{
    ActiveConnection::Ptr activeConnection = m_device->activeConnection();
    QStandardItem *stdItem = findActiveItem();
    if (!stdItem || !activeConnection) {
        if (stdItem) {
            qDebug() << Q_FUNC_INFO << stdItem->data(RoleSsid) << stdItem->data(RoleActive);

            stdItem->setData(false, RoleActive);
            qDebug() << Q_FUNC_INFO << stdItem->data(RoleSsid) << stdItem->data(RoleActive);

        }

        m_icon = "network-wireless-disconnected";
        emit iconChanged();
        m_toolTip = tr("Disconnected - %1").arg(m_device->interfaceName());
        emit toolTipChanged();
        return;
    }

    int strength = stdItem->data(RoleSignalStrength).toInt();
    QString icon = stdItem->data(RoleSignalStrengthIcon).toString();
    if (m_icon != icon) {
        m_icon = icon;
        emit iconChanged();
    }

    QString status;
    QString toolTip;
    switch (activeConnection->state()) {
    case ActiveConnection::Activating:
        status = tr("Connecting");
        toolTip = tr("Connecting to '%1'")
                .arg(activeConnection->connection()->name());
        break;
    case ActiveConnection::Activated:
        status = tr("Connected");
        toolTip = tr("Connected to '%1', %2 (%3%)")
                .arg(activeConnection->connection()->name(),
                     stdItem->data(RoleSsid).toString(),
                     QString::number(strength));
        break;
    case ActiveConnection::Deactivating:
        toolTip = status = tr("Disconnecting");
        break;
    case ActiveConnection::Deactivated:
        toolTip = status = tr("Disconnected");
        break;
    default:
        break;
    }

    if (m_toolTip != toolTip) {
        m_toolTip = toolTip;
        emit toolTipChanged();

        stdItem->setData(status, RoleStatus);
    }
}

void AvailableConnectionsModel::updateAccessPointConnections(QStandardItem *stdItem)
{
    QStringList connections;

    QString bssid = stdItem->data(RoleBssid).toString();
    foreach (const Connection::Ptr &connection, m_device->availableConnections()) {
//        qWarning() << "Connection" << connection->name();
        ConnectionSettings::Ptr settings = connection->settings();
        WirelessSetting::Ptr wifiSetting = settings->setting(Setting::Wireless).dynamicCast<WirelessSetting>();;
        QStringList seenBssid = wifiSetting->seenBssids();

        if (seenBssid.contains(bssid)) {
            connections << connection->path();
        }
    }
    connections.sort();

    if (stdItem->data(RoleConectionPath).toStringList() != connections) {
        stdItem->setData(connections, RoleConectionPath);
    }
}

QStandardItem *AvailableConnectionsModel::findConnectionItem(const QString &path)
{
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *stdItem = item(i);
        if (stdItem->data(RoleKinds).toUInt() & Connection &&
                stdItem->data(RoleConectionPath).toString() == path) {
            return stdItem;
        }
    }

    return 0;
}

QStandardItem *AvailableConnectionsModel::findActiveItem()
{
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *stdItem = item(i);
        if (stdItem->data(RoleActive).toBool()) {
            return stdItem;
        }
    }

    return 0;
}

QStandardItem *AvailableConnectionsModel::findNetworkItem(const QString &id)
{
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *stdItem = item(i);
        if (stdItem->data(RoleNetworkID).toString() == id) {
            return stdItem;
        }
    }

    return 0;
}

QString AvailableConnectionsModel::signalToString(int strength) const
{
    QString level;
    if (strength < 13) {
        level = "network-wireless-connected-00";
    } else if (strength < 38) {
        level = "network-wireless-connected-25";
    } else if (strength < 63) {
        level = "network-wireless-connected-50";
    } else if (strength < 88) {
        level = "network-wireless-connected-75";
    } else {
        level = "network-wireless-connected-100";
    }
    return level;
}
