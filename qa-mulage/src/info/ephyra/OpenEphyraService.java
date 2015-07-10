package info.ephyra;

import info.ephyra.answerselection.AnswerSelection;
import info.ephyra.answerselection.filters.AnswerPatternFilter;
import info.ephyra.answerselection.filters.AnswerTypeFilter;
import info.ephyra.answerselection.filters.DuplicateFilter;
import info.ephyra.answerselection.filters.FactoidSubsetFilter;
import info.ephyra.answerselection.filters.FactoidsFromPredicatesFilter;
import info.ephyra.answerselection.filters.PredicateExtractionFilter;
import info.ephyra.answerselection.filters.QuestionKeywordsFilter;
import info.ephyra.answerselection.filters.ScoreCombinationFilter;
import info.ephyra.answerselection.filters.ScoreNormalizationFilter;
import info.ephyra.answerselection.filters.ScoreSorterFilter;
import info.ephyra.answerselection.filters.StopwordFilter;
import info.ephyra.answerselection.filters.TruncationFilter;
import info.ephyra.answerselection.filters.WebDocumentFetcherFilter;
// import info.ephyra.io.Logger;
import info.ephyra.io.MsgPrinter;
import info.ephyra.nlp.LingPipe;
import info.ephyra.nlp.NETagger;
import info.ephyra.nlp.OpenNLP;
import info.ephyra.nlp.SnowballStemmer;
import info.ephyra.nlp.StanfordNeTagger;
import info.ephyra.nlp.StanfordParser;
import info.ephyra.nlp.indices.FunctionWords;
import info.ephyra.nlp.indices.IrregularVerbs;
import info.ephyra.nlp.indices.Prepositions;
import info.ephyra.nlp.indices.WordFrequencies;
import info.ephyra.nlp.semantics.ontologies.Ontology;
import info.ephyra.nlp.semantics.ontologies.WordNet;
import info.ephyra.querygeneration.Query;
import info.ephyra.querygeneration.QueryGeneration;
import info.ephyra.querygeneration.generators.BagOfTermsG;
import info.ephyra.querygeneration.generators.BagOfWordsG;
import info.ephyra.querygeneration.generators.PredicateG;
import info.ephyra.querygeneration.generators.QuestionInterpretationG;
import info.ephyra.querygeneration.generators.QuestionReformulationG;
import info.ephyra.questionanalysis.AnalyzedQuestion;
import info.ephyra.questionanalysis.QuestionAnalysis;
import info.ephyra.questionanalysis.QuestionInterpreter;
import info.ephyra.questionanalysis.QuestionNormalizer;
import info.ephyra.search.Result;
import info.ephyra.search.Search;
import info.ephyra.search.searchers.BingKM;
import info.ephyra.search.searchers.IndriKM;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.PriorityBlockingQueue;

import org.apache.log4j.Logger;
import org.apache.thrift.TException;

import edu.umich.clarity.service.util.QueryComparator;
import edu.umich.clarity.service.util.TClient;
import edu.umich.clarity.service.util.TServers;
import edu.umich.clarity.thrift.IPAService;
import edu.umich.clarity.thrift.QuerySpec;
import edu.umich.clarity.thrift.RegMessage;
import edu.umich.clarity.thrift.RegReply;
import edu.umich.clarity.thrift.SchedulerService;
import edu.umich.clarity.thrift.THostPort;
import edu.umich.clarity.thrift.LatencySpec;

import java.util.ArrayList;
import com.opencsv.CSVWriter;
import java.io.FileWriter;
import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * <code>OpenEphyra</code> is an open framework for question answering (QA).
 * 
 * @author Nico Schlaefer
 * @version 2008-03-23
 */
public class OpenEphyraService implements IPAService.Iface {
	
	private static final String SERVICE_NAME = "qa";
	private static String SERVICE_IP;
	private static int SERVICE_PORT;
	private static String SCHEDULER_IP;
	private static int SCHEDULER_PORT;
	private static final Logger LOG = Logger.getLogger(OpenEphyraService.class);
	private static List<THostPort> service_list;
	private static String DOWNSTREAM_SERVICE_IP;
	private static int DOWNSTREAM_SERVICE_PORT;
	private static final String QUESTION_FILE_PATH = "/home/hailong/mulage_project/qa-mulage/input/questions.txt";

