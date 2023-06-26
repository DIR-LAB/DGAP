#include <omp.h>
#include <iostream>
#include <getopt.h>
#include <stdlib.h>

#include "graph.h"
#include "sgraph.h"
#include "typekv.h"
#include "graph_view.h"
#include "example.h"
#include "bc.h"
#include "pr.h"
#include "cc.h"
#include "bfs.h"
#include "gapbs/pvector.h"

#define ANALYSIS_PR 0
#define ANALYSIS_BFS 1
#define ANALYSIS_BC 2
#define ANALYSIS_CC 3

index_t residue = 1;
int THD_COUNT = 0;
vid_t _global_vcount = 0;
index_t _edge_count = 0;
int _dir = 1;//directed
int _persist = 0;//no
int _source = 0;//text
int job; // 0 means PageRank, 1 means BFS
string b_input_file, d_input_file, o_dir;

int64_t archiving_threshold = (1 << 27);

struct EdgePair {
  uint32_t u;
  uint32_t v;

  EdgePair() {}

  EdgePair(uint32_t u, uint32_t v) : u(u), v(v) {}
};

typedef pvector<EdgePair> EdgeList;

/// CL Parameters:
/// input files: 2 files
/// out directory
/// num threads
/// num vertices
/// source vertex-id
void print_usage()
{
  string help = "./exe options.\n";
  help += " --help -h: This message.\n";
  help += " --vcount -v: Vertex count\n";
  help += " --i0 -b: first input file\n";
  help += " --i1 -d: second input file\n";
  help += " --odir -o: output directory. This option will also persist the edge log.\n";
  help += " --job -j: job number. Default: 0\n";
  help += " --threadcount --t: Thread count. Default: Cores in your system - 1\n";
  help += " --source  -s: Source vertex-id for graph analysis. Default: 0\n";

  cout << help << endl;
}

const struct option longopts[] =
    {
        {"vcount",    required_argument,  0, 'v'},
        {"help",      no_argument,        0, 'h'},
        {"i0",      required_argument,  0, 'b'},
        {"i1",      required_argument,  0, 'd'},
        {"odir",      required_argument,  0, 'o'},
        {"job",       required_argument,  0, 'j'},
        {"threadcount",  required_argument,  0, 't'},
        {"source",  required_argument,  0, 's'},
        {0,			  0,				  0,  0},
    };

graph* g;

inline void parse_cl_parameters(int argc, char* argv[]) {
  int o;
  int index = 0;
  while ((o = getopt_long(argc, argv, "b:d:j:o:t:v:s:h", longopts, &index)) != -1) {
    switch(o) {
      case 'v':
        sscanf(optarg, "%d", &_global_vcount);
        cout << "Global vcount = " << _global_vcount << endl;
        break;
      case 'h':
        print_usage();
        exit(0);
        break;
      case 'b':
        b_input_file = optarg;
        cout << "first input file = " << b_input_file << endl;
        break;
      case 'd':
        d_input_file = optarg;
        cout << "second input file = " << d_input_file << endl;
        break;
      case 'j':
        job = atoi(optarg);
        break;
      case 'o':
        o_dir = optarg;
        _persist = 1;
        cout << "output dir = " << o_dir << endl;
        break;
      case 't':
        //Thread thing
        THD_COUNT = atoi(optarg);
        break;
      case 's':
        sscanf(optarg, "%d", &_source);
        break;
      default:
        cout << "invalid input " << endl;
        print_usage();
        exit(0);
    }
  }
}

int add_edge(pgraph_t<dst_id_t>* pg, unsigned int src, unsigned int dst)
{
  edgeT_t<dst_id_t> edge;
  edge.src_id = src;
  set_dst(&edge, dst);
  return pg->batch_edge(edge);
}

void check_progress(snap_t<dst_id_t>* snaph)
{
  edgeT_t<dst_id_t>* edgePtr;
  std::cout << "Archived_edges = " << snaph->get_snapmarker() << std::endl; //archived edges
  std::cout << "Non-archived edges : " << snaph->get_nonarchived_edges(edgePtr) << std::endl;
  std::cout << "Degree Out of 1 = " << snaph->get_degree_out(1) << std::endl;
  cout << "----------------" << endl;
}

void load_bgraph(pgraph_t<dst_id_t>* pg) {
  std::ifstream file(b_input_file);
  if (!file.is_open()) {
    std::cout << "Couldn't open file " << b_input_file << std::endl;
    std::exit(-2);
  }

  unsigned int u, v;
  while (file >> u >> v) {
    add_edge(pg, u, v);
    _edge_count += 1;
  }

  file.close();
}

void load_dgraph_BK(pgraph_t<dst_id_t>* pg, snap_t<dst_id_t>* snaph) {
  EdgeList el;
  std::ifstream file(d_input_file);
  if (!file.is_open()) {
    std::cout << "Couldn't open file " << d_input_file << std::endl;
    std::exit(-2);
  }

  unsigned int u, v;
  while (file >> u >> v) {
    el.push_back(EdgePair(u, v));
  }
  file.close();
  std::cout << "dynamic graph loaded into memory." << std::endl;

  uint64_t i = 0;
  for (EdgePair e : el) {
    add_edge(pg, e.u, e.v);
    // todo: why we are archiving after (1 << 16) edge insertion? Does it related to buffer size?
    i += 1;
    if ((i % archiving_threshold) == 0) {
      snaph->update_view();
//      check_progress(snaph);
    }

    if(i%10000000==0) std::cout << "inserted " << i/1000000 << " M dynamic edges." << std::endl;
  }
}

