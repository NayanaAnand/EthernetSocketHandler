#include <iostream>
#include "ossie/ossieSupport.h"

#include "EthernetSocketHandler.h"
int main(int argc, char* argv[])
{
    EthernetSocketHandler_i* EthernetSocketHandler_servant;
    Component::start_component(EthernetSocketHandler_servant, argc, argv);
    return 0;
}

