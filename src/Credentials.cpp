#include "Credentials.h"

Credentials::Credentials() {}

bool Credentials::contains(const QString &connectionId) const
{
    return !m_user[connectionId].isEmpty();
}

void Credentials::get(const QString &connectionId, QString *user, QString *pass) const
{
    *user = m_user[connectionId];
    *pass = m_pass[connectionId];
}

void Credentials::remove(const QString &connectionId)
{
    m_user.remove(connectionId);
    m_pass.remove(connectionId);
}

void Credentials::set(const QString &connectionId, const QString &user, const QString &pass)
{
    //TODO use save storage (like libsecret) to store password
    m_user[connectionId] = user;
    m_pass[connectionId] = pass;
}
