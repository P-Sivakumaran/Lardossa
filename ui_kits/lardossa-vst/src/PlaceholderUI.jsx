import React from "react";
import { useLardossaState } from "./bridge.js";
import "./PlaceholderUI.css";

function KnobRow({ id, label, min, max, step, state, setParam }) {
  const v = state[id];
  const num = typeof v === "number" ? v : parseFloat(v);
  const safe = Number.isFinite(num) ? num : min;
  return (
    <div className="row">
      <label>{label}</label>
      <input
        type="range"
        min={min}
        max={max}
        step={step}
        value={safe}
        onChange={(e) => setParam(id, parseFloat(e.target.value))}
      />
      <span className="val">{safe.toFixed(step < 0.1 ? 2 : 0)}</span>
    </div>
  );
}

export function PlaceholderUI() {
  const { state, setParam, panicAll, loadFactoryPreset, generatePattern } = useLardossaState();

  React.useEffect(() => {
    const b = window.__JUCE__ && window.__JUCE__.backend;
    if (b) b.emitEvent("pageReady", {});
  }, []);

  const presets = Array.isArray(state.presets) ? state.presets : [];

  return (
    <div className="placeholder">
      <h1 className="wordmark">LARDOSSA</h1>
      <p className="tag">Supine Music — FM / Detroit sequencer</p>

      <section className="panel">
        <h2>Macros</h2>
        <KnobRow id="voltage" label="Voltage (Hz)" min={20} max={20000} step={1} state={state} setParam={setParam} />
        <KnobRow id="depth" label="Depth" min={0} max={1} step={0.001} state={state} setParam={setParam} />
        <KnobRow id="pressure" label="Pressure" min={0} max={1} step={0.001} state={state} setParam={setParam} />
        <KnobRow id="decay" label="Decay (ms)" min={1} max={2000} step={1} state={state} setParam={setParam} />
      </section>

      <section className="panel">
        <h2>Transport & patterns</h2>
        <div className="row">
          <label>Algorithm</label>
          <span className="val">{state.algorithm != null ? state.algorithm : "—"}</span>
        </div>
        <div className="row">
          <label>Seq length</label>
          <span className="val">{state.seqLength != null ? state.seqLength : "—"}</span>
        </div>
        <div className="row">
          <button type="button" onClick={() => generatePattern("DetroitBass", 0.5)}>
            Detroit bass
          </button>
          <button type="button" onClick={() => generatePattern("Electro", 0.4)}>
            Electro
          </button>
        </div>
      </section>

      <section className="panel">
        <h2>Presets</h2>
        <div className="row">
          <select
            onChange={(e) => {
              if (e.target.value) loadFactoryPreset(e.target.value);
            }}
            defaultValue=""
          >
            <option value="" disabled>
              Load factory…
            </option>
            {presets.map((p) => (
              <option key={p} value={p}>
                {p}
              </option>
            ))}
          </select>
        </div>
        <div className="row">
          <button type="button" onClick={panicAll}>
            Panic (all notes off)
          </button>
        </div>
      </section>

      <section className="panel mono">
        <pre>{JSON.stringify({ algorithm: state.algorithm, voltage: state.voltage, depth: state.depth, seqLength: state.seqLength }, null, 2)}</pre>
      </section>
    </div>
  );
}
