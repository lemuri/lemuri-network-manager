#ifndef TRAYICON_H
#define TRAYICON_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QQuickView>

#include <NetworkManagerQt/Device>

class IconProvider;
class TrayIcon : public QObject
{
    Q_OBJECT
public:
    explicit TrayIcon(const NetworkManager::Device::Ptr &device, IconProvider *iconProvider, QObject *parent = 0);

    Q_INVOKABLE void activateConnection(const QString &connectionPath);
    Q_INVOKABLE void setIcon(const QString &name);
    Q_INVOKABLE void setToolTip(const QString &text);

private:
    void activated(QSystemTrayIcon::ActivationReason reason);
    void statusChanged(QQuickView::Status);

    QQuickView *m_view;
    NetworkManager::Device::Ptr m_device;
    QSystemTrayIcon *m_icon;
};

#endif // TRAYICON_H
