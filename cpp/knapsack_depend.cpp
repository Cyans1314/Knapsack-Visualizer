/**
 * Dependency Knapsack Problem Algorithm (Main-Attachment Problem)
 * 
 * @author Cyans
 * @affiliation Chang'an University
 * @description Dynamic programming solution for dependency knapsack problem where attachments depend on main items
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <chrono>

using namespace std;

struct Item {
    int weight;
    int value;
    int parent;  // 0 means main item, >0 means attachment depends on main item
};

struct Package {
    int weight;
    int value;
    vector<int> items;  // Contained item indices
    string desc;
};

class JsonBuilder {
private:
    stringstream ss;
    bool firstItem = true;
public:
    void startObject() { ss << "{"; firstItem = true; }
    void endObject() { ss << "}"; }
    void startArray() { ss << "["; firstItem = true; }
    void endArray() { ss << "]"; }
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

int main(int argc, char* argv[]) {
    auto startTime = chrono::high_resolution_clock::now();
    
    if (argc < 3) {
        cout << R"({"code":400,"error":"Insufficient parameters"})";
        return 1;
    }
    
    int capacity = stoi(argv[1]);
    int n = stoi(argv[2]);
    
    vector<Item> items(n);
    map<int, vector<int>> attachments;  // Main item index -> attachment index list
    
    for (int i = 0; i < n && i + 3 < argc; i++) {
        string arg = argv[i + 3];
        size_t comma1 = arg.find(',');
        size_t comma2 = arg.find(',', comma1 + 1);
        items[i].weight = stoi(arg.substr(0, comma1));
        items[i].value = stoi(arg.substr(comma1 + 1, comma2 - comma1 - 1));
        items[i].parent = stoi(arg.substr(comma2 + 1));
        
        if (items[i].parent > 0) {
            attachments[items[i].parent - 1].push_back(i);
        }
    }
    
    // Generate all valid "package" combinations
    vector<Package> packages;
    
    for (int i = 0; i < n; i++) {
        if (items[i].parent == 0) {  // Main item
            vector<int>& atts = attachments[i];
            int numAtts = atts.size();
            
            // Enumerate all attachment combinations (2^numAtts ways)
            for (int mask = 0; mask < (1 << numAtts); mask++) {
                Package pkg;
                pkg.weight = items[i].weight;
                pkg.value = items[i].value;
                pkg.items.push_back(i);
                pkg.desc = "Main" + to_string(i + 1);
                
                for (int j = 0; j < numAtts; j++) {
                    if (mask & (1 << j)) {
                        int attIdx = atts[j];
                        pkg.weight += items[attIdx].weight;
                        pkg.value += items[attIdx].value;
                        pkg.items.push_back(attIdx);
                        pkg.desc += "+Attachment" + to_string(attIdx + 1);
                    }
                }
                packages.push_back(pkg);
            }
        }
    }
    
    int m = packages.size();
    vector<vector<int>> dp(m + 1, vector<int>(capacity + 1, 0));
    
    JsonBuilder json;
    json.startObject();
    json.key("code"); json.value(200);
    json.key("type"); json.value("Dependency Knapsack");
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
    
    // Output package information
    json.key("packages");
    json.startArray();
    for (int i = 0; i < m; i++) {
        json.nextItem();
        json.startObject();
        json.key("w"); json.value(packages[i].weight);
        json.key("v"); json.value(packages[i].value);
        json.key("desc"); json.value(packages[i].desc);
        json.key("items");
        json.startArray();
        for (int idx : packages[i].items) {
            json.nextItem();
            json.value(idx);
        }
        json.endArray();
        json.endObject();
    }
    json.endArray();
    
    json.key("steps");
    json.startArray();
    
    // Convert to 0/1 knapsack for solving
    for (int i = 1; i <= m; i++) {
        int w = packages[i - 1].weight;
        int v = packages[i - 1].value;
        
        for (int j = 0; j <= capacity; j++) {
            json.nextItem();
            json.startObject();
            json.key("row"); json.value(i);
            json.key("col"); json.value(j);
            json.key("package"); json.value(packages[i - 1].desc);
            
            if (j < w) {
                dp[i][j] = dp[i - 1][j];
                json.key("val"); json.value(dp[i][j]);
                json.key("highlight");
                json.startArray();
                json.nextItem();
                json.startObject();
                json.key("r"); json.value(i - 1);
                json.key("c"); json.value(j);
                json.key("type"); json.value("without");
                json.endObject();
                json.endArray();
                json.key("decision"); json.value("skip");
            } else {
                int withoutPkg = dp[i - 1][j];
                int withPkg = dp[i - 1][j - w] + v;
                dp[i][j] = max(withoutPkg, withPkg);
                
                json.key("val"); json.value(dp[i][j]);
                json.key("highlight");
                json.startArray();
                json.nextItem();
                json.startObject();
                json.key("r"); json.value(i - 1);
                json.key("c"); json.value(j);
                json.key("type"); json.value("without");
                json.endObject();
                json.nextItem();
                json.startObject();
                json.key("r"); json.value(i - 1);
                json.key("c"); json.value(j - w);
                json.key("type"); json.value("with");
                json.endObject();
                json.endArray();
                
                json.key("decision"); 
                json.value(dp[i][j] == withoutPkg ? "skip" : "take");
            }
            json.endObject();
        }
    }
    json.endArray();
    
    // Backtrack
    json.key("path");
    json.startArray();
    int j = capacity;
    for (int i = m; i > 0 && j > 0; i--) {
        if (dp[i][j] != dp[i - 1][j]) {
            json.nextItem();
            json.startObject();
            json.key("r"); json.value(i);
            json.key("c"); json.value(j);
            json.key("package"); json.value(packages[i - 1].desc);
            json.key("items");
            json.startArray();
            for (int idx : packages[i - 1].items) {
                json.nextItem();
                json.value(idx);
            }
            json.endArray();
            json.endObject();
            j -= packages[i - 1].weight;
        }
    }
    json.endArray();
    
    json.key("max_value"); json.value(dp[m][capacity]);
    
    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();
    json.key("time_ms"); json.value(duration);
    
    json.key("complexity");
    json.startObject();
    json.key("time"); json.value("O(2^k*C)");
    json.key("space"); json.value("O(m*C)");
    json.key("operations"); json.value(m * (capacity + 1));
    json.key("memory_bytes"); json.value((m + 1) * (capacity + 1) * 4);
    json.endObject();
    
    json.endObject();
    cout << json.str();
    return 0;
}




