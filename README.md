# moos-ivp-marineswarm
Repository for marine swarm applications (SM work)

Setting up internet sharing to PABLO Boxes  
==========================================  
Using instructions originially from:  https://www.raspberrypi.org/forums/viewtopic.php?t=6997&p=87671  

On the PABLO Box, make sure the /etc/network/interfaces file is set up as follows:  

    auto lo  
    
    iface lo inet loopback  
    iface eth0 inet dhcp  
    
    auto eth0:1  
    iface eth0:1 inet static  
      address 192.168.254.100  
      gateway 192.168.254.1  
      netmask 255.255.255.0  
      dns-nameservers 8.8.8.8 8.8.4.4  

And make sure the /etc/resolv.conf file is set up as follows:

    nameserver 10.42.0.1
    nameserver 8.8.8.8

On your laptop set up a static ethernet with the following properties:

    Method: Manual
    Address: 192.168.254.1
    Netmask: 255.255.255.0
    Gateway: 0.0.0.0
    DNS Servers: 8.8.8.8

And connect to the internet via WiFi. Finally, enter the following commands on your laptop:

    sudo iptables --table nat --append POSTROUTING --out-interface wlan0 -j MASQUERADE
    sudo iptables --append FORWARD --in-interface eth0 -j ACCEPT
    sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"


Required Libraries  
==================  

Goby and Protobuf  
-----------------  
Developer libraries:  

    sudo add-apt-repository ppa:dccl-dev/ppa
    sudo add-apt-repository ppa:goby-dev/ppa
    sudo apt-get update
    sudo apt-get install libgoby2-dev libgoby2

Optional MOOS Applications (pAcommsHandler, etc):  

    sudo apt-get install goby2-moos

  
