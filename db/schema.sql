-- ============================================================================
-- AMBATRAIN DATABASE DDL SCHEMA (SUPABASE / POSTGRESQL)
-- Project: AmbaTrain - Web Pemesanan Tiket Kereta Api Gothic Shakespearean
-- Dokumen Terkait: PRD.md & desain.md
-- ============================================================================

-- Ekstensi PostgreSQL yang dibutuhkan
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
CREATE EXTENSION IF NOT EXISTS "pgcrypto";

-- ============================================================================
-- 1. TABEL UTAMA (CORE TABLES)
-- ============================================================================

-- Tabel: users (Para Pengelana / Traveler & Admin / Conductor)
CREATE TABLE IF NOT EXISTS public.users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    role VARCHAR(50) NOT NULL DEFAULT 'user' CHECK (role IN ('user', 'admin')),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Tabel: stations (Gerbang Stasiun Dunia Nyata)
CREATE TABLE IF NOT EXISTS public.stations (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    code VARCHAR(10) NOT NULL UNIQUE,
    name VARCHAR(100) NOT NULL,
    city VARCHAR(100) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Tabel: trains (Lokomotif / Steed of Steel)
-- Kelas sesuai desain.md:
--   sovereign : Eksekutif
--   noble     : Bisnis
--   yeoman    : Ekonomi
CREATE TABLE IF NOT EXISTS public.trains (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    class VARCHAR(50) NOT NULL CHECK (class IN ('sovereign', 'noble', 'yeoman')),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Tabel: carriages (Gerbong Kereta)
CREATE TABLE IF NOT EXISTS public.carriages (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    train_id BIGINT NOT NULL REFERENCES public.trains(id) ON DELETE CASCADE,
    carriage_number INT NOT NULL,
    seat_count INT NOT NULL DEFAULT 0,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_train_carriage UNIQUE (train_id, carriage_number)
);

-- Tabel: seats (Kursi Kereta / Sigil Kursi)
CREATE TABLE IF NOT EXISTS public.seats (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    carriage_id BIGINT NOT NULL REFERENCES public.carriages(id) ON DELETE CASCADE,
    seat_number VARCHAR(10) NOT NULL, -- Contoh: 1A, 1B, 2C, 3D
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_carriage_seat UNIQUE (carriage_id, seat_number)
);

-- Tabel: schedules (Jadwal Keberangkatan)
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

-- Tabel: bookings (Pesanan Tiket / Segel Takdir)
-- Status sesuai PRD.md:
--   held      : Ditahan 15 menit (Awaiting Tribute)
--   paid      : Terbayar / Tersegel (Sealed)
--   cancelled : Dibatalkan (Forsaken)
--   expired   : Kedaluwarsa (Faded to Dust)
CREATE TABLE IF NOT EXISTS public.bookings (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    code VARCHAR(20) NOT NULL UNIQUE, -- Sigil unik (misal: 'AMB-7K3QX9')
    user_id UUID NOT NULL REFERENCES public.users(id) ON DELETE RESTRICT,
    schedule_id BIGINT NOT NULL REFERENCES public.schedules(id) ON DELETE RESTRICT,
    status VARCHAR(20) NOT NULL DEFAULT 'held' CHECK (status IN ('held', 'paid', 'cancelled', 'expired')),
    total_price NUMERIC(12, 2) NOT NULL DEFAULT 0 CHECK (total_price >= 0),
    expires_at TIMESTAMPTZ NOT NULL DEFAULT (NOW() + INTERVAL '15 minutes'),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Tabel: booking_passengers (Daftar Penumpang & Kursi Pesanan)
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

-- Tabel: payments (Tribute / Pembayaran Simulasi)
CREATE TABLE IF NOT EXISTS public.payments (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    booking_id BIGINT NOT NULL UNIQUE REFERENCES public.bookings(id) ON DELETE CASCADE,
    method VARCHAR(50) NOT NULL, -- qris, bank_transfer, credit_card, tribute_crystal
    status VARCHAR(20) NOT NULL DEFAULT 'pending' CHECK (status IN ('pending', 'completed', 'failed', 'refunded')),
    amount NUMERIC(12, 2) NOT NULL CHECK (amount >= 0),
    paid_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- ============================================================================
-- 2. ANTI DOUBLE-BOOKING & INDEKS PERFORMA
-- ============================================================================

-- [PENTING] Kunci Anti Double-Booking: Partial Unique Index
-- Menjamin satu kursi di satu jadwal hanya bisa aktif di 1 pesanan (held/paid).
-- Jika status booking menjadi cancelled/expired dan seat_active = false, kursi langsung terbebas.
CREATE UNIQUE INDEX IF NOT EXISTS uq_active_seat_per_schedule
    ON public.booking_passengers (schedule_id, seat_id)
    WHERE seat_active = TRUE;

-- Indeks Pencarian Jadwal (Filter from, to, date)
CREATE INDEX IF NOT EXISTS idx_schedules_search 
    ON public.schedules (origin_id, destination_id, departure_at);

-- Indeks Riwayat Booking per User (Kronik)
CREATE INDEX IF NOT EXISTS idx_bookings_user_history 
    ON public.bookings (user_id, created_at DESC);

-- Indeks Pencarian Kode Booking Sigil
CREATE INDEX IF NOT EXISTS idx_bookings_code 
    ON public.bookings (code);

-- Indeks Pembersihan Expiry Booking (Background Cleaner)
CREATE INDEX IF NOT EXISTS idx_bookings_expiry 
    ON public.bookings (status, expires_at) 
    WHERE status = 'held';

-- Indeks Pengecekan Denah Kursi Aktif
CREATE INDEX IF NOT EXISTS idx_booking_passengers_active 
    ON public.booking_passengers (schedule_id) 
    WHERE seat_active = TRUE;

-- Indeks Relasi Relasional
CREATE INDEX IF NOT EXISTS idx_seats_carriage 
    ON public.seats (carriage_id);

CREATE INDEX IF NOT EXISTS idx_carriages_train 
    ON public.carriages (train_id);

-- ============================================================================
-- 3. STORED FUNCTIONS & AUTOMATION TRIGGERS
-- ============================================================================

-- Fungsi & Trigger: Otomatis perbarui updated_at
CREATE OR REPLACE FUNCTION public.set_current_timestamp_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS trg_users_updated_at ON public.users;
CREATE TRIGGER trg_users_updated_at
BEFORE UPDATE ON public.users
FOR EACH ROW EXECUTE FUNCTION public.set_current_timestamp_updated_at();

DROP TRIGGER IF EXISTS trg_bookings_updated_at ON public.bookings;
CREATE TRIGGER trg_bookings_updated_at
BEFORE UPDATE ON public.bookings
FOR EACH ROW EXECUTE FUNCTION public.set_current_timestamp_updated_at();

-- Fungsi & Trigger: Sinkronisasi Otomatis seat_active saat status booking berubah
-- Saat status menjadi 'cancelled' atau 'expired', kursi langsung dilepas secara otomatis.
CREATE OR REPLACE FUNCTION public.sync_booking_passenger_seat_status()
RETURNS TRIGGER AS $$
BEGIN
    IF NEW.status IN ('cancelled', 'expired') AND OLD.status NOT IN ('cancelled', 'expired') THEN
        UPDATE public.booking_passengers
        SET seat_active = FALSE
        WHERE booking_id = NEW.id;
    ELSIF NEW.status IN ('held', 'paid') AND OLD.status NOT IN ('held', 'paid') THEN
        UPDATE public.booking_passengers
        SET seat_active = TRUE
        WHERE booking_id = NEW.id;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS trg_booking_status_sync_seats ON public.bookings;
CREATE TRIGGER trg_booking_status_sync_seats
AFTER UPDATE OF status ON public.bookings
FOR EACH ROW EXECUTE FUNCTION public.sync_booking_passenger_seat_status();

-- Fungsi: Pembebasan Booking Kedaluwarsa (Hold Expiry Worker)
-- Dapat dipanggil periodik via cron, pg_cron di Supabase, atau thread timer C++.
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

    -- seat_active pada booking_passengers otomatis diubah jadi FALSE oleh trigger trg_booking_status_sync_seats!
    RETURN v_count;
END;
$$ LANGUAGE plpgsql;

-- ============================================================================
-- 4. ROW LEVEL SECURITY (RLS) UNTUK KEAMANAN SUPABASE
-- Sesuai PRD Bagian 10.4:
-- Backend C++ terhubung menggunakan role `postgres` (superuser/service)
-- sehingga bebas dari RLS.
-- RLS diaktifkan tanpa policy publik agar PostgREST API publik Supabase
-- TIDAK BISA mengakses atau memanipulasi database secara langsung.
-- ============================================================================

ALTER TABLE public.users ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.stations ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.trains ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.carriages ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.seats ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.schedules ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.bookings ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.booking_passengers ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.payments ENABLE ROW LEVEL SECURITY;

-- Selesai Schema DDL

