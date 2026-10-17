#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
// 引入第三方单头文件库
#include "httplib.h"
#include "json.hpp"
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
// 在 Windows 控制台中正确打印 UTF-8 字符串
void printUtf8(const std::string &s) {
    // 转为宽字符串
    int wlen = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    if (wlen <= 0) {
        std::cout << s;
        return;
    }
    std::wstring w;
    w.resize(wlen);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], wlen);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        std::cout << s;
        return;
    }
    DWORD written = 0;
    // 写入不包含末尾的 null
    WriteConsoleW(hOut, w.c_str(), (DWORD)wlen - 1, &written, NULL);
}
#endif

using namespace std;
using json = nlohmann::json;

// 学生结构体
struct Student {
    string id;
    string name;
    string gender;
    vector<double> grades;
};

// 全局学生列表
vector<Student> students;
const string FILE_NAME = "web_students_data.json";

// 将数据保存到 JSON 文件
void saveToFile() {
    json j = json::array();
    for (const auto& s : students) {
        j.push_back({
            {"id", s.id}, {"name", s.name}, 
            {"gender", s.gender}, {"grades", s.grades}
        });
    }
    ofstream out(FILE_NAME);
    out << j.dump(4);
}

// 从 JSON 文件加载数据
void loadFromFile() {
    ifstream in(FILE_NAME);
    if (in.is_open()) {
        try {
            json j;
            in >> j;
            
            // 去重合并
            for (const auto& item : j) {
                string cleaned_id = item["id"];
                cleaned_id.erase(cleaned_id.begin(), find_if(cleaned_id.begin(), cleaned_id.end(), [](unsigned char ch) {
                    return !isspace(ch);
                }));
                cleaned_id.erase(find_if(cleaned_id.rbegin(), cleaned_id.rend(), [](unsigned char ch) {
                    return !isspace(ch);
                }).base(), cleaned_id.end());
                
                bool found = false;
                for (auto& s : students) {
                    if (s.id == cleaned_id) {
                        s.name = item["name"];
                        s.gender = item["gender"];
                        s.grades = item["grades"].get<vector<double>>();
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    students.push_back({
                        cleaned_id, item["name"], item["gender"], 
                        item["grades"].get<vector<double>>()
                    });
                }
            }
            saveToFile(); // 保存修复后的数据
        } catch (json::parse_error& e) {
#ifdef _WIN32
            printUtf8("JSON 解析错误: " + string(e.what()) + "\n");
#else
            cerr << "JSON 解析错误: " << e.what() << "\n";
#endif
        }
    }
}

// 控制台交互界面
void runConsoleUI() {
    int choice;
    while (true) {
#ifdef _WIN32
        printUtf8("\n--- 学生信息管理系统 (控制台模式) ---\n");
        printUtf8("1. 查看所有学生\n");
        printUtf8("2. 添加/修改学生\n");
        printUtf8("3. 删除学生\n");
        printUtf8("4. 退出\n");
        printUtf8("请选择操作: ");
#else
        cout << "\n--- 学生信息管理系统 (控制台模式) ---\n";
        cout << "1. 查看所有学生\n";
        cout << "2. 添加/修改学生\n";
        cout << "3. 删除学生\n";
        cout << "4. 退出\n";
        cout << "请选择操作: ";
#endif
        cin >> choice;

        if (choice == 1) {
            for (const auto& s : students) {
#ifdef _WIN32
                printUtf8("学号: " + s.id + ", 姓名: " + s.name + 
                          ", 性别: " + s.gender + ", 成绩: ");
#else
                cout << "学号: " << s.id << ", 姓名: " << s.name 
                     << ", 性别: " << s.gender << ", 成绩: ";
#endif
                for (double g : s.grades) cout << g << " ";
                cout << "\n";
            }
        } else if (choice == 2) {
            Student s;
#ifdef _WIN32
            printUtf8("学号: "); cin >> s.id;
            printUtf8("姓名: "); cin >> s.name;
            printUtf8("性别: "); cin >> s.gender;
            printUtf8("三门课成绩(用空格分隔): ");
#else
            cout << "学号: "; cin >> s.id;
            cout << "姓名: "; cin >> s.name;
            cout << "性别: "; cin >> s.gender;
            cout << "三门课成绩(用空格分隔): ";
#endif
            double g1, g2, g3;
            cin >> g1 >> g2 >> g3;
            s.grades = {g1, g2, g3};
            
            bool found = false;
            for (auto& cs : students) {
                // Remove trailing whitespace including tabs
                string cs_id = cs.id;
                cs_id.erase(cs_id.find_last_not_of(" \n\r\t")+1);
                string s_id = s.id;
                s_id.erase(s_id.find_last_not_of(" \n\r\t")+1);
                if (cs_id == s_id) {
                    cs = s;
                    found = true;
                    break;
                }
            }
            if (!found) students.push_back(s);
            saveToFile();
#ifdef _WIN32
            printUtf8("保存成功！\n");
#else
            cout << "保存成功！\n";
#endif
        } else if (choice == 3) {
            string id;
#ifdef _WIN32
            printUtf8("请输入要删除的学号: ");
#else
            cout << "请输入要删除的学号: ";
#endif
            cin >> id;
            students.erase(remove_if(students.begin(), students.end(),
                [&](const Student& s) { return s.id == id; }), students.end());
            saveToFile();
#ifdef _WIN32
            printUtf8("删除完成！\n");
#else
            cout << "删除完成！\n";
#endif
        } else if (choice == 4) {
            break;
        } else {
#ifdef _WIN32
            printUtf8("无效选择！\n");
#else
            cout << "无效选择！\n";
#endif
            // 清理输入流，防止无限循环错误
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}

int main() {
    loadFromFile();
#ifdef _WIN32
    // 设置控制台为 UTF-8，避免中文输出乱码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // 增加启动选项
#ifdef _WIN32
    printUtf8("请选择启动模式：\n");
    printUtf8("1. Web UI 模式 (默认浏览器访问)\n");
    printUtf8("2. Console 命令行模式\n");
    printUtf8("请输入你的选择 (1 或 2): ");
#else
    cout << "请选择启动模式：\n";
    cout << "1. Web UI 模式 (默认浏览器访问)\n";
    cout << "2. Console 命令行模式\n";
    cout << "请输入你的选择 (1 或 2): ";
#endif

    string choice;
    getline(cin, choice);
    if (choice == "2") {
        runConsoleUI();
        return 0;
    }

    httplib::Server svr;

    // 1. 提供静态网页服务 (挂载 public 目录)
    svr.set_mount_point("/", "./public");

    // 2. API: 获取学生列表 (支持排序)
    svr.Get("/api/students", [](const httplib::Request& req, httplib::Response& res) {
        auto list_copy = students;
        if (req.has_param("sort") && req.get_param_value("sort") == "true") {
            sort(list_copy.begin(), list_copy.end(), [](const Student& a, const Student& b) {
                double ta = a.grades[0] + a.grades[1] + a.grades[2];
                double tb = b.grades[0] + b.grades[1] + b.grades[2];
                return ta > tb;
            });
        }

        json j = json::array();
        for (const auto& s : list_copy) {
            j.push_back({{"id", s.id}, {"name", s.name}, {"gender", s.gender}, {"grades", s.grades}});
        }
        res.set_content(j.dump(), "application/json");
    });

    // 3. API: 添加或更新学生
    svr.Post("/api/students", [](const httplib::Request& req, httplib::Response& res) {
        auto j = json::parse(req.body);
        string id = j["id"];
        
        // 如果学号存在则覆盖，否则添加
        bool found = false;
        for (auto& s : students) {
            // Remove trailing whitespace including tabs
            string cs_id = s.id;
            cs_id.erase(cs_id.find_last_not_of(" \n\r\t")+1);
            string s_id = id;
            s_id.erase(s_id.find_last_not_of(" \n\r\t")+1);

            if (cs_id == s_id) {
                s.name = j["name"];
                s.gender = j["gender"];
                s.grades = j["grades"].get<vector<double>>();
                found = true; break;
            }
        }
        if (!found) {
            students.push_back({id, j["name"], j["gender"], j["grades"].get<vector<double>>()});
        }
        
        saveToFile();
        res.set_content("{\"status\": \"ok\"}", "application/json");
    });

    // 4. API: 删除学生
    svr.Delete("/api/students", [](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("id")) {
            string id = req.get_param_value("id");
            students.erase(remove_if(students.begin(), students.end(),
                [&](const Student& s) { return s.id == id; }), students.end());
            saveToFile();
        }
        res.set_content("{\"status\": \"ok\"}", "application/json");
    });

    // 启动服务器
#ifdef _WIN32
    printUtf8("服务器已启动，请在浏览器中访问 http://localhost:8080\n");
#else
    cout << "服务器已启动，请在浏览器中访问 http://localhost:8080" << endl;
#endif
    svr.listen("0.0.0.0", 8080);
    return 0;
}