# Cursor Prompt — Font Integration for LARDOSSA

## Task
Integrate the downloaded font files into the LARDOSSA VST plugin's web UI. The fonts should load correctly inside JUCE's `WebBrowserComponent` (which serves files from BinaryData or a local resource provider — not from the internet).

---

## What to do

### 1. Locate the font files
Find all font files in the project. They may be in any of these formats: `.woff2`, `.woff`, `.ttf`, `.otf`. Look in:
- `ui_kits/lardossa-vst/` or any subdirectory
- `fonts/` at the project root
- Any other directory where the user has placed them

List every font file you find and note its path and format before proceeding.

### 2. Copy fonts to the UI assets folder
If the fonts are not already under `ui_kits/lardossa-vst/src/fonts/` (or `ui_kits/lardossa-vst/public/fonts/` if using Vite), copy them there. Create the directory if it doesn't exist.

### 3. Declare `@font-face` rules in `tokens.css`
Open `ui_kits/lardossa-vst/src/tokens.css` (create it if missing). Add one `@font-face` block per font file using the correct format descriptor:

```css
/* Example — adapt family name, src path, and format to match actual files found */

/* If .woff2 files found: */
@font-face {
  font-family: 'FlorDeRuina';
  src: url('./fonts/FlorDeRuina-Flor.woff2') format('woff2');
  font-weight: 100;
  font-style: normal;
  font-display: block;
}
@font-face {
  font-family: 'FlorDeRuina';
  src: url('./fonts/FlorDeRuina-Germen.woff2') format('woff2');
  font-weight: 400;
  font-style: normal;
  font-display: block;
}
@font-face {
  font-family: 'FlorDeRuina';
  src: url('./fonts/FlorDeRuina-Ruina.woff2') format('woff2');
  font-weight: 700;
  font-style: normal;
  font-display: block;
}
@font-face {
  font-family: 'FlorDeRuina';
  src: url('./fonts/FlorDeRuina-Fractura.woff2') format('woff2');
  font-weight: 900;
  font-style: normal;
  font-display: block;
}
@font-face {
  font-family: 'FlorDeRuina';
  src: url('./fonts/FlorDeRuina-Semilla.woff2') format('woff2');
  font-weight: 300;
  font-style: normal;
  font-display: block;
}

/* If .ttf files found: */
@font-face {
  font-family: 'ShareTechMono';
  src: url('./fonts/ShareTechMono-Regular.ttf') format('truetype');
  font-weight: 400;
  font-style: normal;
  font-display: block;
}

/* If .woff2 AND .ttf both exist for same font, list woff2 first as preferred: */
@font-face {
  font-family: 'ShareTechMono';
  src: url('./fonts/ShareTechMono.woff2') format('woff2'),
       url('./fonts/ShareTechMono-Regular.ttf') format('truetype');
  font-weight: 400;
  font-style: normal;
  font-display: block;
}
```

**Format string rules** (use the correct one for each file):
| Extension | format() string |
|---|---|
| `.woff2` | `'woff2'` |
| `.woff` | `'woff'` |
| `.ttf` | `'truetype'` |
| `.otf` | `'opentype'` |

Use `font-display: block` (not `swap`) — this prevents a flash of unstyled text inside the plugin window.

### 4. Update CSS custom properties
In the `:root` block of `tokens.css`, update the font variables to reference the loaded families:

```css
:root {
  /* Script / display — use FlorDeRuina for the LARDOSSA wordmark and decorative text */
  --font-script:   'FlorDeRuina', 'Great Vibes', cursive;

  /* Monospace — use ShareTechMono for knob readouts, step values, parameter displays */
  --font-mono:     'ShareTechMono', 'IBM Plex Mono', monospace;

  /* If any additional fonts were found, add variables for them here */
}
```

### 5. Apply to the plugin wordmark
In `PlaceholderUI.jsx` (or wherever the plugin name "LARDOSSA" is rendered), apply the script font:

```jsx
<h1 style={{ fontFamily: 'var(--font-script)', color: 'var(--color-accent)', fontSize: '2.4rem', fontWeight: 400 }}>
  LARDOSSA
</h1>
```

And ensure all numeric readouts (BPM, Hz, ms, dB values) use:
```css
font-family: var(--font-mono);
```

### 6. Verify fonts load in the JUCE WebBrowserComponent
JUCE's WebBrowserComponent serves files from BinaryData or a local file resource provider — it does **not** fetch from the internet. Confirm that the font files will be bundled correctly:

- If the project uses **Vite** or **Parcel** to build the UI: font files in `src/fonts/` referenced via relative `url()` paths will be automatically copied to `dist/` during build. Verify the build output includes the font files.
- If the project serves files directly from **BinaryData** (C++ side): the font files must be added to the `CMakeLists.txt` BinaryData target alongside the other UI assets. Check `CMakeLists.txt` for the `juce_add_binary_data` call and add the font file paths to it.
- If using a **WebView resource provider** (custom `getResourceForURL`): ensure the C++ resource handler maps font file URLs to the correct BinaryData entries.

### 7. Check for any existing font declarations
Search the entire `ui_kits/` directory for any existing `@font-face` rules or hardcoded `font-family` strings. If found, consolidate them into `tokens.css` and remove duplicates.

---

## Definition of done
- All font files are under `ui_kits/lardossa-vst/src/fonts/` (or `public/fonts/`)
- `tokens.css` has `@font-face` blocks for every font file found, with correct format strings
- `--font-script` and `--font-mono` CSS variables are declared in `:root`
- The LARDOSSA wordmark renders in FlorDeRuina (or whichever script font was found)
- Knob/parameter readouts render in ShareTechMono (or whichever mono font was found)
- Font files are included in the build output / BinaryData so they load inside the JUCE plugin window
- No font loads from the internet (no Google Fonts CDN links)
