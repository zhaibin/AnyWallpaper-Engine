# Current Docs Cleanup and Platform Test Design

Date: 2026-05-18

## Goal

Clean only the current documentation surface for AnyWP Engine, then verify the project with the tests that can run from this macOS workspace and a Windows-focused static test pass.

The cleanup is intentionally conservative. It should reduce stale or misleading documentation without deleting historical release artifacts.

## Scope

In scope:

- Root documentation files: `README.md`, `QUICK_INTEGRATION.md`, `CHANGELOG_CN.md`.
- Current documentation files under `docs/*.md`.
- Documentation indexes and links that point to the current documentation set.
- Test documentation that describes how to run macOS, Flutter, Web SDK, and Windows checks.

Out of scope:

- Everything under `release/`.
- Documentation under `sdk/`, `windows/`, `macos/`, `example/`, `examples/`, `scripts/`, and `templates/`, unless a current root or `docs/` file links to it.
- Code behavior changes.
- Real Windows compilation or runtime testing from the macOS machine.

## Current Findings

- The repository is about 436 MB; `release/` is about 301 MB and will not be cleaned.
- `docs/` is small, about 688 KB, but contains stale navigation and some likely one-off reports.
- `docs/DOCUMENTATION_INDEX.md` is out of date: it reports 44 Markdown files while the current `docs/` directory contains 45, and it references documents that are not present.
- The project test surface includes Flutter tests, Web SDK Jest tests, and Windows batch/PowerShell/CMake test entry points.
- The local macOS sandbox blocks Flutter from writing to its SDK cache unless the command is run with user-approved escalation.

## Cleanup Rules

Delete a current documentation file only when one of these is true:

- It is a one-off historical fix summary or investigation report that is superseded by a current guide.
- It duplicates a maintained current document and does not add unique operational detail.
- It is not linked from the maintained documentation surface and is not useful as a standalone guide.
- Its instructions contradict current project files, scripts, or structure.

Keep a current documentation file when one of these is true:

- It documents public APIs, integration, packaging, release, testing, architecture, troubleshooting, or platform behavior.
- It is the best available source for a current feature.
- Removing it would create uncertainty for Flutter users, Web developers, release maintainers, or platform maintainers.

When a document is kept but navigation is stale, update links and indexes instead of deleting it.

## Proposed Execution

1. Audit root docs and `docs/*.md` for missing links, stale references, duplicate topics, and contradicted instructions.
2. Produce a candidate list of files to delete or keep with reasons.
3. Apply conservative deletions only to clearly unnecessary current docs.
4. Update `docs/README.md`, `docs/DOCUMENTATION_INDEX.md`, and root README links so the current documentation surface matches the files that remain.
5. Run available verification commands.
6. Record Windows testing as static validation unless a Windows host or CI runner is available.

## Test Plan

Run on macOS, with escalation if Flutter needs to update files outside the workspace:

- `flutter analyze`
- `flutter test`
- `cd example && flutter test`

Run Web SDK checks if dependencies can be installed or are already available:

- `cd sdk/src && npm test`
- `cd sdk/src && npm run typecheck`

Run Windows-focused static checks on macOS:

- Verify current docs mention valid Windows scripts and paths.
- Inspect `windows/test`, `windows/CMakeLists.txt`, and related scripts for obvious missing references.
- Report that real Windows build/runtime tests require a Windows environment with Visual Studio Build Tools, Windows SDK, and WebView2.

## Success Criteria

- `release/` remains untouched.
- Current docs and root docs have no obvious stale index entries for files that do not exist.
- Deleted files have clear reasons and are limited to current documentation.
- macOS/Flutter verification results are reported.
- Windows testing is reported honestly as static validation from macOS, with the exact commands or environment needed for full Windows execution.
