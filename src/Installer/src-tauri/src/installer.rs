use anyhow::{Context, Result};
use serde::Serialize;
use std::{
    fs::{self, File},
    io::{Read, Write},
    path::{Path, PathBuf},
    time::{Duration, Instant},
};
use tauri::{
    Emitter,
    Manager,
};

use crate::InstallScope;

// ==== UI events payload ====

#[derive(Serialize, Clone)]
pub struct InstallProgress {
    pub stage: String,   // "preparing" | "extracting" | "path" | "done" | "error"
    pub percent: f64,    // 0..100
    pub current: String, // filename / message
}

fn emit_progress(app: &tauri::AppHandle, stage: &str, percent: f64, current: impl Into<String>) {
    let _ = app.emit(
        "install://progress",
        InstallProgress {
            stage: stage.into(),
            percent,
            current: current.into(),
        },
    );
}

fn emit_done(app: &tauri::AppHandle) {
    let _ = app.emit("install://done", ());
}

// ==== Install dirs (defaults) ====

pub fn default_install_dir(app: &tauri::AppHandle, scope: InstallScope) -> Result<PathBuf> {
    match scope {
        InstallScope::User => default_user_install_dir(app),
        InstallScope::System => default_system_install_dir(),
    }
}

fn default_user_install_dir(app: &tauri::AppHandle) -> Result<PathBuf> {
    let base = app
        .path()
        .resolve("", tauri::path::BaseDirectory::AppLocalData)
        .context("Resolve BaseDirectory::AppLocalData failed")?;

    Ok(base.join("Neoluma"))
}

fn default_system_install_dir() -> Result<PathBuf> {
    #[cfg(windows)]
    {
        let pf = std::env::var_os("ProgramFiles")
            .context("%ProgramFiles% not found")?;
        Ok(PathBuf::from(pf).join("Neoluma"))
    }

    #[cfg(target_os = "macos")]
    {
        Ok(PathBuf::from("/Applications").join("Neoluma"))
    }

    #[cfg(all(unix, not(target_os = "macos")))]
    {
        Ok(PathBuf::from("/opt").join("neoluma"))
    }
}

// ==== PATH update ====

fn guess_bin_dir(dest: &Path) -> PathBuf {
    let bin = dest.join("bin");
    if bin.is_dir() { bin } else { dest.to_path_buf() }
}

#[cfg(windows)]
fn add_to_path_windows(dir: &Path, scope: InstallScope) -> Result<()> {
    use std::ffi::OsStr;
    use winreg::{enums::*, RegKey};

    let dir = dir.to_string_lossy().to_string();

    // user => HKCU, system => HKLM
    let (root, subkey) = match scope {
        InstallScope::User => (RegKey::predef(HKEY_CURRENT_USER), "Environment"),
        InstallScope::System => (
            RegKey::predef(HKEY_LOCAL_MACHINE),
            r"SYSTEM\CurrentControlSet\Control\Session Manager\Environment",
        ),
    };

    let (env, _) = root
        .create_subkey(subkey)
        .with_context(|| format!("Open registry key failed: {}", subkey))?;

    let current: String = env.get_value("Path").unwrap_or_default();

    let mut parts: Vec<String> = current
        .split(';')
        .filter(|s| !s.trim().is_empty())
        .map(|s| s.to_string())
        .collect();

    let already = parts.iter().any(|p| p.eq_ignore_ascii_case(&dir));
    if !already {
        parts.push(dir);
        let new_path = parts.join(";");
        env.set_value("Path", &OsStr::new(&new_path))
            .context("Write Path failed")?;
    }

    Ok(())
}

