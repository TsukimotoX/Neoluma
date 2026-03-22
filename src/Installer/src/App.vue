<script setup lang="ts">
import Titlebar from "./components/Titlebar.vue";
import neolumaLogo from "@/assets/neoluma.svg";
import { computed, onMounted, onUnmounted, ref, watch } from "vue";

import { invoke } from "@tauri-apps/api/core";
import { listen } from "@tauri-apps/api/event";
import { getCurrentWindow } from "@tauri-apps/api/window";

const win = getCurrentWindow();
const closeWin = async () => { await win.close(); };

type ProgressPayload = { stage: string; percent: number; current: string };

// ==== Wizard ====
const page = ref(0);
const total = 4;

// ==== Install options (backend decides defaults) ====
type InstallScope = "user" | "system";
const installScope = ref<InstallScope>("user");

const defaults = ref<{ userPath: string; systemPath: string; canSystemInstall: boolean } | null>(null);

const installPath = ref("");
const addToPath = ref(true);

// when scope flips -> snap to backend default
watch(installScope, (scope) => {
    if (!defaults.value) return;
    installPath.value = scope === "user" ? defaults.value.userPath : defaults.value.systemPath;
});

// ==== Install state ====
const installing = ref(false);
const installed = ref(false);
const errorText = ref("");

const progress = ref(0);
const stage = ref("Idle");
const currentFile = ref("");

// ==== UI computed ====
const track = computed(() => ({
    width: `${total * 100}vw`,
    transform: `translate3d(-${page.value * 100}vw, 0, 0)`,
}));

const dots = computed(() => Array.from({ length: total }, (_, i) => i));
const canGoBack = computed(() => !installing.value && page.value > 0);
const canGoNext = computed(() => !installing.value && page.value < total - 1);

const next = () => { if (canGoNext.value) page.value++; };
const back = () => { if (canGoBack.value) page.value--; };

// ==== Backend call ====
const doInstall = async () => {
    await invoke("install", {
        args: {
            scope: installScope.value,
            destination: installPath.value,
            addToPath: addToPath.value,
        },
    });
};

const restartAsAdmin = async () => {
    await invoke("restart_as_admin", {
        args: {
            scope: installScope.value,
            destination: installPath.value,
            addToPath: addToPath.value,
        },
    });
};

// ==== Flow ====
const startInstall = async () => {
    errorText.value = "";
    installing.value = true;
    installed.value = false;
    progress.value = 0;
    stage.value = "Starting…";
    currentFile.value = "";

    page.value = 2;

    try {
        const d = defaults.value;

        // если выбрали system, но текущий процесс не elevated -> перезапускаем с UAC
        if (installScope.value === "system" && d && !d.canSystemInstall) {
            stage.value = "Requesting Administrator…";
            await restartAsAdmin();
            // текущий процесс скоро закроется сам (backend app.exit)
            return;
        }

        await doInstall();
    } catch (e: any) {
        installing.value = false;
        errorText.value = String(e);
        page.value = 1;
    }
};

// ==== Events wiring ====
let unlistenProgress: null | (() => void) = null;
let unlistenDone: null | (() => void) = null;
let unlistenError: null | (() => void) = null;

onMounted(async () => {
    // subscribe first
    unlistenProgress = await listen<ProgressPayload>("install://progress", (e) => {
        stage.value = e.payload.stage;
        progress.value = Math.max(0, Math.min(100, e.payload.percent));
        currentFile.value = e.payload.current ?? "";
    });

    unlistenDone = await listen("install://done", () => {
        installing.value = false;
        installed.value = true;
        page.value = 3;
    });

    unlistenError = await listen<string>("install://error", (e) => {
        installing.value = false;
        errorText.value = e.payload || "Unknown error";
        page.value = 1;
    });

    // ask backend for default paths
    try {
        const d = await invoke<{ userPath: string; systemPath: string; canSystemInstall: boolean }>(
            "get_install_defaults"
        );

        defaults.value = d;
        installPath.value = d.userPath; // user by default

        const auto = await invoke<boolean>("should_autoinstall");
        if (auto) {
            // UI -> installing page
            errorText.value = "";
            installing.value = true;
            installed.value = false;
            progress.value = 0;
            stage.value = "Starting…";
            currentFile.value = "";
            page.value = 2;

            // backend -> start install job
            await invoke("start_autoinstall");
        }
    } catch (e: any) {
        errorText.value = "Failed to load install defaults: " + String(e);
    }
});

