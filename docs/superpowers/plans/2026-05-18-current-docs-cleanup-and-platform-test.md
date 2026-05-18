# Current Docs Cleanup and Platform Test Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Clean stale current documentation without touching historical releases, then verify macOS, Web SDK, and Windows-facing test surfaces honestly.

**Architecture:** Treat documentation cleanup as a bounded repository maintenance pass. First build evidence from current files, then remove only clearly unnecessary current docs, update navigation, and run platform-appropriate verification.

**Tech Stack:** Markdown, Flutter, Dart analyzer/tests, Node/npm/Jest/TypeScript, Windows batch/PowerShell/CMake static checks.

---

## File Structure

- Modify: `README.md` only if it links to removed or renamed current docs.
- Modify: `QUICK_INTEGRATION.md` only if it links to removed or renamed current docs.
- Keep: `CHANGELOG_CN.md` unless it has broken current-doc links.
- Modify: `docs/README.md` as the primary current documentation entry point.
- Modify: `docs/DOCUMENTATION_INDEX.md` so counts, categories, and links match current files.
- Delete: only `docs/*.md` files that meet the approved deletion rules.
- Do not modify: `release/**`.

## Task 1: Audit Current Documentation

**Files:**
- Read: `README.md`
- Read: `QUICK_INTEGRATION.md`
- Read: `CHANGELOG_CN.md`
- Read: `docs/*.md`
- Do not modify: `release/**`

- [ ] **Step 1: List current root and docs Markdown files**

Run:

```bash
find . -maxdepth 1 -type f -name '*.md' -print | sort
find docs -maxdepth 1 -type f -name '*.md' -print | sort
```

Expected: output contains only root Markdown files and `docs/*.md`, with no `release/` paths.

- [ ] **Step 2: Identify stale links to missing docs**

Run:

```bash
rg -n "\]\(([^)]*\.md)[^)]*\)" README.md QUICK_INTEGRATION.md CHANGELOG_CN.md docs -S
```

Expected: use the output to compare every local `.md` link against existing root/docs files.

- [ ] **Step 3: Identify duplicate or report-style docs**

Run:

```bash
find docs -maxdepth 1 -type f -name '*.md' -exec basename {} \; | sort
```

Expected: candidate cleanup files are limited to current `docs/*.md` files.

- [ ] **Step 4: Write candidate decisions before deleting**

Create a short working list in the session notes with each candidate and one of:

```text
delete: superseded one-off report
keep: current guide/API/reference
update: stale index or stale links
```

Expected: every deletion has a concrete reason.

## Task 2: Apply Conservative Cleanup

**Files:**
- Delete: selected `docs/*.md` only
- Modify: `docs/README.md`
- Modify: `docs/DOCUMENTATION_INDEX.md`
- Modify: root docs only if needed

- [ ] **Step 1: Delete only approved current docs**

Use `apply_patch` delete hunks for each selected `docs/*.md` file.

Expected: `git status --short` shows deleted files only under `docs/`.

- [ ] **Step 2: Rebuild documentation index content**

Update `docs/DOCUMENTATION_INDEX.md` to include:

```markdown
# AnyWP Engine Documentation Index

## Start Here

- `QUICK_START.md`
- `FOR_FLUTTER_DEVELOPERS.md`
- `WEB_DEVELOPER_GUIDE_CN.md`
- `WEB_DEVELOPER_GUIDE.md`
- `PRECOMPILED_DLL_INTEGRATION.md`
- `PRECOMPILED_MACOS_INTEGRATION.md`

## Maintainer References

- `ARCHITECTURE_DESIGN.md`
- `MULTIPLATFORM_ARCHITECTURE.md`
- `MESSAGE_PROTOCOL.md`
- `TESTING_GUIDE.md`
- `RELEASE_GUIDE.md`
- `VERSION_MANAGEMENT.md`
```

Expected: the final file lists only documents that exist.

- [ ] **Step 3: Update `docs/README.md` navigation**

Ensure the file points readers to current docs in practical groups:

```markdown
## Quick Entry Points

- New Flutter users: `QUICK_START.md`
- Flutter integration: `FOR_FLUTTER_DEVELOPERS.md`
- Web wallpaper developers: `WEB_DEVELOPER_GUIDE_CN.md`
- API reference: `DEVELOPER_API_REFERENCE.md`
- Troubleshooting: `TROUBLESHOOTING.md`
- Testing: `TESTING_GUIDE.md`
```

Expected: no links point to deleted files.

- [ ] **Step 4: Update root links if needed**

Run:

```bash
rg -n "docs/.*\.md|\.md\)" README.md QUICK_INTEGRATION.md CHANGELOG_CN.md -S
```

Expected: edit only links that point to deleted or missing current docs.

## Task 3: Validate Documentation Integrity

**Files:**
- Verify: root Markdown files
- Verify: `docs/*.md`

- [ ] **Step 1: Confirm `release/` is untouched**

Run:

```bash
git status --short
```

Expected: no changed paths under `release/`.

- [ ] **Step 2: Check for missing local Markdown links**

Run a small shell/Python or manual check that extracts local `.md` links from root docs and `docs/*.md`, then confirms each target exists.

Expected: no missing local current-doc links remain.

- [ ] **Step 3: Check index does not mention missing docs**

Run:

```bash
rg -n "[A-Z0-9_]+\.md" docs/DOCUMENTATION_INDEX.md docs/README.md
```

Expected: every mentioned file exists in `docs/` or root when applicable.

## Task 4: Run Platform Verification

**Files:**
- Verify: project root
- Verify: `example/`
- Verify: `sdk/src/`
- Verify: `windows/test/`
- Verify: `scripts/`

- [ ] **Step 1: Run macOS Flutter analyzer**

Run:

```bash
flutter analyze
```

Expected: PASS, or report exact analyzer failures. If Flutter needs to write outside the workspace, rerun with approved escalation.

- [ ] **Step 2: Run root Flutter tests**

Run:

```bash
flutter test
```

Expected: PASS, or report exact test failures.

- [ ] **Step 3: Run example Flutter tests**

Run:

```bash
cd example && flutter test
```

Expected: PASS, or report exact test failures.

- [ ] **Step 4: Run Web SDK checks**

Run from `sdk/src`:

```bash
npm test
npm run typecheck
```

Expected: PASS if dependencies are installed. If dependencies are missing and network install is not approved, report that Web SDK tests are blocked by missing dependencies.

- [ ] **Step 5: Run Windows static validation**

Run:

```bash
find windows/test -maxdepth 1 -type f -print | sort
find scripts -maxdepth 1 -type f \( -name '*.bat' -o -name '*.ps1' \) -print | sort
rg -n "run_tests|run_comprehensive_test|test_precompiled_package|Visual Studio|WebView2|CMake" docs windows/test scripts -S
```

Expected: documentation and scripts reference existing Windows test files. Report that real Windows compile/runtime testing requires Windows with Visual Studio Build Tools, Windows SDK, and WebView2.

## Task 5: Final Review

**Files:**
- Review: all changed files

- [ ] **Step 1: Review diff**

Run:

```bash
git diff --stat
git diff -- README.md QUICK_INTEGRATION.md CHANGELOG_CN.md docs
```

Expected: changes are limited to approved current docs and plan/spec files.

- [ ] **Step 2: Summarize cleanup**

Prepare final response with:

```text
Deleted:
- path: reason

Updated:
- path: reason

Verification:
- command: result
```

Expected: user can see exactly what changed and which checks passed or were blocked.
