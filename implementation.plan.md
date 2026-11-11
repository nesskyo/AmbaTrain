# 🚂 AmbaTrain — Master Implementation Plan

> **Product:** AmbaTrain (Gothic Shakespearean Train Booking Platform)  
> **Tech Stack:** C++17/20 (Crow REST API, libpqxx, nlohmann/json, jwt-cpp, libsodium) + Supabase (PostgreSQL) + Vanilla JS / Tailwind CSS (Dark Gothic Theme)  
> **Target Outcome:** End-to-end booking flow, anti double-booking concurrency guarantee (0 collision under 50+ parallel requests), and complete documentation for academic evaluation.

---

## 🎯 Architecture Blueprint

```
[ Frontend: HTML5 / Tailwind / JRPG UI ]
                  │
                  ▼ (HTTPS / JSON + Bearer JWT)
[ Crow Web Framework (C++17/20) ]
   ├── Middleware: CORS, JWT Auth Guard, Rate Limiter
   ├── Controllers: Auth, Stations, Schedules, Bookings, Payments, Admin
   ├── Services: Business Logic, Seat Locking, Expiry Calculator, Hashing
   ├── Repositories: libpqxx Parameterized Queries (Prepared Statements)
   └── Models: C++ Structs + nlohmann::json serializers
                  │
                  ▼ (SSL Session Pooler: Port 5432)
[ Supabase Managed PostgreSQL ]
   ├── Schema: users, stations, trains, carriages, seats, schedules, bookings, booking_passengers, payments
   ├── Anti Double-Booking: Partial Unique Index `uq_active_seat_per_schedule`
   ├── Triggers: Auto updated_at & auto-release seats on cancel/expire
   └── Security: RLS active (PostgREST blocked, C++ backend bypasses via postgres role)
```

---

## 🗓️ 6-Phase Implementation Roadmap

### Phase 1: Foundation & Project Scaffolding
- [x] Analyze PRD & Design System (`PRD.md`, `desain.md`)
- [x] Database Schema DDL with Partial Unique Index (`db/schema.sql`)
- [x] Seed Data: 10 Indonesian Stations, 6 Trains (Sovereign/Noble/Yeoman), 14-day Dynamic Schedules (`db/seed.sql`)
- [x] Complete SQL Queries Catalog (`query.md`)
- [ ] Setup C++ Build System: `vcpkg.json`, `CMakeLists.txt`, `.env.example`
- [ ] Core Config & Database Connection Pooler wrapper (`libpqxx` connection manager)
- [ ] Baseline Endpoint: `GET /api/health` (Checks DB connectivity & latency)

### Phase 2: Authentication & Master Data Management
- [ ] Utilities: Password hashing & verification (`libsodium` / Argon2 / bcrypt)
- [ ] Utilities: JWT Token generation & verification (`jwt-cpp`)
- [ ] Middleware: `AuthMiddleware` (Extracts `Bearer <token>`, validates claims, attaches user identity to context)
- [ ] `AuthRepository` & `AuthService`:
  - `POST /api/auth/register` (Validate email format, unique check, hash password)
  - `POST /api/auth/login` (Verify hash, generate JWT with role & expiry)
  - `GET /api/auth/me` (Profile detail)
- [ ] `StationRepository` & `ScheduleRepository`:
  - `GET /api/stations` (List all origin/destination gates)
  - `GET /api/schedules?from=&to=&date=&pax=` (Dynamic real-time remaining seat calculation)

### Phase 3: Seat Layout & Anti Double-Booking Engine (Core P0)
- [ ] `SeatRepository`:
  - `GET /api/schedules/:id/seats` (Denah kursi gerbong dengan status `available`, `held`, `booked`)
- [ ] `BookingService` Transactional Lock:
  - Acquire row-level lock (`SELECT ... FOR UPDATE`)
  - Check active holds / paid seats
  - Insert `bookings` (status `'held'`, timer 15 menit)
  - Insert `booking_passengers` (`seat_active = TRUE`)
  - Catch PostgreSQL constraint violation (Error code `23505`) and emit HTTP `409 Conflict`
