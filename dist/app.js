const { invoke } = window.__TAURI__.core;
const { listen } = window.__TAURI__.event;

let currentAccount = null;
let installedVersions = [];

// ========== INIT ==========
document.addEventListener('DOMContentLoaded', async () => {
    setupNavigation();
    setupButtons();
    setupDropZones();
    setupSettings();
    await checkLogin();
});

// ========== AUTH ==========
async function checkLogin() {
    try {
        const accounts = await invoke('get_accounts');
        if (accounts.length > 0) {
            currentAccount = accounts[0];
            showDashboard();
        }
    } catch (e) {
        console.log('No accounts found');
    }
}

let authListener = null;

async function login() {
    const btn = document.getElementById('login-btn');
    const status = document.getElementById('login-status');
    btn.disabled = true;
    status.classList.remove('hidden');
    status.style.color = '#00ff88';
    status.innerHTML = 'Requesting login code...';

    // Listen for auth messages from backend
    if (authListener) authListener();
    authListener = await listen('auth-message', (event) => {
        const msg = event.payload;
        if (msg.message_type === 'error') {
            status.style.color = '#ff4466';
            status.textContent = msg.data;
        } else if (msg.message_type === 'success') {
            status.style.color = '#00ff88';
            status.innerHTML = 'Welcome, <strong>' + msg.data + '</strong>!';
        } else {
            // Status message - render with HTML for code display
            status.style.color = '#00ff88';
            status.innerHTML = msg.data.replace(/\n/g, '<br>');
        }
    });

    try {
        currentAccount = await invoke('authenticate');
        showDashboard();
    } catch (e) {
        status.textContent = 'Login failed: ' + e;
        status.style.color = '#ff4466';
    } finally {
        btn.disabled = false;
        if (authListener) { authListener(); authListener = null; }
    }
}

function showDashboard() {
    document.getElementById('login-screen').classList.remove('active');
    document.getElementById('dashboard').classList.add('active');
    document.getElementById('sidebar-username').textContent = currentAccount.username;
    document.getElementById('user-avatar').textContent = currentAccount.username[0].toUpperCase();
    document.getElementById('logout-btn').classList.remove('hidden');
    loadDashboard();
}

async function logout() {
    if (currentAccount) {
        await invoke('logout', { uuid: currentAccount.uuid });
    }
    currentAccount = null;
    document.getElementById('dashboard').classList.remove('active');
    document.getElementById('login-screen').classList.add('active');
    document.getElementById('login-status').classList.add('hidden');
}

async function offlineLogin() {
    const input = document.getElementById('offline-username');
    const status = document.getElementById('login-status');
    const username = input.value.trim();

    if (!username) {
        status.classList.remove('hidden');
        status.style.color = '#ff4466';
        status.textContent = 'Please enter a username';
        return;
    }

    try {
        currentAccount = await invoke('offline_login', { username });
        showDashboard();
    } catch (e) {
        status.classList.remove('hidden');
        status.style.color = '#ff4466';
        status.textContent = e;
    }
}

// ========== NAVIGATION ==========
function setupNavigation() {
    document.querySelectorAll('.nav-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            const section = btn.dataset.section;
            document.querySelectorAll('.section').forEach(s => s.classList.remove('active'));
            document.getElementById('section-' + section).classList.add('active');
            if (section === 'versions') loadVersions();
            if (section === 'mods') loadMods();
            if (section === 'resourcepacks') loadResourcePacks();
            if (section === 'shaders') loadShaders();
            if (section === 'settings') loadSettings();
        });
    });
}

// ========== BUTTONS ==========
function setupButtons() {
    document.getElementById('login-btn').addEventListener('click', login);
    document.getElementById('offline-login-btn').addEventListener('click', offlineLogin);
    document.getElementById('logout-btn').addEventListener('click', logout);
    document.getElementById('play-btn').addEventListener('click', launchGame);
    document.getElementById('rp-install-btn').addEventListener('click', () => installPack('resourcepack'));
    document.getElementById('shader-install-btn').addEventListener('click', () => installPack('shader'));
    document.getElementById('save-settings-btn').addEventListener('click', saveSettings);
    document.getElementById('client-id-save').addEventListener('click', saveClientId);
    document.getElementById('version-select').addEventListener('change', () => {
        if (document.getElementById('section-mods').classList.contains('active')) {
            loadMods();
        }
        updateHomeModsList();
    });
}

