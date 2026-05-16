# Livekit Meet Run Steps

## Prerequisites

1. Install `pnpm` globally:

```bash
npm install -g pnpm
```

2. Install `pm2` globally:

```bash
npm install -g pm2
```

## Setup and Build

1. Install project dependencies:

```bash
pnpm install
```

2. Create the environment file:

```bash
cp .env.example .env.local
```

Update `.env.local` with the required values.

3. Build the project:

```bash
pnpm build
```

4. Enable standalone output in `next.config.js` if you want a standalone deployment package:

```js
/** @type {import('next').NextConfig} */
const nextConfig = {
  output: 'standalone',
};

module.exports = nextConfig;
```

## Start the service from the build

1. Start the app with `pm2`:

```bash
pm2 start "pnpm start" --name livekit-meet
```

## Deploy release build from standalone

1. Build the deployment package:

- Copy the `public` and `static` folder into `.next/standalone/`:

```bash
cp -rf public/ .next/standalone/
cp -rf .next/static/ .next/standalone/.next/
```

- Copy the environment config file into `.next/standalone/`:

```bash
cp -rf .env.local .next/standalone/.env.production
```

- Compress the standalone package and rename the extracted folder to `meetex`:

```bash
tar -C .next --transform='s,^standalone,meetex,' -zcvf ../meetex.tar.gz standalone
```

2. Deploy the package:

- Copy `meetex.tar.gz` to the target VM.

- Extract the package:

```bash
tar -xzf meetex.tar.gz
```

- Quick start the deployed service:

```bash
cd meetex/
pm2 start server.js --name meetex
```

- Fully start the service with ecosystem

```bash
cp <path>/ecosystem.config.js meetex/
cd meetex/
pm2 start ecosystem.config.js
```

