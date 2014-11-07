/****************************
	Name:         UdtWrapper
	Author:       Catofes
	Date:         2014-11-1
	License:      All Right Reserved
	Description:  A tunnel to exchange tcp stream via udt.
*****************************/

#ifndef _CONFIG_
#define _CONFIG_

class Config
{
	public:
		int listenPort;
		string remoteAddress;
		int remotePort;
		int maxPending;
		bool IPV6;

		Config(){
			maxPending = 1024;
			IPV6 = false;
		};
};

#endif
