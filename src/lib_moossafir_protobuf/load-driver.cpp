#include "goby/acomms/modemdriver/driver_base.h"
#include "sim-driver.h"

extern "C"
{
    const char* goby_driver_name() { return "moossafir.sim.driver"; }
    goby::acomms::ModemDriverBase* goby_make_driver() { return new moossafir::SimDriver; }    
}
