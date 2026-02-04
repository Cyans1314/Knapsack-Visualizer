/**
 * Group Knapsack Problem Algorithm
 * 
 * @author Cyans
 * @affiliation Chang'an University
 * @description Dynamic programming solution for group knapsack problem where at most one item can be selected from each group
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
    int group;
    int originalIndex;
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
    map<int, vector<int>> groups;  // group_id -> item indices
    
    for (int i = 0; i < n && i + 3 < argc; i++) {
        string arg = argv[i + 3];
        size_t comma1 = arg.find(',');
        size_t comma2 = arg.find(',', comma1 + 1);
        items[i].weight = stoi(arg.substr(0, comma1));
        items[i].value = stoi(arg.substr(comma1 + 1, comma2 - comma1 - 1));
        items[i].group = stoi(arg.substr(comma2 + 1));
        items[i].originalIndex = i;
        groups[items[i].group].push_back(i);
    }
    
    int numGroups = groups.size();
    vector<int> groupIds;
    for (auto& p : groups) {
        groupIds.push_back(p.first);
    }
    
    vector<vector<int>> dp(numGroups + 1, vector<int>(capacity + 1, 0));
    
    JsonBuilder json;
    json.startObject();
    json.key("code"); json.value(200);
    json.key("type"); json.value("Group Knapsack");
    json.key("capacity"); json.value(capacity);
    
    json.key("items");
    json.startArray();
    for (int i = 0; i < n; i++) {
        json.nextItem();
        json.startObject();
        json.key("w"); json.value(items[i].weight);
        json.key("v"); json.value(items[i].value);
        json.key("g"); json.value(items[i].group);
        json.endObject();
    }
    json.endArray();
    
    // Output group information
    json.key("groups");
    json.startArray();
    for (int gid : groupIds) {
        json.nextItem();
        json.startObject();
        json.key("id"); json.value(gid);
        json.key("items");
        json.startArray();
        for (int idx : groups[gid]) {
            json.nextItem();
            json.value(idx);
        }
        json.endArray();
        json.endObject();
    }
    json.endArray();
    
    json.key("steps");
    json.startArray();
    
    // Group knapsack DP
    for (int g = 0; g < numGroups; g++) {
        int gid = groupIds[g];
        vector<int>& groupItems = groups[gid];
        
        for (int j = 0; j <= capacity; j++) {
            // First inherit value from previous group
            dp[g + 1][j] = dp[g][j];
            int bestChoice = -1;  // -1 means not selected
            
            // Try selecting each item in group
            json.nextItem();
            json.startObject();
            json.key("row"); json.value(g + 1);
            json.key("col"); json.value(j);
            json.key("groupId"); json.value(gid);
            
            json.key("tryItems");
            json.startArray();
            
            for (int idx : groupItems) {
                int w = items[idx].weight;
                int v = items[idx].value;
                
                json.nextItem();
                json.startObject();
                json.key("itemIdx"); json.value(idx);
                json.key("w"); json.value(w);
                json.key("v"); json.value(v);
                
                if (j >= w && dp[g][j - w] + v > dp[g + 1][j]) {
                    dp[g + 1][j] = dp[g][j - w] + v;
                    bestChoice = idx;
                    json.key("canTake"); json.value(1);
                    json.key("newVal"); json.value(dp[g + 1][j]);
                } else {
                    json.key("canTake"); json.value(j >= w ? 1 : 0);
                    json.key("newVal"); json.value(j >= w ? dp[g][j - w] + v : 0);
                }
                json.endObject();
            }
            json.endArray();
            
            json.key("val"); json.value(dp[g + 1][j]);
            json.key("bestChoice"); json.value(bestChoice);
            
            json.key("highlight");
            json.startArray();
            json.nextItem();
            json.startObject();
            json.key("r"); json.value(g);
            json.key("c"); json.value(j);
            json.key("type"); json.value("without");
            json.endObject();
            if (bestChoice >= 0) {
                json.nextItem();
                json.startObject();
                json.key("r"); json.value(g);
                json.key("c"); json.value(j - items[bestChoice].weight);
                json.key("type"); json.value("with");
                json.endObject();
            }
            json.endArray();
            
            json.key("decision"); 
            json.value(bestChoice >= 0 ? "take" : "skip");
            
            json.endObject();
        }
    }
    json.endArray();
    
    // Backtrack
    json.key("path");
    json.startArray();
    int j = capacity;
    for (int g = numGroups - 1; g >= 0 && j > 0; g--) {
        int gid = groupIds[g];
        for (int idx : groups[gid]) {
            int w = items[idx].weight;
            int v = items[idx].value;
            if (j >= w && dp[g + 1][j] == dp[g][j - w] + v) {
                json.nextItem();
                json.startObject();
                json.key("r"); json.value(g + 1);
                json.key("c"); json.value(j);
                json.key("item"); json.value(idx);
                json.key("group"); json.value(gid);
                json.endObject();
                j -= w;
                break;
            }
        }
    }
    json.endArray();
    
    json.key("max_value"); json.value(dp[numGroups][capacity]);
    
    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();
    json.key("time_ms"); json.value(duration);
    
    json.key("complexity");
    json.startObject();
    json.key("time"); json.value("O(G*C*K)");
    json.key("space"); json.value("O(G*C)");
    json.key("operations"); json.value(numGroups * (capacity + 1) * (n / numGroups));
    json.key("memory_bytes"); json.value((numGroups + 1) * (capacity + 1) * 4);
    json.endObject();
    
    json.endObject();
    cout << json.str();
    return 0;
}




