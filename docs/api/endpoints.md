# API Endpoints Reference

## Base URL
Production: https://api.unicorn-hrms.com/v1
Staging: https://staging-api.unicorn-hrms.com/v1
Development: http://localhost:5000/api/v1

## Authentication

All endpoints require authentication unless specified. Include JWT token in header:
Authorization: Bearer {your_token_here}

---

## Employee Endpoints

### Get All Employees

**GET** `/api/employees`

Query Parameters:
- `page` (integer): Page number (default: 1)
- `limit` (integer): Results per page (default: 20, max: 100)
- `department` (string): Filter by department ID
- `status` (string): active, inactive, suspended
- `search` (string): Search by name or email

Response:
```json
{
  "data": [
    {
      "id": "emp-001",
      "firstName": "Ahmed",
      "lastName": "Hassan",
      "email": "ahmed.hassan@company.com",
      "department": "Engineering",
      "position": "Software Engineer",
      "status": "active",
      "hireDate": "2024-01-15",
      "managerId": "emp-100"
    }
  ],
  "pagination": {
    "currentPage": 1,
    "totalPages": 5,
    "totalRecords": 100,
    "limit": 20
  }
}
```

### Get Employee by ID

**GET** `/api/employees/{id}`

Response:
```json
{
  "id": "emp-001",
  "firstName": "Ahmed",
  "lastName": "Hassan",
  "email": "ahmed.hassan@company.com",
  "phone": "+20 123 456 7890",
  "department": {
    "id": "dept-001",
    "name": "Engineering"
  },
  "position": "Software Engineer",
  "status": "active",
  "hireDate": "2024-01-15",
  "manager": {
    "id": "emp-100",
    "name": "Mohamed Ali"
  },
  "address": {
    "street": "123 Street Name",
    "city": "Cairo",
    "country": "Egypt"
  }
}
```

### Create Employee

**POST** `/api/employees`

Required Role: `admin`, `hr_manager`

Request:
```json
{
  "firstName": "Ahmed",
  "lastName": "Hassan",
  "email": "ahmed.hassan@company.com",
  "phone": "+20 123 456 7890",
  "departmentId": "dept-001",
  "position": "Software Engineer",
  "hireDate": "2024-01-15",
  "managerId": "emp-100",
  "salary": 15000,
  "contractType": "full-time"
}
```

### Update Employee

**PUT** `/api/employees/{id}`

Required Role: `admin`, `hr_manager`

### Delete Employee

**DELETE** `/api/employees/{id}`

Required Role: `admin`

---

## Attendance Endpoints

### Clock In

**POST** `/api/attendance/clock-in`

Request:
```json
{
  "employeeId": "emp-001",
  "timestamp": "2025-01-15T08:30:00Z",
  "location": {
    "latitude": 30.0444,
    "longitude": 31.2357
  },
  "note": "Started work"
}
```

Response:
```json
{
  "recordId": "att-001",
  "employeeId": "emp-001",
  "clockInTime": "2025-01-15T08:30:00Z",
  "status": "on-time",
  "message": "Clock in successful"
}
```

### Clock Out

**POST** `/api/attendance/clock-out`

Request:
```json
{
  "employeeId": "emp-001",
  "timestamp": "2025-01-15T17:30:00Z",
  "location": {
    "latitude": 30.0444,
    "longitude": 31.2357
  },
  "note": "End of day"
}
```

### Get Attendance Records

**GET** `/api/attendance/records`

Query Parameters:
- `employeeId` (string): Employee ID
- `startDate` (date): Start date (YYYY-MM-DD)
- `endDate` (date): End date (YYYY-MM-DD)
- `status` (string): present, absent, late, early-leave

### Get Attendance Summary

**GET** `/api/attendance/summary/{employeeId}`

Query Parameters:
- `month` (integer): Month (1-12)
- `year` (integer): Year

Response:
```json
{
  "employeeId": "emp-001",
  "period": "2025-01",
  "totalWorkingDays": 22,
  "daysPresent": 20,
  "daysAbsent": 2,
  "daysLate": 3,
  "totalHours": 176,
  "overtimeHours": 8
}
```

---

## Leave Management Endpoints

### Submit Leave Request

**POST** `/api/leave/request`

Request:
```json
{
  "employeeId": "emp-001",
  "leaveTypeId": "leave-type-001",
  "startDate": "2025-02-01",
  "endDate": "2025-02-05",
  "reason": "Family vacation",
  "attachments": ["document-id-1"]
}
```

Response:
```json
{
  "requestId": "leave-req-001",
  "status": "pending",
  "submittedAt": "2025-01-15T10:00:00Z",
  "approvalChain": [
    {
      "approverId": "emp-100",
      "approverName": "Mohamed Ali",
      "role": "manager",
      "status": "pending"
    }
  ]
}
```

### Get Leave Balance

**GET** `/api/leave/balance/{employeeId}`