// ========== DASHBOARD ==========
async function loadDashboard() {
    try {
        installedVersions = await invoke('get_installed_versions');
    } catch (e) {
        installedVersions = [];
    }

    // Load versions for dropdown
    const select = document.getElementById('version-select');
    select.innerHTML = '';
    try {
        const versions = await invoke('get_versions');
        if (versions.length === 0) {
            throw new Error('No versions returned');
        }
        versions.forEach(v => {
            const opt = document.createElement('option');
            opt.value = v.id;
            opt.textContent = v.id;
            if (installedVersions.includes(v.id)) opt.textContent += ' (installed)';
            select.appendChild(opt);
        });
    } catch (e) {
        console.error('Failed to load versions:', e);
        const fallback = ['1.20.4', '1.20', '1.18.2', '1.18', '1.16.2', '1.12.2', '1.8.9', '1.8'];
        fallback.forEach(id => {
            const opt = document.createElement('option');
            opt.value = id;
            opt.textContent = id;
            if (installedVersions.includes(id)) opt.textContent += ' (installed)';
            select.appendChild(opt);
        });
    }

    // Load profiles
    try {
        const profiles = await invoke('get_profiles');
        const active = await invoke('get_active_profile');
        const select = document.getElementById('profile-select');
        select.innerHTML = '';
        profiles.forEach(p => {
            const opt = document.createElement('option');
            opt.value = p.id;
            opt.textContent = p.name;
            if (p.id === active) opt.selected = true;
            select.appendChild(opt);
        });

        select.addEventListener('change', async () => {
            await invoke('set_active_profile', { profileId: select.value });
        });
    } catch (e) {
        console.error('Failed to load profiles:', e);
    }

    // Load stats
    try {
        const javaPath = await invoke('get_java_path');
        document.getElementById('stat-java').textContent = javaPath.includes('java') ? 'Detected' : 'Not found';
    } catch (e) {}

    try {
        const settings = await invoke('get_settings');
        document.getElementById('stat-ram').textContent = (settings.ram_mb / 1024).toFixed(0) + ' GB';
    } catch (e) {}
}

// ========== VERSIONS ==========
async function loadVersions() {
    const grid = document.getElementById('versions-grid');
    grid.innerHTML = '<p style="color:var(--text-dim)">Loading versions...</p>';

    try {
        const versions = await invoke('get_versions');
        installedVersions = await invoke('get_installed_versions');
        grid.innerHTML = '';

        versions.forEach(v => {
            const installed = installedVersions.includes(v.id);
            const card = document.createElement('div');
            card.className = 'version-card' + (installed ? ' installed' : '');
            card.innerHTML = `
                <span class="version-id">${v.id}</span>
                <span class="version-date">${new Date(v.release_time).toLocaleDateString()}</span>
                <div class="version-actions">
                    ${installed
                        ? `<button class="btn-play-sm" onclick="launchVersion('${v.id}')">Play</button>
                           <button class="btn-danger" onclick="uninstallVersion('${v.id}')">Uninstall</button>`
                        : `<button class="btn-primary" onclick="installVersion('${v.id}')">Install</button>`
                    }
                </div>
            `;
            grid.appendChild(card);
        });
    } catch (e) {
        grid.innerHTML = '<p style="color:var(--red)">Failed to load versions: ' + e + '</p>';
    }
}

async function installVersion(versionId) {
    showDownload('Installing ' + versionId + '...');
    try {
        await invoke('install_version', { versionId });
        loadVersions();
        loadDashboard();
    } catch (e) {
        hideDownload();
        alert('Install failed: ' + e);
    }
}

async function uninstallVersion(versionId) {
    if (!confirm('Uninstall ' + versionId + '?')) return;
    try {
        await invoke('uninstall_version', { versionId });
        loadVersions();
        loadDashboard();
    } catch (e) {
        alert('Uninstall failed: ' + e);
    }
}

function launchVersion(versionId) {
    document.getElementById('version-select').value = versionId;
    launchGame();
}

// ========== LAUNCH ==========
async function launchGame() {
    if (!currentAccount) {
        alert('Please log in first');
        return;
    }

    const versionId = document.getElementById('version-select').value;
    if (!versionId) {
        alert('Please select a version');
        return;
    }

    if (!installedVersions.includes(versionId)) {
        if (confirm(versionId + ' is not installed. Install it now?')) {
            await installVersion(versionId);
        } else {
            return;
        }
    }

    const btn = document.getElementById('play-btn');
    btn.disabled = true;
    btn.querySelector('span').textContent = 'LAUNCHING...';

    try {
        const settings = await invoke('get_settings');
        await invoke('launch_game', {
            versionId,
            username: currentAccount.username,
            uuid: currentAccount.uuid,
            accessToken: currentAccount.access_token,
            ramMb: settings.ram_mb,
        });
    } catch (e) {
        alert('Launch failed: ' + e);
    } finally {
        btn.disabled = false;
        btn.querySelector('span').textContent = 'PLAY';
    }
}

