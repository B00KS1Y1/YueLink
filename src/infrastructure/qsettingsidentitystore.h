#ifndef QSETTINGSIDENTITYSTORE_H
#define QSETTINGSIDENTITYSTORE_H

#include "core/iidentitystore.h"

class QSettingsIdentityStore final : public IIdentityStore
{
public:
    [[nodiscard]] bool load(Network::LocalIdentity *identity,
                            QString *errorMessage) override;
    [[nodiscard]] bool save(const Network::LocalIdentity &identity,
                            QString *errorMessage) override;
};

#endif // QSETTINGSIDENTITYSTORE_H
