Probe Request Flooder
=====================
This is a small tool to generate a probe request flood on a given channel. It
is expecetd that, it can totally block all APs with heavy IO if 802.11w protection not applied. It is based on libnl.

options:

* -i ifname, required, the interface name  
* -c channel number, required, the channel number specified, 1 for 2412MHz  
* -t flood times, optional, how many probe requests will to transmit  
* -d debug flag, optional, display debug information

License: [GPL2](http://www.gnu.org/licenses/gpl-2.0.html)

Author: [Li Shijian](http://github.com/furtherLee)
