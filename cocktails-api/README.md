# 🍸 Cocktails API


<p align="center">
  <img src="https://img.shields.io/badge/Node.js-339933?style=for-the-badge&logo=node.js&logoColor=white" alt="Node.js" />
  <img src="https://img.shields.io/badge/TypeScript-007ACC?style=for-the-badge&logo=typescript&logoColor=white" alt="TypeScript" />
  <img src="https://img.shields.io/badge/Prisma-2C3741?style=for-the-badge&logo=prisma&logoColor=white" alt="Prisma" />
  <img src="https://img.shields.io/badge/Jest-C21325?style=for-the-badge&logo=jest&logoColor=white" alt="Jest" />
</p>

---

Created for https://github.com/Solvro/rekrutacja/blob/main/backend.md


## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Installation](#installation)
- [Environment Setup](#environment-setup)
- [Running the Server](#running-the-server)
- [Testing](#testing)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

---

## Overview
**Cocktails API** is a Node.js backend application built with TypeScript, designed to manage cocktails, ingredients, and users. It leverages Prisma ORM for database management and includes a comprehensive test suite powered by Jest. The repository is structured to support both development and production environments with clear separation of concerns.

---

## Features
- **RESTful API:** Endpoints for cocktails, ingredients, and user management.
- **Database Integration:** Uses Prisma for schema migrations and database operations.
- **Modular Architecture:** Organized codebase with clear separation into handlers, modules, and routes.
- **Testing:** Integrated unit and integration tests with Jest.
- **Environment Configurations:** Easily manage configuration through environment variables.

---

## Project Structure

### Root Structure
```
.
├── example.env                    # Sample environment configuration file
├── jest.config.js                 # Jest configuration
├── LICENSE                        # Project license
├── package.json                   # Node package manifest
├── package-lock.json              # Package lock file
├── prisma                         # Database schema and migrations
│   ├── migrations                 # Migration files for schema updates
│   │   ├── [migration folders]
│   │   └── migration_lock.toml
│   └── schema.prisma              # Prisma schema definition
├── README.md                      # This file
├── src                            # Source code
│   ├── db.ts                      # Database connection and Prisma client
│   ├── handlers                   # API route handlers (cocktail, ingredient, user)
│   ├── index.ts                   # Application entry point
│   ├── modules                    # Custom modules (auth, middleware, types, upload)
│   ├── router.ts                  # Express router definitions
│   └── server.ts                  # Express server setup and configuration
├── tests                          # Testing directory
│   ├── fixtures                   # Test fixtures for sample data
│   ├── integration                # Integration tests for endpoints
│   ├── setup.ts                   # Test environment setup
│   └── unit                       # Unit tests for specific modules
└── uploads                        # Directory for file uploads
```

---

## Installation
1. **Clone the Repository:**
   ```bash
   git clone https://github.com/yourusername/your-repo-name.git
   cd your-repo-name
   ```

2. **Install Dependencies:**
   ```bash
   npm install
   ```

---

## Environment Setup
1. Copy the sample environment file to create your own configuration:
   ```bash
   cp example.env .env
   ```

2. Edit the `.env` file to configure your database connection, API keys, and other settings.

---

## Running the Server
To start the development server:
```bash
npm run dev
```
This command compiles the TypeScript code and launches the server, typically available at http://localhost:3000.

---

## Testing
The project uses Jest for testing. To run the tests, use:
```bash
npm test
```
This command executes both unit and integration tests located in the `tests/` directory.

---

## License
This project is licensed under the MIT License.