// ========== MODS ==========
let currentModCategory = 'all';
let currentVersion = '';

async function loadMods() {
    const grid = document.getElementById('mods-grid');
    grid.innerHTML = '';

    try {
        const versionSelect = document.getElementById('version-select');
        currentVersion = versionSelect ? versionSelect.value : '';

        document.getElementById('mods-version-badge').textContent = currentVersion || 'All';

        const allMods = await invoke('get_mods_for_version', { versionId: currentVersion });
        const profiles = await invoke('get_profiles');
        const activeId = await invoke('get_active_profile');
        const activeProfile = profiles.find(p => p.id === activeId) || profiles[0];

        // Setup overlay toggle
        const overlayInstalled = await invoke('is_overlay_installed', { versionId: currentVersion });
        const overlayToggle = document.getElementById('overlay-toggle');
        const overlayCard = document.getElementById('overlay-toggle-card');
        overlayToggle.checked = overlayInstalled;
        overlayCard.classList.toggle('enabled', overlayInstalled);
        overlayToggle.onchange = async () => {
            if (overlayToggle.checked) {
                await invoke('install_overlay', { versionId: currentVersion });
                overlayCard.classList.add('enabled');
            } else {
                await invoke('remove_overlay', { versionId: currentVersion });
                overlayCard.classList.remove('enabled');
            }
        };

        renderProfileTabs(profiles, activeId);
        renderModGrid(allMods, activeProfile);
        setupCategoryTabs();
        setupAddProfile();
    } catch (e) {
        grid.innerHTML = '<p style="color:var(--red)">Failed to load mods: ' + e + '</p>';
    }
}

function renderProfileTabs(profiles, activeId) {
    const tabs = document.getElementById('profile-tabs');
    tabs.innerHTML = '';
    profiles.forEach(p => {
        const tab = document.createElement('button');
        tab.className = 'profile-tab' + (p.id === activeId ? ' active' : '');
        const canDelete = !['ultra-fps','competitive','quality','vanilla+'].includes(p.id);
        tab.innerHTML = `<span class="tab-icon">${p.icon || '★'}</span>${p.name}${canDelete ? `<button class="tab-close" data-pid="${p.id}">&times;</button>` : ''}`;
        tab.addEventListener('click', async (e) => {
            if (e.target.classList.contains('tab-close')) {
                e.stopPropagation();
                if (confirm('Delete profile "' + p.name + '"?')) {
                    await invoke('delete_profile', { profileId: e.target.dataset.pid });
                    await loadMods();
                }
                return;
            }
            await invoke('set_active_profile', { profileId: p.id });
            await loadMods();
        });
        tabs.appendChild(tab);
    });
}

function renderModGrid(allMods, activeProfile) {
    const grid = document.getElementById('mods-grid');
    grid.innerHTML = '';

    const filtered = currentModCategory === 'all' ? allMods : allMods.filter(m => m.category === currentModCategory);
    let activeCount = 0;

    filtered.forEach(mod => {
        const entry = activeProfile.mods.find(m => m.slug === mod.slug);
        const enabled = entry ? entry.enabled : false;
        if (enabled) activeCount++;

        const card = document.createElement('div');
        card.className = 'mod-card' + (enabled ? ' enabled' : '');
        card.innerHTML = `
            <div class="mod-card-icon cat-${mod.category}">${mod.icon}</div>
            <div class="mod-card-body">
                <div class="mod-card-top">
                    <span class="mod-card-name">${mod.name}</span>
                    <input type="checkbox" class="toggle" data-slug="${mod.slug}" ${enabled ? 'checked' : ''}>
                </div>
                <div class="mod-card-desc">${mod.description}</div>
                <span class="mod-card-category cat-${mod.category}">${mod.category}</span>
            </div>
        `;
        grid.appendChild(card);
    });

    grid.querySelectorAll('.toggle').forEach(toggle => {
        toggle.addEventListener('change', async () => {
            await invoke('toggle_mod', { modSlug: toggle.dataset.slug, enabled: toggle.checked });
            const card = toggle.closest('.mod-card');
            card.classList.toggle('enabled', toggle.checked);
            updateModsStats();
            updateHomeModsList();
        });
    });

    document.getElementById('mods-active-count').textContent = activeCount;
    document.getElementById('mods-total-count').textContent = filtered.length;
}

