#include "commons.h"
// #include <boost/thread.hpp>
// #include <thread>

// using namespace boost;
// using namespace std;

	TClient::TClient() {
		this->TIMEOUT = 0;
	}
IPAServiceClient *TClient::creatIPAClient(string host, int port) {
	boost::shared_ptr<TSocket> socket(new TSocket(host, port));
	boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	IPAServiceClient *client = new IPAServiceClient(protocol);
	this->transport = transport;
	transport->open();
	return client;
}
SchedulerServiceClient *TClient::creatSchedulerClient(string host, int port) {
	boost::shared_ptr<TSocket> socket(new TSocket(host, port));
	boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        SchedulerServiceClient *client = new SchedulerServiceClient(protocol);
	this->transport = transport;
        transport->open();
        return client;
}
	void TClient::close() {
		this->transport->close();
	}
	
	TServers::TServers() {
	}
void buildNonBlockingServer(int port, boost::shared_ptr<TProcessor> &processor) {
	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	TNonblockingServer server(processor, protocolFactory, port);
	server.serve();
	cout << "should never reach this part" << endl;
}

void TServers::launchSingleThreadThriftServer(int port, boost::shared_ptr<TProcessor> &processor, thread &thrift_server) {
	// thread thrift_server = thread(buildNonBlockingServer, port, processor);		
	thrift_server = thread(buildNonBlockingServer, port, processor);		
	// thrift_server.join();
}
