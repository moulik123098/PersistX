// ============================================================
// [DEBUG TUI] All debug/display additions are marked [DEBUG]
// Zero changes to original logic. Every new line is commented.
// Windows VS Code terminal compatible — no windows.h needed
// ============================================================

#include <iostream>
#include <string>
#include <limits>
#include <cstdlib>
// [DEBUG] Added for timestamp, formatting, and log buffer
#include <vector>
#include <chrono>
#include <iomanip>
#include <ctime>

#include "PageManager.h"
#include "QueryEngine.h"

// ============================================================
// [DEBUG] ANSI color codes
// ============================================================
#define RST         "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"

#define FG_BRED     "\033[91m"
#define FG_BGREEN   "\033[92m"
#define FG_BYELLOW  "\033[93m"
#define FG_BBLUE    "\033[94m"
#define FG_BMAGENTA "\033[95m"
#define FG_BCYAN    "\033[96m"
#define FG_BWHITE   "\033[97m"
#define FG_CYAN     "\033[36m"
#define FG_BLUE     "\033[34m"

// ============================================================
// [DEBUG] Debug log ring buffer — stores last N action messages
// ============================================================
static std::vector<std::string> debugLog;  // [DEBUG] ring buffer for log entries
static const int DEBUG_LOG_MAX = 6;        // [DEBUG] max lines shown in log panel

// [DEBUG] Add a timestamped entry to the debug log
void dbgLog(const std::string& tag, const std::string& msg, const std::string& color = FG_BWHITE) {
    auto now = std::chrono::system_clock::now();                     // [DEBUG] current time
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_info = std::localtime(&t);
    char timeBuf[9];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", tm_info);   // [DEBUG] HH:MM:SS format

    std::string line = color + "[" + timeBuf + "] "
                     + BOLD + tag + RST + color + "  " + msg + RST; // [DEBUG] build log string

    debugLog.push_back(line);                                        // [DEBUG] push to buffer
    if ((int)debugLog.size() > DEBUG_LOG_MAX)
        debugLog.erase(debugLog.begin());                            // [DEBUG] drop oldest
}

// ============================================================
// [DEBUG] Draw the debug log panel below the menu
// ============================================================
void dbgDrawLogPanel() {
    std::cout << "\n";
    std::cout << FG_BLUE << BOLD
              << "  +------ DEBUG LOG (last " << DEBUG_LOG_MAX << " events) ------+\n"
              << RST; // [DEBUG] panel header

    if (debugLog.empty()) {
        std::cout << DIM
                  << "  | (no events yet - perform an action)         |\n"
                  << RST; // [DEBUG] placeholder when empty
    } else {
        for (const auto& line : debugLog)
            std::cout << FG_BLUE << BOLD << "  | " << RST << line << "\n"; // [DEBUG] each log line
    }

    std::cout << FG_BLUE << BOLD
              << "  +----------------------------------------------+\n"
              << RST; // [DEBUG] panel footer
}

// ============================================================
// [DEBUG] Menu section label
// ============================================================
void dbgMenuLabel(const std::string& label) {
    std::cout << FG_CYAN   << "  |" << RST
              << FG_BYELLOW << BOLD << "  -- " << label << " --" << RST << "\n"; // [DEBUG] section header
}

// [DEBUG] Single menu option row
void dbgMenuOption(int num, const std::string& desc, bool danger = false) {
    std::cout << FG_CYAN << "  |  " << RST
              << FG_BCYAN << BOLD << std::setw(2) << num << ". " << RST; // [DEBUG] number badge
    if (danger)
        std::cout << FG_BRED  << BOLD << desc << RST << "\n"; // [DEBUG] red for danger
    else
        std::cout << FG_BWHITE << desc << RST << "\n";         // [DEBUG] white for normal
}

