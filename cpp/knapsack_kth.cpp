/**
 * Kth Optimal Solution Problem Algorithm
 * 
 * @author Cyans
 * @affiliation Chang'an University
 * @description Dynamic programming solution for finding the Kth optimal solution instead of just the best solution
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

// Merge two sorted arrays, keep top K
vector<int> mergeTopK(const vector<int>& a, const vector<int>& b, int k) {
    vector<int> result;
    size_t i = 0, j = 0;
    while ((int)result.size() < k && (i < a.size() || j < b.size())) {
        if (i >= a.size()) {
            result.push_back(b[j++]);
        } else if (j >= b.size()) {
            result.push_back(a[i++]);
        } else if (a[i] >= b[j]) {
            result.push_back(a[i++]);
        } else {
            result.push_back(b[j++]);
        }
    }
    return result;
}

int main(int argc, char* argv[]) {
    auto startTime = chrono::high_resolution_clock::now();
    
    if (argc < 4) {
        cout << R"({"code":400,"error":"Insufficient parameters"})";
        return 1;
    }
    
    int capacity = stoi(argv[1]);
    int K = stoi(argv[2]);
    int n = stoi(argv[3]);
    
    vector<Item> items(n);
    for (int i = 0; i < n && i + 4 < argc; i++) {
        string arg = argv[i + 4];
        size_t comma = arg.find(',');
        items[i].weight = stoi(arg.substr(0, comma));
        items[i].value = stoi(arg.substr(comma + 1));
    }
    
    // DP[i][j] = top K values for first i items with capacity j
    vector<vector<vector<int>>> dp(n + 1, 
        vector<vector<int>>(capacity + 1, vector<int>(1, 0)));
    
    JsonBuilder json;
    json.startObject();
    json.key("code"); json.value(200);
    json.key("type"); json.value("Kth Optimal");
    json.key("capacity"); json.value(capacity);
    json.key("k"); json.value(K);
    
    json.key("items");
    json.startArray();
    for (int i = 0; i < n; i++) {
        json.nextItem();
        json.startObject();
        json.key("w"); json.value(items[i].weight);
        json.key("v"); json.value(items[i].value);
        json.endObject();
    }
    json.endArray();
    
    json.key("steps");
    json.startArray();
    
    for (int i = 1; i <= n; i++) {
        int w = items[i - 1].weight;
        int v = items[i - 1].value;
        
        for (int j = 0; j <= capacity; j++) {
            json.nextItem();
            json.startObject();
            json.key("row"); json.value(i);
            json.key("col"); json.value(j);
            
            vector<int> notTake = dp[i - 1][j];
            vector<int> take;
            
            if (j >= w) {
                // Take this item: add v to each value in dp[i-1][j-w]
                for (int val : dp[i - 1][j - w]) {
                    take.push_back(val + v);
                }
            }
            
            // Merge and keep top K
            dp[i][j] = mergeTopK(notTake, take, K);
            
            // Output top K values for current cell
            json.key("vals");
            json.startArray();
            for (int val : dp[i][j]) {
                json.nextItem();
                json.value(val);
            }
            json.endArray();
            
            json.key("val"); json.value(dp[i][j].empty() ? 0 : dp[i][j][0]);
            
            json.key("highlight");
            json.startArray();
            json.nextItem();
            json.startObject();
            json.key("r"); json.value(i - 1);
            json.key("c"); json.value(j);
            json.key("type"); json.value("without");
            json.endObject();
            if (j >= w) {
                json.nextItem();
                json.startObject();
                json.key("r"); json.value(i - 1);
                json.key("c"); json.value(j - w);
                json.key("type"); json.value("with");
                json.endObject();
            }
            json.endArray();
            
            json.key("decision"); json.value("merge");
            json.endObject();
        }
    }
    json.endArray();
    
    json.key("path");
    json.startArray();
    json.endArray();
    
    // Output top K values
    json.key("topK");
    json.startArray();
    for (int val : dp[n][capacity]) {
        json.nextItem();
        json.value(val);
    }
    json.endArray();
    
    json.key("max_value"); json.value(dp[n][capacity].empty() ? 0 : dp[n][capacity][0]);
    json.key("kth_value"); json.value(dp[n][capacity].size() >= K ? dp[n][capacity][K-1] : 0);
    
    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();
    json.key("time_ms"); json.value(duration);
    
    json.key("complexity");
    json.startObject();
    json.key("time"); json.value("O(n*C*K)");
    json.key("space"); json.value("O(n*C*K)");
    json.key("operations"); json.value(n * (capacity + 1) * K);
    json.key("memory_bytes"); json.value((n + 1) * (capacity + 1) * K * 4);
    json.endObject();
    
    json.endObject();
    cout << json.str();
    return 0;
}




