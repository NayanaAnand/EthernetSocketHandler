#ifndef ETHERNETSOCKETHANDLER_BASE_IMPL_BASE_H
#define ETHERNETSOCKETHANDLER_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class EthernetSocketHandler_base : public Component, protected ThreadedComponent
{
    public:
        EthernetSocketHandler_base(const char *uuid, const char *label);
        ~EthernetSocketHandler_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

    private:
};
#endif // ETHERNETSOCKETHANDLER_BASE_IMPL_BASE_H
