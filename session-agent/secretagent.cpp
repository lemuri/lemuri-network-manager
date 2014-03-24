/*
    Copyright 2013 Jan Grulich <jgrulich@redhat.com>
    Copyright 2013 Lukas Tinkl <ltinkl@redhat.com>
    Copyright 2013 by Daniel Nicoletti <dantti12@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "secretagent.h"
#include "passworddialog.h"

#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/GenericTypes>
#include <NetworkManagerQt/GsmSetting>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>

#include <QStringBuilder>

#include <QDebug>

SecretAgent::SecretAgent(QObject* parent):
    NetworkManager::SecretAgent("org.7land.networkmanagement", parent),
    m_dialog(0)
{
    connect(NetworkManager::notifier(), SIGNAL(serviceDisappeared()),
            this, SLOT(killDialogs()));
}

SecretAgent::~SecretAgent()
{
}

NMVariantMapMap SecretAgent::GetSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path, const QString &setting_name,
                                        const QStringList &hints, uint flags)
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "Path:" << connection_path.path();
    qDebug() << "Setting name:" << setting_name;
    qDebug() << "Hints:" << hints;
    qDebug() << "Flags:" << flags;

    QString callId = connection_path.path() % setting_name;
    foreach (const SecretsRequest & request, m_calls) {
        if (request == callId) {
            qWarning() << "GetSecrets was called again! This should not happen, cancelling first call" << connection_path.path() << setting_name;
            CancelGetSecrets(connection_path, setting_name);
            break;
        }
    }

    setDelayedReply(true);
    SecretsRequest request(SecretsRequest::GetSecrets);
    request.callId = callId;
    request.connection = connection;
    request.connection_path = connection_path;
    request.connection_settings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(connection));
    request.flags = static_cast<NetworkManager::SecretAgent::GetSecretsFlags>(flags);
    request.hints = hints;
    request.setting_name = setting_name;
    request.message = message();
    m_calls << request;

    processNext();

    return NMVariantMapMap();
}

void SecretAgent::SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    qDebug() << connection_path.path();

    setDelayedReply(true);
    SecretsRequest::Type type;
    if (hasSecrets(connection)) {
        type = SecretsRequest::SaveSecrets;
    } else {
        type = SecretsRequest::DeleteSecrets;
    }
    SecretsRequest request(type);
    request.connection = connection;
    request.connection_path = connection_path;
    request.message = message();
    m_calls << request;

    processNext();
}

void SecretAgent::DeleteSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    qDebug() << connection_path.path();

    setDelayedReply(true);
    SecretsRequest request(SecretsRequest::DeleteSecrets);
    request.connection = connection;
    request.connection_path = connection_path;
    request.message = message();
    m_calls << request;

    processNext();
}

void SecretAgent::CancelGetSecrets(const QDBusObjectPath &connection_path, const QString &setting_name)
{
    qDebug() << connection_path.path() << setting_name;
    QString callId = connection_path.path() % setting_name;
    for (int i = 0; i < m_calls.size(); ++i) {
        SecretsRequest request = m_calls.at(i);
        if (request.type == SecretsRequest::GetSecrets && callId == request.callId) {
            if (m_dialog == request.dialog) {
                m_dialog = 0;
            }
            delete request.dialog;
            sendError(SecretAgent::AgentCanceled,
                      QLatin1String("Agent canceled the password dialog"),
                      request.message);
            m_calls.removeAt(i);
            break;
        }
    }

    processNext();
}

void SecretAgent::dialogAccepted()
{
    for (int i = 0; i < m_calls.size(); ++i) {
        SecretsRequest request = m_calls[i];
        if (request.type == SecretsRequest::GetSecrets && request.dialog == m_dialog) {
            NMVariantMapMap connection = request.dialog->secrets();
            sendSecrets(connection, request.message);
            NetworkManager::ConnectionSettings::Ptr connectionSettings = request.connection_settings;
            NetworkManager::Connection::Ptr con = NetworkManager::findConnectionByUuid(connectionSettings->uuid());
            if (con) {
                connectionSettings = con->settings();
            }

            if (request.saveSecretsWithoutReply && connectionSettings->connectionType() != NetworkManager::ConnectionSettings::Vpn) {
                bool requestOffline = true;
                if (connectionSettings->connectionType() == NetworkManager::ConnectionSettings::Gsm) {
                    NetworkManager::GsmSetting::Ptr gsmSetting = connectionSettings->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();
                    if (gsmSetting) {
                        if (gsmSetting->passwordFlags().testFlag(NetworkManager::Setting::NotSaved) ||
                            gsmSetting->passwordFlags().testFlag(NetworkManager::Setting::NotRequired)) {
                            requestOffline = false;
                        } else if (gsmSetting->pinFlags().testFlag(NetworkManager::Setting::NotSaved) ||
                                   gsmSetting->pinFlags().testFlag(NetworkManager::Setting::NotRequired)) {
                            requestOffline = false;
                        }
                    }
                } else if (connectionSettings->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
                    NetworkManager::WirelessSetting::Ptr wirelessSetting = connectionSettings->setting(NetworkManager::Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();
                    if (wirelessSetting && !wirelessSetting->security().isEmpty()) {
                        NetworkManager::WirelessSecuritySetting::Ptr wirelessSecuritySetting = connectionSettings->setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
                        if (wirelessSecuritySetting && wirelessSecuritySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::WpaEap) {
                            NetworkManager::Security8021xSetting::Ptr security8021xSetting = connectionSettings->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>();
                            if (security8021xSetting) {
                                if (security8021xSetting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethodFast) ||
                                    security8021xSetting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethodTtls) ||
                                    security8021xSetting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethodPeap)) {
                                    if (security8021xSetting->passwordFlags().testFlag(NetworkManager::Setting::NotSaved) ||
                                        security8021xSetting->passwordFlags().testFlag(NetworkManager::Setting::NotRequired)) {
                                        requestOffline = false;
                                    }
                                }
                            }
                        }
                    }
                }

                if (requestOffline) {
                    SecretsRequest requestOffline(SecretsRequest::SaveSecrets);
                    requestOffline.connection = connection;
                    requestOffline.connection_path = request.connection_path;
                    requestOffline.saveSecretsWithoutReply = true;
                    m_calls << requestOffline;
                }
            }

            m_calls.removeAt(i);
            break;
        }
    }

    m_dialog->deleteLater();
    m_dialog = 0;

    processNext();
}

void SecretAgent::dialogRejected()
{
    for (int i = 0; i < m_calls.size(); ++i) {
        SecretsRequest request = m_calls[i];
        if (request.type == SecretsRequest::GetSecrets && request.dialog == m_dialog) {
            sendError(SecretAgent::UserCanceled,
                      QLatin1String("User canceled the password dialog"),
                      request.message);
            m_calls.removeAt(i);
            break;
        }
    }

    m_dialog->deleteLater();
    m_dialog = 0;

    processNext();
}

void SecretAgent::killDialogs()
{
    int i = 0;
    while (i < m_calls.size()) {
        SecretsRequest request = m_calls[i];
        if (request.type == SecretsRequest::GetSecrets) {
            delete request.dialog;
            m_calls.removeAt(i);
        }

        ++i;
    }
}

void SecretAgent::processNext(bool ignoreWallet)
{
    int i = 0;
    while (i < m_calls.size()) {
        SecretsRequest &request = m_calls[i];
        switch (request.type) {
        case SecretsRequest::GetSecrets:
            if (processGetSecrets(request, ignoreWallet)) {
                m_calls.removeAt(i);
                continue;
            }
            break;
        case SecretsRequest::SaveSecrets:
            if (processSaveSecrets(request, ignoreWallet)) {
                m_calls.removeAt(i);
                continue;
            }
            break;
        case SecretsRequest::DeleteSecrets:
            if (processDeleteSecrets(request, ignoreWallet)) {
                m_calls.removeAt(i);
                continue;
            }
            break;
        }
        ++i;
    }
}

bool SecretAgent::processGetSecrets(SecretsRequest &request, bool ignoreWallet) const
{
    if (m_dialog) {
        return false;
    }

    NetworkManager::Setting::Ptr setting = request.connection_settings->setting(request.setting_name);

    const bool requestNew = request.flags & RequestNew;
    const bool userRequested = request.flags & UserRequested;
    const bool allowInteraction = request.flags & AllowInteraction;
    const bool isVpn = (setting->type() == NetworkManager::Setting::Vpn);

    NMStringMap secretsMap = getSecretsMap(request.connection_settings, request.setting_name);
    if (!requestNew && !secretsMap.isEmpty()) {
        setting->secretsFromStringMap(secretsMap);
        if (!isVpn && setting->needSecrets(requestNew).isEmpty()) {
            // Enough secrets were retrieved from storage
            request.connection[request.setting_name] = setting->secretsToMap();
            sendSecrets(request.connection, request.message);
            return true;
        }
    }

    if (requestNew || (allowInteraction && !setting->needSecrets(requestNew).isEmpty()) || (allowInteraction && userRequested) || (isVpn && allowInteraction)) {
        m_dialog = new PasswordDialog(request.connection, request.flags, request.setting_name);
        connect(m_dialog, SIGNAL(accepted()), this, SLOT(dialogAccepted()));
        connect(m_dialog, SIGNAL(rejected()), this, SLOT(dialogRejected()));
        if (isVpn) {
            m_dialog->setupVpnUi(request.connection_settings);
        } else {
            m_dialog->setupGenericUi(request.connection_settings);
        }
        m_dialog->setSecretsMap(secretsMap);

        if (m_dialog->hasError()) {
            sendError(m_dialog->error(),
                      m_dialog->errorMessage(),
                      request.message);
            delete m_dialog;
            m_dialog = 0;
            return true;
        } else {
            request.dialog = m_dialog;
            request.saveSecretsWithoutReply = !request.connection_settings->permissions().isEmpty();
            qDebug() << Q_FUNC_INFO << request.connection_settings->permissions();
            m_dialog->show();
//            KWindowSystem::setState(m_dialog->winId(), NET::KeepAbove);
//            KWindowSystem::forceActiveWindow(m_dialog->winId());
            return false;
        }
    } else if (isVpn && userRequested) { // just return what we have
        NMVariantMapMap result;
        NetworkManager::VpnSetting::Ptr vpnSetting;
        vpnSetting = request.connection_settings->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
        result.insert("vpn", vpnSetting->secretsToMap());
        sendSecrets(result, request.message);
        return true;
    } else if (setting->needSecrets().isEmpty()) {
        NMVariantMapMap result;
        result.insert(setting->name(), setting->secretsToMap());
        sendSecrets(result, request.message);
        return true;
    } else {
        sendError(SecretAgent::InternalError,
                  QLatin1String("Plasma-nm did not know how to handle the request"),
                  request.message);
        return true;
    }
}

bool SecretAgent::processSaveSecrets(SecretsRequest &request, bool ignoreWallet) const
{
    NetworkManager::ConnectionSettings connectionSettings(request.connection);
    QSettings settings;
    settings.beginGroup(QLatin1String("Connections"));
    foreach (const NetworkManager::Setting::Ptr &setting, connectionSettings.settings()) {
        settings.beginGroup(QLatin1Char('{') % connectionSettings.uuid() % QLatin1Char('}') % QLatin1Char(';') % setting->name());
        NMStringMap secretsMap = setting->secretsToStringMap();
        NMStringMap::ConstIterator i = secretsMap.constBegin();
        while (i != secretsMap.constEnd()) {
            settings.setValue(i.key(), i.value());
            ++i;
        }
        settings.endGroup();
    }
    settings.endGroup();

    if (!request.saveSecretsWithoutReply) {
        QDBusMessage reply = request.message.createReply();
        if (!QDBusConnection::systemBus().send(reply)) {
            qWarning() << "Failed put save secrets reply into the queue";
        }
    }

    return true;
}

bool SecretAgent::processDeleteSecrets(SecretsRequest &request, bool ignoreWallet) const
{
    NetworkManager::ConnectionSettings connectionSettings(request.connection);

    QSettings settings;
    settings.beginGroup(QLatin1String("Connections"));
    foreach (const NetworkManager::Setting::Ptr &setting, connectionSettings.settings()) {
        settings.remove(QLatin1Char('{') % connectionSettings.uuid() % QLatin1Char('}') % QLatin1Char(';') % setting->name());
    }
    settings.endGroup();

    QDBusMessage reply = request.message.createReply();
    if (!QDBusConnection::systemBus().send(reply)) {
        qWarning() << "Failed put delete secrets reply into the queue";
    }

    return true;
}

bool SecretAgent::hasSecrets(const NMVariantMapMap &connection) const
{
    NetworkManager::ConnectionSettings connectionSettings(connection);
    foreach (const NetworkManager::Setting::Ptr &setting, connectionSettings.settings()) {
        if (!setting->secretsToMap().isEmpty()) {
            return true;
        }
    }

    return false;
}

NMStringMap SecretAgent::getSecretsMap(NetworkManager::ConnectionSettings::Ptr connectionSettings, const QString &settingsName) const
{
    QSettings settings;
    settings.beginGroup(QLatin1String("Connections"));
    settings.beginGroup(QLatin1Char('{') % connectionSettings->uuid() % QLatin1Char('}') % QLatin1Char(';') % settingsName);
    NMStringMap ret;

    foreach (const QString &name, settings.allKeys()) {
        ret.insert(name, settings.value(name).toString());
    }
    qDebug() << Q_FUNC_INFO << ret;
    settings.endGroup();
    settings.endGroup();

    return ret;
}

void SecretAgent::sendSecrets(const NMVariantMapMap &secrets, const QDBusMessage &message) const
{
    QDBusMessage reply;
    reply = message.createReply(QVariant::fromValue(secrets));
    if (!QDBusConnection::systemBus().send(reply)) {
        qWarning() << "Failed put the secret into the queue";
    }
}
