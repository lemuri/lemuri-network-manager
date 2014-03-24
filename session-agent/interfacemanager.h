#ifndef INTERFACEMANAGER_H
#define INTERFACEMANAGER_H

#include <QObject>
#include <NetworkManagerQt/Device>

class TrayIcon;
class IconProvider;
class InterfaceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool wirelessEnabled READ wirelessEnabled WRITE setWirelessEnabled NOTIFY wirelessEnabledChanged)
public:
    explicit InterfaceManager(QObject *parent = 0);

    void init();

    bool wirelessEnabled() const;
    void setWirelessEnabled(bool enabled) const;

Q_SIGNALS:
    void wirelessEnabledChanged();

private slots:
    void interfaceAdded(const NetworkManager::Device::Ptr &device);
    void deviceAdded(const QString &uni);
    void deviceRemoved(const QString &uni);

private:
    IconProvider *m_iconProvider;
    QHash<QString, TrayIcon *> m_trayIcons;
};

#endif // INTERFACEMANAGER_H
