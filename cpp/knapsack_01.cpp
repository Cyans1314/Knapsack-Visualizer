/**
 * 0/1 Knapsack Problem Algorithm
 * 
 * @author Cyans
 * @affiliation Chang'an University
 * @description Dynamic programming solution for 0/1 knapsack problem where each item can be selected at most once
 * 
 * Input: Command line arguments in JSON format
 * Output: Standard output in JSON format (including animation steps)
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>

using namespace std;

// Simple JSON parsing
struct Item {
    int weight;
    int value;
};

// JSON output builder (manual concatenation to avoid third-party libraries)
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
    // Start timing
    auto startTime = chrono::high_resolution_clock::now();
    
    // Parse input arguments
    // Format: knapsack_01.exe <capacity> <n> <w1,v1> <w2,v2> ...
    if (argc < 3) {
        cout << R"({"code":400,"error":"Insufficient parameters"})";
        return 1;
    }
    
    int capacity = stoi(argv[1]);
    int n = stoi(argv[2]);
    
    vector<Item> items(n);
    for (int i = 0; i < n && i + 3 < argc; i++) {
        string arg = argv[i + 3];
        size_t comma = arg.find(',');
        items[i].weight = stoi(arg.substr(0, comma));
        items[i].value = stoi(arg.substr(comma + 1));
    }
    
    // DP array
    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));
    
    // Start building JSON
    JsonBuilder json;
    json.startObject();
    
    json.key("code"); json.value(200);
    json.key("type"); json.value("0/1 Knapsack");
    json.key("capacity"); json.value(capacity);
    
    // Output items list
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
    
    // Animation steps
    json.key("steps");
    json.startArray();
    
    // DP calculation process
    for (int i = 1; i <= n; i++) {
        int w = items[i - 1].weight;
        int v = items[i - 1].value;
        
        for (int j = 0; j <= capacity; j++) {
            json.nextItem();
            json.startObject();
            json.key("row"); json.value(i);
            json.key("col"); json.value(j);
            
            if (j < w) {
                // Cannot fit, inherit from previous row
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
            } else {
                // Can choose to take or skip
                int withoutItem = dp[i - 1][j];
                int withItem = dp[i - 1][j - w] + v;
                dp[i][j] = max(withoutItem, withItem);
                
                json.key("val"); json.value(dp[i][j]);
                json.key("highlight");
                json.startArray();
                // Skip
                json.nextItem();
                json.startObject();
                json.key("r"); json.value(i - 1);
                json.key("c"); json.value(j);
                json.key("type"); json.value("without");
                json.endObject();
                // Take
                json.nextItem();
                json.startObject();
                json.key("r"); json.value(i - 1);
                json.key("c"); json.value(j - w);
                json.key("type"); json.value("with");
                json.endObject();
                json.endArray();
            }
            
            json.key("decision"); 
            json.value(dp[i][j] == dp[i-1][j] ? "skip" : "take");
            
            json.endObject();
        }
    }
    json.endArray();
    
    // Backtrack path
    json.key("path");
    json.startArray();
    int j = capacity;
    for (int i = n; i > 0 && j > 0; i--) {
        if (dp[i][j] != dp[i - 1][j]) {
            json.nextItem();
            json.startObject();
            json.key("r"); json.value(i);
            json.key("c"); json.value(j);
            json.key("item"); json.value(i - 1);
            json.endObject();
            j -= items[i - 1].weight;
        }
    }
    json.endArray();
    
    // Final result
    json.key("max_value"); json.value(dp[n][capacity]);
    
    // End timing
    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();
    json.key("time_ms"); json.value(duration);
    
    // Complexity information
    json.key("complexity");
    json.startObject();
    json.key("time"); json.value("O(n*C)");
    json.key("space"); json.value("O(n*C)");
    json.key("operations"); json.value(n * (capacity + 1));
    json.key("memory_bytes"); json.value((n + 1) * (capacity + 1) * 4);
    json.endObject();
    
    json.endObject();
    
    cout << json.str();
    
    return 0;
}



