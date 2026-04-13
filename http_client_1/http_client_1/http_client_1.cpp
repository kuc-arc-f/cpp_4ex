// ============================================================
//  main.cpp  -  HttpClient 使用サンプル
//  Visual Studio 2019/2022  (C++17)
//  ビルド手順:
//    1. 新規 "コンソールアプリ" プロジェクトを作成
//    2. HttpClient.h を同フォルダに配置
//    3. C++ 言語標準を C++17 以上に設定
//    4. winhttp.lib は #pragma comment で自動リンク済み
// ============================================================

#include <iostream>
#include <string>
#include "HttpClient.h"

// wstring ↔ string 変換ヘルパー (Windows ANSI 限定)
static std::string WStrToStr(const std::wstring& ws)
{
    if (ws.empty()) return {};
    int n = ::WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n - 1, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, s.data(), n, nullptr, nullptr);
    return s;
}

// レスポンスを標準出力に表示
static void PrintResponse(const HttpResponse& resp)
{
    std::cout << "=== Response ===" << std::endl;
    std::cout << "Status : " << resp.statusCode
        << " " << WStrToStr(resp.statusText) << std::endl;

    std::cout << "--- Headers ---" << std::endl;
    for (auto& [k, v] : resp.headers)
        std::cout << WStrToStr(k) << ": " << WStrToStr(v) << std::endl;

    std::cout << "--- Body (" << resp.body.size() << " bytes) ---" << std::endl;
    // 長すぎる場合は先頭 512 文字だけ表示
    if (resp.body.size() <= 512)
        std::cout << resp.body << std::endl;
    else
        std::cout << resp.body.substr(0, 512) << "\n...(truncated)" << std::endl;

    std::cout << std::endl;
}

const std::wstring API_URL = L"http://localhost:8000/";

// ============================================================
//  サンプル 1 : GET リクエスト (HTTP)
// ============================================================
void SampleGet()
{
    std::cout << "========== [GET] httpbin.org/get ==========" << std::endl;

    HttpClient client(L"MyApp/1.0");
    auto resp = client.Get(L"http://localhost:8000/todos");
    PrintResponse(resp);
}

// ============================================================
//  サンプル 2 : POST リクエスト (JSON ボディ)
// ============================================================
void SamplePost()
{
    std::cout << "========== [POST] httpbin.org/post ==========" << std::endl;

    HttpClient client;
    std::string jsonBody = R"({"title":"test-1"})";

    auto resp = client.Post(
        L"http://localhost:8000/todos",
        jsonBody,
        L"application/json");

    PrintResponse(resp);
}

// ============================================================
//  サンプル 3 : カスタムヘッダー付き GET
// ============================================================
void SampleGetWithHeaders()
{
    std::cout << "========== [GET with Headers] httpbin.org/headers ==========" << std::endl;

    HttpClient client;
    std::map<std::wstring, std::wstring> headers = {
        { L"X-Request-ID",  L"abc-123"      },
        { L"Authorization", L"Bearer mytoken"},
        { L"Accept",        L"application/json" }
    };

    auto resp = client.Get(L"https://httpbin.org/headers", headers);
    PrintResponse(resp);
}

// ============================================================
//  サンプル 4 : REST API へ PUT
// ============================================================
void SamplePut()
{
    std::cout << "========== [PUT] httpbin.org/put ==========" << std::endl;

    HttpClient client;
    std::string body = R"({"id":1,"status":"updated"})";

    auto resp = client.Put(
        L"https://httpbin.org/put",
        body,
        L"application/json");

    PrintResponse(resp);
}

// ============================================================
//  サンプル 5 : DELETE リクエスト
// ============================================================
void SampleDelete()
{
    std::cout << "========== [DELETE] httpbin.org/delete ==========" << std::endl;

    HttpClient client;
    auto resp = client.Delete(L"https://httpbin.org/delete");
    PrintResponse(resp);
}

// ============================================================
//  サンプル 6 : エラーハンドリング
// ============================================================
void SampleErrorHandling()
{
    std::cout << "========== [Error Handling] ==========" << std::endl;

    try
    {
        HttpClient client;
        // 存在しないホストに接続 → 例外が発生
        auto resp = client.Get(L"https://this-host-does-not-exist.example.invalid/");
        PrintResponse(resp);
    }
    catch (const std::runtime_error& ex)
    {
        std::cerr << "[Exception caught] " << ex.what() << std::endl;
    }

    std::cout << std::endl;
}

// ============================================================
//  main
// ============================================================
int main()
{
    // コンソールを UTF-8 に設定
    ::SetConsoleOutputCP(CP_UTF8);

    try
    {
        SampleGet();
        SamplePost();
        //SampleDelete();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        return 1;
    }

    std::cout << "All samples completed." << std::endl;
    return 0;
}
