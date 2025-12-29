# Authentication API

## Overview

Unicorn HRMS uses JWT (JSON Web Tokens) for authentication and authorization.

## Authentication Flow

1. User submits credentials
2. Server validates credentials
3. Server generates JWT token
4. Client stores token
5. Client includes token in subsequent requests

## Endpoints

### Login

**POST** `/api/auth/login`

Request:
```json
{
  "username": "user@example.com",
  "password": "password123"
}
```

Response:
```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "refreshToken": "refresh_token_here",
  "expiresIn": 3600,
  "user": {
    "id": "user-id",
    "name": "John Doe",
    "email": "user@example.com",
    "role": "employee"
  }
}
```

### Refresh Token

**POST** `/api/auth/refresh-token`

### Logout

**POST** `/api/auth/logout`

### Password Reset

**POST** `/api/auth/reset-password`

## Token Usage

Include token in Authorization header:
