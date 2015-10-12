#ifndef MOOSSAFIRSIMDRIVER20141203H
#define MOOSSAFIRSIMDRIVER20141203H

#include "goby/moos/moos_header.h"

#include "goby/common/time.h"

#include "goby/acomms/modemdriver/driver_base.h"
#include "goby/acomms/acomms_helpers.h"


namespace moossafir
{
    class SimDriver : public goby::acomms::ModemDriverBase
    {
      public:
        SimDriver();
        void startup(const goby::acomms::protobuf::DriverConfig& cfg);
        void shutdown();            
        void do_work();
        void handle_initiate_transmission(const goby::acomms::protobuf::ModemTransmission& m);
    
      private:
        void send_message(const goby::acomms::protobuf::ModemTransmission& msg);
        void receive_message(const goby::acomms::protobuf::ModemTransmission& msg);
            
      private:
        enum { DEFAULT_PACKET_SIZE=64 };
        CMOOSCommClient moos_client_;
        goby::acomms::protobuf::DriverConfig driver_cfg_; // configuration given to you at launch
    };
}

#endif
