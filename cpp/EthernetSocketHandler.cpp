/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "EthernetSocketHandler.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <linux/udp.h>

#define PORT 8080
PREPARE_LOGGING(EthernetSocketHandler_i)

char g_Ip[18];

EthernetSocketHandler_i::EthernetSocketHandler_i(const char *uuid, const char *label) :
    EthernetSocketHandler_base(uuid, label)
{
    // Avoid placing constructor code here. Instead, use the "constructor" function.

}

EthernetSocketHandler_i::~EthernetSocketHandler_i()
{

}

void EthernetSocketHandler_i::constructor()
{
    /***********************************************************************************
     This is the RH constructor. All properties are properly initialized before this function is called 
    ***********************************************************************************/
}

/***********************************************************************************************

    Basic functionality:

        The service function is called by the serviceThread object (of type ProcessThread).
        This call happens immediately after the previous call if the return value for
        the previous call was NORMAL.
        If the return value for the previous call was NOOP, then the serviceThread waits
        an amount of time defined in the serviceThread's constructor.
        
    SRI:
        To create a StreamSRI object, use the following code:
                std::string stream_id = "testStream";
                BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);

    Time:
        To create a PrecisionUTCTime object, use the following code:
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();

        
    Ports:

        Data is passed to the serviceFunction through by reading from input streams
        (BulkIO only). The input stream class is a port-specific class, so each port
        implementing the BulkIO interface will have its own type-specific input stream.
        UDP multicast (dataSDDS and dataVITA49) ports do not support streams.

        The input stream from which to read can be requested with the getCurrentStream()
        method. The optional argument to getCurrentStream() is a floating point number that
        specifies the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.  Constants have been defined for these values, bulkio::Const::BLOCKING and
        bulkio::Const::NON_BLOCKING.

        More advanced uses of input streams are possible; refer to the REDHAWK documentation
        for more details.

        Input streams return data blocks that automatically manage the memory for the data
        and include the SRI that was in effect at the time the data was received. It is not
        necessary to delete the block; it will be cleaned up when it goes out of scope.

        To send data using a BulkIO interface, create an output stream and write the
        data to it. When done with the output stream, the close() method sends and end-of-
        stream flag and cleans up.

        NOTE: If you have a BULKIO dataSDDS or dataVITA49  port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // This example assumes that the component has two ports:
            //  An input (provides) port of type bulkio::InShortPort called dataShort_in
            //  An output (uses) port of type bulkio::OutFloatPort called dataFloat_out
            // The mapping between the port and the class is found
            // in the component base class header file

            bulkio::InShortStream inputStream = dataShort_in->getCurrentStream();
            if (!inputStream) { // No streams are available
                return NOOP;
            }

            // Get the output stream, creating it if it doesn't exist yet
            bulkio::OutFloatStream outputStream = dataFloat_out->getStream(inputStream.streamID());
            if (!outputStream) {
                outputStream = dataFloat_out->createStream(inputStream.sri());
            }

            bulkio::ShortDataBlock block = inputStream.read();
            if (!block) { // No data available
                // Propagate end-of-stream
                if (inputStream.eos()) {
                   outputStream.close();
                }
                return NOOP;
            }

            if (block.sriChanged()) {
                // Update output SRI
                outputStream.sri(block.sri());
            }

            // Get read-only access to the input data
            redhawk::shared_buffer<short> inputData = block.buffer();

            // Acquire a new buffer to hold the output data
            redhawk::buffer<float> outputData(inputData.size());

            // Transform input data into output data
            for (size_t index = 0; index < inputData.size(); ++index) {
                outputData[index] = (float) inputData[index];
            }

            // Write to the output stream; outputData must not be modified after
            // this method call
            outputStream.write(outputData, block.getStartTime());

            return NORMAL;

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the data block's complex() method will return true. Data blocks
        provide a cxbuffer() method that returns a complex interpretation of the
        buffer without making a copy:

            if (block.complex()) {
                redhawk::shared_buffer<std::complex<short> > inData = block.cxbuffer();
                redhawk::buffer<std::complex<float> > outData(inData.size());
                for (size_t index = 0; index < inData.size(); ++index) {
                    outData[index] = inData[index];
                }
                outputStream.write(outData, block.getStartTime());
            }

        Interactions with non-BULKIO ports are left up to the component developer's discretion
        
    Messages:
    
        To receive a message, you need (1) an input port of type MessageEvent, (2) a message prototype described
        as a structure property of kind message, (3) a callback to service the message, and (4) to register the callback
        with the input port.
        
        Assuming a property of type message is declared called "my_msg", an input port called "msg_input" is declared of
        type MessageEvent, create the following code:
        
        void EthernetSocketHandler_i::my_message_callback(const std::string& id, const my_msg_struct &msg){
        }
        
        Register the message callback onto the input port with the following form:
        this->msg_input->registerMessage("my_msg", this, &EthernetSocketHandler_i::my_message_callback);
        
        To send a message, you need to (1) create a message structure, (2) a message prototype described
        as a structure property of kind message, and (3) send the message over the port.
        
        Assuming a property of type message is declared called "my_msg", an output port called "msg_output" is declared of
        type MessageEvent, create the following code:
        
        ::my_msg_struct msg_out;
        this->msg_output->sendMessage(msg_out);

    Accessing the Application and Domain Manager:
    
        Both the Application hosting this Component and the Domain Manager hosting
        the Application are available to the Component.
        
        To access the Domain Manager:
            CF::DomainManager_ptr dommgr = this->getDomainManager()->getRef();
        To access the Application:
            CF::Application_ptr app = this->getApplication()->getRef();
    
    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given the property id as its name.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (EthernetSocketHandler_base).
    
        Simple sequence properties are mapped to "std::vector" of the simple type.
        Struct properties, if used, are mapped to C++ structs defined in the
        generated file "struct_props.h". Field names are taken from the name in
        the properties file; if no name is given, a generated name of the form
        "field_n" is used, where "n" is the ordinal number of the field.
        
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A boolean called scaleInput
              
            if (scaleInput) {
                dataOut[i] = dataIn[i] * scaleValue;
            } else {
                dataOut[i] = dataIn[i];
            }
            
        Callback methods can be associated with a property so that the methods are
        called each time the property value changes.  This is done by calling 
        addPropertyListener(<property>, this, &EthernetSocketHandler_i::<callback method>)
        in the constructor.

        The callback method receives two arguments, the old and new values, and
        should return nothing (void). The arguments can be passed by value,
        receiving a copy (preferred for primitive types), or by const reference
        (preferred for strings, structs and vectors).

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A struct property called status
            
        //Add to EthernetSocketHandler.cpp
        EthernetSocketHandler_i::EthernetSocketHandler_i(const char *uuid, const char *label) :
            EthernetSocketHandler_base(uuid, label)
        {
            addPropertyListener(scaleValue, this, &EthernetSocketHandler_i::scaleChanged);
            addPropertyListener(status, this, &EthernetSocketHandler_i::statusChanged);
        }

        void EthernetSocketHandler_i::scaleChanged(float oldValue, float newValue)
        {
            RH_DEBUG(this->_baseLog, "scaleValue changed from" << oldValue << " to " << newValue);
        }
            
        void EthernetSocketHandler_i::statusChanged(const status_struct& oldValue, const status_struct& newValue)
        {
            RH_DEBUG(this->_baseLog, "status changed");
        }
            
        //Add to EthernetSocketHandler.h
        void scaleChanged(float oldValue, float newValue);
        void statusChanged(const status_struct& oldValue, const status_struct& newValue);

    Logging:

        The member _baseLog is a logger whose base name is the component (or device) instance name.
        New logs should be created based on this logger name.

        To create a new logger,
            rh_logger::LoggerPtr my_logger = this->_baseLog->getChildLogger("foo");

        Assuming component instance name abc_1, my_logger will then be created with the 
        name "abc_1.user.foo".


************************************************************************************************/