// ============================================================
// [DEBUG] Full TUI Menu — replaces the original plain Menu()
// ============================================================
void Menu() {
    system("cls"); // [DEBUG] clear screen on Windows

    std::cout << "\n";
    // [DEBUG] Title box
    std::cout << FG_CYAN << BOLD << "  +==========================================+\n";
    std::cout << "  |" << RST
              << FG_BCYAN   << BOLD << "       PAGE DB ENGINE" << RST
              << FG_BWHITE  << "  -  " << RST
              << FG_BYELLOW << BOLD << "DEBUG MENU" << RST
              << FG_CYAN    << BOLD << "         |\n";
    std::cout << "  +==========================================+\n" << RST;

    // [DEBUG] RECORDS
    std::cout << FG_CYAN << "  |\n" << RST;
    dbgMenuLabel("RECORDS");
    dbgMenuOption(1,  "Insert record");
    dbgMenuOption(2,  "Remove record");
    dbgMenuOption(3,  "Get by key");
    dbgMenuOption(4,  "List all records");

    // [DEBUG] QUERIES
    std::cout << FG_CYAN << "  |\n" << RST;
    dbgMenuLabel("QUERIES");
    dbgMenuOption(5,  "Filter by prefix");
    dbgMenuOption(6,  "Range query");

    // [DEBUG] INDEX
    std::cout << FG_CYAN << "  |\n" << RST;
    dbgMenuLabel("INDEX");
    dbgMenuOption(7,  "Show index");
    dbgMenuOption(8,  "Rebuild index");

    // [DEBUG] STORAGE
    std::cout << FG_CYAN << "  |\n" << RST;
    dbgMenuLabel("STORAGE");
    dbgMenuOption(9,  "Display all pages");
    dbgMenuOption(10, "Buffer pool stats");
    dbgMenuOption(11, "Flush to disk");

    // [DEBUG] DANGER ZONE
    std::cout << FG_CYAN << "  |\n" << RST;
    dbgMenuLabel("DANGER ZONE");
    dbgMenuOption(12, "Crash  (simulate abrupt exit)", true);
    dbgMenuOption(13, "Exit   (safe flush + quit)");

    // [DEBUG] Bottom border
    std::cout << FG_CYAN << "  |\n"
              << "  +==========================================+\n" << RST;

    dbgDrawLogPanel(); // [DEBUG] log panel below menu

    // [DEBUG] Input prompt
    std::cout << "\n  " << FG_BGREEN << BOLD << "> " << RST
              << FG_BWHITE << "Enter choice (1-13): " << RST;
}

// ============================================================
// [DEBUG] Colored action banner shown after picking an option
// ============================================================
void dbgActionBanner(const std::string& title, const std::string& color = FG_BCYAN) {
    std::cout << "\n" << color << BOLD
              << "  +-------------------------------------+\n"
              << "  |  " << std::left << std::setw(35) << title << "|\n"
              << "  +-------------------------------------+\n"
              << RST;
}

// ============================================================
// [DEBUG] Wait for Enter before returning to menu
// ============================================================
void dbgPause() {
    std::cout << "\n  " << DIM << "[ Press ENTER to return to menu ]" << RST;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // [DEBUG] flush newline
    std::cin.get();                                                      // [DEBUG] wait for Enter
}

