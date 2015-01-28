/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium
team <http://www.bh107.org>.

Bohrium is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.

Bohrium is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the
GNU Lesser General Public License along with Bohrium.

If not, see <http://www.gnu.org/licenses/>.
*/
#include <bh.h>
#include <bh_dag.h>
#include <bh_fuse_cache.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/algorithm/string/predicate.hpp> //For iequals()
#include <boost/range/adaptors.hpp>
#include <vector>
#include <map>
#include <iterator>
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>

#define VERBOSE

using namespace std;
using namespace boost;
using namespace bohrium;
using namespace bohrium::dag;

/* Help function that fuses the edges in 'edges2explore' where the 'mask' is true */
pair<int64_t,bool> fuse_mask(int64_t best_cost, const vector<EdgeW> &edges2explore,
                             const GraphDW &graph, const vector<bool> &mask, bh_ir &bhir,
                             GraphD &dag)
{
    bool fusibility=true;
    vector<EdgeW> edges2merge;
    unsigned int i=0;
    BOOST_FOREACH(const EdgeW &e, edges2explore)
    {
        if(mask[i++])
            edges2merge.push_back(e);
    }

    //Help function to find the new location
    struct find_new_location
    {
        Vertex operator()(const map<Vertex, Vertex> &loc_map, Vertex v)
        {
            Vertex v_mapped = loc_map.at(v);
            if(v_mapped == v)
                return v;
            else
                return (*this)(loc_map, v_mapped);
        }
    }find_loc;

    //'loc_map' maps a vertex before the merge to the corresponding vertex after the merge
    map<Vertex, Vertex> loc_map;
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        loc_map[v] = v;
    }

    //Lets record the merges into 'loc_map'
    BOOST_FOREACH(const EdgeW &e, edges2merge)
    {
        Vertex v1 = find_loc(loc_map, source(e, graph.bglW()));
        Vertex v2 = find_loc(loc_map, target(e, graph.bglW()));
        loc_map[v1] = v2;
    }

    //Pack 'loc_map' such that all keys maps directly to a new vertex thus after
    //this point there is no need to call find_loc().
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        Vertex v_mapped = find_loc(loc_map, loc_map.at(v));
        if(v_mapped != v)
            loc_map[v] = v_mapped;
    }

    //Create the new vertices and insert instruction topologically
    map<Vertex, bh_ir_kernel> new_vertices;
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        if(loc_map.at(v) == v)
            new_vertices[v] = bh_ir_kernel(bhir);
    }
    vector<Vertex> topological_order;
    topological_sort(dag, back_inserter(topological_order));
    BOOST_REVERSE_FOREACH(Vertex vertex, topological_order)
    {
        Vertex v = loc_map.at(vertex);
        bh_ir_kernel &k = new_vertices.at(v);
        BOOST_FOREACH(uint64_t idx, dag[vertex].instr_indexes)
        {
            if(not k.fusible(idx))
                fusibility = false;
            k.add_instr(idx);
        }
    }

    //TODO: Remove this assert check
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        if(loc_map.at(v) == v)
            assert(new_vertices[v].instr_indexes.size() > 0);
    }

    //Find the total cost
    int64_t cost=0;
    BOOST_FOREACH(const bh_ir_kernel &k, new_vertices | adaptors::map_values)
    {
        cost += k.cost();
    }

    //Check if we need to continue
    if(cost >= best_cost or not fusibility)
        return make_pair(cost,false);

    //Merge the vertice in the DAG
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        Vertex loc_v = loc_map.at(v);
        if(loc_v == v)
        {
            dag[v] = new_vertices.at(v);
            assert(dag[v].instr_indexes.size() > 0);
        }
        else//Lets merge 'v' into 'loc_v'
        {
            BOOST_FOREACH(Vertex a, adjacent_vertices(v, dag))
            {
                a = loc_map.at(a);
                if(a != loc_v)
                    add_edge(loc_v, a, dag);
            }
            BOOST_FOREACH(Vertex a, inv_adjacent_vertices(v, dag))
            {
                a = loc_map.at(a);
                if(a != loc_v)
                    add_edge(a, loc_v, dag);
            }
            clear_vertex(v, dag);
            dag[v] = bh_ir_kernel(bhir);
        }
    }

    //TODO: remove assert check
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        if(dag[loc_map.at(v)].instr_indexes.size() == 0)
        {
            cout << v << endl;
            cout << loc_map.at(v) << endl;
            assert(1 == 2);
        }
    }

    //Check for cycles
    if(cycles(dag))
    {
        return make_pair(cost,false);
    }

    assert(cost == (int64_t)dag_cost(dag));
    return make_pair(cost,true);
}

