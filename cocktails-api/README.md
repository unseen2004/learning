# Cocktails API

A RESTful API for managing cocktails, ingredients, and users built with TypeScript and Express.

---

## Overview

Node.js backend application with TypeScript that manages cocktails, ingredients, and users. Uses Prisma ORM for database management with PostgreSQL and includes comprehensive testing with Jest.

Created for https://github.com/Solvro/rekrutacja/blob/main/backend.md

---

## Features

- RESTful API endpoints for cocktails, ingredients, and user management
- JWT authentication and authorization
- Prisma ORM with PostgreSQL database
- Image upload with multer
- Comprehensive test suite (unit + integration)
- TypeScript for type safety

---

## Project Structure

```
cocktails-api/
├── src/
│   ├── handlers/          # Route handlers
│   ├── modules/           # Auth, middleware, types
│   ├── db.ts             # Prisma client
│   ├── router.ts         # Route definitions
│   └── server.ts         # Express setup
├── tests/
│   ├── integration/      # API endpoint tests
│   └── unit/             # Module unit tests
├── prisma/
│   ├── schema.prisma     # Database schema
│   └── migrations/       # Schema migrations
└── package.json
```

---

## Installation

Install dependencies:
```bash
npm install
```

---

## Configuration

Copy example environment file:
```bash
cp example.env .env
```

Configure database connection and other settings in `.env`

---

## Running

Development server:
```bash
npm run dev
```

Build for production:
```bash
npm run build
```

---

## Testing

Run all tests:
```bash
npm test
```

Watch mode:
```bash
npm run test-watch
```

---

## License

MIT License

<!-- REPOS-CLI:START -->
[![JavaScript](https://img.shields.io/badge/javascript-%23323330.svg?style=for-the-badge&logo=javascript&logoColor=%23F7DF1E)](https://developer.mozilla.org/en-US/docs/Web/JavaScript)
[![TypeScript](https://img.shields.io/badge/typescript-%23007ACC.svg?style=for-the-badge&logo=typescript&logoColor=white)](https://www.typescriptlang.org/)

**Node.js Dependencies:** @prisma/client, @types/bcrypt, @types/cors, @types/express, @types/jest, @types/jsonwebtoken, @types/morgan, @types/multer, @types/node, @types/supertest, and more...

## Recent Updates

- structure change (2bc64af)
- structure change (60bba8d)
- structure change rs->tools,... (8baf0d7)
- del example . file (a224c10)
- changed github structure[moved repos to this mono-repo] (a090c1b)


<!-- REPOS-CLI:END -->
