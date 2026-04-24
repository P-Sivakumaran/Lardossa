# LARDOSSA design system

Use this document in **Figma**, **Claude Design**, and engineering handoffs. For **Anthropic Claude Design → “Set up your design system”**, paste the **blurb in §0** into the *Company name and blurb* field; keep **GitHub** linked to `P-Sivakumaran/Lardossa` (or your fork).

---

## §0 — Claude Design setup (paste into “Company name and blurb”)

**Company / system name:** LARDOSSA Design System

**Blurb (copy below this line):**

LARDOSSA is a professional **JUCE 8 audio plugin** (VST3 + Standalone): **four-operator FM**, **Detroit-style step sequencer**, and an **embedded React + Vite UI** (900×560) served inside **WebView2**—no CDN fonts; **IBM Plex Sans** and **IBM Plex Mono** are self-hosted; optional **ITC Machine** for the wordmark only when licensed and bundled.

**Visual ethos:** **International Typographic / DIN panel rationalism** adapted for **music hardware**—**near-black field**, **single cyan electrical accent** (`#00D4FF`), **flat panels** with **hairline rules**, **8-pt grid**, **tabular numerics** for every readout. References **rack silkscreen**, **test-instrument clarity**, and **industrial titling**—**not** script faces, **not** glassmorphism, **not** club-neon decoration. Sequencer UI reads as a **matrix instrument** (steps, accent, slide, tie, length).

**Repo:** UI tokens and components live under `ui_kits/lardossa-vst/src/` (`tokens.css`, `PlaceholderUI.*`); C++ chrome in `Source/PluginEditor.*`. Factory presets: `Presets/Factory/*.xml`. **Do not** use the CSS generic `cursive` in stacks (Windows WebView often maps it to Edwardian Script).

---

## §1 — Positioning

**One line:** Precision **machinery under human control**—voltage, timing, pattern, timbre—communicated with **editorial restraint** and **signal-path legibility**.

---

## §2 — Historical lineages (vocabulary)

| Lineage | What we borrow |
|--------|------------------|
| **International Typographic Style** | Modular grid, objective color, **flush hierarchy**, minimal ornament |
| **DIN / control-panel rationalism** | Grouping by function, **label–control–value** triads, **unambiguous numerals** |
| **Mid-century corporate modernism** (restrained) | Single accent hue as **indicator**, large neutrals |
| **Brutalist digital / LCD culture** | **Monospace** for parameters—honesty without hostility |
| **Detroit / industrial *graphics*** | Stark type, **mechanical rhythm** in layout—**not** literal neon |

---

## §3 — Principles

1. **Signal path over scenery**  
2. **One accent family** (chroma = state, not decoration)  
3. **Sans for language; mono for numbers / JSON / timing**  
4. **No script in chrome**  
5. **Wordmark = machine marking** (ITC Machine when embedded; else semibold Plex Sans caps)  
6. **Fixed canvas** 900×560—design in **modules**

---

## §4 — Typography

| Role | Font | Notes |
|------|------|--------|
| Display / wordmark | **ITC Machine** (optional, licensed) | Only when `@font-face` + woff2 in repo |
| UI | **IBM Plex Sans** | Labels, sections, body |
| Readouts | **IBM Plex Mono** | **Tabular lining figures**; right-align in rows |

**Scale (starting point):** wordmark ~32–40 px, **600** if sans-only; section labels **11–12 px**, uppercase, **+0.08–0.12em** tracking; body **12–13 px**; readouts **12 px** mono.

---

## §5 — Color (tokens)

| Token | Example | Use |
|--------|---------|-----|
| Field | `#0A0A0A` | Page void |
| Surface | `#111111` | Panels |
| Border | `#1E1E1E`–`#2A2A2A` | Hairlines |
| Accent | `#00D4FF` | Focus, live values, primary emphasis |
| Accent dim | `#006680` | De-emphasized chrome |

Extend: **warning/clip** in a **separate hue family** from cyan; **WCAG AA+** on text vs field.

---

## §6 — Layout

- **8-pt grid** (4-pt for hairlines)  
- **Panel:** 1 px border, **8–16 px** padding, **no** heavy shadow  
- **Sequencer:** clear **on/off, accent, slide, tie, length** states  

---

## §7 — Motion

**80–160 ms**; **cubic-out** or **linear**—no bouncy easing on audio-linked controls.

---

## §8 — Anti-patterns

Script/luxury display in functional UI; neon gradients; glassmorphism; decorative icons without operational meaning; gray-on-gray parameter text.

---

## §9 — Repo map

| Area | Path |
|------|------|
| Design tokens (CSS) | `ui_kits/lardossa-vst/src/tokens.css` |
| Placeholder UI | `ui_kits/lardossa-vst/src/PlaceholderUI.jsx`, `.css` |
| Bridge / state | `ui_kits/lardossa-vst/src/bridge.js`, `BRIDGE_SCHEMA.md` |
| Figma notes | `ui_kits/lardossa-vst/src/FIGMA_INTEGRATION.md` |
| Plugin editor | `Source/PluginEditor.cpp` |
| Presets | `Presets/Factory/*.xml`, `Source/PresetManager.*` |

---

*Document version: 1.0 — aligned with GitHub `P-Sivakumaran/Lardossa` main branch.*