/* Private class to find the optimal solution through branch and bound */
class Solver
{
public:
    bh_ir &bhir;
    const GraphDW &dag;
    const vector<EdgeW> &edges2explore;
    int64_t best_cost;
    int64_t one_cost;
    GraphD best_dag;
    FuseCache cache;

    #ifdef VERBOSE
        double  purge_count;
        uint64_t explore_count;
    #endif

    /* The constructor */
    Solver(bh_ir &b, const GraphDW &d, const vector<EdgeW> &e, FuseCache &cache):
           bhir(b),dag(d),edges2explore(e), cache(cache)
    {
        //Lets find a good initial guess
        GraphDW new_dag;
        vector<bh_ir_kernel> kernel_list;
        if(cache.lookup(bhir.instr_list, bhir, kernel_list))
        {
            from_kernels(kernel_list, new_dag);
            #ifdef VERBOSE
                cout << "Loading initial guess from the cache" << endl;
            #endif
        }
        else
        {
            new_dag = dag;
            fuse_greedy(new_dag);
        }
        best_dag = new_dag.bglD();
        best_cost = dag_cost(best_dag);

        #ifdef VERBOSE
            purge_count=0;
            explore_count=0;
        #endif
    }

    /* Find the optimal solution through branch and bound */
    void branch_n_bound(vector<bool> mask, unsigned int offset, bool merge_next)
    {
        if(not merge_next)
        {
            GraphD new_dag(dag.bglD());
            mask[offset] = merge_next;
            bool fusibility;
            int64_t cost;
            tie(cost, fusibility) = fuse_mask(best_cost, edges2explore, dag, mask, bhir, new_dag);

            #ifdef VERBOSE
                if(explore_count%1000 == 0)
                {
                    cout << "[" << explore_count << "] " << "purge count: ";
                    cout << purge_count << " / " << pow(2.0,mask.size()) << endl;
                    cout << "cost: " << cost << ", best_cost: " << best_cost;
                    cout << ", fusibility: " << fusibility << endl;
                }
                ++explore_count;
            #endif

            if(cost >= best_cost)
            {
                #ifdef VERBOSE
                    purge_count += pow(2.0, mask.size()-offset-1);
                #endif
                return;
            }
            if(fusibility)
            {
                //Lets save the new best dag
                best_cost = cost;
                best_dag = new_dag;
                assert(dag_validate(best_dag));

                //Lets write the current best to file
                vector<bh_ir_kernel> kernel_list;
                fill_kernel_list(best_dag, kernel_list);
                const InstrIndexesList &i = cache.insert(bhir.instr_list, kernel_list);
                cache.write_to_files();

                #ifdef VERBOSE
                    stringstream ss;
                    string filename;
                    i.get_filename(filename);
                    ss << "new_best_dag-" << filename << ".dot";
                    cout << "write file: " << ss.str() << endl;
                    pprint(best_dag, ss.str().c_str());
                    purge_count += pow(2.0, mask.size()-offset-1);
                #endif
                return;
            }
        }
        if(offset+1 < mask.size())
        {
            branch_n_bound(mask, offset+1, false);
            branch_n_bound(mask, offset+1, true);
        }
    }
};

