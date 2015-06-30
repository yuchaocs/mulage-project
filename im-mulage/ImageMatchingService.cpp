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
#define MATCHING_METHOD 1
#define IMAGE_DATABASE "/home/hailong/sirius/sirius-application/image-matching/matching/landmarks/db"
class ImageMatchingServiceHandler : public IPAServiceIf {
	public:
		// put the model training here so that it only needs to
		// be trained once
		ImageMatchingServiceHandler() {
			this->matcher = new FlannBasedMatcher();
			this->extractor = new SurfDescriptorExtractor();
			this->detector = new SurfFeatureDetector();
			this->budget = 100;
			this->SERVICE_NAME = "im";
			this->SCHEDULER_IP = "141.212.107.226";
			this->SCHEDULER_PORT = 8888;
			this->SERVICE_IP = "clarity28.eecs.umich.edu";
			this->SERVICE_PORT = 9092;
			cout << "building the image matching model..." << endl;
			build_model();
		}
		~ImageMatchingServiceHandler() {
			delete matcher;
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
//			QuerySpec newSpec(query);
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
				cout << "asr queue length is " << this->qq.size() << endl;	
				gettimeofday(&now, 0);
				process_start_time = (now.tv_sec*1E6+now.tv_usec)/1000;
				spec->timestamp.push_back(process_start_time);
		
			//call the query	
				int rand_input = rand() % this->input_list.size();	
				match_img(this->input_list.at(rand_input));
//				match_img(spec->input);
				
				gettimeofday(&now, 0);
				process_end_time = (now.tv_sec*1E6+now.tv_usec)/1000;
				cout << "The queuing time is " << process_start_time - queuing_start_time << "ms, " 
					<< "serving time is " << process_end_time - process_start_time << " ms."<< endl;	
				cout << "Num of completed queries: " << this->num_completed << endl; 
				
				fstream log;
				log.open("im.log", std::ofstream::out | std::ofstream::app);
				log << this->qq.size() << endl;
				log.close();
				
				spec->timestamp.push_back(process_end_time);
				spec->__set_budget( spec->budget - (process_end_time - process_start_time) );

				ThreadSafePriorityQueue<QuerySpec> waiting_queries;		//query queue
				
				for(int i=0;i<this->qq.size();i++) {
					auto waiting_spec = this->qq.wait_and_pop();
					waiting_spec->__set_budget(spec->budget - (process_end_time - process_start_time) );
					waiting_queries.push(*waiting_spec);
				}

				for(int i=0;i<waiting_queries.size();i++)
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
			boost::thread helper(boost::bind(&ImageMatchingServiceHandler::launchQuery, this));
		}

