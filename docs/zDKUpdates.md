# Releases and application updates

Source and downloads: [fantastruco/zDKCalculator](https://github.com/fantastruco/zDKCalculator), public.

## Updating the application

Open **Settings → App updates → Check for updates**. If a newer stable version is available, select **Download update**, followed by **Install & restart**. Checking and downloading happen in the background. Opening the panel alone makes no network request.

The updater contacts GitHub over HTTPS without credentials. It accepts the exact executable name from the configured public release repository and verifies its byte count and GitHub-provided SHA-256 digest. Drafts and prereleases are excluded. A downloaded executable verifies itself again before installation.

Installation waits for the previous process to exit, backs up the original executable, replaces it and restarts the app. A replacement or launch failure attempts to restore the backup. The executable's folder must be writable; the app does not request administrator elevation. Presets, settings and sessions remain in the existing Local AppData folder. The updater replaces the executable only; the ZIP supplies the accompanying documentation and dependency notices.

The binary is currently unsigned. SHA-256 detects mismatched downloads; it does not replace a publisher code-signing certificate. Update trust depends on HTTPS and control of the configured GitHub account and release repository.

## Publishing a version

Requirements: PowerShell, Git authenticated to the repository owner's GitHub account, and a clean source checkout on `main`. GitHub Actions supplies the Windows build environment.

Commit the intended changes first, then run:

```powershell
.\scripts\zDKPublish.ps1 -Version 3.11.1 -NotesFile docs\releases\3.11.1.md
```

Use a stable `major.minor.patch` version and English release notes. The script updates version resources, commits version changes, tags and pushes the source. GitHub Actions builds and tests that tag, packages the portable ZIP, uploads the EXE, ZIP and checksums to a draft, and publishes it after digest verification. Existing published tags are immutable in this workflow. If interrupted before publication, rerun with the same version to queue a retry.

The local Git credential authenticates pushes and retry requests. It is never included in the executable, package or repository. The release job uses GitHub's temporary workflow token with repository-content write permission; no personal access token needs to be stored in Actions secrets. Normal application updates need no account.

GitHub Actions builds and verifies pushes and pull requests and retains downloadable build artifacts. Publishing a stable version tag creates the public release automatically. The local publication command also supports resuming an interrupted draft.

## Verification

`build.ps1` runs metadata validation and version comparisons, Windows SHA-256 vectors, rejection of a staged digest mismatch, executable replacement and restart using isolated fixtures, and the interface suite. A failed UI run writes a framebuffer next to its log.

For a live, read-only check against the public release API:

```powershell
.\dist\DokkanDefenseCalculator.exe --check-updates C:\Temp\update-check.txt
```

The report contains the latest version and expected digest, or an error message.
