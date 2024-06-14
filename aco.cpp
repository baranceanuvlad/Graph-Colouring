#include <bits/stdc++.h>
#include <chrono>
#include <windows.h>
#include <psapi.h>

using namespace std;

const int colony_size = 30;
const int generations = 30;
const double pheromone_decay = 0.15;
const int alpha_value = 2;
const int beta_value = 5;

struct Graph
{
    int node;
    double pheromone_level;
};

vector<vector<Graph>> G;
int n, m; // n - vertices, m - edges
random_device rd;
mt19937 generator(rd());

class Ant
{
    public:
    int start_node;
    int distance;
    int max_colors;
    set<int> colors_available;
    map<int, int> colors_used;

    set<int> viz;
    set<int> neviz;

    Ant(set<int> colors)
    {
        colors_available = colors;
        colors_used.clear();

        max_colors = colors.size();

        uniform_int_distribution<int> distribution(1, n);
        start_node = distribution(generator);

        viz.clear();
        neviz.clear();
        for (int i = 1; i <= n; i++)
        {
            neviz.insert(i);
        }

        Paint();
    }

    void Paint()
    {
        int color = 0;
        for (int i = 1; i <= max_colors; i++)
        {
            if (colors_available.count(i) > 0)
            {
                color = i;
                break;
            }
        }

        // cout << "Start node: " << start_node << " " << color << '\n';
        colors_used.insert({start_node, color});
        viz.insert(start_node);
        neviz.erase(start_node);
    }

    void AssignColor(int node, int color)
    {
        // cout << node << " " << color << '\n';
        colors_used[node] = color;
        viz.insert(node);
        neviz.erase(node);
    }

    double GetPheroValue(int a, int b)
    {
        for (int i = 0; i < G[a].size(); i++)
            if (G[a][i].node == b)
                return G[a][i].pheromone_level;
        return 0;
    }

    int DifferentNeighbours(int x)
    {
        set<int> colors;
        for (int i = 0; i < G[x].size(); i++)
        {
            colors.insert(colors_used[G[x][i].node]);
        }

        return colors.size();
    }

    int GetCandidateNode()
    {
        int candidate_node = 0;
        double max_value = 0;
        vector<double> heuristic_values;
        vector<int> candidates;
        vector<int> available_candidates;
        // cout<<"Neviz nodes: ";
        for (auto x : neviz)
        {
            heuristic_values.push_back(pow(GetPheroValue(start_node, x), alpha_value) * pow(DifferentNeighbours(x), beta_value));
            candidates.push_back(x);
            //cout << x << "\n";
        }
        // cout<<'\n';
        // cout << heuristic_values.size() << '\n';
        for (int j = 0; j < heuristic_values.size(); j++)
        {
            max_value = max(max_value, heuristic_values[j]);
        }

        for (int j = 0; j < candidates.size(); j++)
        {
            if (heuristic_values[j] >= max_value)
            {
                available_candidates.push_back(candidates[j]);
            }
        }

        uniform_int_distribution<int> distribution(0, available_candidates.size() - 1);
        candidate_node = available_candidates[distribution(generator)];
        start_node = candidate_node;
        return candidate_node;
    }

    void StartColoring()
    {
        while (neviz.size() > 0)
        {
            set<int> colors;
            int candidate_node = GetCandidateNode();
            for (int j = 0; j < G[candidate_node].size(); j++)
            {
                // cout << "Neighbor " << G[candidate_node][j].node << " with color " << colors_used[G[candidate_node][j].node] << '\n';
                colors.insert(colors_used[G[candidate_node][j].node]);
            }

            for (auto x : colors_available)
                if (colors.count(x) == 0)
                {
                    AssignColor(candidate_node, x);
                    break;
                }
        }

        set<int> number_of_colors;
        for (auto x : colors_used)
            number_of_colors.insert(x.second);
        distance = number_of_colors.size();
    }

    void LeavePheromoneTrail()
    {
        vector<int> nodes_with_color(colors_used.size());
        int phero_level = 0;
        for (auto x : colors_used)
            nodes_with_color[x.second]++;
        for (int i = 0; i < nodes_with_color.size(); i++)
            phero_level += (nodes_with_color[i] * nodes_with_color[i]);
        
        for (int i = 1; i <= n; i++)
            for (int j = 0; j < G[i].size(); j++)
                if (colors_used.count(i) > 0 && colors_used.count(G[i][j].node) > 0 && colors_used[i] != colors_used[G[i][j].node])
                {
                    //G[i][j].pheromone_level += 1.0;
                    G[i][j].pheromone_level += phero_level;
                }
    }
};

