# moos-ivp-marineswarm

Repository for marine swarm applications (SM work) with moos-ivp - currently only formation control behaviours and associated infrastructure. See thesis at:

https://drive.google.com/open?id=0B7C482m0cX0fSU5uVF9GVWlaYVE


## Setting up internet sharing to PABLO Boxes  

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
    
Then restart networking on PABLO:

    sudo service networking restart


##Required Libraries  

###Goby and Protobuf  

Aptitude via - developer libraries:  

    sudo add-apt-repository ppa:dccl-dev/ppa
    sudo add-apt-repository ppa:goby-dev/ppa
    sudo apt-get update
    sudo apt-get install libgoby2-dev libgoby2

Optional MOOS Applications (pAcommsHandler, etc):  

    sudo apt-get install goby2-moos

OR source:

    wget "https://launchpad.net/goby/2.1/2.1.1/+download/goby2-2.1.1.tar.gz"  
    tar -xvzf goby2-2.1.1.tar.gz  

then get dccl, and install:

    bzr co lp:dccl/3.0 dccl3  
    sudo apt-get install cmake libboost-dev libprotobuf-dev libprotoc-dev protobuf-compiler
    cd dccl3
    ./build.sh

then build goby:  

    cd goby; sudo ./DEPENDENCIES debian/ubuntu
    sudo ./build.sh install
    
Issues when building goby on PABLO:

    building goby with MOOS:  
    cd goby/build
    sudo cmake .. -DMOOS_DIR=/home/student/moos-ivp/MOOS/MOOSCore -Dbuild_moos=ON  
    sudo make install  
    
    possible dependencies:
    you may need libproj-dev and gdal-bin
    you may need libgmp3-dev
    you may need to edit the boost header at /usr/include/boost/thread/xtime.hpp, replacing all instances of TIME_UTC with TIME_UTC_
    
###Armadillo  
 
Dependencies via aptitude:

    sudo apt-get install libarpack++2-dev liblapack-dev libblas-dev libopenblas-base

install OpenBLAS following instructions at: http://www.openblas.net/: ISSUES!!!

    wget "http://github.com/xianyi/OpenBLAS/archive/v0.2.14.tar.gz"
    tar -xvzf v0.2.14.tar.gz
    cd OpenBLAS-0.2.14
    make

Install Armadillo from source:

    wget "http://sourceforge.net/projects/arma/files/armadillo-6.100.0.tar.gz"
    tar -xvzf armadillo-6.100.0.tar.gz
    cd armadillo-6.100.0.tar.gz
    cmake .
    sudo make install

###NLOpt  

Install NLOpt from source: ISSUES!!!

    wget "http://ab-initio.mit.edu/nlopt/nlopt-2.4.2.tar.gz"
    tar -xvzf nlopt-2.4.2.tar.gz
    cd nlopt-2.4.2.tar.gz
    ./configure
    sudo make install

  
