# LARDOSSA WebView bridge

The plugin editor hosts the UI in `WebBrowserComponent` (Win WebView2 on Windows). JUCE forwards **native events** from JavaScript to `LardossaAudioProcessor::handleWebNativeEvent`, and pushes a JSON snapshot with **`stateUpdate`** on a timer once the page has fired **`pageReady`**.

## Native → host (`emitEvent` / `emit`)

| Event | Payload (JSON object) | Behaviour |
|--------|------------------------|-----------|
| `paramChange` | `{ "id": string, "value": number }` | Sets APVTS parameter `id` using **real-world units** (same as `getRawParameterValue` / XML presets). |
| `panicAll` | `{}` | Requests voice panic on the audio thread. |
| `seqStep`, `setStep` | `{ "index": number, "note": number, "vel": number, "accent": bool, "slide": bool, "tie": bool, "len": number, "on": bool }` | Replaces one sequencer step (`len` = step length in sixteenths, normalised in the processor). |
| `generatePattern` | `{ "mode": string, "variety": number }` | `mode`: `DetroitBass`, `DetroitLead`, or `Electro`. `variety` in \([0,1]\) (default 0.5). |
| `loadFactoryPreset` | `{ "name": string }` | Applies embedded factory XML via `PresetManager`. Names match `PresetManager::getFactoryPresetNames()`. |

Optional step helpers (not wired in `PluginEditor.cpp` today, but handled in the processor): `setAccentStep`, `setSlideStep`, `setTieStep` with `{ "index": number, "value": bool }`.

## Host → UI (`stateUpdate`)

Payload is a **JSON string** (or an object with a `value` / `json` string field, depending on the backend). After parsing, React receives a `CustomEvent("lardossa:state")` with `detail` = the object below.

Top-level keys (non-exhaustive):

- **Parameters**: `engineMode`, `algorithm`, `op1Ratio` … `op4Ratio`, `op1Level` … `op4Level`, `op1Feedback`, `voltage`, `depth`, `pressure`, `decay`, envelope and sequencer params, `outputGain`, `filterDrive`, etc. — numeric values in plugin units.
- **`presets`**: array of factory preset display names.
- **`seqSteps`**: array of `{ index, on, note, vel, accent, slide, tie, len }` for the current pattern length.

## React helpers (`bridge.js`)

- `installLardossaBridge()` — subscribes to `stateUpdate` and dispatches `lardossa:state`.
- `useLardossaState()` — `{ state, setParam, panicAll, loadFactoryPreset, generatePattern }` wrapping the table above via `window.__JUCE__.backend`.
