-- ============================================================
--  setup.sql
--  PGVector セットアップ用 SQL スクリプト
--  PostgreSQL 15/16 + pgvector 拡張が必要
-- ============================================================

-- 1. データベース作成 (psql で接続後に実行)
-- CREATE DATABASE vectordb;
-- \c vectordb

-- 2. pgvector 拡張を有効化
CREATE EXTENSION IF NOT EXISTS vector;

-- 3. テーブル作成 (dim=128 の例)
DROP TABLE IF EXISTS embeddings;
CREATE TABLE embeddings (
    id          SERIAL PRIMARY KEY,
    label       TEXT NOT NULL,
    embedding   vector(128) NOT NULL,
    created_at  TIMESTAMPTZ DEFAULT NOW()
);

-- 4. インデックス作成例

-- (A) IVFFlat インデックス (L2 距離)
--     ※ データ挿入後に作成すること
--     lists: sqrt(行数) が目安
CREATE INDEX IF NOT EXISTS embeddings_ivfflat_l2_idx
    ON embeddings
    USING ivfflat (embedding vector_l2_ops)
    WITH (lists = 100);

-- (B) IVFFlat インデックス (コサイン類似度)
CREATE INDEX IF NOT EXISTS embeddings_ivfflat_cos_idx
    ON embeddings
    USING ivfflat (embedding vector_cosine_ops)
    WITH (lists = 100);

-- (C) HNSW インデックス (L2 距離) ※ pgvector 0.5.0 以降
CREATE INDEX IF NOT EXISTS embeddings_hnsw_l2_idx
    ON embeddings
    USING hnsw (embedding vector_l2_ops)
    WITH (m = 16, ef_construction = 64);

-- 5. 検索時パラメータ例
--    IVFFlat: probes を増やすと精度向上 (速度とトレードオフ)
SET ivfflat.probes = 10;

--    HNSW: ef_search を増やすと精度向上
SET hnsw.ef_search = 100;

-- 7. テーブル確認
SELECT COUNT(*) AS total_rows FROM embeddings;
SELECT pg_size_pretty(pg_total_relation_size('embeddings')) AS table_size;
