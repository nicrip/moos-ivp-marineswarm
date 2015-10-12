import lcm
from exlcm import joystick_t

def my_handler(channel, data):
    msg = joystick_t.decode(data)
    print("Received message on channel \"%s\"" % channel)
    print("   values    = %s" % str(msg.values))
    print("")
lc = lcm.LCM()
subscription = lc.subscribe("JOYSTICK", my_handler)

try:
    while True:
        lc.handle()
except KeyboardInterrupt:
    pass
