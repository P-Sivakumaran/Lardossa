import React from "react";

function getJuceBackend() {
  return window.__JUCE__ && window.__JUCE__.backend ? window.__JUCE__.backend : null;
}

function juceEmit(event, data) {
  const b = getJuceBackend();
  if (!b) return false;
  const payload = data != null ? data : {};
  if (typeof b.emitEvent === "function") {
    b.emitEvent(event, payload);
    return true;
  }
  if (typeof b.emit === "function") {
    b.emit(event, payload);
    return true;
  }
  return false;
}

function juceListen(event, handler) {
  const b = getJuceBackend();
  if (!b) return false;
  if (typeof b.addEventListener === "function") {
    b.addEventListener(event, handler);
    return true;
  }
  if (typeof b.addListener === "function") {
    b.addListener(event, handler);
    return true;
  }
  return false;
}

/** Wire JUCE native → `lardossa:state` for React. Call once at startup. */
export function installLardossaBridge() {
  juceListen("stateUpdate", (payload) => {
    try {
      let detail = payload;
      if (typeof payload === "string") detail = JSON.parse(payload);
      else if (payload != null && typeof payload === "object") {
        if (typeof payload.value === "string") detail = JSON.parse(payload.value);
        else if (typeof payload.json === "string") detail = JSON.parse(payload.json);
      }
      window.dispatchEvent(new CustomEvent("lardossa:state", { detail }));
    } catch (e) {
      console.warn("lardossa: state parse", e);
    }
  });
}

/** React hook: live APVTS snapshot + `setParam(id, realUnitValue)`. */
export function useLardossaState() {
  const [state, setState] = React.useState({});

  React.useEffect(() => {
    const handler = (e) => setState(e.detail || {});
    window.addEventListener("lardossa:state", handler);
    return () => window.removeEventListener("lardossa:state", handler);
  }, []);

  const setParam = React.useCallback((id, value) => {
    juceEmit("paramChange", { id, value });
  }, []);

  const panicAll = React.useCallback(() => {
    juceEmit("panicAll", {});
  }, []);

  const loadFactoryPreset = React.useCallback((name) => {
    juceEmit("loadFactoryPreset", { name });
  }, []);

  const generatePattern = React.useCallback((mode, variety) => {
    juceEmit("generatePattern", { mode, variety });
  }, []);

  return { state, setParam, panicAll, loadFactoryPreset, generatePattern };
}