vector<Ant> GenerateStartingColony(set<int> max_colors)
{
    vector<Ant> ans;
    for (int i = 0; i < colony_size; i++)
    {
        Ant ant = Ant(max_colors);
        ans.push_back(ant);
    }

    return ans;
}

set<int> ComputeMaxColors()
{
    int ans = 0;
    set<int> colors;
    for (int i = 0; i < G.size(); i++)
        ans = max(ans, (int) G[i].size());
    for (int i = 1; i <= ans + 1; i++)
        colors.insert(i);
    return colors;
}

void DoDecay()
{
    for (int i = 0; i < G.size(); i++)
    {
        for (int j = 0; j < G[i].size(); j++)
        {
            G[i][j].pheromone_level = G[i][j].pheromone_level * (1 - pheromone_decay);
        }
    }
}

void UpdateSolution(vector<Ant> ants)
{
    int best_solution = 999999;
    int best_ant = 0;
    for (int i = 0; i < ants.size(); i++)
    {
        if (ants[i].distance < best_solution)
        {
            best_solution = ants[i].distance;
            best_ant = i;
        }
    }

    ants[best_ant].LeavePheromoneTrail();
}

void AntColony(ofstream& fout)
{
    set<int> max_colors = ComputeMaxColors();
    int best_result = 999999;
    int worst_result = 0;
    int total_sum = 0;
    int results_number = 0;
    vector<int> all_results;
    map<int, int> best_solution;

    for (int i = 0; i < generations; i++)
    {
        cout << "Generation: " << i + 1 << '\n';

        vector<Ant> ants = GenerateStartingColony(max_colors);
        for (int j = 0; j < ants.size(); j++)
        {
            ants[j].StartColoring();
        }

        DoDecay();
        UpdateSolution(ants);

        int result = 999999;
        map<int, int> solution;
        for (int j = 0; j < ants.size(); j++)
        {
            total_sum += ants[j].distance;
            results_number++;
            all_results.push_back(ants[j].distance);

            if (result > ants[j].distance)
            {
                result = ants[j].distance;
                solution = ants[j].colors_used;
            }
            else if (worst_result < ants[j].distance)
            {
                worst_result = ants[j].distance;
            }
        }

        if (result < best_result)
        {
            best_result = result;
            best_solution = solution;
        }

    }

    double sum_of_squares = 0;
    double mean = (double) ((double) total_sum / (double) results_number);
    for (int i = 0; i < all_results.size(); i++)
        sum_of_squares += (all_results[i] - mean) * (all_results[i] - mean);
    fout << "Minim is " << best_result << " Maxim is " << worst_result << " Mean is " << mean << " Standard deviation is " << (double) (sum_of_squares / results_number) << '\n';
    fout << "Colors used " << best_result << '\n';
    fout << "Coloring is\n";
    for (auto x : best_solution)
    {
        fout << x.second << " ";
    }

    fout << '\n';
}

void ReadGraph(ifstream& fin)
{
    string x;
    while (fin>>x)
    {
        if (x == "p")
        {
            fin >> x; // the word "edge"
            fin >> n >> m;
            break;
        }
    }

    for (int i = 0; i <= n; i++)
        G.push_back(vector<Graph>());
    
    while (fin >> x)
    {
        if (x == "e")
        {
            int a, b;
            fin >> a >> b;
            G[a].push_back({b, 0});
            G[b].push_back({a, 0});
        }
    }
}

SIZE_T getMemoryUsage() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    return pmc.PrivateUsage;
}

int main()
{
    srand(time(NULL));

    string instance_name;
    cout<<"Instance's name (without its extension)\n";
    cin>>instance_name;
    ifstream fin("./instances/" + instance_name + ".col");
    ofstream fout("./results_ACO/" + instance_name + ".out", ios::app);

    ReadGraph(fin);
    
    SIZE_T memoryBefore = getMemoryUsage();

    auto start = std::chrono::high_resolution_clock::now();

    AntColony(fout);

    auto end = std::chrono::high_resolution_clock::now();

    SIZE_T memoryAfter = getMemoryUsage();
        
    std::chrono::duration<double> duration = end - start;
    fout << "Execution time: " << duration.count() << " seconds\n";
    fout << "Memory Used Before: " << memoryBefore / 1024 << " KB\n";
    fout << "Memory Used After: " << memoryAfter / 1024 << " KB\n";
    fout << "Memory Used By Function: " << (memoryAfter - memoryBefore) / 1024 << " KB\n";
    fout << "Colony size: " << colony_size << '\n';
    fout << "Generations: " << generations << '\n';
    fout << "Pheromone Decay: " << pheromone_decay << '\n';
    fout << "Alpha: " << alpha_value << '\n';
    fout << "Beta: " << beta_value << '\n';

    fout << '\n';
    return 0;
}
