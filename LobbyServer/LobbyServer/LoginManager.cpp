#include "Common.h"

LoginManager::LoginManager()
{
    LoadFromFile("users.txt");

    /*driver = sql::mysql::get_mysql_driver_instance();
    conn = std::unique_ptr<sql::Connection>(driver->connect("tcp://127.0.0.1:3306", "root", "password"));
    conn->setSchema("logindata");
    LoadFromDB();*/
}

LoginManager::~LoginManager()
{
    conn->close();
}

// 회원가입
bool LoginManager::RegisterName(const string& username, const string& password)
{
    std::lock_guard<std::mutex> lock(login_mutex);
    for (const auto& user : users) {
        if (user.username == username) {
            cout << "Username already exists." << endl;
            return false;
        }
    }
    cout << "ID: " << username << ", PW: " << password << " - Registered!" << endl;
    // 새 사용자 추가
    SaveToUser("users.txt", username, password);
    return true;
}

// 로그인
bool LoginManager::Login(const string& username, const string& password)
{
    if (logged_in_users[username]) {
        cout << "이미 로그인된 사용자입니다. - ";
        cout << "ID: " << username << endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(login_mutex);
    for (const auto& user : users) {
        if (user.username == username && user.password == password) {
            logged_in_users[username] = true;
            return true;
        }
    }

    cout << "등록되지 않은 아이디거나 비밀번호가 틀렸습니다. - ";
    cout << "ID: " << username << ", PW: " << password << endl;
    return false;

    /*std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->prepareStatement("SELECT COUNT(*) FROM users WHERE username = ? AND password = ?")
    );
    pstmt->setString(1, username);
    pstmt->setString(2, password);

    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    res->next();

    if (res->getInt(1) > 0) {
        logged_in_users[username] = true;
        return true;
    }

    cout << "등록되지 않은 아이디거나 비밀번호가 틀렸습니다." << endl;
    return false;*/
}

bool LoginManager::Logout(const string& username)
{
    if (logged_in_users.find(username) != logged_in_users.end() && logged_in_users[username]) {
        logged_in_users[username] = false;
        cout << username << "님이 로그아웃했습니다." << endl;
        return true;
    }
    cout << "로그인 상태가 아닙니다." << endl;
    return false;
}

// 유저 정보 저장
void LoginManager::SaveToUser(const string& filename, const string& username, const string& password)
{
    users.push_back({ username, password });
    ofstream file(filename, ios::app);
    if (file.is_open()) {
        file << username << " " << password << endl;
        file.close();
    }

    /*std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->prepareStatement("INSERT INTO users (username, password) VALUES (?, ?)")
    );
    pstmt->setString(1, username);
    pstmt->setString(2, password);
    pstmt->executeUpdate();*/
}

// 파일 불러오기
void LoginManager::LoadFromFile(const string& filename)
{
    users.clear();
    ifstream file(filename);
    if (file.is_open()) {
        string username, password;
        while (file >> username >> password) {
            users.push_back({ username, password });
        }
        file.close();
    }
}

void LoginManager::LoadFromDB() {
    std::unique_ptr<sql::Statement> stmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT username, password FROM users"));

    while (res->next()) {
        string username = res->getString("username");
        string password = res->getString("password");
        users.push_back({ username, password });
    }
}