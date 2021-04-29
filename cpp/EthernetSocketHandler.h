#ifndef ETHERNETSOCKETHANDLER_I_IMPL_H
#define ETHERNETSOCKETHANDLER_I_IMPL_H

#include "EthernetSocketHandler_base.h"

class EthernetSocketHandler_i : public EthernetSocketHandler_base
{
    ENABLE_LOGGING
    public:
        EthernetSocketHandler_i(const char *uuid, const char *label);
        ~EthernetSocketHandler_i();

        void constructor();

        int serviceFunction();

    protected:
};

#endif // ETHERNETSOCKETHANDLER_I_IMPL_H
