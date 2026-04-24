import React from "react";
import { createRoot } from "react-dom/client";
import { LardossaApp } from "./LardossaApp.jsx";
import { installLardossaBridge } from "./bridge.js";
import "./tokens.css";

installLardossaBridge();

createRoot(document.getElementById("root")).render(
  <React.StrictMode>
    <LardossaApp />
  </React.StrictMode>
);
