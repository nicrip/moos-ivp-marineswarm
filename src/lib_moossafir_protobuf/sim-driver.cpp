// Copyright 2009-2014 Toby Schneider (https://launchpad.net/~tes)
//                     GobySoft, LLC (2013-)
//                     Massachusetts Institute of Technology (2007-2014)
//                     Goby Developers Team (https://launchpad.net/~goby-dev)
// 
//
// This file is part of the Goby Underwater Autonomy Project Libraries
// ("The Goby Libraries").
//
// The Goby Libraries are free software: you can redistribute them and/or modify
// them under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// The Goby Libraries are distributed in the hope that they will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Goby.  If not, see <http://www.gnu.org/licenses/>.



#include "goby/acomms/modemdriver/driver_exception.h"
#include "goby/common/logger.h"
#include "goby/util/binary.h"
#include "goby/moos/moos_string.h"
#include "goby/moos/modem_id_convert.h"
#include "dccl/binary.h"

#include "sim-driver.h"
#include "sim-driver.pb.h"

using goby::glog;
using namespace goby::common::logger;
using goby::acomms::operator<<;

moossafir::SimDriver::SimDriver()
{
}

void moossafir::SimDriver::startup(const goby::acomms::protobuf::DriverConfig& cfg)
{
    glog.is(DEBUG1) && glog << group(glog_out_group()) << "MOOSSafir SIM driver starting up." << std::endl;

    driver_cfg_ = cfg;

    const std::string& moos_server = driver_cfg_.GetExtension(moossafir::Config::moos_server);
    int moos_port = driver_cfg_.GetExtension(moossafir::Config::moos_port);
    moos_client_.Run(moos_server.c_str(), moos_port, "moossafir.SimDriver");

    int i = 0;
    while(!moos_client_.IsConnected())
    {
        glog.is(DEBUG1) &&
            glog << group(glog_out_group())
                 << "Trying to connect to MOOSDB at "<< moos_server << ":" << moos_port << ", try " << i++ << std::endl;
        sleep(1);
    }
    glog.is(DEBUG1) &&
        glog << group(glog_out_group())
             << "Connected to MOOSDB." << std::endl;

    moos_client_.Register(driver_cfg_.GetExtension(moossafir::Config::incoming_moos_var), 0);
    
} 

void moossafir::SimDriver::shutdown()
{
    moos_client_.Close();
} 

void moossafir::SimDriver::handle_initiate_transmission(
    const goby::acomms::protobuf::ModemTransmission& orig_msg)
{
    // copy so we can modify
    goby::acomms::protobuf::ModemTransmission msg = orig_msg;    

    // allows zero to N third parties modify the transmission before sending.
    signal_modify_transmission(&msg);


    switch(msg.type())
    {
        case goby::acomms::protobuf::ModemTransmission::DATA:
        {
            if(driver_cfg_.modem_id() == msg.src())
            {        
                // this is our transmission
        
                if(msg.rate() < driver_cfg_.ExtensionSize(moossafir::Config::rate_to_bytes))
                    msg.set_max_frame_bytes(driver_cfg_.GetExtension(moossafir::Config::rate_to_bytes, msg.rate()));
                else
                    msg.set_max_frame_bytes(DEFAULT_PACKET_SIZE);
        
                // no data given to us, let's ask for some
                if(msg.frame_size() == 0)
                    ModemDriverBase::signal_data_request(&msg);
        
                // don't send an empty message
                send_message(msg);
            }
        }
        break;


        case goby::acomms::protobuf::ModemTransmission::UNKNOWN:
        case goby::acomms::protobuf::ModemTransmission::ACK:
            break;

    }
    
} 

void moossafir::SimDriver::send_message(const goby::acomms::protobuf::ModemTransmission& msg)
{
    std::string encoded;    
    if(msg.frame_size() && msg.frame(0).size())
        encoded = dccl::hex_encode(msg.frame(0));
    
    int range = driver_cfg_.GetExtension(moossafir::Config::ping_range);
    
    std::stringstream out_ss;
    out_ss << "range=" << range
           << ",modem_id=" << driver_cfg_.modem_id();

    if(!encoded.empty())
        out_ss << ",data=" << encoded;
        
    goby::acomms::protobuf::ModemRaw out_raw;
    out_raw.set_raw(out_ss.str());
    ModemDriverBase::signal_raw_outgoing(out_raw);
        
    const std::string& out_moos_var =
        driver_cfg_.GetExtension(moossafir::Config::outgoing_moos_var);

    glog.is(DEBUG1) &&
        glog << group(glog_out_group())  << out_moos_var << ": " << out_ss.str() << std::endl;

    moos_client_.Notify(out_moos_var, out_ss.str());
}


void moossafir::SimDriver::do_work()
{
    MOOSMSG_LIST msgs;
    if(moos_client_.Fetch(msgs))
    {
        for(MOOSMSG_LIST::iterator it = msgs.begin(),
                end = msgs.end(); it != end; ++it)
        {
            const std::string& in_moos_var = driver_cfg_.GetExtension(moossafir::Config::incoming_moos_var);
            if(it->GetKey() == in_moos_var)
            {
                const std::string& value = it->GetString();
                
                glog.is(DEBUG1) &&
                    glog << group(glog_in_group())  << in_moos_var << ": " << value  << std::endl;
                goby::acomms::protobuf::ModemRaw in_raw;
                in_raw.set_raw(value);
                ModemDriverBase::signal_raw_incoming(in_raw);

                try
                {
                    goby::acomms::protobuf::ModemTransmission m;
                    std::string data;
                    int id;
                    if(goby::moos::val_from_string(data, value, "data"))
                        m.add_frame(dccl::hex_decode(data));

                    if(!goby::moos::val_from_string(id, value, "modem_id"))
                        throw(std::runtime_error("No `modem_id` field"));

                    m.set_src(id);
                    receive_message(m);
                }
                catch(std::exception& e)
                {
                    glog.is(DEBUG1) &&
                        glog << group(glog_in_group())  << warn << "Failed to parse incoming  message: " << e.what() << std::endl;
                }                
            }            
        }
    }
} 

void moossafir::SimDriver::receive_message(const goby::acomms::protobuf::ModemTransmission& msg)
{   
    ModemDriverBase::signal_receive(msg);
}
