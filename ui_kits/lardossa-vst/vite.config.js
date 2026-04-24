import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

export default defineConfig({
  plugins: [react()],
  root: ".",
  base: "./",
  build: {
    outDir: "dist",
    emptyOutDir: true,
    /* Inline woff2 from CSS so JUCE BinaryData only needs index.html + JS + one CSS (no extra font URLs). */
    assetsInlineLimit: 512000,
    rollupOptions: {
      output: {
        entryFileNames: "assets/[name].js",
        chunkFileNames: "assets/[name].js",
        assetFileNames: "assets/[name].[ext]",
      },
    },
  },
});
