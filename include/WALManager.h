#ifndef WALMANAGER_H
#define WALMANAGER_H
 
#include <string>
#include <vector>

enum class WALOp { INSERT, REMOVE, CHECKPOINT };
 
struct WALEntry {
    WALOp op;
    std::string key;
    std::string value;  
};

class WALManager {
private:
    std::string walPath;
 
public:
    WALManager(const std::string& dir = "data");
 
    void append(WALOp op, const std::string& key, const std::string& value = "");

    void writeCheckpoint();
    std::vector<WALEntry> readPending();
 
    std::vector<WALEntry> readAll();
 
    void clear();
 
    bool hasPendingEntries();
};

#endif