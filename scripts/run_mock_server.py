"""
AmbaTrain Lightweight Mock Server (Python Standalone Companion)
Disediakan sebagai pelengkap demo tugas kuliah:
Dapat menjalankan API simulasi AmbaTrain di port 8080 jika C++ compiler belum siap.
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import urllib.parse
import datetime

HOST = "0.0.0.0"
PORT = 8080

# In-memory storage for mock demo
USERS = {
    "traveler@ambatrain.com": {
        "id": "b0000000-0000-0000-0000-000000000002",
        "name": "Lord Noctis (Traveler)",
        "email": "traveler@ambatrain.com",
        "role": "user"
    }
}
STATIONS = [
    {"id": 1, "code": "GMR", "name": "Stasiun Gambir", "city": "Jakarta"},
    {"id": 2, "code": "BD",  "name": "Stasiun Bandung", "city": "Bandung"},
    {"id": 6, "code": "YK",  "name": "Stasiun Yogyakarta (Tugu)", "city": "Yogyakarta"},
    {"id": 7, "code": "SLO", "name": "Stasiun Solo Balapan", "city": "Surakarta"},
    {"id": 8, "code": "SGU", "name": "Stasiun Surabaya Gubeng", "city": "Surabaya"}
]
BOOKINGS = {
    "AMB-ROYAL1": {
        "booking_id": 1,
        "sigil_code": "AMB-ROYAL1",
        "status": "paid",
        "total_price": 1300000.0,
        "booked_at": datetime.datetime.now().isoformat(),
        "origin": {"city": "Jakarta", "name": "Stasiun Gambir", "code": "GMR"},
        "destination": {"city": "Surabaya", "name": "Stasiun Surabaya Gubeng", "code": "SGU"},
        "train": {"name": "Argo Bromo Sovereign", "class": "sovereign"},
        "schedule": {
            "departure_at": (datetime.datetime.now() + datetime.timedelta(hours=2)).isoformat(),
            "arrival_at": (datetime.datetime.now() + datetime.timedelta(hours=10)).isoformat(),
        },
        "payment": {"method": "tribute_crystal", "paid_at": datetime.datetime.now().isoformat()},
        "passengers": [
            {"passenger_name": "Lord Noctis Lucis", "id_number": "7371010101900001", "carriage_number": 1, "seat_number": "1A"},
            {"passenger_name": "Lady Lunafreya", "id_number": "7371010101920002", "carriage_number": 1, "seat_number": "1B"}
        ]
    }
}

class AmbaTrainHandler(BaseHTTPRequestHandler):
    def _send_cors(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type, Authorization")

    def do_OPTIONS(self):
        self.send_response(204)
        self._send_cors()
        self.end_headers()

    def _json(self, status, data):
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self._send_cors()
        self.end_headers()
        self.wfile.write(json.dumps(data).encode("utf-8"))

    def do_GET(self):
        url = urllib.parse.urlparse(self.path)
        path = url.path
        qs = urllib.parse.parse_qs(url.query)

        if path == "/":
            self._json(200, {
                "realm": "AmbaTrain",
                "tagline": "All the world's a station, and all the men and women merely passengers.",
                "server": "Mock Python Companion Server for Demo"
            })
        elif path == "/api/health":
            self._json(200, {
                "status": "healthy",
                "service": "AmbaTrain Mock Engine",
                "database": {"status": "connected", "latency_ms": 1.2},
                "timestamp": datetime.datetime.now().isoformat()
            })
        elif path == "/api/stations":
            self._json(200, STATIONS)
        elif path == "/api/auth/me":
            self._json(200, USERS["traveler@ambatrain.com"])
        elif path == "/api/schedules":
            today = datetime.date.today().isoformat()
            self._json(200, [
                {
                    "schedule_id": 1,
                    "train_name": "Argo Bromo Sovereign",
                    "train_class": "sovereign",
                    "origin_name": "Stasiun Gambir",
                    "origin_code": "GMR",
                    "origin_city": "Jakarta",
                    "destination_name": "Stasiun Surabaya Gubeng",
                    "destination_code": "SGU",
                    "destination_city": "Surabaya",
                    "departure_at": f"{today}T08:00:00Z",
                    "arrival_at": f"{today}T16:30:00Z",
                    "duration": "08j 30m",
                    "price": 650000.0,
                    "available_seats": 38
                },
                {
                    "schedule_id": 2,
                    "train_name": "Taksaka Sovereign",
                    "train_class": "sovereign",
                    "origin_name": "Stasiun Gambir",
                    "origin_code": "GMR",
                    "origin_city": "Jakarta",
                    "destination_name": "Stasiun Yogyakarta",
                    "destination_code": "YK",
                    "destination_city": "Yogyakarta",
                    "departure_at": f"{today}T09:15:00Z",
                    "arrival_at": f"{today}T16:45:00Z",
                    "duration": "07j 30m",
                    "price": 540000.0,
                    "available_seats": 40
                }
            ])
        elif path.startswith("/api/schedules/") and path.endswith("/seats"):
            # Mock seat map
            carriages = []
            for c_num in [1, 2]:
                seats = []
                for r in range(1, 6):
                    for col in ["A", "B", "C", "D"]:
                        st = "available"
                        if c_num == 1 and r == 1 and col in ["A", "B"]:
                            st = "booked"
                        elif c_num == 1 and r == 2 and col == "C":
                            st = "held"
                        seats.append({
                            "seat_id": (c_num - 1) * 20 + (r - 1) * 4 + ord(col) - 64,
                            "seat_number": f"{r}{col}",
                            "status": st
                        })
                carriages.append({"carriage_id": c_num, "carriage_number": c_num, "seats": seats})
            self._json(200, {
                "schedule_id": 1,
                "train_name": "Argo Bromo Sovereign",
                "train_class": "sovereign",
                "carriages": carriages
            })
        elif path.startswith("/api/schedules/"):
            parts = path.split("/")
            sched_id = parts[-1]
            self._json(200, {
                "schedule_id": int(sched_id),
                "train_name": "Argo Bromo Sovereign",
                "train_class": "sovereign",
                "origin_name": "Stasiun Gambir",
                "origin_code": "GMR",
                "destination_name": "Stasiun Surabaya Gubeng",
                "destination_code": "SGU",
                "price": 650000.0
            })
        elif path.startswith("/api/bookings/") and not path.endswith("/pay") and not path.endswith("/cancel"):
            sigil = path.split("/")[-1]
            if sigil in BOOKINGS:
                self._json(200, BOOKINGS[sigil])
            else:
                self._json(404, {"error": {"code": "NOT_FOUND", "message": "Tiket tidak ditemukan."}})
        elif path == "/api/bookings":
            list_res = []
            for b in BOOKINGS.values():
                list_res.append({
                    "booking_id": b["booking_id"],
                    "sigil_code": b["sigil_code"],
                    "status": b["status"],
                    "total_price": b["total_price"],
                    "booked_at": b["booked_at"],
                    "train_name": b["train"]["name"],
                    "train_class": b["train"]["class"],
                    "origin_city": b["origin"]["city"],
                    "destination_city": b["destination"]["city"],
                    "departure_at": b["schedule"]["departure_at"],
                    "arrival_at": b["schedule"]["arrival_at"],
                    "passenger_count": len(b.get("passengers", []))
                })
            self._json(200, list_res)
        else:
            self._json(404, {"error": {"code": "NOT_FOUND", "message": "Endpoint tiada ditemukan."}})

    def do_POST(self):
        url = urllib.parse.urlparse(self.path)
        path = url.path
        length = int(self.headers.get("content-length", 0))
        body = json.loads(self.rfile.read(length).decode("utf-8")) if length > 0 else {}

        if path == "/api/auth/login" or path == "/api/auth/register":
            self._json(200 if path.endswith("login") else 201, {
                "token": "mock-sigil-jwt-token-for-demo-purposes",
                "user": USERS["traveler@ambatrain.com"]
            })
        elif path == "/api/bookings":
            import random
            code = "AMB-" + "".join(random.choices("23456789ABCDEFGHJKLMNPQRSTUVWXYZ", k=6))
            pax = body.get("passengers", [])
            new_b = {
                "booking_id": len(BOOKINGS) + 1,
                "sigil_code": code,
                "status": "held",
                "total_price": 650000.0 * max(1, len(pax)),
                "booked_at": datetime.datetime.now().isoformat(),
                "expires_at": (datetime.datetime.now() + datetime.timedelta(minutes=15)).isoformat(),
                "origin": {"city": "Jakarta", "name": "Stasiun Gambir", "code": "GMR"},
                "destination": {"city": "Surabaya", "name": "Stasiun Surabaya Gubeng", "code": "SGU"},
                "train": {"name": "Argo Bromo Sovereign", "class": "sovereign"},
                "schedule": {
                    "departure_at": (datetime.datetime.now() + datetime.timedelta(hours=2)).isoformat(),
                    "arrival_at": (datetime.datetime.now() + datetime.timedelta(hours=10)).isoformat(),
                },
                "payment": {"method": "unpaid", "paid_at": None},
                "passengers": [
                    {"passenger_name": p.get("name", "Traveler"), "id_number": p.get("id_number", "123"), "carriage_number": 1, "seat_number": f"{p.get('seat_id', 1)}A"}
                    for p in pax
                ]
            }
            BOOKINGS[code] = new_b
            self._json(201, {
                "booking": {
                    "id": new_b["booking_id"],
                    "code": code,
                    "status": "held",
                    "total_price": new_b["total_price"],
                    "expires_at": new_b["expires_at"]
                }
            })
        elif path.endswith("/pay"):
            b_id = int(path.split("/")[-2])
            for b in BOOKINGS.values():
                if b["booking_id"] == b_id:
                    b["status"] = "paid"
                    b["payment"]["method"] = body.get("method", "tribute_crystal")
                    b["payment"]["paid_at"] = datetime.datetime.now().isoformat()
                    return self._json(200, {"payment": b["payment"], "message": "Sealed"})
            self._json(404, {"error": "Booking not found"})
        elif path.endswith("/cancel"):
            b_id = int(path.split("/")[-2])
            for b in BOOKINGS.values():
                if b["booking_id"] == b_id:
                    b["status"] = "cancelled"
                    return self._json(200, {"message": "Cancelled"})
            self._json(404, {"error": "Booking not found"})
        else:
            self._json(404, {"error": "Not Found"})

if __name__ == "__main__":
    print(f"🚀 AmbaTrain Mock Server berjalan di http://localhost:{PORT}")
    print("   (Gunakan ini jika ingin demo frontend instan tanpa kompilasi C++)")
    server = HTTPServer((HOST, PORT), AmbaTrainHandler)
    server.serve_forever()