function setupCategoryTabs() {
    const tabs = document.getElementById('mod-category-tabs');
    tabs.querySelectorAll('.cat-tab').forEach(tab => {
        tab.addEventListener('click', async () => {
            tabs.querySelectorAll('.cat-tab').forEach(t => t.classList.remove('active'));
            tab.classList.add('active');
            currentModCategory = tab.dataset.cat;
            await loadMods();
        });
    });
}

function setupAddProfile() {
    document.getElementById('add-profile-btn').onclick = () => {
        const overlay = document.createElement('div');
        overlay.className = 'modal-overlay';
        overlay.innerHTML = `
            <div class="modal">
                <h3>New Profile</h3>
                <input type="text" id="new-profile-name" placeholder="Profile name" maxlength="30">
                <input type="text" id="new-profile-desc" placeholder="Description" maxlength="60">
                <div class="modal-actions">
                    <button class="btn-secondary" id="modal-cancel">Cancel</button>
                    <button class="btn-primary" id="modal-create">Create</button>
                </div>
            </div>
        `;
        document.body.appendChild(overlay);
        overlay.querySelector('#modal-cancel').onclick = () => overlay.remove();
        overlay.querySelector('#modal-create').onclick = async () => {
            const name = overlay.querySelector('#new-profile-name').value.trim();
            const desc = overlay.querySelector('#new-profile-desc').value.trim();
            if (name) {
                await invoke('create_profile', { name, description: desc || name });
                overlay.remove();
                await loadMods();
            }
        };
        overlay.addEventListener('click', (e) => { if (e.target === overlay) overlay.remove(); });
        overlay.querySelector('#new-profile-name').focus();
    };
}

function updateModsStats() {
    const toggles = document.querySelectorAll('#mods-grid .toggle');
    const active = Array.from(toggles).filter(t => t.checked).length;
    document.getElementById('mods-active-count').textContent = active;
}

async function updateHomeModsList() {
    try {
        const versionSelect = document.getElementById('version-select');
        const versionId = versionSelect ? versionSelect.value : '';
        const activeMods = await invoke('get_active_mods_for_version', { versionId });
        const el = document.getElementById('active-mods-list');
        if (el) {
            el.textContent = activeMods.map(m => m.name).join(', ') || 'None';
        }
    } catch (e) {}
}

// ========== RESOURCE PACKS & SHADERS ==========
async function loadResourcePacks() {
    const list = document.getElementById('rp-list');
    list.innerHTML = '';
    try {
        const packs = await invoke('get_resource_packs');
        if (packs.length === 0) {
            list.innerHTML = '<p style="color:var(--text-dim);padding:10px">No resource packs installed</p>';
            return;
        }
        packs.forEach(pack => {
            const item = document.createElement('div');
            item.className = 'pack-item';
            item.innerHTML = `
                <div>
                    <span class="pack-name">${pack.name}</span>
                    <span class="pack-size">${formatSize(pack.size)}</span>
                </div>
                <button class="btn-danger" onclick="removePack('resourcepack', '${pack.file_name}')">Remove</button>
            `;
            list.appendChild(item);
        });
    } catch (e) {
        list.innerHTML = '<p style="color:var(--red)">Error loading packs</p>';
    }
}

async function loadShaders() {
    const list = document.getElementById('shader-list');
    list.innerHTML = '';
    try {
        const packs = await invoke('get_shader_packs');
        if (packs.length === 0) {
            list.innerHTML = '<p style="color:var(--text-dim);padding:10px">No shader packs installed</p>';
            return;
        }
        packs.forEach(pack => {
            const item = document.createElement('div');
            item.className = 'pack-item';
            item.innerHTML = `
                <div>
                    <span class="pack-name">${pack.name}</span>
                    <span class="pack-size">${formatSize(pack.size)}</span>
                </div>
                <button class="btn-danger" onclick="removePack('shader', '${pack.file_name}')">Remove</button>
            `;
            list.appendChild(item);
        });
    } catch (e) {
        list.innerHTML = '<p style="color:var(--red)">Error loading shaders</p>';
    }
}

async function installPack(type) {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.zip';
    input.onchange = async (e) => {
        const file = e.target.files[0];
        if (!file) return;
        try {
            const path = file.path || file.name;
            if (type === 'resourcepack') {
                await invoke('install_resource_pack', { sourcePath: path });
                loadResourcePacks();
            } else {
                await invoke('install_shader', { sourcePath: path });
                loadShaders();
            }
        } catch (err) {
            alert('Failed to install: ' + err);
        }
    };
    input.click();
}

