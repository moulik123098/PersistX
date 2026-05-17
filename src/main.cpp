#include <iostream>
#include <string>
#include<limits>
#include<cstdlib>

#include "PageManager.h"
#include "QueryEngine.h"


void Menu()
{

    std::cout << "  +===========================================+\n";
    std::cout << "  |         P A G E  D B  E N G I N E         |\n";
    std::cout << "  +===========================================+\n";
    std::cout << "  |                                           |\n";
    std::cout << "  |   RECORDS                                 |\n";
    std::cout << "  |    1.  Insert record                      |\n";
    std::cout << "  |    2.  Remove record                      |\n";
    std::cout << "  |    3.  Get by key                         |\n";
    std::cout << "  |    4.  List all records                   |\n";
    std::cout << "  |                                           |\n";
    std::cout << "  |   QUERIES                                 |\n";
    std::cout << "  |    5.  Filter by prefix                   |\n";
    std::cout << "  |    6.  Range query                        |\n";
    std::cout << "  |                                           |\n";
    std::cout << "  |   INDEX                                   |\n";
    std::cout << "  |    7.  Show index                         |\n";
    std::cout << "  |    8.  Rebuild index                      |\n";
    std::cout << "  |                                           |\n";
    std::cout << "  |   STORAGE                                 |\n";
    std::cout << "  |    9.  Display all pages                  |\n";
    std::cout << "  |   10.  Buffer pool stats                  |\n";
    std::cout << "  |   11.  Flush to disk                      |\n";
    std::cout << "  |                                           |\n";
    std::cout << "  |   12.  Crash  (simulate abrupt exit)      |\n";
    std::cout << "  |   13.  Exit                               |\n";
    std::cout << "  |                                           |\n";
    std::cout << "  +===========================================+\n";
    std::cout << "\n  Choice: ";
}

int main()
{

    PageManager pm;


    // for(int i = 0 ; i < 150 ; i++){
    //     pm.insert(std::to_string(i+1),std::to_string(i+2));
    // } test

    QueryEngine qe(pm.getBuffer(), pm.getPageIds());

    pm.setQueryEngine(&qe);  

    

    short choice;

    while (true)
    {

        Menu();
        std::cin >> choice;

        if (std::cin.fail())
        {
            std::cout << "Invalid input. Enter a number.\n";
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }

        std::string key, value,prefix;

        switch (choice)
        {

        case 1:
            std::cout << "Enter key: ";
            std::cin >> key;
            std::cout << "Enter value: ";
            std::cin >> value;
            pm.insert(key, value);
            break;

        case 2:
            std::cout << "Enter key: ";
            std::cin >> key;
            pm.remove(key);
            break;

        case 3:
            std::cout << "Enter key: ";
            std::cin >> key;
            {
                auto result = qe.get(key);
                if(result) std::cout << "Found: " << *result << "\n";
                    else std::cout << "Not found.\n";
                }
                break;
            

        case 4:
            qe.displayAll();
            break;

        case 5:
        {
            std::cout << "Enter prefix: ";
            std::cin >> prefix;
            qe.displayPrefix(prefix);
            break;
        }

         case 6:
        {
            std::string start,end;
            std::cout << "Enter start key: "; 
            std::cin >> start;
            std::cout << "Enter end key: ";   
            std::cin >> end;
            qe.displayRange(start, end);
            break;
        }    
 
        case 7:
            qe.displayIndex();
            break;
 
         case 8:
            qe.rebuild();
            break;

        case 9:
            pm.display();
            break;

        case 10:
            pm.displayBufferStats();
            break;

        case 11:
            pm.flushAll();
            break;

        case 12:
            std::cerr << "\n[CRASH] Simulating abrupt engine failure!\n"
                      << "[CRASH] Buffer NOT flushed. WAL NOT checkpointed.\n"
                      << "[CRASH] On next startup, WAL redo will replay all\n"
                      << "[CRASH] operations logged since the last checkpoint.\n"
                      << "\nFatal error: unhandled exception — process terminated.\n"
                      << "Exit code: 1\n";
            std::exit(1); 

        case 13:
            pm.flushAll();  
            std::cout << "Exiting...\n";
            return 0;

        default:
            std::cout << "Invalid choice. Enter 1-12.\n";
        }
    }
}