	private static double budget;
	private static int core_id;
	private static final String FIFO_POLICY = "fifo";
	private static final String PRIORITY_POLICY = "priority";
	private BlockingQueue<QuerySpec> queryQueue;
	private static SchedulerService.Client scheduler_client;
	private static CSVWriter csvWriter = null;
	private static final String FILE_HEADER = "qsize";
	private static OpenEphyra qaInstance = null;
	private static int total_num_question = 0;
	
	private static final int INPUT_RECYCLE = 100;
	
	public OpenEphyraService(String service_ip, String service_port, String scheduler_ip, String scheduler_port, String queue_policy, String core_id, String budget) {
		this.SERVICE_PORT = new Integer(service_port);
		this.SERVICE_IP = service_ip;
		this.SCHEDULER_IP = scheduler_ip;
		this.SCHEDULER_PORT = new Integer(scheduler_port);
		this.core_id = new Integer(core_id);
		this.budget = new Double(budget);
		if (queue_policy.equalsIgnoreCase(PRIORITY_POLICY)) {
			this.queryQueue = new PriorityBlockingQueue<QuerySpec>(500, new QueryComparator<QuerySpec>());
		} else if (queue_policy.equalsIgnoreCase(FIFO_POLICY)) {
			this.queryQueue = new LinkedBlockingQueue<QuerySpec>();
		}
	}

	private int getTotalLineNum(File file) {
        	String line = null;  
        	int line_num = 0;
		try {
			FileReader in = new FileReader(file);  
        		LineNumberReader reader = new LineNumberReader(in);  
        		while((line = reader.readLine()) != null) {  
            			line_num++;  
        		}  
        		reader.close();  
        		in.close(); 
		} catch(IOException ex) {
			ex.printStackTrace();
		} 
        	return line_num;
	}
	
	private String getQuestion(File file, int line) {
		String content = null;
                int line_num = 0;
                try {
			FileReader in = new FileReader(file);
                	LineNumberReader reader = new LineNumberReader(in);
                        while((content = reader.readLine()) != null) {
				if(line_num == line)
					break;
                                line_num++;
                        }
                        reader.close();
                        in.close();
                } catch(IOException ex) {
                        ex.printStackTrace();
                }
                return content;
	}
	public void initialize() {
		total_num_question = getTotalLineNum(new File(QUESTION_FILE_PATH));
		qaInstance = new OpenEphyra();
		try {
            		csvWriter = new CSVWriter(new FileWriter("/home/hailong/mulage_project/qa-mulage/qa-" + SERVICE_PORT + ".csv"), ',', CSVWriter.NO_QUOTE_CHARACTER);
            		csvWriter.writeNext(FILE_HEADER.split(","));
			csvWriter.flush();
        	} catch (IOException e) {
        	    e.printStackTrace();
        	}
		try {
			scheduler_client = TClient.creatSchedulerClient(SCHEDULER_IP,
					SCHEDULER_PORT);
		} catch (IOException ex) {
			LOG.error("Error creating thrift scheduler client"
					+ ex.getMessage());
		}
		try {
			THostPort hostPort = new THostPort(SERVICE_IP, SERVICE_PORT);
			RegMessage regMessage = new RegMessage(SERVICE_NAME, hostPort,
					budget);
			LOG.info("registering to command center runnig at " + SCHEDULER_IP
					+ ":" + SCHEDULER_PORT);
			scheduler_client.registerBackend(regMessage);
			LOG.info("service stage " + SERVICE_NAME
					+ " successfully registered itself at " + SERVICE_IP + ":"
					+ SERVICE_PORT);
		} catch (TException ex) {
			LOG.error("Error registering backend service " + ex.getMessage());
		}
		new Thread(new processQueryRunnable()).start();
	}

	@Override
	public int reportQueueLength() {
		return this.queryQueue.size();
	}