	private:
		vector<String> input_list;
		QuerySpec newSpec;
		int num_completed;
		ThreadSafePriorityQueue<QuerySpec> qq;		//query queue
		struct timeval tp;
		struct timeval tv1, tv2;
		double budget;
		string SERVICE_NAME;
		string SCHEDULER_IP;
		int SCHEDULER_PORT;
		string SERVICE_IP;
		int SERVICE_PORT;
		String DOWNSTREAM_SERVICE_IP;
		int DOWNSTREAM_SERVICE_PORT;
		FeatureDetector *detector;
		DescriptorMatcher *matcher;
		DescriptorExtractor *extractor;
		vector<string> imgNames;
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
		void match_img(string &query_img) {
			
			// save the query image into local disk
                        // gettimeofday(&tp, NULL);
                        // long int timestamp = tp.tv_sec * 1000000 + tp.tv_usec;
			// ostringstream sstream;
			// sstream << timestamp;
                        // string image_path = "input-" + sstream.str() + ".jpg";
                        // ofstream imagefile(image_path.c_str(), ios::binary);
                        // imagefile.write(query_img.c_str(), query_img.size());
                        // imagefile.close();
			cout << "image query is " << query_img << endl;
			string image_path = query_img;
			
			
                        gettimeofday(&tv1, NULL);
			// feature extraction
                        Mat imgInput = imread(image_path, CV_LOAD_IMAGE_GRAYSCALE);
                        vector<KeyPoint> features;
                        // gettimeofday(&tv1, NULL);
                        detector->detect(imgInput, features);
                        // gettimeofday(&tv2, NULL);

			 // feature description
                        Mat descriptors;
                        // gettimeofday(&tv1, NULL);
                        extractor->compute(imgInput, features, descriptors);
                        descriptors.convertTo(descriptors, CV_32F);
                        // gettimeofday(&tv2, NULL);

			// image matching
			// gettimeofday(&tv1, NULL);
			string response = exec_match(descriptors, MATCHING_METHOD);
			gettimeofday(&tv2, NULL);

			long int runtimematching = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
			cout << "The matching image is " << response << endl;
			cout << "Image Matching Time: " << fixed << setprecision(2) << (double)runtimematching / 1000 << "(ms)" << endl;
		}
		void build_model() {
			vector<Mat> trainDesc;
        		FeatureDetector *detector = new SurfFeatureDetector();
        		DescriptorExtractor *extractor = new SurfDescriptorExtractor();

        		// Generate descriptors from the image db
        		fs::path p = fs::system_complete(IMAGE_DATABASE);
			fs::directory_iterator end_iter;
			for(fs::directory_iterator dir_itr(p); dir_itr != end_iter; ++dir_itr) {
                		string img_name(dir_itr->path().string());
                		Mat img = imread(img_name, CV_LOAD_IMAGE_GRAYSCALE);
				
				// feature extraction
                		vector<KeyPoint> keypoints;
				detector->detect(img, keypoints);

				// feature description
				Mat descriptor;
				extractor->compute(img, keypoints, descriptor);
                		trainDesc.push_back(descriptor);
                		imgNames.push_back(img_name);
			}

			// train the model
			matcher->add(trainDesc);
			matcher->train();

			// Clean up
        		delete detector;
        		delete extractor;
		}
		string exec_match(Mat testDesc, int match_method) {
			vector<vector<DMatch> > knnMatches;
			vector<DMatch> annMatches;
			vector<int> bestMatches(imgNames.size(), 0);

			// apply the required matching method
			switch(match_method) {
				case 1:	{
					int knn = 1;
					matcher->knnMatch(testDesc, knnMatches, knn);
                                        // Filter results
                                        for(vector<vector<DMatch> >::const_iterator it = knnMatches.begin(); it != knnMatches.end(); ++it) {
                                                for(vector<DMatch>::const_iterator it2 = it->begin(); it2 != it->end(); ++it2) {
                                                         ++bestMatches[(*it2).imgIdx];
                                                }
                                        }

				}
					break;
				case 2: {
					matcher->match(testDesc, annMatches);
					for(vector<DMatch>::const_iterator iter = annMatches.begin(); iter != annMatches.end(); ++iter) {
						++bestMatches[(*iter).imgIdx];
					}
				}
					break;
			}
			
        		// Filter results
			/*
        		for(vector<vector<DMatch> >::const_iterator it = knnMatches.begin(); it != knnMatches.end(); ++it){
                		for(vector<DMatch>::const_iterator it2 = it->begin(); it2 != it->end(); ++it2){
                       			 ++bestMatches[(*it2).imgIdx];
                		}
        		}
			*/

        		// Find best match
        		int bestScore = 0;
        		int bestIdx = -1;
        		for(int i = 0; i < bestMatches.size(); ++i){
                		if(bestMatches[i] >= bestScore){
                        		bestScore = bestMatches[i];
                        		bestIdx = i;
                		}
        		}
			string imgBaseName = string(fs::basename(imgNames.at(bestIdx)));
			boost::replace_all(imgBaseName, "-", " ");
			return imgBaseName;
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
	ImageMatchingServiceHandler *ImageMatchingService = new ImageMatchingServiceHandler();
  	boost::shared_ptr<ImageMatchingServiceHandler> handler(ImageMatchingService);
  	// boost::shared_ptr<ImageMatchingServiceHandler> handler(new ImageMatchingServiceHandler());
	boost::shared_ptr<TProcessor> processor(new IPAServiceProcessor(handler));
  	// boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(9092));
  	// boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

  	// TNonblockingServer server(processor, serverTransport, transportFactory, protocolFactory);
  	// TNonblockingServer server(processor, protocolFactory, 9092);
	TServers tServer;
	thread thrift_server;
	cout << "Starting the image matching service..." << endl;

	fstream log;
	log.open("im.log", std::ofstream::out | std::ofstream::app);
	log << "qlen" << endl;
	log.close();

	// tServer.launchSingleThreadThriftServer(9093, processor);
	tServer.launchSingleThreadThriftServer(9092, processor, thrift_server);
	ImageMatchingService->initialize();
	// server.serve();
	cout << "Done..." << endl;
	thrift_server.join();
	/*
	while(true) {
	}
	*/
	return 0;
}