onUnmounted(() => {
    unlistenProgress?.(); unlistenDone?.(); unlistenError?.();
});
</script>

<template>
    <div class="shell">
        <Titlebar />

        <div class="pages" :style="track">
            <!-- 0 -->
            <section class="page">
                <div class="card hero">
                    <img :src="neolumaLogo" class="logo" alt="Neoluma" />
                    <div class="h1">Welcome to Neoluma ✨</div>
                    <div class="muted">Let’s install it for you — quickly and cleanly.</div>

                    <button class="btn primary" @click="next">Next</button>
                </div>
            </section>

            <!-- 1 -->
            <section class="page">
                <div class="card">
                    <div class="h1">Installation options</div>
                    <div class="muted">User-level install by default (no admin needed).</div>

                    <div class="form">
                        <label class="label">Install location</label>
                        <input class="input" v-model="installPath" placeholder="…" :disabled="installing" />

                        <label class="check">
                            <input type="checkbox" v-model="addToPath" :disabled="installing" />
                            <span>Add Neoluma to PATH</span>
                        </label>

                        <label class="label">Install scope</label>
                        <select class="input" v-model="installScope" :disabled="installing">
                            <option value="user">User (no admin)</option>
                            <option value="system">System (will ask admin)</option>
                        </select>

                        <div class="hint" v-if="installScope === 'system' && defaults && !defaults.canSystemInstall">
                            Windows will ask for Administrator permissions (UAC) ✅
                        </div>

                        <button
                            v-if="installScope === 'system' && defaults && !defaults.canSystemInstall"
                            class="btn primary"
                            @click="restartAsAdmin"
                            :disabled="installing"
                        >
                            Restart as Administrator
                        </button>

                        <div v-if="errorText" class="error">
                            <div class="errorTitle">Install failed 😵</div>
                            <div class="errorText">{{ errorText }}</div>
                        </div>

                        <div class="row">
                            <button class="btn ghost" @click="back" :disabled="!canGoBack">Back</button>
                            <button class="btn primary" @click="startInstall" :disabled="installing || !installPath">
                                Install
                            </button>
                        </div>
                    </div>
                </div>
            </section>

            <!-- 2 -->
            <section class="page">
                <div class="card">
                    <img :src="neolumaLogo" class="logo small" alt="Neoluma" />
                    <div class="h1">Installing…</div>
                    <div class="muted">
                        <span class="pill">{{ stage }}</span>
                        <span v-if="currentFile" class="mono"> — {{ currentFile }}</span>
                    </div>

                    <div class="progressWrap">
                        <div class="progressBar">
                            <div class="progressFill" :style="{ width: progress + '%' }"></div>
                        </div>
                        <div class="progressText">{{ Math.round(progress) }}%</div>
                    </div>

                    <button class="btn ghost" disabled>Wait…</button>

                    <div class="hint" v-if="addToPath && progress > 90">
                        After install, reopen your terminal to pick up PATH ✅
                    </div>
                </div>
            </section>

            <!-- 3 -->
            <section class="page">
                <div class="card hero">
                    <img :src="neolumaLogo" class="logo" alt="Neoluma" />
                    <div class="h1">Installation successful! 🎉</div>
                    <div class="muted">You can close this window.</div>
                    <button class="btn primary" @click="closeWin">Close</button>
                </div>
            </section>
        </div>

        <div class="stepper">
            <div
                v-for="i in dots"
                :key="i"
                class="dot"
                :class="{ active: i === page, done: i < page }"
            />
        </div>
    </div>
</template>

<style>
:root{
    --bg0:#0b0b10;
    --bg1:#12121c;
    --card:#161625cc;
    --stroke:#ffffff14;
    --text:#e9f7ff;
    --muted:#b8c6d1;
    --accent1:#b23fff;
    --accent2:#ff27fb;
    --err:#ff6b6b;
}

