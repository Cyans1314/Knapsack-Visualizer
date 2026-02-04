/**
 * Mixed Knapsack Problem Algorithm
 * 
 * @author Cyans
 * @affiliation Chang'an University
 * @description Dynamic programming solution for mixed knapsack problem with combined item types (0/1, complete, multiple)
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
    int type;  // 0: 0/1 knapsack, 1: complete knapsack, 2: multiple knapsack (default count 3)
    int count; // Count for multiple knapsack
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
    for (int i = 0; i < n && i + 3 < argc; i++) {
        string arg = argv[i + 3];
        size_t comma1 = arg.find(',');
        size_t comma2 = arg.find(',', comma1 + 1);
        items[i].weight = stoi(arg.substr(0, comma1));
        items[i].value = stoi(arg.substr(comma1 + 1, comma2 - comma1 - 1));
        items[i].type = stoi(arg.substr(comma2 + 1));
        items[i].count = (items[i].type == 2) ? 3 : 1;  // Multiple knapsack default 3
    }
    
    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));
    
    JsonBuilder json;
    json.startObject();
    json.key("code"); json.value(200);
    json.key("type"); json.value("Mixed Knapsack");
    json.key("capacity"); json.value(capacity);
    
    json.key("items");
    json.startArray();
    for (int i = 0; i < n; i++) {
        json.nextItem();
        json.startObject();
        json.key("w"); json.value(items[i].weight);
        json.key("v"); json.value(items[i].value);
        json.key("t"); json.value(items[i].type);
        json.endObject();
    }
    json.endArray();
    
    json.key("steps");
    json.startArray();
    
    for (int i = 1; i <= n; i++) {
        int w = items[i - 1].weight;
        int v = items[i - 1].value;
        int type = items[i - 1].type;
        
        // First copy previous row
        for (int j = 0; j <= capacity; j++) {
            dp[i][j] = dp[i - 1][j];
        }
        
        for (int j = 0; j <= capacity; j++) {
            json.nextItem();
            json.startObject();
            json.key("row"); json.value(i);
            json.key("col"); json.value(j);
            json.key("itemType"); json.value(type);
            
            string typeStr = (type == 0) ? "0/1" : (type == 1) ? "Complete" : "Multiple";
            json.key("typeStr"); json.value(typeStr);
            
            int oldVal = dp[i - 1][j];
            int newVal = oldVal;
            int fromRow = i - 1, fromCol = j;
            bool took = false;
            
            if (type == 0) {
                // 0/1 knapsack
                if (j >= w && dp[i - 1][j - w] + v > newVal) {
                    newVal = dp[i - 1][j - w] + v;
                    fromRow = i - 1;
                    fromCol = j - w;
                    took = true;
                }
            } else if (type == 1) {
                // Complete knapsack
                if (j >= w && dp[i][j - w] + v > newVal) {
                    newVal = dp[i][j - w] + v;
                    fromRow = i;
                    fromCol = j - w;
                    took = true;
                }
            } else {
                // Multiple knapsack (simplified, max 3)
                for (int k = 1; k <= items[i-1].count && k * w <= j; k++) {
                    if (dp[i - 1][j - k * w] + k * v > newVal) {
                        newVal = dp[i - 1][j - k * w] + k * v;
                        fromRow = i - 1;
                        fromCol = j - k * w;
                        took = true;
                    }
                }
            }
            
            dp[i][j] = newVal;
            
            json.key("val"); json.value(dp[i][j]);
            json.key("highlight");
            json.startArray();
            json.nextItem();
            json.startObject();
            json.key("r"); json.value(i - 1);
            json.key("c"); json.value(j);
            json.key("type"); json.value("without");
            json.endObject();
            if (took) {
                json.nextItem();
                json.startObject();
                json.key("r"); json.value(fromRow);
                json.key("c"); json.value(fromCol);
                json.key("type"); json.value("with");
                json.endObject();
            }
            json.endArray();
            
            json.key("decision"); json.value(took ? "take" : "skip");
            json.endObject();
        }
    }
    json.endArray();
    
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
            
            int w = items[i - 1].weight;
            int type = items[i - 1].type;
            if (type == 1) {
                // Complete knapsack may select multiple times
                while (j >= w && dp[i][j] == dp[i][j - w] + items[i-1].value) {
                    j -= w;
                }
            } else {
                j -= w;
            }
        }
    }
    json.endArray();
    
    json.key("max_value"); json.value(dp[n][capacity]);
    
    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();
    json.key("time_ms"); json.value(duration);
    
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




