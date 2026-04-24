# Figma → LARDOSSA UI

The production UI is a **Vite + React** app under `ui_kits/lardossa-vst/`. Figma is the visual source of truth for layout, typography, and colour; the code should match components and tokens rather than inventing one-off values.

## Typography (offline / JUCE WebView)

Typography is **self-hosted** under `src/fonts/` with `@font-face` in `src/tokens.css` (no CDN): **IBM Plex Sans** and **IBM Plex Mono**. The wordmark uses `--font-display` (IBM Plex Sans by default). Do **not** use the CSS generic `cursive` in stacks — on Windows WebView it often resolves to **Edwardian Script**. For a licensed **ITC Machine** logo, add the woff2 under `src/fonts/`, declare `@font-face` for `"ITC Machine"`, and set `--font-display` to list it before IBM Plex Sans. Numeric readouts use `--font-mono`. Vite inlines woff2 into the built CSS for JUCE BinaryData.

## Workflow

1. **Tokens** — Map Figma colour and spacing variables to CSS in `src/tokens.css` (or extend that file). Prefer semantic names (`--surface`, `--accent`) over raw hex in components. Map Figma text styles to `--font-display`, `--font-sans`, and `--font-mono` rather than hardcoding family names in JSX.
2. **Components** — Build React components that mirror Figma frames (sections, knobs, step grid). Keep sizing compatible with the fixed **900×560** plugin editor window unless the product spec changes.
3. **Bridge** — UI controls call `setParam(id, value)` from `useLardossaState()` with the same `id` strings as `Parameters.h` / `BRIDGE_SCHEMA.md`. Do not send normalised 0–1 values unless the parameter is defined that way in the processor.
4. **Figma MCP** — When iterating from a Figma file, use the Figma MCP `get_design_context` for structure and screenshots, then adapt output to this project’s stack (plain CSS modules or CSS files here, not Tailwind unless you add it).

## Files

- `src/FigmaRoot.jsx` — optional shell for design-system-aligned layout.
- `src/PlaceholderUI.jsx` — baseline layout until Figma screens are fully implemented.
- `src/LardossaApp.jsx` — app entry composition.

After design changes, run `npm run build` so `dist/` (including `dist/assets/index.js` and `dist/assets/index.css`) is updated before embedding via CMake `LardossaUI` BinaryData.
