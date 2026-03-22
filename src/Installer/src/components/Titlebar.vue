<script setup lang="ts">
import { getCurrentWindow } from "@tauri-apps/api/window";

const win = getCurrentWindow();

const onMin = async () => { await win.minimize(); };
const onClose = async () => { await win.close(); };

const onDragMouseDown = async (e: MouseEvent) => {
    // чтобы клики по кнопкам не запускали drag
    if ((e.target as HTMLElement)?.closest(".buttons")) return;
    try { await win.startDragging(); } catch { /* ignore */ }
};
</script>

<template>
    <div class="titlebar" @mousedown.left="onDragMouseDown" data-tauri-drag-region>
        <div class="brand" data-tauri-drag-region>
            <div class="dot purple"></div>
            <span>Neoluma Installer</span>
        </div>

        <div class="buttons" data-tauri-drag-region="false">
            <button class="btn" data-tauri-drag-region="false" @click="onMin" title="Minimize">
                <i class="fa-solid fa-window-minimize"></i>
            </button>
            <button class="btn close" data-tauri-drag-region="false" @click="onClose" title="Close">
                <i class="fa-solid fa-xmark"></i>
            </button>
        </div>
    </div>
</template>

<style scoped>
.titlebar{
    height: 44px;
    width: 100vw;
    display:flex;
    align-items:center;
    justify-content:space-between;
    padding: 0 10px;
    box-sizing:border-box;
    user-select:none;
    background: #0f0f18aa;
    border-bottom: 1px solid #ffffff14;
    backdrop-filter: blur(10px);
}

.brand{
    display:flex;
    align-items:center;
    gap: 10px;
    font-weight: 900;
    color: #e9f7ff;
    letter-spacing: 0.2px;
}

.dot{
    width: 10px;
    height: 10px;
    border-radius: 50%;
}

.dot.purple{
    background: linear-gradient(45deg, #b23fff, #ff27fb);
    box-shadow: 0 0 18px #b23fff55;
}

.buttons{
    display:flex;
    gap: 6px;
}

.btn{
    width: 34px;
    height: 34px;
    border: none;
    background: transparent;
    color: #fff;
    border-radius: 10px;
    cursor: pointer;
}

.btn:hover{
    background: rgba(255,255,255,0.08);
}

.close:hover{
    background: rgba(255,80,80,0.25);
}
</style>