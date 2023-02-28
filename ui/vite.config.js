import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react-swc'
import { viteSingleFile } from "vite-plugin-singlefile"
// https://vitejs.dev/config/

// We use the viteSingleFile plugin to bundle the UI into a single file
// from https://stackoverflow.com/questions/67031295/how-to-open-a-static-website-in-localhost-but-generated-with-vite-and-without-ru
export default defineConfig({ 
    server: {
        port: 3000,
    },
    plugins: [react(), viteSingleFile()],
})
