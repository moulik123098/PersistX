#include "WALManager.h"

#include<iostream>
#include<fstream>
#include<filesystem>

namespace fs = std::filesystem;
 
WALManager::WALManager(const std::string& dir) {
    if (!fs::exists(dir))
        fs::create_directory(dir);
    walPath = dir + "/wal.log";
}

void WALManager::append(WALOp op, const std::string& key, const std::string& value) {
    
    std::ofstream file(walPath, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "[WAL ERROR] Cannot open wal.log.\n";
        return;
    }
 
    if (op == WALOp::INSERT)
        file << "INSERT|" << key << "|" << value << "\n";
    else
        file << "REMOVE|" << key << "\n";
 
    file.flush();
 
    std::cout << "[WAL] Logged "
              << (op == WALOp::INSERT ? "INSERT" : "REMOVE")
              << " key=\"" << key << "\"\n";
}

void WALManager::writeCheckpoint() {
    std::ofstream file(walPath, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "[WAL ERROR] Cannot open wal.log for checkpoint.\n";
        return;
    }
    file << "CHECKPOINT\n";
    file.flush();
    std::cout << "[WAL] Checkpoint written — all ops before this point are safely on disk.\n";
}

std::vector<WALEntry> WALManager::readPending() {
    std::vector<WALEntry> window;
    if (!fs::exists(walPath)) return window;
 
    std::ifstream file(walPath);
    if (!file.is_open()) return window;
 
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
 
        if (line == "CHECKPOINT") {
            
            window.clear();
            continue;
        }
        if (line.substr(0, 7) == "INSERT|") {
            std::string rest = line.substr(7);
            size_t delim = rest.find('|');
            if (delim == std::string::npos) continue;
            window.push_back({WALOp::INSERT,
                              rest.substr(0, delim),
                              rest.substr(delim + 1)});
        } else if (line.substr(0, 7) == "REMOVE|") {
            window.push_back({WALOp::REMOVE, line.substr(7), ""});
        }
    }
    return window;
}

std::vector<WALEntry> WALManager::readAll() {
    std::vector<WALEntry> entries;
    if (!fs::exists(walPath)) return entries;
 
    std::ifstream file(walPath);
    if (!file.is_open()) return entries;
 
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line == "CHECKPOINT") continue;
 
        if (line.substr(0, 7) == "INSERT|") {
            std::string rest = line.substr(7);
            size_t delim = rest.find('|');
            if (delim == std::string::npos) continue;
            entries.push_back({WALOp::INSERT,
                               rest.substr(0, delim),
                               rest.substr(delim + 1)});
        }
        else if (line.substr(0, 7) == "REMOVE|") {
            entries.push_back({WALOp::REMOVE, line.substr(7), ""});
        }
    }
    return entries;
}
 
void WALManager::clear() {
    std::ofstream file(walPath, std::ios::trunc);
    std::cout << "[WAL] Log cleared after successful flush.\n";
}
 
bool WALManager::hasPendingEntries() {
    return !readPending().empty();
}
 