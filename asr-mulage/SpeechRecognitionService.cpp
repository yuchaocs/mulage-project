// import the thrift headers
// #include <thrift/concurrency/ThreadManager.h>
// #include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
// #include <thrift/server/TSimpleServer.h>
// #include <thrift/server/TThreadPoolServer.h>
// #include <thrift/server/TThreadedServer.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>

// import common utility headers
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <fstream>
#include <sys/time.h>
#include <iomanip>
#include <boost/filesystem.hpp>

#include <stdlib.h>
#include <time.h>

// import opencv headers
#include "opencv2/core/core.hpp"
#include "opencv2/core/types_c.h"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/gpu.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/gpu/gpu.hpp"
#include "boost/program_options.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/algorithm/string/replace.hpp"

// import the service headers
#include "IPAService.h"
#include "SchedulerService.h"
#include "service_constants.h"
#include "service_types.h"
#include "types_constants.h"
#include "types_types.h"
#include "commons.h"

// import the Thread Safe Priority Queue
#include "ThreadSafePriorityQueue.hpp"

// define the namespace
using namespace std;
using namespace cv;

namespace fs = boost::filesystem;

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

namespace std {
	template<> struct less<QuerySpec> {
		bool operator()(const QuerySpec& k1, const QuerySpec& k2) const {
			return k1.budget < k2.budget;
		}
	};
}

// define the constant
// #define THREAD_WORKS 16
// FIXME this is only used for testing, command line option is required
// #define IMAGE_DATABASE "/home/hailong/sirius/sirius-application/image-matching/matching/landmarks/db"
// #define MATCHING_METHOD 1
// #define IMAGE_DATABASE "/home/hailong/sirius/sirius-application/image-matching/matching/landmarks/db"
class SpeechRecognitionServiceHandler : public IPAServiceIf {
	public:
		// put the model training here so that it only needs to
		// be trained once
		SpeechRecognitionServiceHandler() {
			this->budget = 100;
			this->SERVICE_NAME = "asr";
			this->SCHEDULER_IP = "141.212.107.226";
			this->SCHEDULER_PORT = 8888;
			this->SERVICE_IP = "clarity28.eecs.umich.edu";
			this->SERVICE_PORT = 9093;
		}
		~SpeechRecognitionServiceHandler() {
		}
		
		void updatBudget(const double budget) {
			this->budget = budget;
    			cout << "service " << this->SERVICE_NAME << " at " << this->SERVICE_IP << ":" << this->SERVICE_PORT << " update its budget to " << this->budget << endl;
  		}

		void submitQuery(const  ::QuerySpec& query) {
    			// Your implementation goes here
			// 1. record the time when the query comming into the queue 
			struct timeval now;
			gettimeofday(&now, 0);
			int64_t current=(now.tv_sec*1E6+now.tv_usec)/1000;
			newSpec = query;
			
			newSpec.timestamp.push_back(current);
			// 2. put the query to a thread saft queue structure
			qq.push(newSpec);
			// 3. a private class generates a helper thread to process the query from the queue
  		}
		
		void launchQuery() {
			cout << "keep watching whether there are queries in the queue" << endl;
				struct timeval now;
				int64_t queuing_start_time;
				int64_t process_start_time;
				int64_t process_end_time;

			while(1) {
//				std::shared_ptr<QuerySpec> spec = this->qq.wait_and_pop();
				auto spec = this->qq.wait_and_pop();
				queuing_start_time = spec->timestamp.at(spec->timestamp.size()-1);
				cout << "===================================================================" << endl;
				cout << "ASR queue length is " << this->qq.size() << endl;	
				
				gettimeofday(&now, 0);
				process_start_time = (now.tv_sec*1E6+now.tv_usec)/1000;
				spec->timestamp.push_back(process_start_time);
				
			//call the query	
				int rand_input = rand() % this->input_list.size();	
				execute_asr(this->input_list.at(rand_input));
//				execute_asr(spec->input);
				
				gettimeofday(&now, 0);
				process_end_time = (now.tv_sec*1E6+now.tv_usec)/1000;
				this->num_completed++;

				cout << "Queuing time is " << process_start_time - queuing_start_time << " ms, " 
					<< "Serving time is " << process_end_time - process_start_time << " ms."<< endl;	
				cout << "Num of completed queries: " << this->num_completed << endl; 
				cout << "===================================================================" << endl;
				
				fstream log;
				log.open("asr.log", std::ofstream::out | std::ofstream::app);
				log << this->qq.size() << endl;
				log.close();
				
				spec->timestamp.push_back(process_end_time);
				spec->__set_budget( spec->budget - (process_end_time - process_start_time) );

				ThreadSafePriorityQueue<QuerySpec> waiting_queries;		//query queue
				
				int length = this->qq.size();
				for(int i=0;i<length;i++) {
					auto waiting_spec = this->qq.wait_and_pop();
					waiting_spec->__set_budget(spec->budget - (process_end_time - process_start_time) );
					waiting_queries.push(*waiting_spec);
				}
				
				for(int i=0;i<length;i++)
					qq.push( *(waiting_queries.wait_and_pop()) );
				
				this->service_client->submitQuery(*spec);
			}
		}

