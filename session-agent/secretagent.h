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

#ifndef PLASMA_NM_SECRET_AGENT_H
#define PLASMA_NM_SECRET_AGENT_H

#include <NetworkManagerQt/SecretAgent>
#include <NetworkManagerQt/ConnectionSettings>

//#include <kdemacros.h>

class PasswordDialog;
class SecretsRequest {
public:
    enum Type {
        GetSecrets,
        SaveSecrets,
        DeleteSecrets
    };
    explicit SecretsRequest(Type _type) :
        type(_type),
        flags(NetworkManager::SecretAgent::None),
        saveSecretsWithoutReply(false),
        dialog(0)
    {}
    inline bool operator==(const QString &other) const {
        return callId == other;
    }
    Type type;
    QString callId;
    NMVariantMapMap connection;
    QDBusObjectPath connection_path;
    NetworkManager::ConnectionSettings::Ptr connection_settings;
    QString setting_name;
    QStringList hints;
    NetworkManager::SecretAgent::GetSecretsFlags flags;
    /**
     * When a user connection is called on GetSecrets,
     * the secret agent is supposed to save the secrets
     * typed by user, when true proccessSaveSecrets
     * should skip the DBus reply.
     */
    bool saveSecretsWithoutReply;
    QDBusMessage message;
    PasswordDialog *dialog;
};

class SecretAgent : public NetworkManager::SecretAgent
{
    Q_OBJECT
public:
    explicit SecretAgent(QObject* parent = 0);
    virtual ~SecretAgent();

public Q_SLOTS:
    virtual NMVariantMapMap GetSecrets(const NMVariantMapMap&, const QDBusObjectPath&, const QString&, const QStringList&, uint);
    virtual void SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path);
    virtual void DeleteSecrets(const NMVariantMapMap &, const QDBusObjectPath &);
    virtual void CancelGetSecrets(const QDBusObjectPath &, const QString &);

private Q_SLOTS:
    void dialogAccepted();
    void dialogRejected();
    void killDialogs();

private:
    void processNext(bool ignoreWallet = false);
    /**
     * @brief processGetSecrets requests
     * @param request the request we are processing
     * @param ignoreWallet true if the code should avoid Wallet
     * nomally if it failed to open
     * @return true if the item was processed
     */
    bool processGetSecrets(SecretsRequest &request, bool ignoreWallet) const;
    bool processSaveSecrets(SecretsRequest &request, bool ignoreWallet) const;
    bool processDeleteSecrets(SecretsRequest &request, bool ignoreWallet) const;

    /**
     * @brief hasSecrets verifies if the desired connection has secrets to store
     * @param connection map with or without secrets
     * @return true if the connection has secrets, false otherwise
     */
    bool hasSecrets(const NMVariantMapMap &connection) const;
    NMStringMap getSecretsMap(NetworkManager::ConnectionSettings::Ptr settings, const QString &settingsName) const;
    void sendSecrets(const NMVariantMapMap &secrets, const QDBusMessage &message) const;

    mutable PasswordDialog *m_dialog;
    QList<SecretsRequest> m_calls;
};

#endif // PLASMA_NM_SECRET_AGENT_H
