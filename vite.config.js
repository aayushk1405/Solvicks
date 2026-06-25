import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// [https://vitejs.dev/config/](https://vitejs.dev/config/)
export default defineConfig({
  plugins: [react()],
  server: {
    // This ensures WASM files are served with the correct MIME type
    fs: {
      strict: false,
    }
  }
})
