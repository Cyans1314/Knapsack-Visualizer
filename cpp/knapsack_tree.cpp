/**
 * Tree Knapsack Problem Algorithm
 * 
 * @author Cyans
 * @affiliation Chang'an University
 * @description Dynamic programming solution for tree knapsack problem with tree-structured dependencies between items
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>

using namespace std;

struct Item {
    int weight;
    int value;
    int parent;  // Parent node number (1-based), 0 means root node
};

class JsonBuilder {
private:
    stringstream ss;
    bool firstItem = true;
public:
    void startObject() { ss << "{"; firstItem = true; }
    void endObject() { ss << "}"; firstItem = false; }
    void startArray() { ss << "["; firstItem = true; }
    void endArray() { ss << "]"; firstItem = false; }
    void key(const string& k) {
        if (!firstItem) ss << ",";
        ss << "\"" << k << "\":";
        firstItem = false;
    }
    void value(int v) { ss << v; }
    void value(const string& v) { ss << "\"" << v << "\""; }
    void value(double v) { ss << v; }
    void nextItem() {
        if (!firstItem) ss << ",";
        firstItem = false;
    }
    string str() { return ss.str(); }
};

vector<vector<int>> children;
vector<Item> items;
vector<vector<int>> dp;
int capacity;
JsonBuilder json;

void dfs(int u) {
    // Initialize: must select current node to select child nodes
    int w = items[u].weight;
    int v = items[u].value;
    
    // Process all child nodes first
    for (int child : children[u]) {
        dfs(child);
    }
    
    // Initialize DP array for current node
    // DP[u][j] represents max value in subtree rooted at u with capacity j
    for (int j = w; j <= capacity; j++) {
        dp[u][j] = v;  // At least select current node
    }
    
    // Merge child node DP values (similar to group knapsack)
    for (int child : children[u]) {
        // Output step: merge child node
        json.nextItem();
        json.startObject();
        json.key("node"); json.value(u);
        json.key("childNode"); json.value(child);
        json.key("action"); json.value("merge");
        
        // Enumerate capacity from large to small to avoid reuse
        for (int j = capacity; j >= w; j--) {
            for (int k = 0; k <= j - w; k++) {
                if (dp[u][j] < dp[u][j - k] + dp[child][k]) {
                    dp[u][j] = dp[u][j - k] + dp[child][k];
                }
            }
        }
        
        json.key("dpValues");
        json.startArray();
        for (int j = 0; j <= capacity; j++) {
            json.nextItem();
            json.value(dp[u][j]);
        }
        json.endArray();
        
        json.endObject();
    }
    
    // Output step: current node processing complete
    json.nextItem();
    json.startObject();
    json.key("node"); json.value(u);
    json.key("action"); json.value("complete");
    json.key("w"); json.value(w);
    json.key("v"); json.value(v);
    
    // Find capacity corresponding to optimal value
    int bestJ = w;
    for (int j = w; j <= capacity; j++) {
        if (dp[u][j] > dp[u][bestJ]) {
            bestJ = j;
        }
    }
    json.key("col"); json.value(bestJ);
    json.key("val"); json.value(dp[u][bestJ]);
    json.key("decision"); json.value(dp[u][bestJ] > 0 ? "take" : "skip");
    
    json.key("dpValues");
    json.startArray();
    for (int j = 0; j <= capacity; j++) {
        json.nextItem();
        json.value(dp[u][j]);
    }
    json.endArray();
    
    // Highlight parent node
    if (items[u].parent > 0) {
        json.key("parentNode"); json.value(items[u].parent - 1);
        json.key("highlight");
        json.startArray();
        json.nextItem();
        json.startObject();
        json.key("node"); json.value(items[u].parent - 1);
        json.key("type"); json.value("parent");
        json.endObject();
        json.endArray();
    } else {
        json.key("highlight");
        json.startArray();
        json.endArray();
    }
    
    json.endObject();
}

int main(int argc, char* argv[]) {
    auto startTime = chrono::high_resolution_clock::now();
    
    if (argc < 3) {
        cout << R"({"code":400,"error":"Insufficient parameters"})";
        return 1;
    }
    
    capacity = stoi(argv[1]);
    int n = stoi(argv[2]);
    
    items.resize(n);
    children.resize(n);
    dp.resize(n, vector<int>(capacity + 1, 0));
    
    vector<int> roots;
    
    for (int i = 0; i < n && i + 3 < argc; i++) {
        string arg = argv[i + 3];
        size_t comma1 = arg.find(',');
        size_t comma2 = arg.find(',', comma1 + 1);
        items[i].weight = stoi(arg.substr(0, comma1));
        items[i].value = stoi(arg.substr(comma1 + 1, comma2 - comma1 - 1));
        items[i].parent = stoi(arg.substr(comma2 + 1));
        
        if (items[i].parent == 0) {
            roots.push_back(i);
        } else {
            children[items[i].parent - 1].push_back(i);
        }
    }
    
    json.startObject();
    json.key("code"); json.value(200);
    json.key("type"); json.value("Tree Knapsack");
    json.key("capacity"); json.value(capacity);
    
    json.key("items");
    json.startArray();
    for (int i = 0; i < n; i++) {
        json.nextItem();
        json.startObject();
        json.key("w"); json.value(items[i].weight);
        json.key("v"); json.value(items[i].value);
        json.key("p"); json.value(items[i].parent);
        json.endObject();
    }
    json.endArray();
    
    // Output tree structure
    json.key("tree");
    json.startObject();
    json.key("roots");
    json.startArray();
    for (int r : roots) {
        json.nextItem();
        json.value(r);
    }
    json.endArray();
    json.key("children");
    json.startArray();
    for (int i = 0; i < n; i++) {
        json.nextItem();
        json.startArray();
        for (int c : children[i]) {
            json.nextItem();
            json.value(c);
        }
        json.endArray();
    }
    json.endArray();
    json.endObject();
    
    json.key("steps");
    json.startArray();
    
    // DFS for each root node
    for (int root : roots) {
        dfs(root);
    }
    
    json.endArray();
    
    // Calculate final result: sum of max values from all root nodes
    // If multiple roots, need to merge
    int maxValue = 0;
    if (roots.size() == 1) {
        maxValue = dp[roots[0]][capacity];
    } else {
        // Multiple root nodes, need one group knapsack
        vector<int> finalDp(capacity + 1, 0);
        for (int root : roots) {
            for (int j = capacity; j >= 0; j--) {
                for (int k = 0; k <= j; k++) {
                    finalDp[j] = max(finalDp[j], finalDp[j - k] + dp[root][k]);
                }
            }
        }
        maxValue = finalDp[capacity];
    }
    
    // Backtrack path
    json.key("path");
    json.startArray();
    // Only mark selected root nodes
    for (int root : roots) {
        if (dp[root][capacity] > 0) {
            json.nextItem();
            json.startObject();
            json.key("node"); json.value(root);
            json.key("val"); json.value(dp[root][capacity]);
            json.endObject();
        }
    }
    json.endArray();
    
    json.key("max_value"); json.value(maxValue);
    
    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();
    json.key("time_ms"); json.value(duration);
    
    json.key("complexity");
    json.startObject();
    json.key("time"); json.value("O(n*C^2)");
    json.key("space"); json.value("O(n*C)");
    json.key("operations"); json.value(n * (capacity + 1) * (capacity + 1));
    json.key("memory_bytes"); json.value(n * (capacity + 1) * 4);
    json.endObject();
    
    json.endObject();
    cout << json.str();
    return 0;
}




