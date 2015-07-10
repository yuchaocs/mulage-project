#ifndef COMMONS_H
#define COMMONS_H

#include "IPAService.h"
#include "SchedulerService.h"
#include <string>
#include <boost/thread.hpp>
#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>


using namespace std;
using namespace boost;

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

class TClient {
	private:
		int TIMEOUT;
		boost::shared_ptr<TTransport> transport;
	public:
		TClient();
		IPAServiceClient *creatIPAClient(string host, int port);
		SchedulerServiceClient *creatSchedulerClient(string host, int port);
		void close();
};

class TServers {
	private:
		// void buildNonBlockingServer(int port, TProcessor *processor);
	public:
		TServers();
		void launchSingleThreadThriftServer(int port, boost::shared_ptr<TProcessor> &processor, thread &thrift_server);
};

#endif
