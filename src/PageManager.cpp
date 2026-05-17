#include "PageManager.h"
#include "QueryEngine.h"
#include <iostream>

PageManager::PageManager() : nextPageId(0), disk("data"), buffer(disk), wal("data"), qe(nullptr) {
    auto pages = disk.readAllPages();

    if (wal.hasPendingEntries()) recoverFromWAL(pages);

    if (pages.empty()) {
        std::cout << "[PAGE MANAGER] No existing data found. Starting fresh.\n";
        allocatePage();
    } else {
        for (auto& p : pages) {
            pageIds.push_back(p.getPageId());
            if (p.getPageId() >= nextPageId)
                nextPageId = p.getPageId() + 1;
        }
        std::cout << "[PAGE MANAGER] Restored " << pageIds.size() << " page(s).\n";
    }
}

void PageManager::recoverFromWAL(std::vector<Page>& pages) {
    auto entries = wal.readPending();
    std::cout << "[WAL] Found " << entries.size()
              << " unfinished operation(s) since last checkpoint. Redoing...\n";
 
    for (auto& entry : entries) {
        if (entry.op == WALOp::INSERT) {
            bool found = false;
            for (auto& page : pages) {
                if (page.search(entry.key)) {
                    page.remove(entry.key);
                    page.insert(entry.key, entry.value);
                    found = true;
                    break;
                }
            }
            if (!found) {
                for (auto& page : pages) {
                    if (!page.isFull()) {
                        page.insert(entry.key, entry.value);
                        found = true;
                        break;
                    }
                }
            }
            std::cout << "[WAL REDO] INSERT key=\"" << entry.key << "\"\n";
        }
        else if (entry.op == WALOp::REMOVE) {
            for (auto& page : pages) {
                if (page.search(entry.key)) {
                    page.remove(entry.key);
                    break;
                }
            }
            std::cout << "[WAL REDO] REMOVE key=\"" << entry.key << "\"\n";
        }
    }

    for (auto& page : pages)
        disk.writePage(page);
    wal.writeCheckpoint();
    std::cout << "[WAL] Redo complete. State restored from crash.\n";
}

Page& PageManager::allocatePage() {
    int id = nextPageId++;
    pageIds.push_back(id);
    Page newPage(id);
    disk.writePage(newPage);
    return buffer.getPage(id);
}

void PageManager::insert(const std::string& key, const std::string& value) {
    wal.append(WALOp::INSERT, key, value);

    int lastId = pageIds.back();
    if (buffer.getPage(lastId).isFull()) {
        std::cout << "[PAGE MANAGER] Page " << lastId << " full. Allocating new page.\n";
        allocatePage();
        lastId = pageIds.back();
    }
    Page& page = buffer.getPage(lastId);
    page.insert(key, value);
    buffer.markDirty(lastId);

    if (qe) {
        int slotIndex = (int)page.getRecordCount() - 1;
        qe->IndexInsert(key, lastId, slotIndex);
    }
}

std::optional<std::string> PageManager::search(const std::string& key) {
    for (int id : pageIds) {
        auto result = buffer.getPage(id).search(key);
        if (result) return result;
    }
    return std::nullopt;
}

bool PageManager::remove(const std::string& key) {
    wal.append(WALOp::REMOVE, key);

    for (int id : pageIds) {
        Page& page = buffer.getPage(id);
        if (page.remove(key)) {
            buffer.markDirty(id);
            if (qe) qe->IndexRemove(key);
            return true;
        }
    }
    std::cout << "[PAGE MANAGER] Key \"" << key << "\" not found.\n";
    return false;
}

void PageManager::display() {
    std::cout << "\n[PAGE MANAGER] " << pageIds.size() << " page(s)\n";
    std::cout << std::string(42, '=') << "\n";
    for (int id : pageIds) {
        buffer.getPage(id).display();
        std::cout << "\n";
    }
}

Page* PageManager::getPage(int pageId) {
    for (int id : pageIds)
        if (id == pageId) return &buffer.getPage(pageId);
    return nullptr;
}

size_t PageManager::recordCount() {
    size_t total = 0;
    for (int id : pageIds)
        total += buffer.getPage(id).getRecordCount();
    return total;
}

void PageManager::flushAll() {
    buffer.flushAll();
    wal.writeCheckpoint();
}