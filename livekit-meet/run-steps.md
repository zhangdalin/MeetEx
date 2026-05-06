# Livekit Meet Run Steps

## Prerequisites

1. Install pnpm globally

	npm install -g pnpm

2. Install pm2 globally

	npm install -g pm2

## Setup And Build

1. Install project dependencies

	pnpm install

2. Create environment file

	Copy .env.example to .env.local and update values as needed.

3. Build project

	pnpm build

4. If want to deploy standalone mode, need to change the next.config.js

		/** @type {import('next').NextConfig} */
		const nextConfig = {
			output: 'standalone',
		};

		module.exports = nextConfig;

## Start Service

1. Start with pm2

	pm2 start "pnpm start" --name livekit-meet