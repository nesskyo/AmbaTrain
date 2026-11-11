-- ============================================================================
-- AMBATRAIN DATABASE SEED DATA (SUPABASE / POSTGRESQL)
-- Data Awal: Stasiun Nyata, Kereta Tematik 3 Kelas, Denah Kursi, Jadwal 14 Hari
-- ============================================================================

-- Bersihkan data lama jika ingin reset (Urutan menghapus memperhatikan Foreign Key)
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

-- ============================================================================
-- 1. SEED USERS (Pengguna Demo & Admin Conductor)
-- Password Default: 'Password123!' (Format bcrypt / dummy hash untuk demo)
-- ============================================================================
INSERT INTO public.users (id, name, email, password_hash, role) VALUES
    ('a0000000-0000-0000-0000-000000000001', 'High Conductor (Admin)', 'admin@ambatrain.com', '$2a$12$6K6m.Zg0sYq6Qo12fLwRveWbQyUoU9K4A1vFj1H7M3b0N8s4L5e2G', 'admin'),
    ('b0000000-0000-0000-0000-000000000002', 'Lord Noctis (Traveler)', 'traveler@ambatrain.com', '$2a$12$6K6m.Zg0sYq6Qo12fLwRveWbQyUoU9K4A1vFj1H7M3b0N8s4L5e2G', 'user'),
    ('c0000000-0000-0000-0000-000000000003', 'Lady Ophelia', 'ophelia@ambatrain.com', '$2a$12$6K6m.Zg0sYq6Qo12fLwRveWbQyUoU9K4A1vFj1H7M3b0N8s4L5e2G', 'user');

-- ============================================================================
-- 2. SEED STATIONS (Gerbang Nyata di Indonesia)
-- Sesuai PRD Bagian 10.6 & Desain Bagian 1.1
-- ============================================================================
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

-- ============================================================================
-- 3. SEED TRAINS (Steed of Steel - 3 Tingkatan Kelas)
-- sovereign: Eksekutif Megah | noble: Bisnis Elegan | yeoman: Ekonomi Rakyat
-- ============================================================================
INSERT INTO public.trains (name, class) VALUES
    ('Argo Bromo Sovereign', 'sovereign'),
    ('Taksaka Sovereign',    'sovereign'),
    ('Lodaya Noble',         'noble'),
    ('Senja Utama Noble',    'noble'),
    ('Matarmaja Yeoman',     'yeoman'),
    ('Airlangga Yeoman',     'yeoman');

-- ============================================================================
-- 4. SEED CARRIAGES & SEATS (Gerbong & Kursi)
-- Membuat 2 gerbong untuk setiap kereta, dan 20 kursi (Baris 1-5, Kolom A-D) per gerbong.
-- Total kursi per kereta: 40 kursi (Sangat ideal untuk demo & uji konkurensi).
-- ============================================================================
DO $$
DECLARE
    r_train RECORD;
    v_c_id BIGINT;
    v_c_num INT;
    v_row INT;
    v_col TEXT;
BEGIN
    FOR r_train IN SELECT id FROM public.trains LOOP
        -- Buat 2 gerbong per kereta
        FOR v_c_num IN 1..2 LOOP
            INSERT INTO public.carriages (train_id, carriage_number, seat_count)
            VALUES (r_train.id, v_c_num, 20)
            RETURNING id INTO v_c_id;

            -- Buat kursi: 1A, 1B, 1C, 1D ... sampai 5D
            FOR v_row IN 1..5 LOOP
                FOREACH v_col IN ARRAY ARRAY['A', 'B', 'C', 'D'] LOOP
                    INSERT INTO public.seats (carriage_id, seat_number)
                    VALUES (v_c_id, v_row || v_col);
                END LOOP;
            END LOOP;
        END LOOP;
    END LOOP;
END $$;

-- ============================================================================
-- 5. SEED SCHEDULES (Jadwal Otomatis 14 Hari ke Depan)
-- Memanfaatkan rentang tanggal dinamis CURRENT_DATE sampai CURRENT_DATE + 13 Hari
-- ============================================================================
DO $$
DECLARE
    i INT;
    d DATE;
    id_gmr BIGINT;
    id_bd  BIGINT;
    id_yk  BIGINT;
    id_sgu BIGINT;
    id_slo BIGINT;
    id_train_argo BIGINT;
    id_train_taksaka BIGINT;
    id_train_lodaya BIGINT;
    id_train_senja BIGINT;
    id_train_matarmaja BIGINT;
    id_train_airlangga BIGINT;
