#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <QMap>
#include <QString>

class Credentials
{
  public:
    Credentials();
    bool contains(const QString &connectionId) const;
    void get(const QString &connectionId, QString *user, QString *pass) const;
    void remove(const QString &connectionId);
    void set(const QString &connectionId, const QString &user, const QString &pass);

  private:
    QMap<QString /*connectionId*/, QString> m_user;
    QMap<QString /*connectionId*/, QString> m_pass;
};

#endif // CREDENTIALS_H
