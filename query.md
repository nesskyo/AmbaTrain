# ⚔️ AmbaTrain — Supabase (PostgreSQL) Database Queries & Schema Guide

> *"All the world's a station, and all the men and women merely passengers."*  
> Panduan lengkap perancangan basis data Supabase untuk **AmbaTrain** (C++ Crow Backend + Dark Gothic Shakespearean Theme). Dokumen ini diselaraskan 100% dengan **`PRD.md`** dan **`desain.md`**.

---

## 📑 Daftar Isi
1. [Struktur Relasi & ERD Konseptual](#1-struktur-relasi--erd-konseptual)
2. [DDL Schema (`db/schema.sql`)](#2-ddl-schema-dbschemasql)
3. [Mekanisme Anti Double-Booking & Locking](#3-mekanisme-anti-double-booking--locking)
4. [Stored Functions & Triggers Otomatis](#4-stored-functions--triggers-otomatis)
5. [Row Level Security (RLS) di Supabase](#5-row-level-security-rls-di-supabase)
6. [Seed Data (`db/seed.sql`)](#6-seed-data-dbseedsql)
7. [Katalog Query Operasional Backend C++ (Crow REST API)](#7-katalog-query-operasional-backend-c-crow-rest-api)
8. [Panduan Eksekusi di Supabase Dashboard](#8-panduan-eksekusi-di-supabase-dashboard)

---

## 1. Struktur Relasi & ERD Konseptual

```
users (1) ────────< bookings (∞) ────────> schedules (1) ────────> trains (1)
                        │                     │                       │
                        │                     │                       ├──< carriages (∞)
                        │                     │                       │       └──< seats (∞)
                        │                     ├──> stations (origin)  │               │
                        │                     └──> stations (dest)    │               │
                        │                                             │               │
                        └───< booking_passengers (∞) ─────────────────┴───────────────┘
                                      │ (schedule_id, seat_id) [WHERE seat_active = true]
                                      │
bookings (1) ─────────── (1) payments (Tribute)
```

---

## 2. DDL Schema (`db/schema.sql`)

Salin query berikut ke **SQL Editor** di Supabase untuk menginisialisasi tabel-tabel utama:

```sql
-- ============================================================================
-- AMBATRAIN DATABASE DDL SCHEMA (SUPABASE / POSTGRESQL)
-- ============================================================================

-- Aktifkan ekstensi UUID dan PGCrypto
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
CREATE EXTENSION IF NOT EXISTS "pgcrypto";

-- 1. TABEL USERS (Para Pengelana & High Conductor)
CREATE TABLE IF NOT EXISTS public.users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    role VARCHAR(50) NOT NULL DEFAULT 'user' CHECK (role IN ('user', 'admin')),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- 2. TABEL STATIONS (Gerbang Stasiun Dunia Nyata)
CREATE TABLE IF NOT EXISTS public.stations (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    code VARCHAR(10) NOT NULL UNIQUE,
    name VARCHAR(100) NOT NULL,
    city VARCHAR(100) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- 3. TABEL TRAINS (Lokomotif / Steed of Steel)
-- Kelas: sovereign (Eksekutif), noble (Bisnis), yeoman (Ekonomi)
CREATE TABLE IF NOT EXISTS public.trains (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    class VARCHAR(50) NOT NULL CHECK (class IN ('sovereign', 'noble', 'yeoman')),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- 4. TABEL CARRIAGES (Gerbong Kereta)
CREATE TABLE IF NOT EXISTS public.carriages (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    train_id BIGINT NOT NULL REFERENCES public.trains(id) ON DELETE CASCADE,
    carriage_number INT NOT NULL,
    seat_count INT NOT NULL DEFAULT 0,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_train_carriage UNIQUE (train_id, carriage_number)
);

-- 5. TABEL SEATS (Sigil Kursi)
CREATE TABLE IF NOT EXISTS public.seats (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    carriage_id BIGINT NOT NULL REFERENCES public.carriages(id) ON DELETE CASCADE,
    seat_number VARCHAR(10) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_carriage_seat UNIQUE (carriage_id, seat_number)
);

-- 6. TABEL SCHEDULES (Jadwal Keberangkatan)
CREATE TABLE IF NOT EXISTS public.schedules (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    train_id BIGINT NOT NULL REFERENCES public.trains(id) ON DELETE RESTRICT,
    origin_id BIGINT NOT NULL REFERENCES public.stations(id) ON DELETE RESTRICT,
    destination_id BIGINT NOT NULL REFERENCES public.stations(id) ON DELETE RESTRICT,
    departure_at TIMESTAMPTZ NOT NULL,
    arrival_at TIMESTAMPTZ NOT NULL,
    price NUMERIC(12, 2) NOT NULL CHECK (price >= 0),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT chk_schedule_stations CHECK (origin_id <> destination_id),
    CONSTRAINT chk_schedule_times CHECK (arrival_at > departure_at)
);

-- 7. TABEL BOOKINGS (Segel Takdir / Sigil Pemesanan)
-- Status: held (Awaiting Tribute), paid (Sealed), cancelled (Forsaken), expired (Faded to Dust)
CREATE TABLE IF NOT EXISTS public.bookings (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    code VARCHAR(20) NOT NULL UNIQUE,
    user_id UUID NOT NULL REFERENCES public.users(id) ON DELETE RESTRICT,
    schedule_id BIGINT NOT NULL REFERENCES public.schedules(id) ON DELETE RESTRICT,
    status VARCHAR(20) NOT NULL DEFAULT 'held' CHECK (status IN ('held', 'paid', 'cancelled', 'expired')),
    total_price NUMERIC(12, 2) NOT NULL DEFAULT 0 CHECK (total_price >= 0),
    expires_at TIMESTAMPTZ NOT NULL DEFAULT (NOW() + INTERVAL '15 minutes'),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- 8. TABEL BOOKING_PASSENGERS (Penumpang & Penguncian Kursi)
CREATE TABLE IF NOT EXISTS public.booking_passengers (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    booking_id BIGINT NOT NULL REFERENCES public.bookings(id) ON DELETE CASCADE,
    schedule_id BIGINT NOT NULL REFERENCES public.schedules(id) ON DELETE RESTRICT,
    seat_id BIGINT NOT NULL REFERENCES public.seats(id) ON DELETE RESTRICT,
    name VARCHAR(255) NOT NULL,
    id_number VARCHAR(50) NOT NULL,
    seat_active BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- 9. TABEL PAYMENTS (Tribute / Pembayaran Simulasi)
CREATE TABLE IF NOT EXISTS public.payments (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    booking_id BIGINT NOT NULL UNIQUE REFERENCES public.bookings(id) ON DELETE CASCADE,
    method VARCHAR(50) NOT NULL,
    status VARCHAR(20) NOT NULL DEFAULT 'pending' CHECK (status IN ('pending', 'completed', 'failed', 'refunded')),
    amount NUMERIC(12, 2) NOT NULL CHECK (amount >= 0),
    paid_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
```

---

## 3. Mekanisme Anti Double-Booking & Locking

Sesuai **PRD Bagian 11.3 & 15**, AmbaTrain menjamin **0 double-booking** bahkan di bawah 50+ request konkuren menggunakan 2 lapis proteksi:

### Lapis 1: Database Partial Unique Index
```sql
CREATE UNIQUE INDEX IF NOT EXISTS uq_active_seat_per_schedule
    ON public.booking_passengers (schedule_id, seat_id)
    WHERE seat_active = TRUE;
```
> **Mengapa Partial Index?**  
> Kursi yang dipesan pada jadwal yang sama hanya dikunci jika `seat_active = TRUE` (yaitu saat berstatus `held` atau `paid`). Bila pemesanan dibatalkan (`cancelled`) atau kedaluwarsa (`expired`), `seat_active` berubah menjadi `FALSE`, sehingga kursi tersebut langsung bisa dipesan oleh pengelana lain tanpa bentrok index!

### Indeks Performa Tambahan
```sql
-- Indeks Pencarian Rute & Tanggal
CREATE INDEX IF NOT EXISTS idx_schedules_search 
    ON public.schedules (origin_id, destination_id, departure_at);

-- Indeks Kronik Riwayat Pemesanan Pengguna
CREATE INDEX IF NOT EXISTS idx_bookings_user_history 
    ON public.bookings (user_id, created_at DESC);

-- Indeks Sigil Kode Booking
CREATE INDEX IF NOT EXISTS idx_bookings_code 
    ON public.bookings (code);

-- Indeks Expiry Background Worker
CREATE INDEX IF NOT EXISTS idx_bookings_expiry 
    ON public.bookings (status, expires_at) 
    WHERE status = 'held';

-- Indeks Denah Kursi Aktif
CREATE INDEX IF NOT EXISTS idx_booking_passengers_active 
    ON public.booking_passengers (schedule_id) 
    WHERE seat_active = TRUE;
```

---

## 4. Stored Functions & Triggers Otomatis

### A. Auto Timestamp `updated_at`
```sql
CREATE OR REPLACE FUNCTION public.set_current_timestamp_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_users_updated_at
BEFORE UPDATE ON public.users
FOR EACH ROW EXECUTE FUNCTION public.set_current_timestamp_updated_at();

CREATE TRIGGER trg_bookings_updated_at
BEFORE UPDATE ON public.bookings
FOR EACH ROW EXECUTE FUNCTION public.set_current_timestamp_updated_at();
```

### B. Otomatis Lepas Kursi saat Status Batal/Kedaluwarsa
```sql
CREATE OR REPLACE FUNCTION public.sync_booking_passenger_seat_status()
RETURNS TRIGGER AS $$
BEGIN
    -- Jika status berubah menjadi cancelled atau expired, nonaktifkan kunci kursi
    IF NEW.status IN ('cancelled', 'expired') AND OLD.status NOT IN ('cancelled', 'expired') THEN
        UPDATE public.booking_passengers
        SET seat_active = FALSE
        WHERE booking_id = NEW.id;
    -- Jika booking aktif kembali (held/paid)
    ELSIF NEW.status IN ('held', 'paid') AND OLD.status NOT IN ('held', 'paid') THEN
        UPDATE public.booking_passengers
        SET seat_active = TRUE
        WHERE booking_id = NEW.id;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_booking_status_sync_seats
AFTER UPDATE OF status ON public.bookings
FOR EACH ROW EXECUTE FUNCTION public.sync_booking_passenger_seat_status();
```

### C. Fungsi Pembebasan Hold Kedaluwarsa (15 Menit Expiry)
```sql
CREATE OR REPLACE FUNCTION public.expire_held_bookings()
RETURNS INTEGER AS $$
DECLARE
    v_count INTEGER;
BEGIN
    WITH expired_batch AS (
        UPDATE public.bookings
        SET status = 'expired',
            updated_at = NOW()
        WHERE status = 'held'
          AND expires_at <= NOW()
        RETURNING id
    )
    SELECT COUNT(*) INTO v_count FROM expired_batch;

    RETURN v_count;
END;
$$ LANGUAGE plpgsql;
```

> **Catatan `pg_cron` (Opsional di Supabase Dashboard):**  
> Kamu bisa menjadwalkan fungsi ini berjalan otomatis setiap menit di Supabase:
> ```sql
> -- Jalankan setelah mengaktifkan ekstensi pg_cron di menu Database Extensions
> SELECT cron.schedule(
>     'expire-held-bookings-job',
>     '* * * * *',
>     $$ SELECT public.expire_held_bookings(); $$
> );
> ```

---

## 5. Row Level Security (RLS) di Supabase

Sesuai **PRD Bagian 10.4**:
- Backend C++ terhubung ke Supabase melalui role superuser `postgres` (Session Pooler port 5432) yang memiliki hak `BYPASSRLS`.
- Dengan mengaktifkan RLS pada seluruh tabel tanpa membuat policy untuk `anon` atau `authenticated`, kita memastikan PostgREST API bawaan Supabase **tertutup total untuk publik**. Pintu gerbang satu-satunya adalah backend C++.

```sql
ALTER TABLE public.users ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.stations ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.trains ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.carriages ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.seats ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.schedules ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.bookings ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.booking_passengers ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.payments ENABLE ROW LEVEL SECURITY;
```

---

## 6. Seed Data (`db/seed.sql`)

Salin query berikut untuk memasukkan data awal: 10 Stasiun kota besar, 6 Lokomotif (3 kelas: Sovereign, Noble, Yeoman), denah gerbong & kursi, jadwal dinamis 14 hari, dan akun demo.

```sql
-- Bersihkan data lama
TRUNCATE TABLE 
    public.payments,
    public.booking_passengers,
    public.bookings,
    public.schedules,
    public.seats,
    public.carriages,
    public.trains,
    public.stations,
    public.users
RESTART IDENTITY CASCADE;

-- 1. SEED USERS (Password default demo: 'Password123!')
INSERT INTO public.users (id, name, email, password_hash, role) VALUES
    ('a0000000-0000-0000-0000-000000000001', 'High Conductor (Admin)', 'admin@ambatrain.com', '$2a$12$6K6m.Zg0sYq6Qo12fLwRveWbQyUoU9K4A1vFj1H7M3b0N8s4L5e2G', 'admin'),
    ('b0000000-0000-0000-0000-000000000002', 'Lord Noctis (Traveler)', 'traveler@ambatrain.com', '$2a$12$6K6m.Zg0sYq6Qo12fLwRveWbQyUoU9K4A1vFj1H7M3b0N8s4L5e2G', 'user'),
    ('c0000000-0000-0000-0000-000000000003', 'Lady Ophelia', 'ophelia@ambatrain.com', '$2a$12$6K6m.Zg0sYq6Qo12fLwRveWbQyUoU9K4A1vFj1H7M3b0N8s4L5e2G', 'user');

-- 2. SEED STATIONS (Gerbang Nyata di Indonesia)
INSERT INTO public.stations (code, name, city) VALUES
    ('GMR', 'Stasiun Gambir', 'Jakarta'),
    ('BD',  'Stasiun Bandung', 'Bandung'),
    ('CN',  'Stasiun Cirebon', 'Cirebon'),
    ('SMT', 'Stasiun Semarang Tawang', 'Semarang'),
    ('PWT', 'Stasiun Purwokerto', 'Purwokerto'),
    ('YK',  'Stasiun Yogyakarta (Tugu)', 'Yogyakarta'),
    ('SLO', 'Stasiun Solo Balapan', 'Surakarta'),
    ('SGU', 'Stasiun Surabaya Gubeng', 'Surabaya'),
    ('ML',  'Stasiun Malang', 'Malang'),
    ('MDI', 'Stasiun Mandai', 'Makassar');

-- 3. SEED TRAINS (3 Kategori Kelas Kereta)
INSERT INTO public.trains (name, class) VALUES
    ('Argo Bromo Sovereign', 'sovereign'),
    ('Taksaka Sovereign',    'sovereign'),
    ('Lodaya Noble',         'noble'),
    ('Senja Utama Noble',    'noble'),
    ('Matarmaja Yeoman',     'yeoman'),
    ('Airlangga Yeoman',     'yeoman');

-- 4. SEED CARRIAGES & SEATS (2 Gerbong per kereta, 20 Kursi per gerbong: 1A s/d 5D)
DO $$
DECLARE
    r_train RECORD;
    v_c_id BIGINT;
    v_c_num INT;
    v_row INT;
    v_col TEXT;
BEGIN
    FOR r_train IN SELECT id FROM public.trains LOOP
        FOR v_c_num IN 1..2 LOOP
            INSERT INTO public.carriages (train_id, carriage_number, seat_count)
            VALUES (r_train.id, v_c_num, 20)
            RETURNING id INTO v_c_id;

            FOR v_row IN 1..5 LOOP
                FOREACH v_col IN ARRAY ARRAY['A', 'B', 'C', 'D'] LOOP
                    INSERT INTO public.seats (carriage_id, seat_number)
                    VALUES (v_c_id, v_row || v_col);
                END LOOP;
            END LOOP;
        END LOOP;
    END LOOP;
END $$;

-- 5. SEED JADWAL 14 HARI KE DEPAN SECARA DINAMIS
DO $$
DECLARE
    i INT;
    d DATE;
    id_gmr BIGINT; id_bd BIGINT; id_yk BIGINT; id_sgu BIGINT; id_slo BIGINT;
    id_argo BIGINT; id_taksaka BIGINT; id_lodaya BIGINT; 
    id_senja BIGINT; id_matarmaja BIGINT; id_airlangga BIGINT;
BEGIN
    SELECT id INTO id_gmr FROM public.stations WHERE code = 'GMR';
    SELECT id INTO id_bd  FROM public.stations WHERE code = 'BD';
    SELECT id INTO id_yk  FROM public.stations WHERE code = 'YK';
    SELECT id INTO id_sgu FROM public.stations WHERE code = 'SGU';
    SELECT id INTO id_slo FROM public.stations WHERE code = 'SLO';

    SELECT id INTO id_argo FROM public.trains WHERE name = 'Argo Bromo Sovereign';
    SELECT id INTO id_taksaka FROM public.trains WHERE name = 'Taksaka Sovereign';
    SELECT id INTO id_lodaya FROM public.trains WHERE name = 'Lodaya Noble';
    SELECT id INTO id_senja FROM public.trains WHERE name = 'Senja Utama Noble';
    SELECT id INTO id_matarmaja FROM public.trains WHERE name = 'Matarmaja Yeoman';
    SELECT id INTO id_airlangga FROM public.trains WHERE name = 'Airlangga Yeoman';

    FOR i IN 0..13 LOOP
        d := CURRENT_DATE + i;

        -- 1. Gambir -> Surabaya Gubeng [Sovereign]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_argo, id_gmr, id_sgu, (d + TIME '08:00:00')::TIMESTAMPTZ, (d + TIME '16:30:00')::TIMESTAMPTZ, 650000.00);

        -- 2. Gambir -> Yogyakarta [Sovereign]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_taksaka, id_gmr, id_yk, (d + TIME '09:15:00')::TIMESTAMPTZ, (d + TIME '16:45:00')::TIMESTAMPTZ, 540000.00);

        -- 3. Bandung -> Solo Balapan [Noble]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_lodaya, id_bd, id_slo, (d + TIME '07:00:00')::TIMESTAMPTZ, (d + TIME '15:20:00')::TIMESTAMPTZ, 360000.00);

        -- 4. Yogyakarta -> Gambir [Noble]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_senja, id_yk, id_gmr, (d + TIME '18:30:00')::TIMESTAMPTZ, (d + 1 + TIME '02:00:00')::TIMESTAMPTZ, 380000.00);

        -- 5. Gambir -> Solo Balapan [Yeoman]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_matarmaja, id_gmr, id_slo, (d + TIME '10:30:00')::TIMESTAMPTZ, (d + TIME '19:45:00')::TIMESTAMPTZ, 195000.00);

        -- 6. Surabaya Gubeng -> Gambir [Yeoman]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_airlangga, id_sgu, id_gmr, (d + TIME '11:00:00')::TIMESTAMPTZ, (d + TIME '21:30:00')::TIMESTAMPTZ, 220000.00);
    END LOOP;
END $$;

-- 6. SEED SAMPLE BOOKING (Contoh Royal Pass / Sealed)
DO $$
DECLARE
    v_sched_id BIGINT;
    v_seat1 BIGINT;
    v_seat2 BIGINT;
    v_booking_id BIGINT;
BEGIN
    SELECT id INTO v_sched_id FROM public.schedules WHERE origin_id = (SELECT id FROM public.stations WHERE code = 'GMR') LIMIT 1;

    SELECT s.id INTO v_seat1 
    FROM public.seats s
    JOIN public.carriages c ON s.carriage_id = c.id
    JOIN public.schedules sc ON sc.train_id = c.train_id
    WHERE sc.id = v_sched_id ORDER BY s.id ASC LIMIT 1;

    SELECT s.id INTO v_seat2 
    FROM public.seats s
    JOIN public.carriages c ON s.carriage_id = c.id
    JOIN public.schedules sc ON sc.train_id = c.train_id
    WHERE sc.id = v_sched_id AND s.id <> v_seat1 ORDER BY s.id ASC LIMIT 1;

    INSERT INTO public.bookings (code, user_id, schedule_id, status, total_price, expires_at)
    VALUES ('AMB-ROYAL1', 'b0000000-0000-0000-0000-000000000002', v_sched_id, 'paid', 1300000.00, NOW() + INTERVAL '1 day')
    RETURNING id INTO v_booking_id;

    INSERT INTO public.booking_passengers (booking_id, schedule_id, seat_id, name, id_number, seat_active)
    VALUES 
        (v_booking_id, v_sched_id, v_seat1, 'Lord Noctis Lucis', '7371010101900001', TRUE),
        (v_booking_id, v_sched_id, v_seat2, 'Lady Lunafreya', '7371010101920002', TRUE);

    INSERT INTO public.payments (booking_id, method, status, amount, paid_at)
    VALUES (v_booking_id, 'tribute_crystal', 'completed', 1300000.00, NOW());
END $$;
```

---

## 7. Katalog Query Operasional Backend C++ (Crow REST API)

Berikut adalah query SQL terparameterisasi (`prepared statements` via `libpqxx`) untuk setiap endpoint C++:

### 7.1 Autentikasi Pengguna
#### Registrasi Pengguna (`POST /api/auth/register`)
```sql
INSERT INTO public.users (name, email, password_hash, role)
VALUES ($1, $2, $3, 'user')
RETURNING id, name, email, role, created_at;
```

#### Login Pengguna (`POST /api/auth/login`)
```sql
SELECT id, name, email, password_hash, role
FROM public.users
WHERE email = $1;
```

#### Ambil Profil User (`GET /api/auth/me`)
```sql
SELECT id, name, email, role, created_at
FROM public.users
WHERE id = $1;
```

---

### 7.2 Stasiun & Pencarian Jadwal
#### Daftar Seluruh Stasiun (`GET /api/stations`)
```sql
SELECT id, code, name, city
FROM public.stations
ORDER BY city ASC, name ASC;
```

#### Cari Jadwal + Hitung Sisa Kursi Dinamis (`GET /api/schedules?from=X&to=Y&date=Z&pax=N`)
Query ini menghitung sisa kursi secara real-time dengan mengurangkan total kursi kereta dengan kursi yang aktif terpesan (`held` belum kedaluwarsa atau `paid`):
```sql
SELECT 
    sc.id AS schedule_id,
    t.name AS train_name,
    t.class AS train_class,
    st_orig.name AS origin_name,
    st_orig.code AS origin_code,
    st_orig.city AS origin_city,
    st_dest.name AS destination_name,
    st_dest.code AS destination_code,
    st_dest.city AS destination_city,
    sc.departure_at,
    sc.arrival_at,
    (sc.arrival_at - sc.departure_at) AS duration,
    sc.price,
    -- Hitung sisa kursi aktual
    (
        SELECT COUNT(s.id)
        FROM public.seats s
        JOIN public.carriages c ON s.carriage_id = c.id
        WHERE c.train_id = t.id
    ) - (
        SELECT COUNT(bp.id)
        FROM public.booking_passengers bp
        JOIN public.bookings b ON bp.booking_id = b.id
        WHERE bp.schedule_id = sc.id
          AND bp.seat_active = TRUE
          AND (b.status = 'paid' OR (b.status = 'held' AND b.expires_at > NOW()))
    ) AS available_seats
FROM public.schedules sc
JOIN public.trains t ON sc.train_id = t.id
JOIN public.stations st_orig ON sc.origin_id = st_orig.id
JOIN public.stations st_dest ON sc.destination_id = st_dest.id
WHERE sc.origin_id = $1 
  AND sc.destination_id = $2
  AND sc.departure_at::DATE = $3::DATE
ORDER BY sc.departure_at ASC;
```

---

### 7.3 Visual Denah Kursi Gerbong (`GET /api/schedules/:id/seats`)
Query ini mengembalikan semua gerbong dan status setiap kursi (`available`, `held`, atau `booked`), memperhitungkan timer 15 menit:
```sql
SELECT 
    c.id AS carriage_id,
    c.carriage_number,
    s.id AS seat_id,
    s.seat_number,
    CASE 
        WHEN bp.id IS NULL THEN 'available'
        WHEN b.status = 'paid' THEN 'booked'
        WHEN b.status = 'held' AND b.expires_at > NOW() THEN 'held'
        ELSE 'available'
    END AS seat_status
FROM public.schedules sc
JOIN public.carriages c ON c.train_id = sc.train_id
JOIN public.seats s ON s.carriage_id = c.id
LEFT JOIN public.booking_passengers bp 
    ON bp.seat_id = s.id 
    AND bp.schedule_id = sc.id 
    AND bp.seat_active = TRUE
LEFT JOIN public.bookings b 
    ON bp.booking_id = b.id 
    AND b.status IN ('held', 'paid')
WHERE sc.id = $1
ORDER BY c.carriage_number ASC, s.seat_number ASC;
```

---

### 7.4 Alur Transaksi Pemesanan Tiket (Anti Double-Booking Transaction)
Dijalankan dalam satu transaksi database di service layer C++:

```sql
-- Langkah 1: Mulai Transaksi
BEGIN;

-- Langkah 2: Kunci baris kursi yang diminta (Row-Level Locking)
-- Mencegah transaksi lain mengubah data kursi secara bersamaan
SELECT id 
FROM public.seats 
WHERE id IN ($seat_id_1, $seat_id_2)
FOR UPDATE;

-- Langkah 3: Verifikasi apakah kursi masih tersedia (belum dipesan aktif)
SELECT bp.seat_id
FROM public.booking_passengers bp
JOIN public.bookings b ON bp.booking_id = b.id
WHERE bp.schedule_id = $schedule_id
  AND bp.seat_id IN ($seat_id_1, $seat_id_2)
  AND bp.seat_active = TRUE
  AND (b.status = 'paid' OR (b.status = 'held' AND b.expires_at > NOW()));
-- [LOGIKA C++]: Jika hasil query di atas > 0 baris, lakukan ROLLBACK dan balas HTTP 409 Conflict!

-- Langkah 4: Masukkan pesanan utama (Hold 15 menit)
INSERT INTO public.bookings (
    code, 
    user_id, 
    schedule_id, 
    status, 
    total_price, 
    expires_at
)
VALUES (
    $code,               -- Contoh: 'AMB-7K3QX9'
    $user_id, 
    $schedule_id, 
    'held', 
    $total_price, 
    NOW() + INTERVAL '15 minutes'
)
RETURNING id, code, status, total_price, expires_at;

-- Langkah 5: Masukkan masing-masing data penumpang dan kunci kursinya
-- Jika ada transaksi lain yang lolos, Partial Unique Index 'uq_active_seat_per_schedule'
-- akan melempar exception unik, membatalkan transaksi seketika.
INSERT INTO public.booking_passengers (
    booking_id, 
    schedule_id, 
    seat_id, 
    name, 
    id_number, 
    seat_active
)
VALUES 
    ($booking_id, $schedule_id, $seat_id_1, $name_1, $id_number_1, TRUE),
    ($booking_id, $schedule_id, $seat_id_2, $name_2, $id_number_2, TRUE);

-- Langkah 6: Komit Transaksi
COMMIT;
```

---

### 7.5 Pembayaran Simulasi Tribute (`POST /api/bookings/:id/pay`)
```sql
BEGIN;

-- 1. Validasi apakah booking masih dalam status 'held' dan belum kedaluwarsa
SELECT id, total_price, status, expires_at
FROM public.bookings
WHERE id = $booking_id AND user_id = $user_id
FOR UPDATE;

-- 2. Ubah status booking menjadi 'paid' (Sealed)
UPDATE public.bookings
SET status = 'paid',
    updated_at = NOW()
WHERE id = $booking_id 
  AND status = 'held'
  AND expires_at > NOW();

-- 3. Rekam jejak pembayaran Tribute
INSERT INTO public.payments (booking_id, method, status, amount, paid_at)
VALUES ($booking_id, $method, 'completed', $amount, NOW())
RETURNING id, booking_id, method, status, paid_at;

COMMIT;
```

---

### 7.6 Pembatalan Pesanan (`POST /api/bookings/:id/cancel`)
```sql
-- Saat status diubah menjadi 'cancelled', trigger trg_booking_status_sync_seats
-- secara otomatis akan mengubah booking_passengers.seat_active = FALSE
UPDATE public.bookings
SET status = 'cancelled',
    updated_at = NOW()
WHERE id = $booking_id 
  AND user_id = $user_id
  AND status IN ('held', 'paid')
RETURNING id, code, status;
```

---

### 7.7 Tampilan E-Ticket / Royal Pass (`GET /api/bookings/:code`)
```sql
SELECT 
    b.id AS booking_id,
    b.code AS sigil_code,
    b.status,
    b.total_price,
    b.created_at AS booked_at,
    b.expires_at,
    u.name AS user_name,
    u.email AS user_email,
    t.name AS train_name,
    t.class AS train_class,
    orig.name AS origin_station,
    orig.code AS origin_code,
    orig.city AS origin_city,
    dest.name AS destination_station,
    dest.code AS destination_code,
    dest.city AS destination_city,
    sc.departure_at,
    sc.arrival_at,
    p.method AS payment_method,
    p.paid_at,
    -- Agregasi daftar penumpang dan kursi sebagai format JSON
    COALESCE(
        json_agg(
            json_build_object(
                'passenger_name', bp.name,
                'id_number', bp.id_number,
                'carriage_number', c.carriage_number,
                'seat_number', s.seat_number
            )
        ) FILTER (WHERE bp.id IS NOT NULL), '[]'::json
    ) AS passengers
FROM public.bookings b
JOIN public.users u ON b.user_id = u.id
JOIN public.schedules sc ON b.schedule_id = sc.id
JOIN public.trains t ON sc.train_id = t.id
JOIN public.stations orig ON sc.origin_id = orig.id
JOIN public.stations dest ON sc.destination_id = dest.id
LEFT JOIN public.payments p ON p.booking_id = b.id
LEFT JOIN public.booking_passengers bp ON bp.booking_id = b.id
LEFT JOIN public.seats s ON bp.seat_id = s.id
LEFT JOIN public.carriages c ON s.carriage_id = c.id
WHERE b.code = $1 AND (b.user_id = $2 OR $is_admin = TRUE)
GROUP BY 
    b.id, b.code, b.status, b.total_price, b.created_at, b.expires_at,
    u.name, u.email, t.name, t.class, 
    orig.name, orig.code, orig.city,
    dest.name, dest.code, dest.city,
    sc.departure_at, sc.arrival_at, p.method, p.paid_at;
```

---

### 7.8 Kronik Riwayat Pemesanan User (`GET /api/bookings`)
```sql
SELECT 
    b.id,
    b.code AS sigil_code,
    b.status,
    b.total_price,
    b.created_at,
    b.expires_at,
    t.name AS train_name,
    t.class AS train_class,
    orig.city AS origin_city,
    dest.city AS destination_city,
    sc.departure_at,
    sc.arrival_at,
    COUNT(bp.id) AS passenger_count
FROM public.bookings b
JOIN public.schedules sc ON b.schedule_id = sc.id
JOIN public.trains t ON sc.train_id = t.id
JOIN public.stations orig ON sc.origin_id = orig.id
JOIN public.stations dest ON sc.destination_id = dest.id
LEFT JOIN public.booking_passengers bp ON bp.booking_id = b.id
WHERE b.user_id = $1
GROUP BY b.id, b.code, b.status, b.total_price, b.created_at, b.expires_at,
         t.name, t.class, orig.city, dest.city, sc.departure_at, sc.arrival_at
ORDER BY b.created_at DESC;
```

---

### 7.9 Panel Admin (The Conductor's Chamber)
#### Statistik Ringkasan Dashboard (`GET /api/admin/dashboard`)
```sql
SELECT 
    (SELECT COUNT(*) FROM public.bookings) AS total_bookings,
    (SELECT COUNT(*) FROM public.bookings WHERE status = 'paid') AS total_sealed_bookings,
    (SELECT COUNT(*) FROM public.bookings WHERE status = 'held' AND expires_at > NOW()) AS active_holds,
    (SELECT COALESCE(SUM(amount), 0) FROM public.payments WHERE status = 'completed') AS total_revenue,
    (SELECT COUNT(*) FROM public.users WHERE role = 'user') AS total_passengers;
```

#### Monitoring Semua Pesanan Terbaru (`GET /api/admin/bookings`)
```sql
SELECT 
    b.id,
    b.code,
    u.name AS passenger_account,
    u.email,
    b.status,
    b.total_price,
    sc.departure_at,
    t.name AS train_name,
    orig.code AS origin,
    dest.code AS destination,
    b.created_at
FROM public.bookings b
JOIN public.users u ON b.user_id = u.id
JOIN public.schedules sc ON b.schedule_id = sc.id
JOIN public.trains t ON sc.train_id = t.id
JOIN public.stations orig ON sc.origin_id = orig.id
JOIN public.stations dest ON sc.destination_id = dest.id
ORDER BY b.created_at DESC
LIMIT 50;
```

---

## 8. Panduan Eksekusi di Supabase Dashboard

1. **Buka Supabase Dashboard**: Pilih project kamu.
2. **Buka Menu SQL Editor**: Klik ikon terminal / SQL di sidebar kiri.
3. **Jalankan Schema**:
   - Salin isi dari berkas [`db/schema.sql`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/db/schema.sql)
   - Tempel ke SQL Editor lalu klik tombol **Run**.
4. **Jalankan Seed Data**:
   - Salin isi dari berkas [`db/seed.sql`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/db/seed.sql)
   - Tempel ke SQL Editor lalu klik tombol **Run**.
5. **Koneksi C++ (libpqxx)**:
   - Ambil connection string dari **Project Settings -> Database -> Connection Pooling**.
   - Pilih mode **Session (Port 5432)**.
   - Contoh format `.env`:
     ```env
     DATABASE_URL=postgresql://postgres.[PROJECT-REF]:[PASSWORD]@aws-0-[REGION].pooler.supabase.com:5432/postgres?sslmode=require
     ```

