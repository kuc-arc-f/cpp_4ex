// ============================================================
//  main.cpp
//  PGVector C++ サンプル - ベクトル登録・検索デモ
// ============================================================

#include "pgvector_client.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <iomanip>

// ============================================================
//  ユーティリティ: ランダムベクトル生成
// ============================================================
FloatVec makeRandomVec(int dim, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    FloatVec v(dim);
    for (auto& x : v) x = dist(rng);
    return v;
}

// ============================================================
//  ユーティリティ: L2 正規化
// ============================================================
FloatVec normalize(FloatVec v) {
    float norm = 0.0f;
    for (float x : v) norm += x * x;
    norm = std::sqrt(norm);
    if (norm > 1e-9f)
        for (auto& x : v) x /= norm;
    return v;
}

// ============================================================
//  ユーティリティ: 検索結果表示
// ============================================================
void printResults(const std::vector<SearchResult>& results,
                  const std::string& title)
{
    std::cout << "\n=== " << title << " ===\n";
    std::cout << std::left
              << std::setw(6)  << "Rank"
              << std::setw(6)  << "ID"
              << std::setw(20) << "Label"
              << std::setw(12) << "Distance"
              << "\n";
    std::cout << std::string(44, '-') << "\n";

    int rank = 1;
    for (const auto& r : results) {
        std::cout << std::setw(6)  << rank++
                  << std::setw(6)  << r.id
                  << std::setw(20) << r.label
                  << std::fixed << std::setprecision(6)
                  << std::setw(12) << r.distance
                  << "\n";
    }
}

// ============================================================
//  メイン
// ============================================================
int main() {
    std::cout << "============================\n";
    std::cout << "  PGVector C++ サンプル\n";
    std::cout << "============================\n\n";

    // ---------------------------------------------------------
    //  接続設定 (環境に合わせて変更)
    // ---------------------------------------------------------
    PGConnConfig cfg;
    cfg.host     = "localhost";
    cfg.port     = 5432;
    cfg.dbname   = "mydb";
    cfg.user     = "root";
    cfg.password = "admin";

    constexpr int DIM  = 128;   // ベクトル次元数
    constexpr int TOPK = 3;     // 近傍検索件数

    PGVectorClient client(cfg);

    try {
        // =====================================================
        //  1. 接続
        // =====================================================
        client.connect();

        // =====================================================
        //  2. スキーマ初期化
        // =====================================================
        client.initSchema(DIM);

        // =====================================================
        //  3. サンプルベクトルを登録
        // =====================================================
        std::cout << "\n--- ベクトル登録 ---\n";

        struct Item {
            std::string label;
            unsigned    seed;
        };

        std::vector<Item> items = {
            {"cat",        1001},
            {"dog",        1002},
            {"fish",       1003},
            {"bird",       1004},
            {"rabbit",     1005},
            {"elephant",   1006},
            {"lion",       1007},
            {"tiger",      1008},
            {"bear",       1009},
            {"wolf",       1010},
        };

        for (const auto& item : items) {
            FloatVec vec = normalize(makeRandomVec(DIM, item.seed));
            client.insertEmbedding(item.label, vec);
        }

        // =====================================================
        //  4. HNSW インデックス作成 (任意)
        //     ※ データ挿入後に作成する方が効率的
        // =====================================================
        std::cout << "\n--- インデックス作成 ---\n";
        client.createHNSWIndex(/*m=*/16, /*ef_construction=*/64);

        // =====================================================
        //  5. L2 距離近傍検索
        // =====================================================
        std::cout << "\n--- L2 距離近傍検索 ---\n";

        // "cat" に近いベクトルで検索
        FloatVec queryL2 = normalize(makeRandomVec(DIM, 1001));  // cat と同じシード
        auto resultsL2   = client.searchNN(queryL2, TOPK);
        printResults(resultsL2, "L2 近傍検索 (クエリ ≒ cat)");

        // =====================================================
        //  6. コサイン類似度検索
        // =====================================================
        std::cout << "\n--- Cosine類似度検索 ---\n";

        FloatVec queryCos = normalize(makeRandomVec(DIM, 1007));  // lion と同じシード
        auto resultsCos   = client.searchCosine(queryCos, TOPK);
        printResults(resultsCos, "Cosine類似度検索 (クエリ ≒ lion)");

        // =====================================================
        //  7. 完全にランダムなクエリで検索
        // =====================================================
        FloatVec queryRnd = normalize(makeRandomVec(DIM, 9999));
        auto resultsRnd   = client.searchNN(queryRnd, TOPK);
        printResults(resultsRnd, "L2 近傍検索 (ランダムクエリ)");

    }
    catch (const std::exception& ex) {
        std::cerr << "\n[ERROR] " << ex.what() << "\n";
        return 1;
    }

    std::cout << "\n============================\n";
    std::cout << "  完了\n";
    std::cout << "============================\n";
    return 0;
}

