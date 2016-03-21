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

		int finTimeout; //gc will release sockets which in halfclose stat and no package go throw in fin_timeout seconds;
		int keepliveTimeout;//gc will release sockets which no package go throw in keeplive_time seconds;

		/************************************
		 * gc follow this model
		 *
		 *		if_gc		=	random(0,1) < (t / max_t)^2 && random(0,1) < (nowsockets/s)^2;
		 *		gc_num		=	random(1, (sockets size) * p * (t / max_t) ^ 3  
		 *
		 *		"t" is average time accross between two tsocket wakeup.
		 *				t = 0.2*t + (time accross now)*0.8
		 *
		 *		So in each time tcp wakeup , caculate t and to choice if start garbage collection. 
		 *		Garbage collection will try to collection gc_num sockets.
		 *
		 ***********************************/

		float maxT; //microsecond
		float maxS; //if sockets equal maxS, gc will start without consider sockets num;
		float p; //average garbage socket in sockets

		int maxSlowBuffer;

		Config(){
			maxPending = 1024;
			IPV6 = false;
			finTimeout = 60;
			keepliveTimeout = 8000;
			maxT = 100000;
			maxS = 900;
			p = 0.2;
			maxSlowBuffer=2000000;
		};
};

#endif