	@Override
	public void updatBudget(double budget) throws TException {
		this.budget = budget;
		String command = "sudo cpufreq-set -c " + this.core_id + " -f " + (budget * 1000000);
		String output = null;
		try {
			Process p = Runtime.getRuntime().exec(command);
			BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
            		BufferedReader stdError = new BufferedReader(new InputStreamReader(p.getErrorStream()));
            		// read the output from the command
            		while ((output = stdInput.readLine()) != null) {
                		LOG.info("the output of cpufreq command is " + output);
            		}
             
            		// read any errors from the attempted command
            		while ((output = stdError.readLine()) != null) {
                		LOG.error("failed to execute cpufreq: " + output);
            		}
			LOG.info("service " + SERVICE_NAME + " at " + SERVICE_IP + ":"
					+ SERVICE_PORT + " update its budget to " + budget);
			} catch (IOException e) {
				LOG.error("failed to update the frequency budget " + e.getMessage());
			}
	}

	@Override
	public void submitQuery(QuerySpec query) throws TException {
		// timestamp the query when it is enqueued (start)
		LatencySpec latencySpec = new LatencySpec();
		latencySpec.setInstance_id(this.SERVICE_NAME + "_" + this.SERVICE_IP + "_" + this.SERVICE_PORT);
		long queuing_start_time = System.currentTimeMillis();
		latencySpec.setQueuing_start_time(queuing_start_time);
		query.getTimestamp().add(latencySpec);
		// update the budget of all queries waiting in the queue
                List<QuerySpec> waiting_queries = new ArrayList<QuerySpec>();
                queryQueue.drainTo(waiting_queries);
                for (QuerySpec waiting_query : waiting_queries) {
			List<LatencySpec> latencyStat = waiting_query.getTimestamp();
			long existing_queuing_time = latencyStat.get(latencyStat.size() - 1).getQueuing_start_time();
			waiting_query.setFloatingBudget(waiting_query.getBudget() - (queuing_start_time - existing_queuing_time));
                }
                queryQueue.addAll(waiting_queries);
		
		try {
			query.setFloatingBudget(query.getBudget());
			queryQueue.put(query);
		} catch (InterruptedException e) {
			LOG.error("failed to enqueue the query " + e.getMessage());
		}
	}

	private class processQueryRunnable implements Runnable {
		private long counter = 0;

