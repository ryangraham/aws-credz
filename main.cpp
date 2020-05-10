#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <curl/curl.h>

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

int password_prompt(std::string &password)
{
    struct termios tty;

    // Get current terminal settings
    if (tcgetattr(STDIN_FILENO, &tty) != 0)
        return -1;

    // Unset ECHO flag
    tty.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) != 0)
        return -1;

    // Prompt for password
    std::cout << "Password: ";
    std::cin >> password;

    // Set ECHO flag
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);

    return 0;
}

void username_prompt(std::string &username)
{
    std::cout << "Username: ";
    std::cin >> username;
}

int main(void)
{
    std::string username = "";
    std::string password = "";

    username_prompt(username);
    password_prompt(password);
    std::cout << std::endl;
    //std::cout << password << std::endl;

    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charset: utf-8");

    // TODO: use nlohmann
    std::string payload = R"({ "username": ")" + username + R"(", "password": ")" + password + R"(" })";
    std::string url = "https://blvd.okta.com/api/v1/authn";

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        std::cout << readBuffer << std::endl;
    }
    return 0;
}