- [ ] Background Expiry Thread in C++ or PostgreSQL cron:
  - Invokes `SELECT public.expire_held_bookings()` every 60 seconds.

### Phase 4: Payment Simulation, E-Ticket (Royal Pass), & Chronicle
- [ ] Payment Simulation Endpoint (`POST /api/bookings/:id/pay`):
  - Verify booking is still within 15-minute hold window
  - Update status to `'paid'`
  - Insert `payments` record (`completed`)
- [ ] Cancellation Endpoint (`POST /api/bookings/:id/cancel`):
  - Update status to `'cancelled'`
  - DB Trigger automatically sets `seat_active = FALSE`
- [ ] E-Ticket / Royal Pass Endpoint (`GET /api/bookings/:code`):
  - Formats detailed ticket with Sigil code, train class, origin/destination, and passengers
- [ ] Chronicle / History Endpoint (`GET /api/bookings`):
  - List user's booking chronicles sorted by latest date

### Phase 5: Dark Gothic Shakespearean Frontend (Vanilla JS + Tailwind)
- [ ] Design Token Setup: Void, Obsidian, Midnight, Blood Crimson, Antique Gold, Crystal Azure
- [ ] Typography integration: Cinzel Decorative, Cinzel, Cormorant Garamond, Inter, JetBrains Mono
- [ ] Pages:
  - `index.html`: Hero Gothic Station, Journey Search Panel, Popular Routes
  - `schedules.html`: Route results, class filters (Sovereign/Noble/Yeoman), train cards
  - `seats.html`: Signature Interactive Carriage Blueprint with pulsing crystal seats
  - `checkout.html`: Passenger details & 15-minute countdown hourglass
  - `ticket.html`: Royal Pass parchment scroll with QR code representation
  - `chronicle.html`: Journey history quest log
  - `auth.html`: Enter the Realm / Swear Thy Oath (Login/Register)
  - `admin.html`: The Conductor's Chamber (Data monitoring & stats)

### Phase 6: Automated Concurrency Testing & Final Documentation
- [ ] Parallel Concurrency Test Script (Python / k6 / bash):
  - Simulates 50 simultaneous HTTP requests booking the exact same seat
  - Verifies: Exactly 1 receives HTTP `201 Created`, remaining 49 receive HTTP `409 Conflict`
- [ ] Expiry Verification Test:
  - Creates a held booking, waits or sets `HOLD_MINUTES=1`, confirms status transitions to `expired` and seat becomes available
- [ ] Dockerization: `Dockerfile` (Multi-stage C++ build) & `docker-compose.yml`
- [ ] Final Academic Report Deliverables (ERD diagram, API docs, Postman collection)

---

## 🔒 Concurrency Test Verification Script Outline

```python
# scripts/test_concurrency.py
import concurrent.futures
import requests

URL = "http://localhost:8080/api/bookings"
HEADERS = {"Authorization": "Bearer <TOKEN>", "Content-Type": "application/json"}
PAYLOAD = {
    "schedule_id": 1,
    "passengers": [{"name": "Tester", "id_number": "1234567890", "seat_id": 5}]
}

def book_seat():
    return requests.post(URL, json=PAYLOAD, headers=HEADERS).status_code

with concurrent.futures.ThreadPoolExecutor(max_workers=50) as executor:
    results = list(executor.map(lambda _: book_seat(), range(50)))

success_count = results.count(201)
conflict_count = results.count(409)

print(f"Results: Success (201) = {success_count}, Conflicts (409) = {conflict_count}")
assert success_count == 1, "Double booking bug detected!"
assert conflict_count == 49, "Concurrency handling failed!"
print("✅ Concurrency Test PASSED: Anti-Double-Booking verified!")
```
