/**
 * Multiple Knapsack Problem Algorithm
 * 
 * @author Cyans
 * @affiliation Chang'an University
 * @description Dynamic programming solution for multiple knapsack problem using binary decomposition optimization
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
    int count;
};

struct SplitItem {
    int weight;
    int value;
    int originalIndex;  // Original item index
    int splitCount;     // Count represented after split
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
        items[i].count = stoi(arg.substr(comma2 + 1));
    }
    
    // Binary decomposition
    vector<SplitItem> splitItems;
    for (int i = 0; i < n; i++) {
        int cnt = items[i].count;
        for (int k = 1; k <= cnt; k *= 2) {
            splitItems.push_back({
                items[i].weight * k,
                items[i].value * k,
                i,
                k
            });
            cnt -= k;
        }
        if (cnt > 0) {
            splitItems.push_back({
                items[i].weight * cnt,
                items[i].value * cnt,
                i,
                cnt
            });
        }
    }
    
    int m = splitItems.size();
    vector<vector<int>> dp(m + 1, vector<int>(capacity + 1, 0));
    
    JsonBuilder json;
    json.startObject();
    json.key("code"); json.value(200);
    json.key("type"); json.value("Multiple Knapsack");
    json.key("capacity"); json.value(capacity);
    
    // Original items
    json.key("items");
    json.startArray();
    for (int i = 0; i < n; i++) {
        json.nextItem();
        json.startObject();
        json.key("w"); json.value(items[i].weight);
        json.key("v"); json.value(items[i].value);
        json.key("c"); json.value(items[i].count);
        json.endObject();
    }
    json.endArray();
    
    // Split items
    json.key("splitItems");
    json.startArray();
    for (int i = 0; i < m; i++) {
        json.nextItem();
        json.startObject();
        json.key("w"); json.value(splitItems[i].weight);
        json.key("v"); json.value(splitItems[i].value);
        json.key("orig"); json.value(splitItems[i].originalIndex);
        json.key("cnt"); json.value(splitItems[i].splitCount);
        json.endObject();
    }
    json.endArray();
    
    json.key("steps");
    json.startArray();
    
    // Convert to 0/1 knapsack for solving
    for (int i = 1; i <= m; i++) {
        int w = splitItems[i - 1].weight;
        int v = splitItems[i - 1].value;
        
        for (int j = 0; j <= capacity; j++) {
            json.nextItem();
            json.startObject();
            json.key("row"); json.value(i);
            json.key("col"); json.value(j);
            json.key("origItem"); json.value(splitItems[i - 1].originalIndex);
            
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
                int withoutItem = dp[i - 1][j];
                int withItem = dp[i - 1][j - w] + v;
                dp[i][j] = max(withoutItem, withItem);
                
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
                json.value(dp[i][j] == withoutItem ? "skip" : "take");
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
            json.key("item"); json.value(splitItems[i - 1].originalIndex);
            json.key("splitCnt"); json.value(splitItems[i - 1].splitCount);
            json.endObject();
            j -= splitItems[i - 1].weight;
        }
    }
    json.endArray();
    
    json.key("max_value"); json.value(dp[m][capacity]);
    
    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();
    json.key("time_ms"); json.value(duration);
    
    json.key("complexity");
    json.startObject();
    json.key("time"); json.value("O(C*Σlog(k))");
    json.key("space"); json.value("O(m*C)");
    json.key("operations"); json.value(m * (capacity + 1));
    json.key("memory_bytes"); json.value((m + 1) * (capacity + 1) * 4);
    json.endObject();
    
    json.endObject();
    cout << json.str();
    return 0;
}