BEGIN
    SELECT id INTO id_gmr FROM public.stations WHERE code = 'GMR';
    SELECT id INTO id_bd  FROM public.stations WHERE code = 'BD';
    SELECT id INTO id_yk  FROM public.stations WHERE code = 'YK';
    SELECT id INTO id_sgu FROM public.stations WHERE code = 'SGU';
    SELECT id INTO id_slo FROM public.stations WHERE code = 'SLO';

    SELECT id INTO id_train_argo FROM public.trains WHERE name = 'Argo Bromo Sovereign';
    SELECT id INTO id_train_taksaka FROM public.trains WHERE name = 'Taksaka Sovereign';
    SELECT id INTO id_train_lodaya FROM public.trains WHERE name = 'Lodaya Noble';
    SELECT id INTO id_train_senja FROM public.trains WHERE name = 'Senja Utama Noble';
    SELECT id INTO id_train_matarmaja FROM public.trains WHERE name = 'Matarmaja Yeoman';
    SELECT id INTO id_train_airlangga FROM public.trains WHERE name = 'Airlangga Yeoman';

    -- Generate jadwal untuk 14 hari
    FOR i IN 0..13 LOOP
        d := CURRENT_DATE + i;

        -- 1. Rute Jakarta (Gambir) -> Surabaya Gubeng [Sovereign]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_train_argo, id_gmr, id_sgu, (d + TIME '08:00:00')::TIMESTAMPTZ, (d + TIME '16:30:00')::TIMESTAMPTZ, 650000.00);

        -- 2. Rute Jakarta (Gambir) -> Yogyakarta [Sovereign]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_train_taksaka, id_gmr, id_yk, (d + TIME '09:15:00')::TIMESTAMPTZ, (d + TIME '16:45:00')::TIMESTAMPTZ, 540000.00);

        -- 3. Rute Bandung -> Solo Balapan [Noble]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_train_lodaya, id_bd, id_slo, (d + TIME '07:00:00')::TIMESTAMPTZ, (d + TIME '15:20:00')::TIMESTAMPTZ, 360000.00);

        -- 4. Rute Yogyakarta -> Jakarta (Gambir) [Noble]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_train_senja, id_yk, id_gmr, (d + TIME '18:30:00')::TIMESTAMPTZ, (d + 1 + TIME '02:00:00')::TIMESTAMPTZ, 380000.00);

        -- 5. Rute Jakarta (Gambir) -> Malang via Solo [Yeoman]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_train_matarmaja, id_gmr, id_slo, (d + TIME '10:30:00')::TIMESTAMPTZ, (d + TIME '19:45:00')::TIMESTAMPTZ, 195000.00);

        -- 6. Rute Surabaya Gubeng -> Jakarta (Gambir) [Yeoman]
        INSERT INTO public.schedules (train_id, origin_id, destination_id, departure_at, arrival_at, price)
        VALUES (id_train_airlangga, id_sgu, id_gmr, (d + TIME '11:00:00')::TIMESTAMPTZ, (d + TIME '21:30:00')::TIMESTAMPTZ, 220000.00);
    END LOOP;
END $$;

-- ============================================================================
-- 6. SEED SAMPLE BOOKING (Contoh Booking Berhasil / Tersegel & Ditahan)
-- ============================================================================
DO $$
DECLARE
    v_sched_id BIGINT;
    v_seat1 BIGINT;
    v_seat2 BIGINT;
    v_booking_id BIGINT;
BEGIN
    -- Ambil satu jadwal contoh untuk hari ini
    SELECT id INTO v_sched_id FROM public.schedules WHERE origin_id = (SELECT id FROM public.stations WHERE code = 'GMR') LIMIT 1;

    -- Ambil dua kursi pertama dari gerbong kereta jadwal tersebut
    SELECT s.id INTO v_seat1 
    FROM public.seats s
    JOIN public.carriages c ON s.carriage_id = c.id
    JOIN public.schedules sc ON sc.train_id = c.train_id
    WHERE sc.id = v_sched_id
    ORDER BY s.id ASC LIMIT 1;

    SELECT s.id INTO v_seat2 
    FROM public.seats s
    JOIN public.carriages c ON s.carriage_id = c.id
    JOIN public.schedules sc ON sc.train_id = c.train_id
    WHERE sc.id = v_sched_id AND s.id <> v_seat1
    ORDER BY s.id ASC LIMIT 1;

    -- 1. Buat pesanan status 'paid' (Royal Pass / Sealed)
    INSERT INTO public.bookings (code, user_id, schedule_id, status, total_price, expires_at)
    VALUES ('AMB-ROYAL1', 'b0000000-0000-0000-0000-000000000002', v_sched_id, 'paid', 1300000.00, NOW() + INTERVAL '1 day')
    RETURNING id INTO v_booking_id;

    -- Pasang 2 penumpang
    INSERT INTO public.booking_passengers (booking_id, schedule_id, seat_id, name, id_number, seat_active)
    VALUES 
        (v_booking_id, v_sched_id, v_seat1, 'Lord Noctis Lucis', '7371010101900001', TRUE),
        (v_booking_id, v_sched_id, v_seat2, 'Lady Lunafreya', '7371010101920002', TRUE);

    -- Pasang pembayaran Tribute
    INSERT INTO public.payments (booking_id, method, status, amount, paid_at)
    VALUES (v_booking_id, 'tribute_crystal', 'completed', 1300000.00, NOW());
END $$;