int NetToAscii(unsigned long ip)
{
   unsigned char *bytes = (unsigned char *)&ip;
   snprintf(g_Ip, sizeof(g_Ip), "%d.%d.%d.%d",
   bytes[0], bytes[1], bytes[2], bytes[3]);
   return 1;
}

int EthernetSocketHandler_i::serviceFunction()
{
    //RH_DEBUG(this->_baseLog, "serviceFunction() example log message");
        	int loc_buff_len;
        	        int loc_sock_len;
        	        int loc_sd;
        	        char loc_buffer[1560];

        	        struct sockaddr_in s_addr;
        	        struct ethhdr *loc_ethhd_ptr;
        	        struct iphdr *loc_iphd_ptr;
			struct icmphdr *loc_icmphdr_ptr;
			struct udphdr *loc_udphdr_ptr;

        	        loc_sd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

        	        if(loc_sd < 0)
        	        {
        	                perror("Socket creation is failed\n");
        	                LOG_INFO(EthernetSocketHandler_i, "Socket creation is failed");
        	                exit(0);
        	        }
        	        else
        	        {
        	                printf("socket creation is done\n");
        	                LOG_INFO(EthernetSocketHandler_i, "Socket creation is done");
        	        }

        	        loc_sock_len = sizeof(s_addr);
        	        memset(&loc_buffer, 0, sizeof(loc_buffer));
        	        while(1)
        	        {
        	                loc_buff_len = recvfrom(loc_sd, &loc_buffer, sizeof(loc_buffer),  0, (struct sockaddr*)&s_addr,(socklen_t *)&loc_sock_len);

        	                if(loc_buff_len > 0)
        	                {
        	                    loc_ethhd_ptr = (struct ethhdr *)(loc_buffer);
        	                    if(ntohs(loc_ethhd_ptr->h_proto) == ETH_P_IP)
        	                    {
        	                        loc_iphd_ptr = (struct iphdr *)(loc_buffer + sizeof(struct ethhdr));
        	                        //printf("%d\n", loc_iphd_ptr->protocol);
        	                        switch(loc_iphd_ptr->protocol)
        	                        {
        	                            case 1:
        	                            {
						loc_icmphdr_ptr = (struct icmphdr *)(loc_buffer + sizeof(struct icmphdr));
						//if((loc_icmphdr_ptr->type == ICMP_ECHO) && (loc_icmphdr_ptr->code == 0))
						//{
						    NetToAscii(loc_iphd_ptr->saddr);
        	                                    LOG_INFO(EthernetSocketHandler_i, "ICMP packet received from " << g_Ip);
						//}
        	                                break;
        	                            }
					    case 17:
					    {
						loc_udphdr_ptr = (struct udphdr *)(loc_buffer + sizeof(struct udphdr));
						if(loc_udphdr_ptr->source == PORT)
						{
						loc_buffer[loc_buff_len] = '\0';
						LOG_INFO(EthernetSocketHandler_i, "Data received : " << loc_buffer);
						memset(&loc_buffer, 0, sizeof(loc_buffer));
						strcpy(loc_buffer, "SCA compliance - Data communication");
						s_addr.sin_family = AF_INET;
    						s_addr.sin_port = htons(PORT);
    						s_addr.sin_addr.s_addr = INADDR_ANY;
						loc_buff_len = sendto(loc_sd, (const char *)loc_buffer, strlen(loc_buffer), MSG_CONFIRM, (const struct sockaddr*)&s_addr, sizeof(s_addr));
					    	
						if(loc_buff_len < 0)
						{
							LOG_INFO(EthernetSocketHandler_i, "Data sent back successfully");
						}
						}
						break;
					    }
        	                            default:
        	                                break;
        	                         }
        	                     }
        	                }
        	         }

        	        close(loc_sd);


	return 0;


}