		@Override
		public void run() {
			LOG.info("starting the helper thread to scan the incoming queue to process the query");
			while (true) {
				try {
					QuerySpec query = queryQueue.take();
					// String csvEntry = "" + queryQueue.size() + ",";
					// csvWriter.writeNext(csvEntry.split(","));
					// csvWriter.flush();
					// timestamp the query when it is enqueued (end)
					// this is also the timestamp for the start of serving
					// (start)
					// Random randGen = new Random();
					// int randLine = randGen.nextInt(total_num_question);
					int randLine = new Integer(query.getName()).intValue() % INPUT_RECYCLE;
					String question = getQuestion(new File(QUESTION_FILE_PATH), randLine);
					
					LatencySpec latencySpec = query.getTimestamp().get(
                                                        query.getTimestamp().size() - 1);
					long queuing_start_time = latencySpec.getQueuing_start_time();
					long process_start_time = System.currentTimeMillis();
					latencySpec.setServing_start_time(process_start_time);
					/**
					 * TODO 1. use the latency model to predict the serving time
					 * 2. based on the queuing and serving time to see if the
					 * query processing would be within the QoS budget 3. based
					 * on the DVFS and query performance model, trading off the
					 * latency and energy efficiency 4. set the DVFS to the
					 * appropriate level 5. logic to process the query 6. change
					 * DVFS to the base setting 7. update the budget of queries
					 * waiting in the queue
					 */
					// Thread.sleep(100);
					// qaInstance.commandLine(new String(query.bufferForInput().array()).trim());
					qaInstance.commandLine(question);
					long process_end_time = System.currentTimeMillis();
					LOG.info("===============================================");
					LOG.info("there are " + queryQueue.size() + " queries waiting in the queue");
					LOG.info("the queuing time for the query is "
                                                        + (process_start_time - queuing_start_time) + "ms");
					LOG.info("the serving time for the query is "
							+ (process_end_time - process_start_time) + "ms");
					counter++;
					LOG.info(counter + " queries have been processed so far...");
					LOG.info("===============================================");
					// timestamp the query when it is served (end)
					latencySpec.setServing_end_time(process_end_time);
					// update the query budget
					query.setBudget(query.getBudget()
							- (process_end_time - queuing_start_time));
					LOG.info("enqueing query to command center at "
								+ SCHEDULER_IP + ":" + SCHEDULER_PORT);
					try {
						scheduler_client.enqueueFinishedQuery(query);
					} catch (TException e) {
						LOG.error("Error failed to submit query to command center at "
								+ SCHEDULER_IP
								+ ":"
								+ SCHEDULER_PORT
								+ e.getMessage());
					}
				} catch (InterruptedException e) {
					LOG.error("failed to pop the query from the queue"
							+ e.getMessage());
				} 
				// catch (IOException e) {
                    		// 	e.printStackTrace();
                		// }
			}
		}
	}
	/**
	 * Entry point of Ephyra. Initializes the engine and starts the command line
	 * interface.
	 * 
	 * @param args command line arguments are ignored
	 */
	public static void main(String[] args) throws IOException, TException {
		// enable output of status and error messages
                // MsgPrinter.enableStatusMsgs(true);
                // MsgPrinter.enableErrorMsgs(true);

		OpenEphyraService qaService = new OpenEphyraService(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
		IPAService.Processor<IPAService.Iface> processor = new IPAService.Processor<IPAService.Iface>(
				qaService);
		TServers.launchSingleThreadThriftServer(SERVICE_PORT, processor);
		LOG.info("starting " + SERVICE_NAME + " service at " + SERVICE_IP + ":" + SERVICE_PORT+ " running on core " + core_id + " with frequency " + budget + "GHz");
		qaService.initialize();
		// MsgPrinter.printStatusMsg("QA service started at " + SERVICE_IP + ":" + SERVICE_PORT);
		// set log file and enable logging
		// Logger.setLogfile("log/OpenEphyra");
		// Logger.enableLogging(true);
		
		// initialize Ephyra and start command line interface
		// (new OpenEphyra()).commandLine(args[0].trim());
	}
	
	/**
	 * <p>Creates a new instance of Ephyra and initializes the system.</p>
	 * 
	 * <p>For use as a standalone system.</p>
	 */
	private class OpenEphyra{

	/** Factoid question type. */
        protected static final String FACTOID = "FACTOID";
        /** List question type. */
        protected static final String LIST = "LIST";

        /** Maximum number of factoid answers. */
        protected static final int FACTOID_MAX_ANSWERS = 1;
        /** Absolute threshold for factoid answer scores. */
        protected static final float FACTOID_ABS_THRESH = 0;
        /** Relative threshold for list answer scores (fraction of top score). */
        protected static final float LIST_REL_THRESH = 0.1f;

        /** Serialized classifier for score normalization. */
        public static final String NORMALIZER =
                "res/scorenormalization/classifiers/" +
                "AdaBoost70_" +
                "Score+Extractors_" +
                "TREC10+TREC11+TREC12+TREC13+TREC14+TREC15+TREC8+TREC9" +
                ".serialized";

        /** The directory of Ephyra, required when Ephyra is used as an API. */
        protected String dir;

	public OpenEphyra() {
		this("");
	}
	
	
	/**
	 * <p>Creates a new instance of Ephyra and initializes the system.</p>
	 * 
	 * <p>For use as an API.</p>
	 * 
	 * @param dir directory of Ephyra
	 */
	public OpenEphyra(String dir) {
		this.dir = dir;
		
		MsgPrinter.printInitializing();
		
		// create tokenizer
		MsgPrinter.printStatusMsg("Creating tokenizer...");
		if (!OpenNLP.createTokenizer(dir +
				"res/nlp/tokenizer/opennlp/EnglishTok.bin.gz"))
			MsgPrinter.printErrorMsg("Could not create tokenizer.");
//		LingPipe.createTokenizer();
		
		// create sentence detector
		MsgPrinter.printStatusMsg("Creating sentence detector...");
		if (!OpenNLP.createSentenceDetector(dir +
				"res/nlp/sentencedetector/opennlp/EnglishSD.bin.gz"))
			MsgPrinter.printErrorMsg("Could not create sentence detector.");
		LingPipe.createSentenceDetector();
		
		// create stemmer
		MsgPrinter.printStatusMsg("Creating stemmer...");
		SnowballStemmer.create();
		
		// create part of speech tagger
		MsgPrinter.printStatusMsg("Creating POS tagger...");
		if (!OpenNLP.createPosTagger(
				dir + "res/nlp/postagger/opennlp/tag.bin.gz",
				dir + "res/nlp/postagger/opennlp/tagdict"))
			MsgPrinter.printErrorMsg("Could not create OpenNLP POS tagger.");
//		if (!StanfordPosTagger.init(dir + "res/nlp/postagger/stanford/" +
//				"wsj3t0-18-bidirectional/train-wsj-0-18.holder"))
//			MsgPrinter.printErrorMsg("Could not create Stanford POS tagger.");
		
		// create chunker
		MsgPrinter.printStatusMsg("Creating chunker...");
		if (!OpenNLP.createChunker(dir +
				"res/nlp/phrasechunker/opennlp/EnglishChunk.bin.gz"))
			MsgPrinter.printErrorMsg("Could not create chunker.");
		
		// create syntactic parser
		MsgPrinter.printStatusMsg("Creating syntactic parser...");
//		if (!OpenNLP.createParser(dir + "res/nlp/syntacticparser/opennlp/"))
//			MsgPrinter.printErrorMsg("Could not create OpenNLP parser.");
		try {
			StanfordParser.initialize();
		} catch (Exception e) {
			MsgPrinter.printErrorMsg("Could not create Stanford parser.");
		}
		
		// create named entity taggers
		MsgPrinter.printStatusMsg("Creating NE taggers...");
		NETagger.loadListTaggers(dir + "res/nlp/netagger/lists/");
		NETagger.loadRegExTaggers(dir + "res/nlp/netagger/patterns.lst");
		MsgPrinter.printStatusMsg("  ...loading models");
//		if (!NETagger.loadNameFinders(dir + "res/nlp/netagger/opennlp/"))
//			MsgPrinter.printErrorMsg("Could not create OpenNLP NE tagger.");
		if (!StanfordNeTagger.isInitialized() && !StanfordNeTagger.init())
			MsgPrinter.printErrorMsg("Could not create Stanford NE tagger.");
		MsgPrinter.printStatusMsg("  ...done");
		
		// create linker
//		MsgPrinter.printStatusMsg("Creating linker...");
//		if (!OpenNLP.createLinker(dir + "res/nlp/corefresolver/opennlp/"))
//			MsgPrinter.printErrorMsg("Could not create linker.");
		
		// create WordNet dictionary
		MsgPrinter.printStatusMsg("Creating WordNet dictionary...");
		if (!WordNet.initialize(dir +
				"res/ontologies/wordnet/file_properties.xml"))
			MsgPrinter.printErrorMsg("Could not create WordNet dictionary.");
		
		// load function words (numbers are excluded)
		MsgPrinter.printStatusMsg("Loading function verbs...");
		if (!FunctionWords.loadIndex(dir +
				"res/indices/functionwords_nonumbers"))
			MsgPrinter.printErrorMsg("Could not load function words.");
		
		// load prepositions
		MsgPrinter.printStatusMsg("Loading prepositions...");
		if (!Prepositions.loadIndex(dir +
				"res/indices/prepositions"))
			MsgPrinter.printErrorMsg("Could not load prepositions.");
		
		// load irregular verbs
		MsgPrinter.printStatusMsg("Loading irregular verbs...");
		if (!IrregularVerbs.loadVerbs(dir + "res/indices/irregularverbs"))
			MsgPrinter.printErrorMsg("Could not load irregular verbs.");
		
		// load word frequencies
		MsgPrinter.printStatusMsg("Loading word frequencies...");
		if (!WordFrequencies.loadIndex(dir + "res/indices/wordfrequencies"))
			MsgPrinter.printErrorMsg("Could not load word frequencies.");
		
		// load query reformulators
		MsgPrinter.printStatusMsg("Loading query reformulators...");
		if (!QuestionReformulationG.loadReformulators(dir +
				"res/reformulations/"))
			MsgPrinter.printErrorMsg("Could not load query reformulators.");
		
		// load answer types
//		MsgPrinter.printStatusMsg("Loading answer types...");
//		if (!AnswerTypeTester.loadAnswerTypes(dir +
//				"res/answertypes/patterns/answertypepatterns"))
//			MsgPrinter.printErrorMsg("Could not load answer types.");
		
		// load question patterns
		MsgPrinter.printStatusMsg("Loading question patterns...");
		if (!QuestionInterpreter.loadPatterns(dir +
				"res/patternlearning/questionpatterns/"))
			MsgPrinter.printErrorMsg("Could not load question patterns.");
		
		// load answer patterns
		MsgPrinter.printStatusMsg("Loading answer patterns...");
		if (!AnswerPatternFilter.loadPatterns(dir +
				"res/patternlearning/answerpatterns/"))
			MsgPrinter.printErrorMsg("Could not load answer patterns.");
	}
	
	/**
	 * Reads a line from the command prompt.
	 * 
	 * @return user input
	 */
	protected String readLine() {
		try {
			return new java.io.BufferedReader(new
				java.io.InputStreamReader(System.in)).readLine();
		}
		catch(java.io.IOException e) {
			return new String("");
		}
	}
	
	/**
	 * Initializes the pipeline for factoid questions.
	 */
	protected void initFactoid() {
		// question analysis
		Ontology wordNet = new WordNet();
		// - dictionaries for term extraction
		QuestionAnalysis.clearDictionaries();
		QuestionAnalysis.addDictionary(wordNet);
		// - ontologies for term expansion
		QuestionAnalysis.clearOntologies();
		QuestionAnalysis.addOntology(wordNet);
		
		// query generation
		QueryGeneration.clearQueryGenerators();
		QueryGeneration.addQueryGenerator(new BagOfWordsG());
		QueryGeneration.addQueryGenerator(new BagOfTermsG());
		QueryGeneration.addQueryGenerator(new PredicateG());
		QueryGeneration.addQueryGenerator(new QuestionInterpretationG());
		QueryGeneration.addQueryGenerator(new QuestionReformulationG());
		
		// search
		// - knowledge miners for unstructured knowledge sources
		Search.clearKnowledgeMiners();
//		Search.addKnowledgeMiner(new BingKM());
//		Search.addKnowledgeMiner(new GoogleKM());
//		Search.addKnowledgeMiner(new YahooKM());
		for (String[] indriIndices : IndriKM.getIndriIndices())
			Search.addKnowledgeMiner(new IndriKM(indriIndices, false));
//		for (String[] indriServers : IndriKM.getIndriServers())
//			Search.addKnowledgeMiner(new IndriKM(indriServers, true));
		// - knowledge annotators for (semi-)structured knowledge sources
		Search.clearKnowledgeAnnotators();
		
		// answer extraction and selection
		// (the filters are applied in this order)
		AnswerSelection.clearFilters();
		// - answer extraction filters
		AnswerSelection.addFilter(new AnswerTypeFilter());
		AnswerSelection.addFilter(new AnswerPatternFilter());
		//AnswerSelection.addFilter(new WebDocumentFetcherFilter());
		AnswerSelection.addFilter(new PredicateExtractionFilter());
		AnswerSelection.addFilter(new FactoidsFromPredicatesFilter());
		AnswerSelection.addFilter(new TruncationFilter());
		// - answer selection filters
		AnswerSelection.addFilter(new StopwordFilter());
		AnswerSelection.addFilter(new QuestionKeywordsFilter());
		AnswerSelection.addFilter(new ScoreNormalizationFilter(NORMALIZER));
		AnswerSelection.addFilter(new ScoreCombinationFilter());
		AnswerSelection.addFilter(new FactoidSubsetFilter());
		AnswerSelection.addFilter(new DuplicateFilter());
		AnswerSelection.addFilter(new ScoreSorterFilter());
	}
	
	/**
	 * Runs the pipeline and returns an array of up to <code>maxAnswers</code>
	 * results that have a score of at least <code>absThresh</code>.
	 * 
	 * @param aq analyzed question
	 * @param maxAnswers maximum number of answers
	 * @param absThresh absolute threshold for scores
	 * @return array of results
	 */
	protected Result[] runPipeline(AnalyzedQuestion aq, int maxAnswers,
								  float absThresh) {
		// query generation
		MsgPrinter.printGeneratingQueries();
		Query[] queries = QueryGeneration.getQueries(aq);
		
		// search
		MsgPrinter.printSearching();
		Result[] results = Search.doSearch(queries);
		
		// answer selection
		MsgPrinter.printSelectingAnswers();
		results = AnswerSelection.getResults(results, maxAnswers, absThresh);
		
		return results;
	}
	
	/**
	 * Returns the directory of Ephyra.
	 * 
	 * @return directory
	 */
	public String getDir() {
		return dir;
	}
	
	/**
	 * <p>A command line interface for Ephyra.</p>
	 * 
	 * <p>Repeatedly queries the user for a question, asks the system the
	 * question and prints out and logs the results.</p>
	 * 
	 * <p>The command <code>exit</code> can be used to quit the program.</p>
	 */
	public void commandLine(String query_input) {
//		while (true) {
			// query user for question, quit if user types in "exit"
//			MsgPrinter.printQuestionPrompt();
//			String question = readLine().trim();

			String question = query_input.trim();

			if (question.equalsIgnoreCase("exit")) System.exit(0);
			
			// determine question type and extract question string
			String type;
			if (question.matches("(?i)" + FACTOID + ":.*+")) {
				// factoid question
				type = FACTOID;
				question = question.split(":", 2)[1].trim();
			} else if (question.matches("(?i)" + LIST + ":.*+")) {
				// list question
				type = LIST;
				question = question.split(":", 2)[1].trim();
			} else {
				// question type unspecified
				type = FACTOID;  // default type
			}
			
			// ask question
			Result[] results = new Result[0];
			if (type.equals(FACTOID)) {
				// Logger.logFactoidStart(question);
				results = askFactoid(question, FACTOID_MAX_ANSWERS,
						FACTOID_ABS_THRESH);
				// Logger.logResults(results);
				// Logger.logFactoidEnd();
			} else if (type.equals(LIST)) {
				// Logger.logListStart(question);
				results = askList(question, LIST_REL_THRESH);
				// Logger.logResults(results);
				// Logger.logListEnd();
			}
			
			// print answers
			MsgPrinter.printAnswers(results);
		//}
	}
	
	/**
	 * Asks Ephyra a factoid question and returns up to <code>maxAnswers</code>
	 * results that have a score of at least <code>absThresh</code>.
	 * 
	 * @param question factoid question
	 * @param maxAnswers maximum number of answers
	 * @param absThresh absolute threshold for scores
	 * @return array of results
	 */
	public Result[] askFactoid(String question, int maxAnswers,
							   float absThresh) {
		// initialize pipeline
		initFactoid();
		
		// analyze question
		MsgPrinter.printAnalyzingQuestion();
		AnalyzedQuestion aq = QuestionAnalysis.analyze(question);
		
		// get answers
		Result[] results = runPipeline(aq, maxAnswers, absThresh);
		
		return results;
	}
	
	/**
	 * Asks Ephyra a factoid question and returns a single result or
	 * <code>null</code> if no answer could be found.
	 * 
	 * @param question factoid question
	 * @return single result or <code>null</code>
	 */
	public Result askFactoid(String question) {
		Result[] results = askFactoid(question, 1, 0);
		
		return (results.length > 0) ? results[0] : null;
	}
	
	/**
	 * Asks Ephyra a list question and returns results that have a score of at
	 * least <code>relThresh * top score</code>.
	 * 
	 * @param question list question
	 * @param relThresh relative threshold for scores
	 * @return array of results
	 */
	public Result[] askList(String question, float relThresh) {
		question = QuestionNormalizer.transformList(question);
		
		Result[] results = askFactoid(question, Integer.MAX_VALUE, 0);
		
		// get results with a score of at least relThresh * top score
		ArrayList<Result> confident = new ArrayList<Result>();
		if (results.length > 0) {
			float topScore = results[0].getScore();
			
			for (Result result : results)
				if (result.getScore() >= relThresh * topScore)
					confident.add(result);
		}
		
		return confident.toArray(new Result[confident.size()]);
	}
	}
}
