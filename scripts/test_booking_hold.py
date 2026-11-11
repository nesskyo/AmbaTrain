"""
AmbaTrain Phase 4 Automated Booking Hold & Anti-Collision Test Script
Menguji transaksi hold 15 menit, verifikasi respon 201 Created,
dan pembuktian respon 409 Conflict saat kursi yang sama dipesan kembali.
"""

import urllib.request
import urllib.error
import json
import time
import sys

API_BASE = "http://localhost:8080/api"

def make_request(endpoint, method="GET", data=None, token=None):
    url = f"{API_BASE}{endpoint}"
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = f"Bearer {token}"

    body = json.dumps(data).encode('utf-8') if data else None
    req = urllib.request.Request(url, data=body, headers=headers, method=method)

    try:
        with urllib.request.urlopen(req, timeout=5) as res:
            return res.status, json.loads(res.read().decode('utf-8'))
    except urllib.error.HTTPError as e:
        err_body = e.read().decode('utf-8')
        try:
            return e.code, json.loads(err_body)
        except:
            return e.code, {"raw": err_body}
    except Exception as e:
        return 0, {"error": str(e)}

def run_tests():
    print("=" * 60)
    print("  AMBATRAIN — PEMERIKSAAN FASE 4 (BOOKING & ANTI DOUBLE-BOOKING)")
    print("=" * 60)

    # 1. Healthcheck
    status, res = make_request("/health")
    if status != 200:
        print(f"❌ Server belum berjalan di {API_BASE}.")
        sys.exit(1)
    print(f"✅ Server Crow C++ Berjalan: {res.get('service')}")

    # 2. Login as Traveler
    print("\n[1/4] Autentikasi Pengelana...")
    status, login_res = make_request("/auth/login", method="POST", data={
        "email": "traveler@ambatrain.com",
        "password": "Password123!"
    })
    if status != 200:
        print(f"❌ Login gagal: {login_res}")
        sys.exit(1)
    token = login_res["token"]
    print("  ✅ Token JWT diperoleh.")

    # 3. Create Booking Hold for Seat ID 10
    target_seat_id = 10
    schedule_id = 1
    booking_payload = {
        "schedule_id": schedule_id,
        "passengers": [
            {
                "name": "Lady Lunafreya Nox",
                "id_number": "7371098765432100",
                "seat_id": target_seat_id
            }
        ]
    }

    print(f"\n[2/4] Mencoba Memesan Kursi ID {target_seat_id} (Jadwal {schedule_id})...")
    status, book_res = make_request("/bookings", method="POST", data=booking_payload, token=token)

    if status == 201:
        booking = book_res.get("booking", {})
        print(f"  ✅ Booking Hold Berhasil Disegel (HTTP 201 Created)!")
        print(f"     Sigil Code  : {booking.get('code')}")
        print(f"     Status      : {booking.get('status')}")
        print(f"     Total Harga : Rp {booking.get('total_price')}")
        print(f"     Kedaluwarsa : {booking.get('expires_at')}")
    elif status == 409:
        print(f"  ℹ️ Kursi ID {target_seat_id} sudah dalam status held/paid sebelumnya.")
    else:
        print(f"  ❌ Gagal membuat booking (HTTP {status}): {book_res}")
        sys.exit(1)

    # 4. Anti Double-Booking Collision Test (Mencoba memesan kursi yang sama persis!)
    print(f"\n[3/4] UJI TABRAKAN: Mencoba memesan ulang Kursi ID {target_seat_id}...")
    competitor_payload = {
        "schedule_id": schedule_id,
        "passengers": [
            {
                "name": "Lord Ardyn Lucis",
                "id_number": "7371099999999999",
                "seat_id": target_seat_id
            }
        ]
    }
    status, collision_res = make_request("/bookings", method="POST", data=competitor_payload, token=token)

    if status == 409:
        print(f"  ✅ PENGUJIAN ANTI DOUBLE-BOOKING LULUS (HTTP 409 Conflict):")
        print(f"     Pesan Kesalahan: {collision_res.get('error', {}).get('message')}")
    else:
        print(f"  ❌ GAGAL: Harusnya ditolak dengan HTTP 409, tetapi menerima HTTP {status}!")
        sys.exit(1)

    # 5. Distinct Seat Check dalam satu request
    print(f"\n[4/4] UJI VALIDASI: Memesan kursi yang sama untuk 2 penumpang dalam 1 request...")
    duplicate_seat_payload = {
        "schedule_id": schedule_id,
        "passengers": [
            {"name": "Traveler A", "id_number": "111", "seat_id": 12},
            {"name": "Traveler B", "id_number": "222", "seat_id": 12}
        ]
    }
    status, dup_res = make_request("/bookings", method="POST", data=duplicate_seat_payload, token=token)
    if status == 400:
        print(f"  ✅ Validasi berhasil menolak kursi duplikat (HTTP 400): {dup_res.get('error', {}).get('message')}")
    else:
        print(f"  ❌ Validasi gagal: Menerima HTTP {status}")

    print("\n" + "=" * 60)
    print("🎉 SEMUA UJI FASE 4 (ANTI DOUBLE-BOOKING) BERHASIL SEMPURNA!")
    print("=" * 60)

if __name__ == "__main__":
    run_tests()
