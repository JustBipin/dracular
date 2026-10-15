# Dracular

**Dracular** is an independent fork of [Okular](https://okular.kde.org), KDE's universal document viewer, with a dark mode built for reading: **text and backgrounds turn dark, images stay exactly as they are.**

Okular's built-in "Change Colors" options invert the whole page, so photos, figures and scans end up as negatives. Dracular's *Dark Reader* mode recolors only the text and page background and leaves embedded images untouched.

[Demo video](https://youtu.be/Xf0qtwL_RVU)

## Features

- **Dark Reader mode** for **PDF** and **EPUB** documents.
- **Images are preserved.** Embedded raster images are excluded from recoloring using an exact per-pixel mask recorded while the page is rendered, not a color heuristic, so grayscale photos and muted scans are not mistaken for text.
- **Colors keep their identity.** Black-and-white content is mapped to the text and background colors, while colored content (colored text, links, highlights) has its brightness inverted but its hue kept, so green stays green and remains readable on a dark background.
- **Choice of background:** the default dark gray (`#232627`), the current KDE color scheme's window color, or any custom color.
- **One-click toggle** from the menu bar, independent of Okular's existing color-mode menu. Both stay in sync.
- Everything else Okular does: annotations, forms, signatures, search, bookmarks, and the many other formats Okular supports.

### Using Dark Reader

- Toggle it with the **Dark Reader** action in the menu bar. The action is only enabled for PDF and EPUB documents.
- There is no default keyboard shortcut. Assign one in **Settings → Configure Keyboard Shortcuts** (search for "Dark Reader").
- Choose the background in **Settings → Configure Okular → Accessibility**: select *Dark Reader* as the color mode, then pick *Default*, *From color scheme* or *Custom*.

## Known limitations

- Supported formats: PDF and EPUB only. Other formats are unchanged from Okular.
- Image protection for PDFs needs a build against Poppler's core library (see [Building](#building)). Without it, Dark Reader still works but recolors images too.
- For PDFs, image protection works on local files only and is not available for password-protected documents; those fall back to recoloring the whole page.
- Images are excluded by bounding box. Text printed directly over an image is not recolored, and a rotated or skewed image may leave a thin sliver of surrounding background untouched.
- PDF rendering opens the file a second time to build the image mask, which increases memory use for large documents.
- The text color is currently fixed (`#eff0f1`); only the background is configurable.

## Building

Dracular builds like Okular. It tracks Okular's development branch, so it needs recent Qt and KDE Frameworks, which many stable distributions do not ship yet.

Requirements:

- Qt 6.6 or newer
- KDE Frameworks 6
- Poppler 24.08 or newer with the Qt 6 bindings
- **Poppler core headers** (`SplashOutputDev.h` and friends), which some distributions only install when Poppler is built with `ENABLE_UNSTABLE_API_ABI_HEADERS`. CMake only reports the core library as optional, but the exact image mask depends on it. After configuring, you can confirm it is active:

  ```bash
  grep -q darkreaderoutputdev.cpp build/build.ninja && echo "image mask: enabled"
  ```

The easiest way to pull in most dependencies is your distribution's Okular build dependencies:

```bash
# Fedora
sudo dnf builddep okular
# Debian / Ubuntu
sudo apt build-dep okular
```

Then build and install into a private prefix:

```bash
git clone https://github.com/GaurabPrasai/dracular.git
cd dracular
cmake -S . -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DOKULAR_UI=desktop \
      -DCMAKE_INSTALL_PREFIX="$HOME/.local/dracular"
cmake --build build
cmake --install build
```

Run it with the helper script that CMake generates, which sets up the environment for a non-system prefix:

```bash
source build/prefix.sh
okular
```

> **Note:** Dracular currently installs as `okular` and uses Okular's application ID and configuration. Do not install it into the same prefix as a system Okular, and expect it to share settings with any other Okular on your account.

To run the test suite (needs a display or Xvfb, and an installed build):

```bash
cmake --install build
source build/prefix.sh
ctest --test-dir build --output-on-failure
```

## Project status and upstream

Dracular is based on Okular's `master` branch and is maintained independently. It is not affiliated with or endorsed by KDE. Dracular's own changes are kept as small and self-contained as practical so that upstream changes, including bug and security fixes, can be merged in. Features are added to Dracular on its own schedule and do not depend on upstream acceptance.

For general Okular usage (annotations, forms, signatures, supported formats), see the [Okular handbook](https://docs.kde.org/?application=okular). Bugs that also reproduce in stock Okular belong in [KDE's bug tracker](https://bugs.kde.org/enter_bug.cgi?product=okular); bugs in Dark Reader or anything else specific to this fork belong in this repository's [issues](https://github.com/GaurabPrasai/dracular/issues).

## Contributing

Issues and pull requests are welcome.

- Open pull requests against `master`. CI builds the project and checks formatting.
- Code is formatted with **clang-format 19** using the repository's `_clang-format` configuration. To format a file: `clang-format -i path/to/file.cpp`.
- Keep Dracular-specific code in new files where possible, with minimal hooks into existing Okular code. This keeps future upstream merges manageable.
- Keep the SPDX license header on every file you add.

## Credits and license

Dracular is a fork of [Okular](https://invent.kde.org/graphics/okular) by the KDE community and its contributors. The Dark Reader - Dracular is maintained by [Bipin Thapa](https://github.com/justbipin) and [Gaurab Prasai](https://github.com/gaurabprasai).

Okular and Dracular are free software, licensed under the GPL (see the SPDX headers in each file and the [`LICENSES`](LICENSES) directory). Original copyright notices from Okular are retained.
