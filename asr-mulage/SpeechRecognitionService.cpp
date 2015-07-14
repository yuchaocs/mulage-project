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
//#include "ThreadSafePriorityQueue.hpp"
#include "ThreadPool.hpp"

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
//			return k1.budget < k2.budget;
			return k1.floatingBudget < k2.floatingBudget;
		}
	};
}

class FIFO_QuerySpec {
	public:
		FIFO_QuerySpec(const QuerySpec& spec){
			this->qs = spec;
			struct timeval now;
			gettimeofday(&now, 0);
			this->ts=now.tv_sec*1E6+now.tv_usec;
		}
		
		QuerySpec qs;
		int64_t ts;
};

namespace std {
	template<> struct less<FIFO_QuerySpec> {
		bool operator()(const FIFO_QuerySpec& k1, const FIFO_QuerySpec& k2) const {
			return k1.ts < k2.ts;
		}
	};
}

#define NEXT_STAGE "imm"
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

		SpeechRecognitionServiceHandler(String service_ip, int service_port, String scheduler_ip, int scheduler_port, String queue_type, int core, float freq) {
//			this->budget = 100;
			this->budget = freq;
			this->SERVICE_NAME = "asr";
			this->SCHEDULER_IP = scheduler_ip;
			this->SCHEDULER_PORT = scheduler_port;
			this->SERVICE_IP = service_ip;
			this->SERVICE_PORT = service_port;
			this->QUEUE_TYPE = queue_type;
			this->CORE = core;
			this->input_recycle = 100;
			cout << "service: "<< this->SERVICE_IP <<":"<<this->SERVICE_PORT << ", scheduler: "<< this->SCHEDULER_IP <<":"<<this->SCHEDULER_PORT<< endl;
		}
		~SpeechRecognitionServiceHandler() {
		}
		
		void stealParentInstance(const  ::THostPort& hostPort) {
			TClient tClient_service;
            IPAServiceClient *service_client = tClient_service.creatIPAClient(hostPort.ip, hostPort.port);
			tClient_service.close();

			std::vector<::QuerySpec> queryList;
			service_client->stealQueuedQueries(queryList);

			if(this->QUEUE_TYPE == "priority") {
				for(std::vector<::QuerySpec>::iterator it = queryList.begin(); it != queryList.end(); ++it) {
					struct timeval now;
					gettimeofday(&now, 0);
					int64_t current=(now.tv_sec*1E6+now.tv_usec)/1000;
					it->timestamp.at(it->timestamp.size()-1).queuing_start_time =  current - it->timestamp.at(it->timestamp.size()-1).queuing_start_time;
					it->timestamp.at(it->timestamp.size()-1).instance_id = this->SERVICE_NAME + "_" + this->SERVICE_IP + "_" + to_string(this->SERVICE_PORT);
					this->qq.push(*it);
				}
			}
			else if(this->QUEUE_TYPE == "fifo") {
				for(std::vector<::QuerySpec>::iterator it = queryList.begin(); it != queryList.end(); ++it) {
					
					struct timeval now;
					gettimeofday(&now, 0);
					int64_t current=(now.tv_sec*1E6+now.tv_usec)/1000;
					it->timestamp.at(it->timestamp.size()-1).queuing_start_time =  current - it->timestamp.at(it->timestamp.size()-1).queuing_start_time;
					it->timestamp.at(it->timestamp.size()-1).instance_id = this->SERVICE_NAME + "_" + this->SERVICE_IP + "_" + to_string(this->SERVICE_PORT);
					FIFO_QuerySpec newFIFOSpec(*it);
					this->fifo_qq.push(newFIFOSpec);
				}
			}
		}
		void stealQueuedQueries(std::vector< ::QuerySpec> & querySpecList) {
			if(this->QUEUE_TYPE == "priority") {
				int stealNum = this->qq.size()/2;
			
				auto querySpec = this->qq.try_pop();
				
				while(stealNum > -1 && querySpec != nullptr ) {
					LatencySpec latencySpec = querySpec->timestamp.at(querySpec->timestamp.size()-1);
					struct timeval now;
					gettimeofday(&now, 0);
					int64_t current=(now.tv_sec*1E6+now.tv_usec)/1000;
					querySpec->timestamp.at(querySpec->timestamp.size()-1).queuing_start_time = current - querySpec->timestamp.at(querySpec->timestamp.size()-1).queuing_start_time;
					querySpecList.push_back(*querySpec);
					stealNum--;
				}
			}
			else if(this->QUEUE_TYPE == "fifo") {
				int stealNum = this->fifo_qq.size()/2;
				auto querySpec = this->fifo_qq.try_pop();
				
				while(stealNum > -1 && querySpec != nullptr ) {
					LatencySpec latencySpec = querySpec->qs.timestamp.at(querySpec->qs.timestamp.size()-1);
					struct timeval now;
					gettimeofday(&now, 0);
					int64_t current=(now.tv_sec*1E6+now.tv_usec)/1000;
					querySpec->qs.timestamp.at(querySpec->qs.timestamp.size()-1).queuing_start_time = current - querySpec->qs.timestamp.at(querySpec->qs.timestamp.size()-1).queuing_start_time;
					querySpecList.push_back(querySpec->qs);
					stealNum--;
				}
			}
		}
			

		int32_t reportQueueLength() {
			if(this->QUEUE_TYPE == "priority") {
				return this->qq.size();
			}
			else if(this->QUEUE_TYPE == "fifo") {
				return this->fifo_qq.size();
			}
		}		
		void updatBudget(const double budget) {
			this->budget = budget;
			char command[50];
			sprintf(command, "sudo cpufreq-set -c %d -f %d", this->CORE, (int)(budget*1000000));
			int ret = std::system(command);
    			cout << "service " << this->SERVICE_NAME << " at " << this->SERVICE_IP << ":" << this->SERVICE_PORT << " update its budget to " << this->budget << endl;
  		}

		void submitQuery(const  ::QuerySpec& query) {
    			// Your implementation goes here
			// 1. record the time when the query comming into the queue 
			struct timeval now;
			gettimeofday(&now, 0);
			int64_t current=(now.tv_sec*1E6+now.tv_usec)/1000;
		
			LatencySpec latencySpec;
			latencySpec.instance_id = this->SERVICE_NAME+"_"+this->SERVICE_IP+"_"+to_string(this->SERVICE_PORT);
			latencySpec.queuing_start_time = current;

			if(this->QUEUE_TYPE == "priority") {
				newSpec = query;
				newSpec.timestamp.push_back(latencySpec);
			
				newSpec.floatingBudget=newSpec.budget;
				ThreadSafePriorityQueue<QuerySpec> waiting_queries;		//query queue
				auto waiting_spec = this->qq.try_pop();
			
				while(waiting_spec != nullptr) {
					waiting_spec->floatingBudget=waiting_spec->budget - (current- waiting_spec->timestamp.at(waiting_spec->timestamp.size()-1).queuing_start_time);
					waiting_queries.push(*waiting_spec);
					waiting_spec = this->qq.try_pop();
				}
				
				int length = waiting_queries.size();	
				for(int i=0;i<length;i++)
				qq.push( *(waiting_queries.wait_and_pop()) );
			
			// 2. put the query to a thread saft queue structure
				qq.push(newSpec);
			}
			else if(this->QUEUE_TYPE == "fifo") {
				FIFO_QuerySpec newFIFOSpec(query);
//				newFIFOSpec.qs.timestamp.push_back(current);
				newFIFOSpec.qs.timestamp.push_back(latencySpec);
				
				newFIFOSpec.qs.floatingBudget=newFIFOSpec.qs.budget;
				
				ThreadSafePriorityQueue<FIFO_QuerySpec> waiting_queries;		//query queue
				auto waiting_spec = this->fifo_qq.try_pop();
				while(waiting_spec != nullptr) {
					waiting_spec->qs.floatingBudget=waiting_spec->qs.budget - (current- waiting_spec->qs.timestamp.at(waiting_spec->qs.timestamp.size()-1).queuing_start_time);
					waiting_queries.push(*waiting_spec);
					waiting_spec = this->fifo_qq.try_pop();
				}
				
				int length = waiting_queries.size();	
				for(int i=0;i<length;i++)
					fifo_qq.push( *(waiting_queries.wait_and_pop()) );
				// 2. put the query to a thread saft queue structure
				fifo_qq.push(newFIFOSpec);
			}
			// 3. a private class generates a helper thread to process the query from the queue
  		}
		
		void launchQuery() {
			cout << "keep watching whether there are queries in the queue" << endl;
				struct timeval now;
				int64_t queuing_start_time;
				int64_t process_start_time;
				int64_t process_end_time;
				int log_file = rand() % this->input_list.size();
				
				fstream log;
				log.open("asr"+std::to_string(this->SERVICE_PORT)+".csv", std::ofstream::out);
				log << "qlen" << endl;
				log.close();

			while(1) {
//				std::shared_ptr<QuerySpec> spec = this->qq.wait_and_pop();
				if(this->QUEUE_TYPE == "priority") {
					auto spec = this->qq.wait_and_pop();
					queuing_start_time = spec->timestamp.at(spec->timestamp.size()-1).queuing_start_time;
//					queuing_start_time = spec->timestamp.at(spec->timestamp.size()-1);
					cout << "===================================================================" << endl;
					cout << "ASR queue length is " << this->qq.size() << endl;	
				
					gettimeofday(&now, 0);
					process_start_time = (now.tv_sec*1E6+now.tv_usec)/1000;
					spec->timestamp.at(spec->timestamp.size()-1).serving_start_time = process_start_time;
//					spec->timestamp.push_back(process_start_time);
				
			//call the query	
					int rand_input = atoi(spec->name.c_str()) % this->input_recycle;
				// int rand_input = rand() % this->input_list.size();	
					execute_asr(this->input_list.at(rand_input));
//					execute_asr(spec->input);
				
					gettimeofday(&now, 0);
					process_end_time = (now.tv_sec*1E6+now.tv_usec)/1000;
					this->num_completed++;

					cout << "Queuing time is " << process_start_time - queuing_start_time << " ms, " 
						<< "Serving time is " << process_end_time - process_start_time << " ms."<< endl;	
					cout << "Num of completed queries: " << this->num_completed << endl; 
					cout << "===================================================================" << endl;
				
					log.open("asr"+std::to_string(this->SERVICE_PORT)+".csv", std::ofstream::out | std::ofstream::app);
					log << this->qq.size() << endl;
					log.close();
				
					spec->timestamp.at(spec->timestamp.size()-1).serving_end_time = process_end_time;
//					spec->timestamp.push_back(process_end_time);
//					spec->__set_budget( spec->budget - (process_end_time - process_start_time) );
					spec->__set_budget( spec->budget - (process_end_time - queuing_start_time) );
/*
					ThreadSafePriorityQueue<QuerySpec> waiting_queries;		//query queue
				
					auto waiting_spec = this->qq.try_pop();
					while(waiting_spec != nullptr) {
						waiting_spec->__set_budget(spec->budget - (process_end_time - process_start_time) );
						waiting_queries.push(*waiting_spec);
						waiting_spec = this->qq.try_pop();
					}
					int length = waiting_queries.size();
					for(int i=0;i<length;i++)
						qq.push( *(waiting_queries.wait_and_pop()) );
*/				
					THostPort hostport;
					TClient tClient_scheduler;
					SchedulerServiceClient *scheduler_client = tClient_scheduler.creatSchedulerClient(this->SCHEDULER_IP, this->SCHEDULER_PORT);
					scheduler_client->consultAddress(hostport, NEXT_STAGE);
					tClient_scheduler.close();

					TClient tClient_service;
					IPAServiceClient *service_client = tClient_service.creatIPAClient(hostport.ip, hostport.port);
					service_client->submitQuery(*spec);
					tClient_service.close();
				} 
				else if (this->QUEUE_TYPE == "fifo") {
					auto spec = this->fifo_qq.wait_and_pop();
//					queuing_start_time = spec->qs.timestamp.at(spec->qs.timestamp.size()-1);
					queuing_start_time = spec->qs.timestamp.at(spec->qs.timestamp.size()-1).queuing_start_time;
					cout << "=============================================================" << endl;
					cout << "ASR queue length is " << this->fifo_qq.size() << endl;	
				
					gettimeofday(&now, 0);
					process_start_time = (now.tv_sec*1E6+now.tv_usec)/1000;
//					spec->qs.timestamp.push_back(process_start_time);
					spec->qs.timestamp.at(spec->qs.timestamp.size()-1).serving_start_time = process_start_time;
			//call the query	
					int rand_input = atoi(spec->qs.name.c_str()) % this->input_recycle;
				// int rand_input = rand() % this->input_list.size();	
					execute_asr(this->input_list.at(rand_input));
//				execute_asr(spec->input);
				
					gettimeofday(&now, 0);
					process_end_time = (now.tv_sec*1E6+now.tv_usec)/1000;
					this->num_completed++;

					cout << "Queuing time is " << process_start_time - queuing_start_time << " ms, " 
						<< "Serving time is " << process_end_time - process_start_time << " ms."<< endl;	
					cout << "Num of completed queries: " << this->num_completed << endl; 
					cout << "=============================================================" << endl;
				
					log.open("asr"+std::to_string(this->SERVICE_PORT)+".csv", std::ofstream::out | std::ofstream::app);
					log << this->fifo_qq.size() << endl;
					log.close();
				
//					spec->qs.timestamp.push_back(process_end_time);
					spec->qs.timestamp.at(spec->qs.timestamp.size()-1).serving_end_time = process_end_time;
//					spec->qs.__set_budget(spec->qs.budget-(process_end_time - process_start_time) );
					spec->qs.__set_budget( spec->qs.budget - (process_end_time - queuing_start_time) );
/*
					ThreadSafePriorityQueue<FIFO_QuerySpec> waiting_queries;		//query queue
					
				
					auto waiting_spec = this->fifo_qq.try_pop();
					while(waiting_spec != nullptr) {
						waiting_spec->qs.__set_budget(spec->qs.budget - (process_end_time - process_start_time) );
						waiting_queries.push(*waiting_spec);
						waiting_spec = this->fifo_qq.try_pop();
					}
					int length = waiting_queries.size();
					for(int i=0;i<length;i++)
						fifo_qq.push( *(waiting_queries.wait_and_pop()) );
*/					
					THostPort hostport;
					TClient tClient_scheduler;
					SchedulerServiceClient *scheduler_client = tClient_scheduler.creatSchedulerClient(this->SCHEDULER_IP, this->SCHEDULER_PORT);
					scheduler_client->consultAddress(hostport, NEXT_STAGE);
					tClient_scheduler.close();

					TClient tClient_service;
					IPAServiceClient *service_client = tClient_service.creatIPAClient(hostport.ip, hostport.port);
					service_client->submitQuery(spec->qs);
					tClient_service.close();
				}
			}
		}

		void initialize() {
			// 1. register to the command center
			TClient tClient;
			SchedulerServiceClient *scheduler_client = tClient.creatSchedulerClient(this->SCHEDULER_IP, this->SCHEDULER_PORT);
			THostPort hostPort;
			hostPort.ip = this->SERVICE_IP;
			hostPort.port = this->SERVICE_PORT;
			RegMessage regMessage;
			regMessage.app_name = this->SERVICE_NAME;
			regMessage.endpoint = hostPort;
			regMessage.budget = this->budget;
			cout << "registering to command center runnig at " << this->SCHEDULER_IP << ":" << this->SCHEDULER_PORT << endl;	
			scheduler_client->registerBackend(regMessage);
			tClient.close();
			cout << "service stage " << this->SERVICE_NAME << " successfully registered itself at " << this->SERVICE_IP << ":" << this->SERVICE_PORT << endl;
		
			this->num_completed = 0;	
			//parse the possible inputs	
			ifstream input("input.txt");
			for(string line; getline(input, line); ) {
				input_list.push_back(line);
//				cout << line << endl;
			}
			// 2. launch the helper thread
			boost::thread helper(boost::bind(&SpeechRecognitionServiceHandler::launchQuery, this));
//			int ret = std::system("cpufreq-set -c 0 -f 1500000");

			ThreadPool<void> tp(2);
			boost::shared_future<void> f = tp.enqueue(boost::bind(&SpeechRecognitionServiceHandler::launchQuery, this), 1000);
		}

	private:
		int input_recycle;
		vector<String> input_list;
		QuerySpec newSpec;
		int num_completed;
		ThreadSafePriorityQueue<QuerySpec> qq;		//query queue
		ThreadSafePriorityQueue<FIFO_QuerySpec> fifo_qq;		//query queue
		double budget;
		string SERVICE_NAME;
		string SCHEDULER_IP;
		int SCHEDULER_PORT;
		string SERVICE_IP;
		int SERVICE_PORT;
		int CORE;
		string QUEUE_TYPE;
		// String DOWNSTREAM_SERVICE_IP;
		// int DOWNSTREAM_SERVICE_PORT;
		// IPAServiceClient *service_client;
		// SchedulerServiceClient *scheduler_client;
		// THostPort randomAssignService(vector<THostPort> *service_list) {
			// initialize random seed
		// 	srand(time(NULL));
			// generate random number between 0 and size of the candidate list
		//	int choice = rand() % service_list->size();
		//	THostPort hostPort = service_list->at(choice);
		//	return hostPort;
		// }
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
	
	int service_port;
	String service_ip;
	int scheduler_port;
	String scheduler_ip;
	String queue_type;
	int core;
	float freq;

	service_ip = argv[1];
	service_port = atoi(argv[2]);
	scheduler_ip = argv[3];
	scheduler_port = atoi(argv[4]);
	queue_type = argv[5];
	core = atoi(argv[6]);
	freq = atof(argv[7]);

	// initial the image matching server
	// TThreadPoolServer server(processor, serverTransport, transportFactory, protocolFactory, threadManager);
	// boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
//	SpeechRecognitionServiceHandler *speechRecognition = new SpeechRecognitionServiceHandler();
	SpeechRecognitionServiceHandler *speechRecognition = new SpeechRecognitionServiceHandler(service_ip, service_port, scheduler_ip, scheduler_port, queue_type, core, freq);
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
	
	
	// tServer.launchSingleThreadThriftServer(9092, processor);
	tServer.launchSingleThreadThriftServer(service_port, processor, thrift_server);
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
