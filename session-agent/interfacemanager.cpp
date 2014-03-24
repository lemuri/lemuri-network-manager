#include "interfacemanager.h"

#include "IconProvider.h"
#include "trayicon.h"

#include <NetworkManagerQt/Manager>

#include <QWidget>

using namespace NetworkManager;

InterfaceManager::InterfaceManager(QObject *parent) :
    QObject(parent),
    m_iconProvider(new IconProvider)
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved,
            this, &InterfaceManager::deviceRemoved);
}

void InterfaceManager::init()
{
    connect(notifier(), &Notifier::deviceAdded,
            this, &InterfaceManager::deviceAdded);
    connect(notifier(), &Notifier::wirelessEnabledChanged,
            this, &InterfaceManager::wirelessEnabledChanged);

    foreach (const Device::Ptr &device, networkInterfaces()) {
        interfaceAdded(device);
    }

//    QWidget *widget = new QWidget(0, Qt::Popup);
//    widget->show();
//    widget->setAttribute();
//    qDebug() << widget->windowFlags() << widget->windowType();
}

bool InterfaceManager::wirelessEnabled() const
{
    return isWirelessEnabled();
}

void InterfaceManager::setWirelessEnabled(bool enabled) const
{
    NetworkManager::setWirelessEnabled(enabled);
}

void InterfaceManager::interfaceAdded(const Device::Ptr &device)
{
    TrayIcon *trayIcon = new TrayIcon(device, m_iconProvider, this);
    m_trayIcons.insert(device->uni(), trayIcon);
}

void InterfaceManager::deviceAdded(const QString &uni)
{
    Device::Ptr device = findNetworkInterface(uni);
    if (device) {
        interfaceAdded(device);
    }
}

void InterfaceManager::deviceRemoved(const QString &uni)
{
    TrayIcon *trayIcon = m_trayIcons.take(uni);
    delete trayIcon;
}

