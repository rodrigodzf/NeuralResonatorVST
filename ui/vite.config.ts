import react from '@vitejs/plugin-react-swc'
import { defineConfig } from 'vite'
import { viteSingleFile } from 'vite-plugin-singlefile'

// We use the viteSingleFile plugin to bundle the UI into a single file
// from https://stackoverflow.com/questions/67031295/how-to-open-a-static-website-in-localhost-but-generated-with-vite-and-without-ru
export default defineConfig({
    server: {
        port: 3000,
    },
    plugins: [react(), viteSingleFile()],
})
