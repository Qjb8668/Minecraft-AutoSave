#include "io.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <errhandlingapi.h>
#include <errno.h>
#include <exception>
#include <fileapi.h>
#include <ios>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <cstdio>
#include <fstream>
#include <time.h>
#include <cstdlib>
#include <direct.h>
#include "Windows.h"
#include <thread>
#include "conio.h"
#include "unistd.h"
using namespace std;

bool workingSignal = false;
bool autoSaveReady = false;
bool exitSignal = false;

void getAllDirName(string path, vector<string> &files) { 
    intptr_t handle = 0;
    struct _finddata_t fileInfo;
    const char *p = (path + string("\\*")).c_str();

    for (handle = _findfirst(p, &fileInfo); ;) {
        if (fileInfo.attrib == _A_SUBDIR) {
            if (strcmp(fileInfo.name, ".") != 0 && strcmp(fileInfo.name, "..") != 0 && strcmp(fileInfo.name, "backup")) {
                files.push_back(path + string("\\") + fileInfo.name);
            }
        }

        if (_findnext(handle, &fileInfo) == -1)
            break;
    }
    _findclose(handle);
}

void printAllDirPaths(vector<string> paths) {
    cout << "Here are all saves in this diratory:" << endl;
    for (int i = 0; i != paths.size(); ++i)
        cout << paths[i] << endl;
}

void paths2names(vector<string> paths, vector<string> &names) {
    for (int i = 0; i != paths.size(); ++ i) {
        int posOfDelimiter = paths[i].find_last_of('\\');
        names.push_back(paths[i].substr(posOfDelimiter + 1, paths[i].length() - posOfDelimiter));
    }
}

void chooseSaves(vector<string> names, string &name) {
    if (names.size() == 1) {
        cout << "Is your save named \"" << names[0] << "\" (yes or no):";
        string ans;
        cin >> ans;
        if (ans == "yes" || ans == "y")
            name = names[0];
        else
            cout << "There are no proper saves here!" << endl;
    } else {
        cout << "Which save you want to save automatically (just input the name):";
        string ans;
        cin >> ans;
        if (count(names.begin(), names.end(), ans) == 1)
            name = ans;
        else
            cout << "There is no saves named \"" << ans << "\"!" << endl;
    }
}

string currentTime2String() {
    time_t rawTime = time(NULL);
    tm *currentTime = localtime(&rawTime);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d-%H-%M-%S", currentTime);
    return string(buffer);
}

bool backup(string name) { 
    string path = ".\\backup";
    if (_access(path.c_str(), 0) == -1) {
        if (CreateDirectory(path.c_str(), NULL) == true)
            cout << "\"backup\" directory has been created!" << endl;
        else {
            cout << "create \"backup\" directory failed!" << endl;
            return false;
        }
    }

    string timeStr = currentTime2String();
    CreateDirectory((path + "\\" + timeStr).c_str(), NULL);
    FILE *pPipe  = _popen((".\\7zr.exe a -r \"" + path + "\\" + timeStr + "\\" + name + ".7z\" " +
           "\".\\" + name + "\"").c_str(), "r");
    _pclose(pPipe);
    cout << "[" << timeStr << "]" << "Backup Done!" << endl;
    return true;
}

bool cmp(string a, string b) { return (a > b); }

void restore(string name) {
    string path = ".\\backup";
    vector<string> dirs;
    
    getAllDirName(path, dirs);
    sort(dirs.begin(), dirs.end(), cmp);
    string restore_path = dirs[0];
    cout << "Current saves will be removed! Are you sure? (yes or no):";
    string ans;
    cin >> ans;
    if (ans == "y" || ans == "yes") {
        if (_access((".\\" + name).c_str(), 0) == -1)
            system(("rd .\\" + name + " \/S \/Q").c_str());
        FILE *pPipe = _popen((".\\7zr.exe x -r -y \"" + restore_path + "\\" + name + ".7z\"").c_str(), "r");
        _pclose(pPipe);
        MoveFile((restore_path + "\\" + name).c_str(), (".\\" + name).c_str());
        cout << "Restore Done!" << endl;
    } else {
        cout << "Restore has been cancelled!" << endl;
    }
}

void help() {
    cout << "This is a tool to automatically backup you Minecraft svaes! You can also manually backup, restore your saves! \n\"b\" or \"backup\" -- backup \n\"r\" or \"restore\" -- restore the lastest saves \n\"e\" or \"exit\" -- exit the tool \n\"h\" or \"help\" -- print this help" << endl;
}

void console(string name) {
    exitSignal = false;

    for (; 1 != 0; ) {
        string ans;
        cout << "input command " << "(" << this_thread::get_id() << "):";
        cout.flush();
        workingSignal = false;

        while (true) {
            if (kbhit()) {
                char t = getche();
                if (t == 13)
                    break;
                else {
                    char tt[2];
                    tt[0] = t;
                    tt[1] = '\0';
                    ans.append(tt);
                }
            } else if (autoSaveReady) {
                break;
            } else {
                usleep(100000);
            }
        }
        if (autoSaveReady) {
            break;
        }
        
        workingSignal = true;

        if (ans == "b" || ans == "backup") {
            backup(name);
        } else if (ans == "r" || ans == "restore") {
            restore(name);
        } else if (ans == "e" || ans == "exit") {
            exitSignal = true;
            break;
        } else if (ans == "h" || ans == "help") {
            help();
        } else {
            usleep(100000);
        }
    }

}

void AutoSave(string name) {
    backup(name);

    for (; 1 != 0;) {
        thread myThread(console, name);

        sleep(300);
        autoSaveReady = true;
        myThread.join();
        if (exitSignal)
            return;

        cout << "\r" << "                          " << "\r" << "Auto backup ...." << endl;
        backup(name);
        autoSaveReady = false;
    }
}

int main() {
    vector<string> dirPaths;
    vector<string> dirNames;
    string saveName = "";

    getAllDirName(".", dirPaths);
    printAllDirPaths(dirPaths);
    paths2names(dirPaths, dirNames);
    chooseSaves(dirNames, saveName);

    AutoSave(saveName);

    system("pause");
    return 0;
}