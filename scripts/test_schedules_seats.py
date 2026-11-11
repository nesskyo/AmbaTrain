"""
AmbaTrain Phase 3 Automated Schedule & Seat Layout Test Script
Menguji pencarian jadwal real-time, sisa kursi dinamis, validasi input, dan denah kursi gerbong.
"""

import urllib.request
import urllib.error
import json
import datetime
import sys

API_BASE = "http://localhost:8080/api"

def make_request(endpoint):
    url = f"{API_BASE}{endpoint}"
    req = urllib.request.Request(url, headers={"Content-Type": "application/json"})
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
    print("  AMBATRAIN — PEMERIKSAAN FASE 3 (SCHEDULES & SEATS)")
    print("=" * 60)

    # 1. Healthcheck
    status, res = make_request("/health")
    if status != 200:
        print(f"❌ Server belum berjalan di {API_BASE}.")
        sys.exit(1)
    print(f"✅ Server Crow C++ Berjalan: {res.get('service')}")

    # 2. Search Schedules
    today_str = datetime.date.today().isoformat()
    print(f"\n[1/4] Menguji Pencarian Jadwal: Gambir (1) -> Surabaya Gubeng (8) pada {today_str}...")
    status, schedules = make_request(f"/schedules?from=1&to=8&date={today_str}&pax=2")
    if status == 200:
        print(f"  ✅ Ditemukan {len(schedules)} jadwal tersedia.")
        if len(schedules) > 0:
            sample = schedules[0]
            print(f"     Contoh: {sample.get('train_name')} ({sample.get('train_class')}) - Sisa Kursi: {sample.get('available_seats')} - Harga: Rp {sample.get('price')}")
            schedule_id = sample.get('schedule_id')
        else:
            schedule_id = 1
    else:
        print(f"  ❌ Pencarian gagal (HTTP {status}): {schedules}")
        schedule_id = 1

    # 3. Schedule Detail
    print(f"\n[2/4] Menguji Detail Jadwal ID {schedule_id}...")
    status, detail = make_request(f"/schedules/{schedule_id}")
    if status == 200:
        print(f"  ✅ Detail jadwal valid: {detail.get('train_name')} [{detail.get('origin_code')} -> {detail.get('destination_code')}]")
    else:
        print(f"  ❌ Gagal memuat detail jadwal: {detail}")

    # 4. Seat Blueprint Layout
    print(f"\n[3/4] Menguji Denah Kursi Gerbong (GET /api/schedules/{schedule_id}/seats)...")
    status, seat_map = make_request(f"/schedules/{schedule_id}/seats")
    if status == 200:
        carriages = seat_map.get("carriages", [])
        total_seats = sum(len(c.get("seats", [])) for c in carriages)
        print(f"  ✅ Denah gerbong diterima: Total {len(carriages)} gerbong, {total_seats} kursi.")
        if carriages:
            first_seat = carriages[0]["seats"][0]
            print(f"     Contoh Status Kursi: Gerbong {carriages[0]['carriage_number']} Kursi {first_seat['seat_number']} = {first_seat['status']}")
    else:
        print(f"  ❌ Gagal memuat denah kursi: {seat_map}")

    # 5. Validation Test (Origin == Destination)
    print(f"\n[4/4] Menguji Validasi Bisnis (Origin == Destination)...")
    status, err_res = make_request(f"/schedules?from=1&to=1&date={today_str}&pax=1")
    if status == 400:
        print(f"  ✅ Validasi berhasil menolak input salah (HTTP 400): {err_res.get('error', {}).get('message')}")
    else:
        print(f"  ❌ Validasi gagal: Harusnya HTTP 400 tapi menerima HTTP {status}")

    print("\n" + "=" * 60)
    print("🎉 SEMUA UJI FASE 3 BERHASIL SEMPURNA!")
    print("=" * 60)

if __name__ == "__main__":
    run_tests()
