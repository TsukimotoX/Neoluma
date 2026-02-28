mod installer;

use tauri::{Emitter, Manager};
use tauri::path::BaseDirectory;

// ==== DTOs ====

#[derive(serde::Serialize)]
struct InstallDefaults {
    userPath: String,
    systemPath: String,
    canSystemInstall: bool,
}

#[derive(serde::Deserialize, Clone, Copy)]
#[serde(rename_all = "lowercase")]
enum InstallScope {
    User,
    System,
}

#[derive(serde::Deserialize)]
struct InstallArgs {
    scope: InstallScope,
    destination: Option<String>,
    addToPath: bool,
}

// ==== Commands ====

#[tauri::command]
fn get_install_defaults(app: tauri::AppHandle) -> Result<InstallDefaults, String> {
    let user = installer::default_install_dir(&app, InstallScope::User)
        .map_err(|e| format!("[NeolumaInstaller] default user dir failed: {e:#}"))?;

    let system = installer::default_install_dir(&app, InstallScope::System)
        .map_err(|e| format!("[NeolumaInstaller] default system dir failed: {e:#}"))?;

    let can_system = installer::can_system_install();
    Ok(InstallDefaults {
        userPath: user.to_string_lossy().to_string(),
        systemPath: system.to_string_lossy().to_string(),
        canSystemInstall: can_system,
    })
}

#[tauri::command]
fn restart_as_admin(app: tauri::AppHandle, args: InstallArgs) -> Result<(), String> {
    installer::restart_as_admin(&app, args)
        .map_err(|e| format!("[NeolumaInstaller] restart_as_admin failed: {e:#}"))
}

#[tauri::command]
fn install(app: tauri::AppHandle, args: InstallArgs) -> Result<(), String> {
    // ==== payload.zip location ====
    let mut zip_path = app
        .path()
        .resolve("payload.zip", BaseDirectory::Resource)
        .map_err(|e| format!("[NeolumaInstaller] resolve payload.zip failed: {e}"))?;

    if !zip_path.exists() {
        let alt = app
            .path()
            .resource_dir()
            .map_err(|e| format!("[NeolumaInstaller] resource_dir failed: {e}"))?
            .join("resources")
            .join("payload.zip");

        if alt.exists() {
            zip_path = alt;
        }
    }

    if !zip_path.exists() {
        return Err(format!("[NeolumaInstaller] payload.zip not found at {:?}", zip_path));
    }

    // ==== destination ====
    let destination = match args.destination.as_deref().filter(|s| !s.trim().is_empty()) {
        Some(custom) => std::path::PathBuf::from(custom),
        None => installer::default_install_dir(&app, args.scope)
            .map_err(|e| format!("[NeolumaInstaller] default destination failed: {e:#}"))?,
    };

    let add_to_path = args.addToPath;

    // ==== async install ====
    tauri::async_runtime::spawn({
        let app = app.clone();
        async move {
            if let Err(e) = installer::install_payload_zip(app.clone(), zip_path, destination, args.scope, add_to_path) {
                let _ = app.emit("install://error", format!("{:#}", e));
            }
        }
    });

    Ok(())
}

// ==== Entrypoint ====

pub fn run() {
    tauri::Builder::default()
        .setup(|app| {
            // if the app is launched with --neoluma-install=1, it starts installation immediately
            if installer::parse_autoinstall_args().is_some() {
                if let Some(w) = app.get_webview_window("main") {
                    let _ = w.emit("install://autostart", ());
                }

                let handle = app.handle().clone();
                let handle2 = handle.clone();

                tauri::async_runtime::spawn(async move {
                    if let Err(e) = installer::autoinstall(handle2).await {
                        let _ = handle.emit("install://error", format!("{:#}", e));
                    }
                });
            }
            Ok(())
        })
        .invoke_handler(tauri::generate_handler![get_install_defaults, install, restart_as_admin])
        .run(tauri::generate_context!())
        .expect("[NeolumaInstaller] Error while running tauri application:");
}