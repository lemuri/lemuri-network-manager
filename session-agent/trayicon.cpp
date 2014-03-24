#include "trayicon.h"

#include "IconProvider.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>

#include <QApplication>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlContext>
#include <QDebug>
#include <QX11Info>

#include <xcb/xcb.h>

using namespace NetworkManager;

TrayIcon::TrayIcon(const Device::Ptr &device, IconProvider *iconProvider, QObject *parent) :
    QObject(parent),
    m_device(device),
    m_icon(new QSystemTrayIcon(this))
{
    if (device->type() == Device::Wifi) {
        m_icon->setIcon(QIcon::fromTheme("network-wireless"));
    } else if (device->type() == Device::Ethernet) {
        m_icon->setIcon(QIcon::fromTheme("network-wired"));
    } else {
        m_icon->setIcon(QIcon::fromTheme("network-defaultroute"));
    }

    m_icon->setToolTip(device->interfaceName());
    connect(m_icon, &QSystemTrayIcon::activated,
            this, &TrayIcon::activated);

    m_view = new QQuickView();
    m_view->setFlags(Qt::FramelessWindowHint);
    m_view->setWidth(200);
    m_view->setHeight(300);
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);
    m_view->setColor(QColor(Qt::transparent));
    m_view->rootContext()->setContextProperty("deviceUni", device->uni());
    m_view->rootContext()->setContextProperty("Device", this);
    m_view->engine()->addImageProvider(QLatin1String("icon"), iconProvider);

    m_view->setSource(QUrl(QLatin1String("qrc:/qml/main.qml")));
    qDebug() << m_view->flags() << m_view->type();


    xcb_connection_t *connection = QX11Info::connection();

    xcb_window_t winId = m_view->winId();

    xcb_intern_atom_cookie_t windowType =
                xcb_intern_atom(connection, 0, strlen("_NET_WM_WINDOW_TYPE"),
                                "_NET_WM_WINDOW_TYPE");
    xcb_intern_atom_cookie_t toolbarType =
                xcb_intern_atom(connection, 0,
                                strlen("_NET_WM_WINDOW_TYPE_TOOLBAR"),
                                "_NET_WM_WINDOW_TYPE_TOOLBAR");

    xcb_intern_atom_reply_t *windowTypeReply =
                xcb_intern_atom_reply(connection, windowType, NULL);
    xcb_intern_atom_reply_t *toolbarTypeReply =
                xcb_intern_atom_reply(connection, toolbarType, NULL);

    const uint32_t data[] = {
            toolbarTypeReply->atom
    };

    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, winId,
                            windowTypeReply->atom, XCB_ATOM_ATOM,
                            (u_int8_t)32, sizeof(data)/sizeof(data[0]), &data);

    xcb_flush(connection);

    m_icon->show();
}

void TrayIcon::activateConnection(const QString &connectionPath)
{
    QDBusPendingReply<QDBusObjectPath> reply;
    reply = NetworkManager::activateConnection(connectionPath, m_device->uni(), QString());
    reply.waitForFinished();
//    updateActiveConnection();
    if (reply.isError()) {
//        KMessageBox::error(this,
//                           i18n("Failed to activate network %1:\n%2", name, reply.error().message()));
    }
}

void TrayIcon::setIcon(const QString &name)
{
    m_icon->setIcon(QIcon::fromTheme(name));
}

void TrayIcon::setToolTip(const QString &tip)
{
    m_icon->setToolTip(tip);
}

void TrayIcon::activated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        qDebug() << m_view->isVisible() << QCursor::pos() << QCursor::pos();
//        m_view->setVisible(!m_view->isVisible());
        if (m_view->isVisible()) {
            m_view->hide();
        } else {
            if (m_device->type() == NetworkManager::Device::Wifi) {
                NetworkManager::WirelessDevice::Ptr wifi = m_device.dynamicCast<NetworkManager::WirelessDevice>();
                wifi->requestScan();
            }

            m_view->setX(QCursor::pos().x());
            m_view->setY(QCursor::pos().y() - m_view->height());
            m_view->show();
//                m_view->rootObject()->setFocus(true, Qt::PopupFocusReason);
//            m_view->rootObject()->forceActiveFocus();
        }
//        m_view->fo
//        m_view->setWindowState();
    }
}