#[cfg(not(windows))]
fn add_to_path_unix_user(dir: &Path) -> Result<()> {
    // User-level PATH via shell rc/profile files.
    // You can't really make system-level install without root adequately, so uhh... we only apply user variant for now.
    use std::env;

    let home = env::var("HOME").context("HOME is not set")?;
    let dir_str = dir.to_string_lossy();

    let marker_start = "# >>> Neoluma installer >>>";
    let marker_end = "# <<< Neoluma installer <<<";
    let export_line = format!(r#"export PATH="{}:$PATH""#, dir_str);

    let targets = [".profile", ".bashrc", ".zshrc", ".zprofile"];

    for f in targets {
        let path = Path::new(&home).join(f);

        let mut content = String::new();
        if path.exists() {
            content = fs::read_to_string(&path).unwrap_or_default();
            if content.contains(marker_start) && content.contains(&export_line) {
                continue;
            }
        }

        let mut out = content;
        if !out.ends_with('\n') {
            out.push('\n');
        }

        out.push_str(marker_start);
        out.push('\n');
        out.push_str(&export_line);
        out.push('\n');
        out.push_str(marker_end);
        out.push('\n');

        fs::write(&path, out).with_context(|| format!("Write {:?} failed", path))?;
    }

    Ok(())
}

#[cfg(windows)]
pub fn restart_as_admin(app: &tauri::AppHandle, args: crate::InstallArgs) -> anyhow::Result<()> {
    use windows_sys::Win32::UI::Shell::ShellExecuteW;
    use windows_sys::Win32::UI::WindowsAndMessaging::SW_SHOW;

    let exe = std::env::current_exe().context("current_exe failed")?;

    let dest = args.destination.unwrap_or_default();
    let scope = match args.scope {
        crate::InstallScope::User => "user",
        crate::InstallScope::System => "system",
    };
    let add_to_path = if args.addToPath { "1" } else { "0" };

    let params = format!(
        r#"--neoluma-install=1 --scope={} --dest="{}" --add-to-path={}"#,
        scope,
        dest.replace('"', r#"\""#),
        add_to_path
    );

    fn to_wide(s: &std::ffi::OsStr) -> Vec<u16> {
        use std::os::windows::ffi::OsStrExt;
        s.encode_wide().chain(std::iter::once(0)).collect()
    }

    let verb = to_wide(std::ffi::OsStr::new("runas"));
    let file = to_wide(exe.as_os_str());
    let params_w = to_wide(std::ffi::OsStr::new(&params));

    // ShellExecuteW returns value > 32 on success
    let code = unsafe {
        ShellExecuteW(
            0,
            verb.as_ptr(),
            file.as_ptr(),
            params_w.as_ptr(),
            std::ptr::null(),
            SW_SHOW,
        )
    };

    if (code as isize) <= 32 {
        anyhow::bail!("UAC elevation was cancelled or failed (ShellExecuteW code={}).", code);
    }

    app.exit(0);
    Ok(())
}

// ==== Core install ====

pub fn install_payload_zip(
    app: tauri::AppHandle,
    zip_path: PathBuf,
    destination: PathBuf,
    scope: InstallScope,
    add_to_path: bool,
) -> Result<()> {
    emit_progress(&app, "preparing", 0.0, "Creating directories…");

    #[cfg(windows)]
    {
        if matches!(scope, InstallScope::System) && !is_elevated() {
            anyhow::bail!("System install requires Administrator privileges. Please restart the installer as Administrator.");
        }
    }

    fs::create_dir_all(&destination)
        .with_context(|| format!("Create destination {:?} failed", destination))?;

    // ==== Pre-calc total bytes for progress ====
    let total_bytes = zip_total_bytes(&zip_path)?;
    let mut written: u64 = 0;
    let mut last_emit = Instant::now();

    // ==== Extract ====
    let file = File::open(&zip_path).with_context(|| format!("Open {:?} failed", zip_path))?;
    let mut archive = zip::ZipArchive::new(file).context("Bad zip archive")?;

    for i in 0..archive.len() {
        let mut zipped = archive.by_index(i).context("Zip read failed")?;

        let Some(rel) = zipped.enclosed_name().map(|p| p.to_owned()) else {
            continue;
        };

        let out_path = destination.join(rel);

        if zipped.is_dir() {
            fs::create_dir_all(&out_path)
                .with_context(|| format!("Create dir {:?} failed", out_path))?;
            continue;
        }

        if let Some(parent) = out_path.parent() {
            fs::create_dir_all(parent)
                .with_context(|| format!("Create parent {:?} failed", parent))?;
        }

        // progress ping
        emit_progress(
            &app,
            "extracting",
            percent(written, total_bytes),
            out_path.file_name().unwrap_or_default().to_string_lossy(),
        );

        // write file
        let mut out = File::create(&out_path)
            .with_context(|| format!("Create file {:?} failed", out_path))?;

        let mut buf = [0u8; 64 * 1024];
        loop {
            let n = zipped.read(&mut buf).context("Zip read chunk failed")?;
            if n == 0 { break; }
            out.write_all(&buf[..n]).context("Write chunk failed")?;

            written = written.saturating_add(n as u64);

            if last_emit.elapsed() >= Duration::from_millis(60) {
                last_emit = Instant::now();
                emit_progress(
                    &app,
                    "extracting",
                    percent(written, total_bytes),
                    out_path.file_name().unwrap_or_default().to_string_lossy(),
                );
            }
        }

        // permissions on unix
        #[cfg(unix)]
        {
            use std::os::unix::fs::PermissionsExt;
            if let Some(mode) = zipped.unix_mode() {
                let _ = fs::set_permissions(&out_path, fs::Permissions::from_mode(mode));
            }
        }
    }

    // ==== PATH ====
    if add_to_path {
        emit_progress(&app, "path", 99.0, "Updating PATH…");

        let bin_dir = guess_bin_dir(&destination);

        #[cfg(windows)]
        add_to_path_windows(&bin_dir, scope).context("PATH update failed (Windows)")?;

        #[cfg(not(windows))]
        {
            // on unix system-level PATH = root territory. always making user-level installation for now
            let _ = scope;
            add_to_path_unix_user(&bin_dir).context("PATH update failed (Unix)")?;
        }
    }

    emit_progress(&app, "done", 100.0, "Done");
    emit_done(&app);
    Ok(())
}

// ==== helpers ====

fn zip_total_bytes(zip_path: &Path) -> Result<u64> {
    let file = File::open(zip_path).with_context(|| format!("Open {:?} failed", zip_path))?;
    let mut archive = zip::ZipArchive::new(file).context("Bad zip archive")?;

    let mut total = 0u64;
    for i in 0..archive.len() {
        let f = archive.by_index(i).context("Zip read failed")?;
        total = total.saturating_add(f.size());
    }
    Ok(total)
}

fn percent(written: u64, total: u64) -> f64 {
    if total == 0 { return 0.0; }
    ((written as f64 / total as f64) * 100.0).min(100.0)
}

#[cfg(windows)]
fn is_elevated() -> bool {
    use windows_sys::Win32::Foundation::CloseHandle;
    use windows_sys::Win32::Security::{
        GetTokenInformation,
        TokenElevation,
        TOKEN_ELEVATION,
        TOKEN_QUERY,
    };
    use windows_sys::Win32::System::Threading::{GetCurrentProcess, OpenProcessToken};

    unsafe {
        let mut token = 0;

        if OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &mut token) == 0 {
            return false;
        }

        let mut elevation = TOKEN_ELEVATION { TokenIsElevated: 0 };
        let mut size = std::mem::size_of::<TOKEN_ELEVATION>() as u32;

        let result = GetTokenInformation(
            token,
            TokenElevation,
            &mut elevation as *mut _ as *mut _,
            size,
            &mut size,
        );

        CloseHandle(token);

        if result == 0 {
            return false;
        }

        elevation.TokenIsElevated != 0
    }
}

pub fn can_system_install() -> bool {
    #[cfg(windows)]
    { is_elevated() }

    #[cfg(not(windows))]
    { false } // i think i repeated it twice above already
}

#[derive(Clone)]
pub struct AutoInstallRequest {
    pub scope: crate::InstallScope,
    pub destination: Option<String>,
    pub add_to_path: bool,
}

pub fn parse_autoinstall_args() -> Option<AutoInstallRequest> {
    let mut install = false;
    let mut scope: Option<crate::InstallScope> = None;
    let mut dest: Option<String> = None;
    let mut add_to_path: Option<bool> = None;

    for a in std::env::args() {
        if a == "--neoluma-install=1" {
            install = true;
        } else if let Some(v) = a.strip_prefix("--scope=") {
            scope = Some(match v {
                "system" => crate::InstallScope::System,
                _ => crate::InstallScope::User,
            });
        } else if let Some(v) = a.strip_prefix("--dest=") {
            let v = v.trim_matches('"');
            if !v.is_empty() { dest = Some(v.to_string()); }
        } else if let Some(v) = a.strip_prefix("--add-to-path=") {
            add_to_path = Some(v == "1" || v.eq_ignore_ascii_case("true"));
        }
    }

    if !install { return None; }

    Some(AutoInstallRequest {
        scope: scope.unwrap_or(crate::InstallScope::System),
        destination: dest,
        add_to_path: add_to_path.unwrap_or(true),
    })
}

pub async fn autoinstall(app: tauri::AppHandle) -> anyhow::Result<()> {
    use tauri::path::BaseDirectory;

    let req = parse_autoinstall_args().expect("autoinstall called without args");

    // payload.zip location
    let mut zip_path = app
        .path()
        .resolve("payload.zip", BaseDirectory::Resource)
        .context("resolve payload.zip failed")?;

    if !zip_path.exists() {
        let alt = app
            .path()
            .resource_dir()
            .context("resource_dir failed")?
            .join("resources")
            .join("payload.zip");

        if alt.exists() {
            zip_path = alt;
        }
    }

    anyhow::ensure!(zip_path.exists(), "payload.zip not found at {:?}", zip_path);

    let destination = match req.destination.as_deref().filter(|s| !s.trim().is_empty()) {
        Some(custom) => std::path::PathBuf::from(custom),
        None => default_install_dir(&app, req.scope)?,
    };

    install_payload_zip(app, zip_path, destination, req.scope, req.add_to_path)?;
    Ok(())
}