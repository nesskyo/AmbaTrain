// AmbaTrain Global Frontend Configuration
// Mendukung localhost dan backend yang di-deploy (misal: Render/Railway/Fly.io)

const urlParamApi = new URLSearchParams(window.location.search).get('api');
if (urlParamApi) {
    localStorage.setItem('ambatrain_api_base', urlParamApi);
}

window.API_BASE = localStorage.getItem('ambatrain_api_base') || 'http://localhost:8080/api';

console.log('[AmbaTrain] Connected to API Engine:', window.API_BASE);