html, body {
    height: 100%;
    width: 100%;
    margin: 0;
    background: radial-gradient(1200px 700px at 20% 20%, #2d1b5a55, transparent 60%),
    radial-gradient(900px 600px at 80% 40%, #6b1f6b44, transparent 55%),
    linear-gradient(180deg, var(--bg0), var(--bg1));
}

body {
    font-family: Inter, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    color: var(--text);
    overflow: hidden;
}

.shell{
    height: 100vh;
    width: 100vw;
    display:flex;
    flex-direction:column;
}

.pages{
    display:flex;
    flex:1;
    transition: transform 360ms cubic-bezier(.2,.8,.2,1);
}

.page{
    width:100vw;
    display:flex;
    justify-content:center;
    align-items:center;
    padding: 24px;
    box-sizing:border-box;
}

.card{
    width:min(720px, 92vw);
    padding: 28px;
    border-radius: 18px;
    background: var(--card);
    border: 1px solid var(--stroke);
    box-shadow: 0 18px 70px #00000066;
    backdrop-filter: blur(10px);
    display:flex;
    flex-direction:column;
    gap: 14px;
}

.card.hero{
    align-items:center;
    text-align:center;
    gap: 10px;
}

.h1{
    font-size: 22px;
    font-weight: 800;
    letter-spacing: 0.2px;
}

.muted{
    color: var(--muted);
    font-weight: 600;
}

.mono{
    font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
    font-weight: 600;
    color: #d4e9ff;
}

.form{
    margin-top: 6px;
    display:flex;
    flex-direction:column;
    gap: 10px;
}

.label{
    font-weight: 800;
    color: #dff4ff;
}

.input{
    height: 44px;
    border-radius: 12px;
    border: 1px solid var(--stroke);
    background: #0f0f18aa;
    color: var(--text);
    padding: 0 12px;
    outline: none;
}

.input:focus{
    border-color: #ffffff2a;
    box-shadow: 0 0 0 4px #b23fff22;
}

.check{
    display:flex;
    gap: 10px;
    align-items:center;
    user-select:none;
    color: var(--muted);
    font-weight: 700;
}

.row{
    display:flex;
    justify-content:space-between;
    gap: 10px;
    margin-top: 10px;
}

.btn{
    height: 44px;
    border-radius: 12px;
    border: 1px solid var(--stroke);
    background: #0f0f18aa;
    color: var(--text);
    font-weight: 800;
    cursor: pointer;
    padding: 0 16px;
}

.btn:disabled{
    opacity: 0.6;
    cursor: not-allowed;
}

.btn.primary{
    border: none;
    background: linear-gradient(45deg, var(--accent1), var(--accent2));
    box-shadow: 0 14px 36px #b23fff22;
}

.btn.ghost{
    background: transparent;
}

.logo{
    width: 180px;
    height: 180px;
    object-fit: contain;
    filter: drop-shadow(0 18px 40px #00000055);
}

.logo.small{
    width: 120px;
    height: 120px;
}

.progressWrap{
    margin-top: 8px;
    display:flex;
    align-items:center;
    gap: 12px;
}

.progressBar{
    flex:1;
    height: 12px;
    border-radius: 999px;
    background: #ffffff10;
    border: 1px solid var(--stroke);
    overflow:hidden;
}

.progressFill{
    height: 100%;
    width: 0%;
    background: linear-gradient(90deg, var(--accent1), var(--accent2));
    transition: width 100ms linear;
}

.progressText{
    width: 54px;
    text-align:right;
    color: var(--muted);
    font-weight: 900;
}

.pill{
    display:inline-block;
    padding: 4px 10px;
    border-radius: 999px;
    background: #ffffff10;
    border: 1px solid var(--stroke);
    font-weight: 800;
}

.hint{
    margin-top: 6px;
    color: #cce9ff;
    font-weight: 700;
    opacity: 0.9;
}

.error{
    margin-top: 6px;
    padding: 12px;
    border-radius: 12px;
    border: 1px solid #ff6b6b44;
    background: #ff6b6b14;
}

.errorTitle{
    font-weight: 900;
    color: #ffd6d6;
}

.errorText{
    margin-top: 6px;
    color: #ffecec;
    font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
    font-size: 12px;
    white-space: pre-wrap;
}

.stepper{
    height: 34px;
    display:flex;
    justify-content:center;
    align-items:center;
    gap: 8px;
    padding-bottom: 10px;
}

.dot{
    width: 8px; height: 8px;
    border-radius: 50%;
    background: #ffffff22;
    border: 1px solid var(--stroke);
}

.dot.active{
    width: 10px; height: 10px;
    background: linear-gradient(45deg, var(--accent1), var(--accent2));
    border: none;
}

.dot.done{
    background: #ffffff44;
}
</style>