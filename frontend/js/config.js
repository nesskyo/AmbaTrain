// AmbaTrain Global Frontend Configuration
// Terhubung otomatis ke Live Render Cloud Backend

const urlParamApi = new URLSearchParams(window.location.search).get('api');
if (urlParamApi) {
    let clean = urlParamApi.trim();
    if (clean.endsWith('/')) clean = clean.slice(0, -1);
    if (!clean.endsWith('/api')) clean += '/api';
    localStorage.setItem('ambatrain_api_base', clean);
}

// Default otomatis ke Live Render Backend yang sudah aktif!
window.API_BASE = localStorage.getItem('ambatrain_api_base') || 'https://ambatrain.onrender.com/api';

window.promptApiUrl = function() {
    const current = window.API_BASE;
    const newUrl = prompt(
        'URL Backend aktif saat ini:\n' + current + '\n\nMasukkan URL baru jika ingin mengubah:',
        current
    );
    if (newUrl && newUrl.trim()) {
        let clean = newUrl.trim();
        if (clean.endsWith('/')) clean = clean.slice(0, -1);
        if (!clean.endsWith('/api')) clean += '/api';
        localStorage.setItem('ambatrain_api_base', clean);
        alert('URL Backend berhasil disimpan ke: ' + clean);
        window.location.reload();
    }
};

console.log('[AmbaTrain] Connected to API Engine:', window.API_BASE);