void get_edges2explore(const GraphDW &dag, vector<EdgeW> &edges2explore)
{
    BOOST_FOREACH(const EdgeW &e, edges(dag.bglW()))
    {
        edges2explore.push_back(e);
    }
    sort_weights(dag.bglW(), edges2explore);
    string order;
    {
        const char *t = getenv("BH_FUSER_OPTIMAL_ORDER");
        if(t == NULL)
            order ="regular";
        else
            order = t;
    }
    if(not iequals(order, "regular"))
    {
        if(iequals(order, "reverse"))
        {
            reverse(edges2explore.begin(), edges2explore.end());
        }
        else if(iequals(order, "random"))
        {
            random_shuffle(edges2explore.begin(), edges2explore.end());
        }
        else
        {
            cerr << "FUSER-OPTIMAL: unknown BH_FUSER_OPTIMAL_ORDER: " << order << endl;
            order = "regular";
        }
    }
    cout << "BH_FUSER_OPTIMAL_ORDER: " << order << endl;
}


/* Fuse the 'dag' optimally */
void fuse_optimal(bh_ir &bhir, const GraphDW &dag, GraphD &output, FuseCache &cache)
{
    //The list of edges that we should try to merge
    vector<EdgeW> edges2explore;
    get_edges2explore(dag, edges2explore);
    if(edges2explore.size() == 0)
        return;

    //First we check the trivial case where all kernels are merged
    vector<bool> mask(edges2explore.size(), true);
    {
        GraphD new_dag(dag.bglD());
        bool fuse = fuse_mask(numeric_limits<int64_t>::max(), edges2explore,
                                        dag, mask, bhir, new_dag).second;
        if(fuse)
        {
            output = new_dag;
            return;
        }
    }

    Solver solver(bhir, dag, edges2explore, cache);
    if(mask.size() > 100)
    {
        cout << "FUSER-OPTIMAL: ABORT the size of the search space is too large: 2^";
        cout << mask.size() << "!" << endl;
    }
    else
    {
        cout << "FUSER-OPTIMAL: the size of the search space is 2^" << mask.size() << "!" << endl;
        solver.branch_n_bound(mask, 0, false);
        solver.branch_n_bound(mask, 0, true);
    }
    output = solver.best_dag;
}

void timer_handler(int signum)
{
    cout << "ABORT! - timeout" << endl;
    exit(-1);
}
void set_abort_timer()
{
    const char *bh_fuser_timeout = getenv("BH_FUSER_TIMEOUT");
    if(bh_fuser_timeout == NULL)
        return;
    long int timeout = strtol(bh_fuser_timeout, NULL, 10);
    cout << "[ABORT] Fuse-Abort timeout is " << timeout << " sec" << endl;

    struct sigaction sa;
    struct itimerval timer;

    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = &timer_handler;
    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = timeout;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 100000;

    setitimer (ITIMER_REAL, &timer, NULL);
}

void do_fusion(bh_ir &bhir, FuseCache &cache)
{
    set_abort_timer();

    GraphDW dag;
    from_bhir(bhir, dag);
    fuse_gentle(dag);
    dag.transitive_reduction();
    assert(dag_validate(dag.bglD()));

    GraphD output;
    fuse_optimal(bhir, dag, output, cache);
    if(num_vertices(output) == 0)
        output = dag.bglD();

    assert(dag_validate(output));
    fill_kernel_list(output, bhir.kernel_list);
}

void fuser(bh_ir &bhir, FuseCache &cache)
{
    if(bhir.kernel_list.size() != 0)
        throw logic_error("The kernel_list is not empty!");

    BatchHash batch(bhir.instr_list);
//    if(not cache.lookup(batch, bhir, bhir.kernel_list))
    {
        do_fusion(bhir, cache);
        cache.insert(batch, bhir.kernel_list);
    }
}