void load_dgraph(pgraph_t<dst_id_t>* pg, snap_t<dst_id_t>* snaph) {
  EdgeList el;
  std::ifstream file(d_input_file);
  if (!file.is_open()) {
    std::cout << "Couldn't open file " << d_input_file << std::endl;
    std::exit(-2);
  }

  unsigned int u, v;
  while (file >> u >> v) {
    el.push_back(EdgePair(u, v));
  }
  file.close();
  std::cout << "dynamic graph loaded into memory." << std::endl;

//  uint64_t i = 0;
  int64_t sz = el.size();

  int64_t round = 0, st;
  while(round < sz) {
    st = round;
    round = std::min((round + archiving_threshold), sz);
    #pragma omp parallel for
    for (int64_t i=st; i<round; i+=1) {
      add_edge(pg, el[i].u, el[i].v);
    }
    // todo: need to uncomment the following line next time
    waitfor_archive_durable(pg, mywtime());
  }
  _edge_count += sz;
}

pgraph_t<dst_id_t> *pgraph;

///this call is equivalent to: plaingraph_manager_t<T>::schema_plaingraphd()
void schema(graph *g){
  g->cf_info = new cfinfo_t *[2];
  g->p_info = new pinfo_t[2];

  pinfo_t *p_info = g->p_info;
  cfinfo_t *info = 0;
  const char *longname = 0;
  const char *shortname = 0;

  longname = "gtype";
  shortname = "gtype";
  info = new typekv_t;
  g->add_columnfamily(info);
  info->add_column(p_info, longname, shortname);
  ++p_info;

  longname = "friend";
  shortname = "friend";
  info = new dgraph<dst_id_t>;
  g->add_columnfamily(info);
  info->add_column(p_info, longname, shortname);
  info->flag1 = 1;
  info->flag2 = 1;
  ++p_info;
  pgraph = static_cast<pgraph_t<dst_id_t>*>(info);
}

void setup()
{
//    THD_COUNT = omp_get_max_threads() - 1;// - 3;
  cout << "Threads Count = " << THD_COUNT << endl;

  // construct graph similar to main.cpp
  g = new graph;
  g->set_odir(o_dir);// Lets not make the edges durable
  ///this call is equivalent to: plaingraph_manager_t<T>::schema_plaingraphd()
  schema(g);

  ///these calls are equivalent to: plaingraph_manager_t<T>::setup_graph
  typekv_t* typekv = g->get_typekv();
  typekv->manual_setup(_global_vcount, true);
  pgraph->prep_graph_baseline(eADJ);

  /// these calls are equivalent to: plaingraph_manager_t<T>::prep_graph2
  // create archiving/snapshot thread and persist/dis-write threads.
  g->file_open(true);
  g->create_threads(true, true); // Lets make edges durable
  // todo: not sure whether we need the following line???
//  g->prep_graph_baseline();
  g->type_store(o_dir);
}

int main(int argc, char* argv[]) {
  double start, end;

  parse_cl_parameters(argc, argv);
  setup();

  snap_t<dst_id_t>* snaph = create_static_view(pgraph, SIMPLE_MASK|V_CENTRIC);
  edgeT_t<dst_id_t>* edgePtr;

  cout << "----------------" << endl;
  check_progress(snaph);

  //---- base graph insertion -----//
  start = mywtime();
  load_bgraph(pgraph);

  //Lets wait for all the edges to archive
  waitfor_archive_durable(pgraph, start);
  end = mywtime();
  cout << "B-graph load time = " << (end - start) << endl;

  //---- dynamic graph insertion -----//
  start = mywtime();
  load_dgraph(pgraph, snaph);

  //Lets wait for all the edges to archive
  waitfor_archive_durable(pgraph, start);
  check_progress(snaph);
  end = mywtime();
  cout << "D-graph load time = " << (end - start) << endl;

  if(job == ANALYSIS_PR) {
    start = mywtime();
    snaph->update_view();
    g->type_store(o_dir);
    pvector<ScoreT> pr_ret = run_pr(snaph, 20);
    end = mywtime();
    cout << "PageRank time = " << (end - start) << endl;
    PrintTopPRScores(snaph, pr_ret);
  }

  if(job == ANALYSIS_BFS) {
    start = mywtime();
    snaph->update_view();
    g->type_store(o_dir);
    pvector<vid_t> bfs_ret = run_bfs(snaph, _edge_count, _source);
    end = mywtime();
    cout << "BFS time = " << (end - start) << endl;
    PrintBFSStats(snaph, bfs_ret);
  }

  if(job == ANALYSIS_BC) {
    start = mywtime();
    snaph->update_view();
    g->type_store(o_dir);
    pvector<ScoreT> bc_ret = run_bc(snaph, _edge_count, _source, 1);
    end = mywtime();
    cout << "BC time = " << (end - start) << endl;
    PrintTopScores(snaph, bc_ret);
  }

  if(job == ANALYSIS_CC) {
    start = mywtime();
    snaph->update_view();
    g->type_store(o_dir);
    pvector<vid_t> cc_ret = run_cc(snaph);
    end = mywtime();
    cout << "CC time = " << (end - start) << endl;
    PrintCompStats(snaph, cc_ret);
  }

  return 0;
}
