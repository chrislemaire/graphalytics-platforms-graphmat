/*
 * Copyright 2015 Delft University of Technology
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <limits>
#include <omp.h>
#include <stdint.h>
#include <algorithm>
#include <iostream>
#include <chrono>
#include "boost/serialization/vector.hpp"

#include "GraphMatRuntime.h"
#include "common.hpp"

#ifdef GRANULA
#include "granula.hpp"
#endif

using namespace std;

class custom_label_type;

typedef int label_type;
typedef label_type msg_type;

template<typename T>
class serializable_vector : public GraphMat::Serializable {
  public:
    std::vector<T> v;
  public:
    friend boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
      ar & v;
    }
};
typedef serializable_vector<label_type> reduce_type;
typedef custom_label_type vertex_value_type;

class custom_label_type {
    public:
    	label_type label;
	custom_label_type() : label(0) { }
	custom_label_type(const int& x) : label(x) { }
	bool operator!=(const custom_label_type& t) {
		return label != t.label;
	}
	
	label_type get_output() {
		return label;
	}
	friend ostream& operator<<(ostream& out, const custom_label_type& t) {
		out << t.label;
		return out;
	}
};

class CommunityDetectionProgram: public GraphMat::GraphProgram<msg_type, reduce_type, vertex_value_type> {
    public:
        CommunityDetectionProgram(int isDirected) {
            order = (isDirected)?(GraphMat::ALL_EDGES):(GraphMat::OUT_EDGES);
            activity = GraphMat::ALL_VERTICES;
	    process_message_requires_vertexprop = false;
        }

        bool send_message(const vertex_value_type& vertex, msg_type& msg) const {
            msg = vertex.label;
            return true;
        }

        void process_message(const msg_type& msg, const int edge, const vertex_value_type& vertex, reduce_type& result) const {
            result.v.clear();
            result.v.push_back(msg);
        }

        void reduce_function(reduce_type& total, const reduce_type& partial) const {
            total.v.insert(total.v.end(), partial.v.begin(), partial.v.end());
        }

        void apply(const reduce_type& total, vertex_value_type& vertex) {
	    assert(total.v.size() > 0);
	    if (total.v.size() == 1) {
		vertex = total.v[0];
	    } else {
		auto total_copy = total;
		std::sort(total_copy.v.begin(), total_copy.v.end());
	    	int max_freq = 1;
            	label_type max_freq_label = total_copy.v[0];
		int curr_freq = 1;
	    	for (int i = 1; i <total_copy.v.size(); i++) {
			if (total_copy.v[i] == total_copy.v[i-1])  {
			  curr_freq++;
			  if (curr_freq > max_freq) {
			    max_freq = curr_freq;
			    max_freq_label = total_copy.v[i];
                          }
			} else {
			  curr_freq = 1;
			}
	    	}
            	vertex = max_freq_label;
	    }
        }
};

string getEpoch() {
    return to_string(chrono::duration_cast<chrono::milliseconds>
        (chrono::system_clock::now().time_since_epoch()).count());
}


int main(int argc, char *argv[]) {

#ifdef GRANULA
    granula::startMonitorProcess(getpid());
#endif

    if (argc < 3) {
        cerr << "usage: " << argv[0] << " <graph file> <niterations> <job id> <isDirected> [output file]" << endl;
        return EXIT_FAILURE;;
    }
	
    MPI_Init(&argc, &argv);

    bool is_master = GraphMat::get_global_myrank() == 0;
    char *filename = argv[1];
    int niterations = atoi(argv[2]);
    string jobId = argc > 3 ? argv[3] : NULL;
    int isDirected = argc > 4 ? atoi(argv[4]) : 1;
    char *output = argc > 5 ? argv[5] : NULL;

#ifdef GRANULA
    granula::linkNode(jobId);
    granula::linkProcess(getpid(), jobId);
    granula::operation graphmatJob("GraphMat", "Id.Unique", "Job", "Id.Unique");
    cout<<graphmatJob.getOperationInfo("StartTime", graphmatJob.getEpoch())<<endl;

    granula::operation loadGraph("GraphMat", "Id.Unique", "LoadGraph", "Id.Unique");
    cout<<loadGraph.getOperationInfo("StartTime", loadGraph.getEpoch())<<endl;
#endif

    timer_start();

    timer_next("load graph");
    GraphMat::Graph<vertex_value_type> graph;
    //graph.ReadMTX(filename);
    graph.ReadGraphMatBin(filename);

#ifdef GRANULA
    cout<<loadGraph.getOperationInfo("EndTime", loadGraph.getEpoch())<<endl;
#endif

    timer_next("initialize engine");

    for (size_t i = 1; i <= graph.getNumberOfVertices(); i++) {
	if (graph.vertexNodeOwner(i)) {
        	graph.setVertexproperty(i, label_type(i));
	}
    }

    CommunityDetectionProgram prog(isDirected);
    auto ctx = GraphMat::graph_program_init(prog, graph);

#ifdef GRANULA
    granula::operation processGraph("GraphMat", "Id.Unique", "ProcessGraph", "Id.Unique");
    cout<<processGraph.getOperationInfo("StartTime", processGraph.getEpoch())<<endl;
#endif

    if (is_master) cout<< "Processing starts at: " + getEpoch() + "\n" <<endl;
    timer_next("run algorithm");
    GraphMat::run_graph_program(&prog, graph, niterations, &ctx);
    if (is_master) cout<< "Processing ends at: " + getEpoch() + "\n" <<endl;

#ifdef GRANULA
    cout<<processGraph.getOperationInfo("EndTime", processGraph.getEpoch())<<endl;
#endif

#ifdef GRANULA
    granula::operation offloadGraph("GraphMat", "Id.Unique", "OffloadGraph", "Id.Unique");
    cout<<offloadGraph.getOperationInfo("StartTime", offloadGraph.getEpoch())<<endl;
#endif

    timer_next("print output");
    print_graph<vertex_value_type, int, label_type>(output, graph, MPI_INT);

#ifdef GRANULA
    cout<<offloadGraph.getOperationInfo("EndTime", offloadGraph.getEpoch())<<endl;
#endif

    timer_next("deinitialize engine");
    GraphMat::graph_program_clear(ctx);

    timer_end();

#ifdef GRANULA
    cout<<graphmatJob.getOperationInfo("EndTime", graphmatJob.getEpoch())<<endl;
        granula::stopMonitorProcess(getpid());
#endif

    MPI_Finalize();
    return EXIT_SUCCESS;
}
