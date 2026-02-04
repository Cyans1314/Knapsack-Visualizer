/**
 * 2D Knapsack Problem Algorithm
 * 
 * @author Cyans
 * @affiliation Chang'an University
 * @description Dynamic programming solution for 2D knapsack problem with weight and volume constraints
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
    int volume;
    int value;
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
    
    if (argc < 4) {
        cout << R"({"code":400,"error":"Insufficient parameters"})";
        return 1;
    }
    
    int capacity = stoi(argv[1]);   // Weight limit
    int capacity2 = stoi(argv[2]);  // Volume limit
    int n = stoi(argv[3]);
    
    vector<Item> items(n);
    for (int i = 0; i < n && i + 4 < argc; i++) {
        string arg = argv[i + 4];
        size_t comma1 = arg.find(',');
        size_t comma2 = arg.find(',', comma1 + 1);
        items[i].weight = stoi(arg.substr(0, comma1));
        items[i].volume = stoi(arg.substr(comma1 + 1, comma2 - comma1 - 1));
        items[i].value = stoi(arg.substr(comma2 + 1));
    }
    
    // 3D DP: dp[i][j][k] = max value with first i items, weight <= j, volume <= k
    // For visualization, we show 2D slices at fixed volume
    vector<vector<vector<int>>> dp(n + 1, 
        vector<vector<int>>(capacity + 1, 
            vector<int>(capacity2 + 1, 0)));
    
    JsonBuilder json;
    json.startObject();
    json.key("code"); json.value(200);
    json.key("type"); json.value("2D Cost");
    json.key("capacity"); json.value(capacity);
    json.key("capacity2"); json.value(capacity2);
    
    json.key("items");
    json.startArray();
    for (int i = 0; i < n; i++) {
        json.nextItem();
        json.startObject();
        json.key("w"); json.value(items[i].weight);
        json.key("m"); json.value(items[i].volume);
        json.key("v"); json.value(items[i].value);
        json.endObject();
    }
    json.endArray();
    
    json.key("steps");
    json.startArray();
    
    // DP calculation (record steps for all volume dimensions for visualization)
    for (int i = 1; i <= n; i++) {
        int w = items[i - 1].weight;
        int m = items[i - 1].volume;
        int v = items[i - 1].value;
        
        for (int j = 0; j <= capacity; j++) {
            for (int k = 0; k <= capacity2; k++) {
                int oldVal = dp[i - 1][j][k];
                int newVal = oldVal;
                bool took = false;
                
                if (j >= w && k >= m) {
                    int withItem = dp[i - 1][j - w][k - m] + v;
                    if (withItem > newVal) {
                        newVal = withItem;
                        took = true;
                    }
                }
                dp[i][j][k] = newVal;
                
                // Record steps for each volume slice
                json.nextItem();
                json.startObject();
                json.key("row"); json.value(i);
                json.key("col"); json.value(j);
                json.key("vol"); json.value(k);
                json.key("val"); json.value(dp[i][j][k]);
                
                json.key("highlight");
                json.startArray();
                json.nextItem();
                json.startObject();
                json.key("r"); json.value(i - 1);
                json.key("c"); json.value(j);
                json.key("v"); json.value(k);
                json.key("type"); json.value("without");
                json.endObject();
                if (took) {
                    json.nextItem();
                    json.startObject();
                    json.key("r"); json.value(i - 1);
                    json.key("c"); json.value(j - w);
                    json.key("v"); json.value(k - m);
                    json.key("type"); json.value("with");
                    json.endObject();
                }
                json.endArray();
                
                json.key("decision"); json.value(took ? "take" : "skip");
                json.endObject();
            }
        }
    }
    json.endArray();
    
    // Backtrack
    json.key("path");
    json.startArray();
    int j = capacity, k = capacity2;
    for (int i = n; i > 0 && j > 0 && k > 0; i--) {
        if (dp[i][j][k] != dp[i - 1][j][k]) {
            json.nextItem();
            json.startObject();
            json.key("r"); json.value(i);
            json.key("c"); json.value(j);
            json.key("item"); json.value(i - 1);
            json.endObject();
            j -= items[i - 1].weight;
            k -= items[i - 1].volume;
        }
    }
    json.endArray();
    
    json.key("max_value"); json.value(dp[n][capacity][capacity2]);
    
    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();
    json.key("time_ms"); json.value(duration);
    
    json.key("complexity");
    json.startObject();
    json.key("time"); json.value("O(n*C*M)");
    json.key("space"); json.value("O(n*C*M)");
    json.key("operations"); json.value(n * (capacity + 1) * (capacity2 + 1));
    json.key("memory_bytes"); json.value((n + 1) * (capacity + 1) * (capacity2 + 1) * 4);
    json.endObject();
    
    json.endObject();
    cout << json.str();
    return 0;
}




