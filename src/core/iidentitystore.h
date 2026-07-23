#ifndef IIDENTITYSTORE_H
#define IIDENTITYSTORE_H

#include "networktypes.h"

#include <QString>

class IIdentityStore
{
public:
    virtual ~IIdentityStore() = default;

    [[nodiscard]] virtual bool load(Network::LocalIdentity *identity,
                                    QString *errorMessage) = 0;
    [[nodiscard]] virtual bool save(const Network::LocalIdentity &identity,
                                    QString *errorMessage) = 0;
};

#endif // IIDENTITYSTORE_H
