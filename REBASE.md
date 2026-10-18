# Rebasing Dracular on upstream Okular

Dracular tracks KDE Okular's `master` plus a stack of small, topical
commits. To stay mergeable, we **rebase, never merge** upstream, and we
**never squash** the topic commits (small commits keep conflict scope
small and reviewable).

## One-time setup

```bash
git remote add upstream https://invent.kde.org/graphics/okular.git
```

## The commit stack (bottom to top)

| Commit | Contents | Likely conflicts |
|---|---|---|
| `fb65ee9` Feat/dark reader (#1) | Dark Reader core: PDF mask, recolor, action | `part/part.cpp`, `gui/pagepainter.*` |
| `d5608beb` Enable Dark Reader for EPUB | Text-document mask | `core/textdocument*` |
| `fb5729c` Dark Reader performance | Mask off lock, LRU caches, painter fast path | `generator_pdf.*`, `pagepainter.*` |
| `ca6fdb8` Branding | Name, logo, desktop/metainfo | `shell/*`, `aboutdata.h`, `icons/CMakeLists.txt` |
| `34c78db` Moon icon | Toggle action icon | `part/part.cpp` (one line) |
| `89412f2` Poppler compat probe | `SplashOutputDev` signature check | `CMakeLists.txt`, `config-okular.h.cmake` |
| `87522bf` Flatpak manifest | `org.kde.dracular.json` | New file, rarely conflicts |
| `windows.yml` + this file | Windows CI, this doc | New files, rarely conflicts |

(Hashes are illustrative; `git log --oneline` shows the current ones.
The rule is the shape, not the hashes: feature, perf, branding,
compat, packaging, CI — in that order.)

## Rebase procedure

```bash
git fetch upstream
git rebase upstream/master
# resolve conflicts, keeping Dracular blocks intact (see below)
cmake --build build
ctest --test-dir build -R "appstreamtest|imageScalingTest" --output-on-failure
```

Then rebuild the Flatpak from the rebased tree and re-cut the bundle
(see "Release checklist"). If upstream renamed something Dracular
hooks into (e.g. generator or painter APIs), prefer adapting the
Dracular side inside its existing marked blocks over reformatting
upstream code.

## Conflict guide

Dracular code lives in marked blocks (`// Dark Reader ...`) and new
files (`darkreaderoutputdev.*`, `org.kde.dracular.*`, dracular icons).
During a rebase:

- Keep the Dracular blocks; accept upstream around them.
- `part/part.cpp`: our hunks are the action setup, toggle slot, and
  `updateViewActions` gating — small and self-contained.
- Branding one-liners (`main.cpp` window icon, `welcomescreen.cpp`
  icon, desktop `Name=`/`Icon=`) re-apply trivially.
- If a conflict spans an entire file upstream rewrote, `git checkout
  --theirs` the file, then re-apply just the Dracular hunks from the
  pre-rebase commit (`git show <old-sha> -- <file>`).

## Contributor rules (keep future rebases cheap)

1. New Dracular code goes in **new files** where possible.
2. Hooks into upstream files stay inside short, commented blocks.
3. One topic per commit; don't mix branding/perf/packaging.
4. Don't reformat upstream code (it poisons every future rebase).
5. Every commit must keep `cmake --build build` green.

## Release checklist

```bash
# Linux bundle (single installable file)
flatpak-builder --disable-rofiles-fuse \
  --repo=$HOME/.cache/dracular-flatpak/repo --force-clean \
  --state-dir=$HOME/.cache/dracular-flatpak \
  $HOME/.cache/dracular-flatpak/build-stable org.kde.dracular.json
flatpak build-bundle $HOME/.cache/dracular-flatpak/repo \
  $HOME/dracular.flatpak org.kde.dracular stable
# Windows: tag and let CI build the installer
git tag dracular-v1
git push origin dracular-v1
```

Attach `dracular.flatpak` to the same GitHub Release the Windows
installer lands on, so each `dracular-v*` tag ships both artifacts.