async function removePack(type, fileName) {
    try {
        if (type === 'resourcepack') {
            await invoke('remove_resource_pack', { fileName });
            loadResourcePacks();
        } else {
            await invoke('remove_shader', { fileName });
            loadShaders();
        }
    } catch (e) {
        alert('Failed to remove: ' + e);
    }
}

// ========== DROP ZONES ==========
function setupDropZones() {
    ['rp-drop-zone', 'shader-drop-zone'].forEach(id => {
        const zone = document.getElementById(id);
        zone.addEventListener('dragover', (e) => {
            e.preventDefault();
            zone.classList.add('dragover');
        });
        zone.addEventListener('dragleave', () => zone.classList.remove('dragover'));
        zone.addEventListener('drop', (e) => {
            e.preventDefault();
            zone.classList.remove('dragover');
            const type = id.includes('rp') ? 'resourcepack' : 'shader';
            const files = e.dataTransfer.files;
            if (files.length > 0) {
                handleDropFile(type, files[0]);
            }
        });
    });
}

async function handleDropFile(type, file) {
    try {
        const path = file.path || file.name;
        if (type === 'resourcepack') {
            await invoke('install_resource_pack', { sourcePath: path });
            loadResourcePacks();
        } else {
            await invoke('install_shader', { sourcePath: path });
            loadShaders();
        }
    } catch (e) {
        alert('Failed to install: ' + e);
    }
}

// ========== SETTINGS ==========
function setupSettings() {
    const ramSlider = document.getElementById('setting-ram');
    ramSlider.addEventListener('input', () => {
        document.getElementById('ram-display').textContent = ramSlider.value + ' GB';
    });
}

async function loadSettings() {
    try {
        const settings = await invoke('get_settings');
        document.getElementById('setting-java-path').value = settings.java_path || '';
        document.getElementById('setting-ram').value = settings.ram_mb / 1024;
        document.getElementById('ram-display').textContent = (settings.ram_mb / 1024) + ' GB';
        document.getElementById('setting-game-dir').value = settings.game_dir || '';
        document.getElementById('setting-window-size').value = settings.window_width + 'x' + settings.window_height;
    } catch (e) {}
    try {
        const clientId = await invoke('get_client_id');
        document.getElementById('setting-client-id').value = clientId;
    } catch (e) {}
}

async function saveClientId() {
    const clientId = document.getElementById('setting-client-id').value.trim();
    try {
        await invoke('set_client_id', { clientId });
        alert('Client ID saved!');
    } catch (e) {
        alert('Failed to save: ' + e);
    }
}

async function saveSettings() {
    const ramVal = parseInt(document.getElementById('setting-ram').value);
    const sizeParts = document.getElementById('setting-window-size').value.split('x');
    const settings = {
        java_path: document.getElementById('setting-java-path').value || null,
        ram_mb: ramVal * 1024,
        game_dir: document.getElementById('setting-game-dir').value || null,
        window_width: parseInt(sizeParts[0]),
        window_height: parseInt(sizeParts[1]),
    };
    try {
        await invoke('save_settings', { settings });
        alert('Settings saved!');
    } catch (e) {
        alert('Failed to save: ' + e);
    }
}

// ========== DOWNLOAD PROGRESS ==========
function showDownload(title) {
    document.getElementById('download-overlay').classList.remove('hidden');
    document.getElementById('download-title').textContent = title;
    document.getElementById('download-status').textContent = 'Starting...';
    document.getElementById('progress-fill').style.width = '0%';
}

function hideDownload() {
    document.getElementById('download-overlay').classList.add('hidden');
}

function formatSize(bytes) {
    if (bytes < 1024) return bytes + ' B';
    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
    return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
}

// ========== EVENT LISTENERS ==========
listen('download-progress', (event) => {
    const status = document.getElementById('download-status');
    if (status) status.textContent = event.payload;
});

listen('download-complete', () => {
    hideDownload();
    loadDashboard();
});

listen('game-launching', () => {
    const btn = document.getElementById('play-btn');
    btn.disabled = true;
    btn.querySelector('span').textContent = 'LAUNCHING...';
});

listen('game-launched', () => {
    const btn = document.getElementById('play-btn');
    btn.disabled = false;
    btn.querySelector('span').textContent = 'PLAY';
});