		void initialize() {
			// 1. register to the command center
			TClient tClient;
			this->scheduler_client = tClient.creatSchedulerClient(this->SCHEDULER_IP, this->SCHEDULER_PORT);
			THostPort hostPort;
			hostPort.ip = this->SERVICE_IP;
			hostPort.port = this->SERVICE_PORT;
			RegMessage regMessage;
			regMessage.app_name = this->SERVICE_NAME;
			regMessage.endpoint = hostPort;
			regMessage.budget = this->budget;
			cout << "registering to command center runnig at " << this->SCHEDULER_IP << ":" << this->SCHEDULER_PORT << endl;	
			RegReply regReply;
			this->scheduler_client->registerBackend(regReply, regMessage);
			cout << "service stage " << this->SERVICE_NAME << " successfully registered itself at " << this->SERVICE_IP << ":" << this->SERVICE_PORT << endl;
			this->service_list = &regReply.service_list;
			if (this->service_list != NULL)
				cout << "Received " << this->service_list->size() << " downstream service candidates" << endl;
			else
				cout << "Received 0 downstream service candidates" << endl;
			if (this->service_list != NULL && this->service_list->size() != 0) {
				THostPort downstream_hostPort = randomAssignService(this->service_list);
				this->DOWNSTREAM_SERVICE_IP = downstream_hostPort.ip;
				this->DOWNSTREAM_SERVICE_PORT = downstream_hostPort.port;
				this->service_client = tClient.creatIPAClient(this->DOWNSTREAM_SERVICE_IP,
						this->DOWNSTREAM_SERVICE_PORT);
			} else {
				cout << "no downstream service candidates are found by command center"<< endl;
				if (regReply.final_stage) {
					cout << "reaching the final service stage of the workflow" << endl;
				}
			}
		
			this->num_completed = 0;	
			//parse the possible inputs	
			ifstream input("input.txt");
			for(string line; getline(input, line); ) {
				input_list.push_back(line);
//				cout << line << endl;
			}
			// 2. launch the helper thread
			boost::thread helper(boost::bind(&SpeechRecognitionServiceHandler::launchQuery, this));
		}

	private:
		vector<String> input_list;
		QuerySpec newSpec;
		int num_completed;
		ThreadSafePriorityQueue<QuerySpec> qq;		//query queue
		double budget;
		string SERVICE_NAME;
		string SCHEDULER_IP;
		int SCHEDULER_PORT;
		string SERVICE_IP;
		int SERVICE_PORT;
		String DOWNSTREAM_SERVICE_IP;
		int DOWNSTREAM_SERVICE_PORT;
		IPAServiceClient *service_client;
		SchedulerServiceClient *scheduler_client;
		vector<THostPort> *service_list; 
		THostPort randomAssignService(vector<THostPort> *service_list) {
			// initialize random seed
			srand(time(NULL));
			// generate random number between 0 and size of the candidate list
			int choice = rand() % service_list->size();
			THostPort hostPort = service_list->at(choice);
			return hostPort;
		}
		string execute_asr(string input) {
			// TODO 1. transform the binary file into a local wav file
			// 2. pass the wav file path to pocketsphinx system call
			// struct timeval tp;
			// gettimeofday(&tp, NULL);
			// long int timestamp = tp.tv_sec * 1000000 + tp.tv_usec;
			// ostringstream sstream;
                        // sstream << timestamp;
			// string wav_path = "query-" + sstream.str() + ".wav";
			// ofstream wavfile(wav_path.c_str(), ios::binary);
                        // wavfile.write(input.c_str(), input.size());
                        // wavfile.close();
			// string cmd = "./pocketsphinx_continuous -infile " + wav_path;
			string cmd = "./pocketsphinx_continuous -logfn /dev/null -infile " + input;
			char *cstr = new char[cmd.length() + 1];
			strcpy(cstr, cmd.c_str());
			return exec_cmd(cstr);
		}
		string exec_cmd(char *cmd) {
			FILE* pipe = popen(cmd, "r");
			if (!pipe)
				return "ERROR";
			char buffer[128];
			string result = "";
			while(!feof(pipe)) {
				if(fgets(buffer, 128, pipe) != NULL)
					result += buffer;
    			}
			pclose(pipe);
			return result;
		}
};

int main(int argc, char **argv){
	// initial the transport factory
	// boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	// boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(9092));
	// initial the protocal factory
	// boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	// initial the request handler
	// boost::shared_ptr<ImageMatchingServiceHandler> handler(new ImageMatchingServiceHandler());
	// initial the processor
	// boost::shared_ptr<TProcessor> processor(new ImageMatchingServiceProcessor(handler));
	// initial the thread manager and factory
	// boost::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(THREAD_WORKS);
	// boost::shared_ptr<PosixThreadFactory> threadFactory = boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
	// threadManager->threadFactory(threadFactory);
	// threadManager->start();
	
	// initial the image matching server
	// TThreadPoolServer server(processor, serverTransport, transportFactory, protocolFactory, threadManager);
	// boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	SpeechRecognitionServiceHandler *speechRecognition = new SpeechRecognitionServiceHandler();
  	boost::shared_ptr<SpeechRecognitionServiceHandler> handler(speechRecognition);
  	// boost::shared_ptr<ImageMatchingServiceHandler> handler(new ImageMatchingServiceHandler());
	boost::shared_ptr<TProcessor> processor(new IPAServiceProcessor(handler));
  	// boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(9092));
  	// boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

  	// TNonblockingServer server(processor, serverTransport, transportFactory, protocolFactory);
  	// TNonblockingServer server(processor, protocolFactory, 9092);
	TServers tServer;
	thread thrift_server;
	cout << "Starting the speech recognition service..." << endl;
	
	fstream log;
	log.open("asr.log", std::ofstream::out);
	log << "qlen" << endl;
	log.close();
	
	// tServer.launchSingleThreadThriftServer(9092, processor);
	tServer.launchSingleThreadThriftServer(9093, processor, thrift_server);
	speechRecognition->initialize();
	// server.serve();
	cout << "Done..." << endl;
	thrift_server.join();
	/*
	while(true) {
	}
	*/
	return 0;
}