Response:
```json
{
  "employeeId": "emp-001",
  "leaveTypes": [
    {
      "typeId": "leave-type-001",
      "typeName": "Annual Leave",
      "total": 21,
      "used": 5,
      "remaining": 16,
      "pending": 3
    },
    {
      "typeId": "leave-type-002",
      "typeName": "Sick Leave",
      "total": 15,
      "used": 2,
      "remaining": 13,
      "pending": 0
    }
  ]
}
```

### Approve/Reject Leave

**PUT** `/api/leave/approve/{requestId}`

Required Role: `manager`, `hr_manager`, `admin`

Request:
```json
{
  "status": "approved",
  "comment": "Approved for the requested dates",
  "approverId": "emp-100"
}
```

### Get Leave History

**GET** `/api/leave/history/{employeeId}`

---

## Performance Evaluation Endpoints

### Create Evaluation

**POST** `/api/performance/evaluation`

Required Role: `manager`, `hr_manager`, `admin`

Request:
```json
{
  "employeeId": "emp-001",
  "evaluatorId": "emp-100",
  "cycleId": "cycle-2025-q1",
  "evaluationDate": "2025-01-15",
  "kpis": [
    {
      "kpiId": "kpi-001",
      "score": 4.5,
      "comment": "Excellent performance"
    }
  ],
  "attitudeScore": 4.0,
  "overallComment": "Great team player",
  "goals": [
    {
      "description": "Lead new project",
      "targetDate": "2025-06-30"
    }
  ]
}
```

### Get Employee KPIs

**GET** `/api/performance/kpi/{employeeId}`

Query Parameters:
- `cycleId` (string): Evaluation cycle ID
- `year` (integer): Year

Response:
```json
{
  "employeeId": "emp-001",
  "cycle": "2025-Q1",
  "kpis": [
    {
      "kpiId": "kpi-001",
      "name": "Code Quality",
      "category": "technical",
      "target": 90,
      "actual": 92,
      "score": 4.5,
      "weight": 30
    }
  ],
  "overallScore": 4.3,
  "rating": "Exceeds Expectations"
}
```

### Get Performance Reports

**GET** `/api/performance/reports`

Query Parameters:
- `departmentId` (string)
- `cycleId` (string)
- `format` (string): json, pdf, excel

---

## CRM Integration Endpoints

### Get Call Records

**GET** `/api/crm/call-records`

Query Parameters:
- `employeeId` (string)
- `startDate` (date)
- `endDate` (date)
- `status` (string): completed, missed, ongoing

Response:
```json
{
  "data": [
    {
      "callId": "call-001",
      "employeeId": "emp-001",
      "customerId": "cust-001",
      "customerName": "ABC Company",
      "startTime": "2025-01-15T10:00:00Z",
      "duration": 300,
      "status": "completed",
      "recordingUrl": "https://storage.com/recordings/call-001.mp3",
      "qualityScore": 4.5,
      "notes": "Customer satisfied with service"
    }
  ]
}
```

### Sync CRM Data

**POST** `/api/crm/sync`

Required Role: `admin`, `system`

---

## Reports Endpoints

### Generate Report

**POST** `/api/reports/generate`

Request:
```json
{
  "reportType": "attendance-summary",
  "parameters": {
    "departmentId": "dept-001",
    "startDate": "2025-01-01",
    "endDate": "2025-01-31"
  },
  "format": "pdf"
}
```

Response:
```json
{
  "reportId": "report-001",
  "status": "processing",
  "estimatedTime": 30,
  "downloadUrl": null
}
```

### Download Report

**GET** `/api/reports/download/{reportId}`

---

## Error Responses

### 400 Bad Request
```json
{
  "error": {
    "code": "VALIDATION_ERROR",
    "message": "Invalid request parameters",
    "details": [
      {
        "field": "email",
        "message": "Invalid email format"
      }
    ]
  }
}
```

### 401 Unauthorized
```json
{
  "error": {
    "code": "UNAUTHORIZED",
    "message": "Authentication required"
  }
}
```

### 403 Forbidden
```json
{
  "error": {
    "code": "FORBIDDEN",
    "message": "Insufficient permissions"
  }
}
```

### 404 Not Found
```json
{
  "error": {
    "code": "NOT_FOUND",
    "message": "Resource not found"
  }
}
```

### 500 Internal Server Error
```json
{
  "error": {
    "code": "INTERNAL_ERROR",
    "message": "An unexpected error occurred",
    "requestId": "req-12345"
  }
}
```

---

## Rate Limiting

- **Standard endpoints:** 100 requests per minute
- **Authentication endpoints:** 10 requests per minute
- **Report generation:** 5 requests per minute

Rate limit headers:
X-RateLimit-Limit: 100
X-RateLimit-Remaining: 95
X-RateLimit-Reset: 1642234567

---

For webhook configuration, see [webhooks.md](webhooks.md)