// ============================================================
// main() — logic 100% identical to original
// ============================================================
int main()
{
    system("color 0"); // [DEBUG] enables ANSI color support on Windows — no windows.h needed

    PageManager pm; // original — untouched
    dbgLog("INIT", "PageManager initialized", FG_BGREEN); // [DEBUG] log

    QueryEngine qe(pm.getBuffer(), pm.getPageIds()); // original — untouched
    dbgLog("INIT", "QueryEngine initialized", FG_BGREEN); // [DEBUG] log

    pm.setQueryEngine(&qe); // original — untouched

    short choice; // original — untouched

    while (true) // original — untouched
    {
        Menu(); // [DEBUG] TUI version
        std::cin >> choice;

        if (std::cin.fail()) // original — untouched
        {
            dbgLog("ERROR", "Invalid input - not a number", FG_BRED); // [DEBUG] log
            std::cout << FG_BRED << "  Invalid input. Enter a number.\n" << RST;
            std::cin.clear();            // original — untouched
            std::cin.ignore(1000, '\n'); // original — untouched
            continue;                    // original — untouched
        }

        std::string key, value, prefix; // original — untouched

        switch (choice) // original — untouched
        {

        case 1: // original — untouched
            dbgActionBanner("INSERT RECORD", FG_BGREEN); // [DEBUG] banner
            std::cout << "  " << FG_BYELLOW << "Key   : " << RST; std::cin >> key;   // [DEBUG] styled prompt
            std::cout << "  " << FG_BYELLOW << "Value : " << RST; std::cin >> value; // [DEBUG] styled prompt
            pm.insert(key, value); // original — untouched
            dbgLog("INSERT", "key=\"" + key + "\" val=\"" + value + "\"", FG_BGREEN); // [DEBUG] log
            dbgPause(); // [DEBUG] wait
            break;

        case 2: // original — untouched
            dbgActionBanner("REMOVE RECORD", FG_BRED); // [DEBUG] banner
            std::cout << "  " << FG_BYELLOW << "Key : " << RST; std::cin >> key; // [DEBUG] styled prompt
            pm.remove(key); // original — untouched
            dbgLog("REMOVE", "key=\"" + key + "\"", FG_BRED); // [DEBUG] log
            dbgPause(); // [DEBUG] wait
            break;

        case 3: // original — untouched
            dbgActionBanner("GET BY KEY", FG_BCYAN); // [DEBUG] banner
            std::cout << "  " << FG_BYELLOW << "Key : " << RST; std::cin >> key; // [DEBUG] styled prompt
            {
                auto result = qe.get(key); // original — untouched
                if (result) {
                    std::cout << "  " << FG_BGREEN << BOLD << "Found: " << RST << *result << "\n";
                    dbgLog("GET", "key=\"" + key + "\" -> FOUND", FG_BGREEN); // [DEBUG] log
                } else {
                    std::cout << "  " << FG_BRED << "Not found.\n" << RST;
                    dbgLog("GET", "key=\"" + key + "\" -> NOT FOUND", FG_BRED); // [DEBUG] log
                }
            }
            dbgPause(); // [DEBUG] wait
            break;

        case 4: // original — untouched
            dbgActionBanner("LIST ALL RECORDS", FG_BCYAN); // [DEBUG] banner
            qe.displayAll(); // original — untouched
            dbgLog("QUERY", "displayAll()", FG_BCYAN); // [DEBUG] log
            dbgPause(); // [DEBUG] wait
            break;

        case 5: // original — untouched
            dbgActionBanner("FILTER BY PREFIX", FG_BCYAN); // [DEBUG] banner
            {
                std::cout << "  " << FG_BYELLOW << "Prefix : " << RST; std::cin >> prefix;
                qe.displayPrefix(prefix); // original — untouched
                dbgLog("QUERY", "filterByPrefix(\"" + prefix + "\")", FG_BCYAN); // [DEBUG] log
            }
            dbgPause(); // [DEBUG] wait
            break;

        case 6: // original — untouched
            dbgActionBanner("RANGE QUERY", FG_BCYAN); // [DEBUG] banner
            {
                std::string start, end; // original — untouched
                std::cout << "  " << FG_BYELLOW << "Start key : " << RST; std::cin >> start;
                std::cout << "  " << FG_BYELLOW << "End key   : " << RST; std::cin >> end;
                qe.displayRange(start, end); // original — untouched
                dbgLog("QUERY", "rangeQuery(\"" + start + "\"->\"" + end + "\")", FG_BCYAN); // [DEBUG] log
            }
            dbgPause(); // [DEBUG] wait
            break;

        case 7: // original — untouched
            dbgActionBanner("SHOW INDEX", FG_BMAGENTA); // [DEBUG] banner
            qe.displayIndex(); // original — untouched
            dbgLog("INDEX", "displayIndex()", FG_BMAGENTA); // [DEBUG] log
            dbgPause(); // [DEBUG] wait
            break;

        case 8: // original — untouched
            dbgActionBanner("REBUILD INDEX", FG_BYELLOW); // [DEBUG] banner
            qe.rebuild(); // original — untouched
            dbgLog("INDEX", "rebuild() called", FG_BYELLOW); // [DEBUG] log
            dbgPause(); // [DEBUG] wait
            break;

        case 9: // original — untouched
            dbgActionBanner("DISPLAY ALL PAGES", FG_BBLUE); // [DEBUG] banner
            pm.display(); // original — untouched
            dbgLog("STORAGE", "display() all pages", FG_BBLUE); // [DEBUG] log
            dbgPause(); // [DEBUG] wait
            break;

        case 10: // original — untouched
            dbgActionBanner("BUFFER POOL STATS", FG_BBLUE); // [DEBUG] banner
            pm.displayBufferStats(); // original — untouched
            dbgLog("BUFFER", "displayStats()", FG_BBLUE); // [DEBUG] log
            dbgPause(); // [DEBUG] wait
            break;

        case 11: // original — untouched
            dbgActionBanner("FLUSH TO DISK", FG_BYELLOW); // [DEBUG] banner
            pm.flushAll(); // original — untouched
            dbgLog("DISK", "flushAll() - WAL checkpoint written", FG_BYELLOW); // [DEBUG] log
            dbgPause(); // [DEBUG] wait
            break;

        case 12: // original — untouched
            // [DEBUG] Red crash banner before original crash output
            std::cout << "\n" << FG_BRED << BOLD
                      << "  +------------------------------------------+\n"
                      << "  |   *** CRASH - SIMULATING ABRUPT EXIT ***   |\n"
                      << "  |   Buffer NOT flushed. WAL NOT checkpointed |\n"
                      << "  +------------------------------------------+\n"
                      << RST;
            dbgLog("CRASH", "Abrupt exit - WAL NOT flushed", FG_BRED); // [DEBUG] log
            // original crash output — untouched
            std::cerr << "\n[CRASH] Simulating abrupt engine failure!\n"
                      << "[CRASH] Buffer NOT flushed. WAL NOT checkpointed.\n"
                      << "[CRASH] On next startup, WAL redo will replay all\n"
                      << "[CRASH] operations logged since the last checkpoint.\n"
                      << "\nFatal error: unhandled exception — process terminated.\n"
                      << "Exit code: 1\n";
            std::exit(1); // original — untouched

        case 13: // original — untouched
            dbgActionBanner("EXITING SAFELY...", FG_BGREEN); // [DEBUG] banner
            pm.flushAll(); // original — untouched
            dbgLog("SYSTEM", "Safe exit - flushAll() complete", FG_BGREEN); // [DEBUG] log
            std::cout << "  " << FG_BGREEN << BOLD << "Exiting...\n" << RST;
            return 0; // original — untouched

        default: // original — untouched
            dbgLog("ERROR", "Invalid choice: " + std::to_string(choice), FG_BRED); // [DEBUG] log
            std::cout << "  " << FG_BRED << "Invalid choice. Enter 1-13.\n" << RST;
            dbgPause(); // [DEBUG] wait
        }
    }
}