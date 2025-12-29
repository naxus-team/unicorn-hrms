# System Architecture Overview

## Introduction

Unicorn HRMS is built on a modern, scalable architecture designed to handle enterprise-level human resources management requirements.

## Architecture Principles

1. **Modularity** - Independent, loosely coupled modules
2. **Scalability** - Horizontal and vertical scaling capabilities
3. **Security** - Security-first design approach
4. **Maintainability** - Clean code and clear separation of concerns
5. **Performance** - Optimized for speed and efficiency
6. **Reliability** - High availability and fault tolerance

## System Components

### High-Level Architecture
┌─────────────────────────────────────────────────────────┐
│                    Presentation Layer                    │
├──────────────────┬──────────────────┬───────────────────┤
│   Web Desktop    │  Mobile (iOS)    │  Mobile (Android) │
│   Application    │   Application    │   Application     │
└────────┬─────────┴────────┬─────────┴─────────┬─────────┘
│                  │                   │
└──────────────────┼───────────────────┘
│
┌──────▼──────┐
│   API Gateway│
│  (Nginx/IIS) │
└──────┬──────┘
│
┌─────────────────┼─────────────────┐
│                 │                 │
┌────▼────┐      ┌────▼────┐     ┌─────▼────┐
│  Auth   │      │   API   │     │ WebSocket│
│ Service │      │ Service │     │ Service  │
└────┬────┘      └────┬────┘     └─────┬────┘
│                │                 │
└────────────────┼─────────────────┘
│
┌───────────▼───────────┐
│   Business Logic      │
│   Layer (.NET Core)   │
└───────────┬───────────┘
│
┌────────────────┼────────────────┐
│                │                │
┌────▼────┐     ┌────▼────┐     ┌────▼────┐
│Database │     │  Cache  │     │  Queue  │
│(Postgre │     │ (Redis) │     │(RabbitMQ│
│SQL/MySQL│     │         │     │  /Kafka)│
└─────────┘     └─────────┘     └─────────┘

## Layer Architecture

### 1. Presentation Layer

**Web Desktop Application:**
- Modern single-page application
- Responsive design
- Real-time updates via WebSockets
- Progressive Web App (PWA) capabilities

**Mobile Applications:**
- Native iOS (Swift)
- Native Android (Kotlin/Java)
- Offline-first architecture
- Push notifications
- Biometric authentication

### 2. API Gateway

**Responsibilities:**
- Request routing
- Load balancing
- SSL termination
- Rate limiting
- API versioning
- Request/response transformation

**Technologies:**
- Nginx (Linux)
- IIS (Windows)
- Rate limiting middleware
- CORS handling

### 3. Application Layer

**API Services:**
UnicornHRMS.API/
├── Controllers/
│   ├── AuthController.cs
│   ├── EmployeeController.cs
│   ├── AttendanceController.cs
│   ├── LeaveController.cs
│   └── PerformanceController.cs
├── Middleware/
│   ├── AuthenticationMiddleware.cs
│   ├── ExceptionHandlingMiddleware.cs
│   └── RateLimitingMiddleware.cs
└── Filters/
├── ValidateModelAttribute.cs
└── AuthorizeAttribute.cs

### 4. Business Logic Layer

**Core Domain:**
UnicornHRMS.Core/
├── Entities/
│   ├── Employee.cs
│   ├── Attendance.cs
│   ├── LeaveRequest.cs
│   └── PerformanceEvaluation.cs
├── Interfaces/
│   ├── IEmployeeService.cs
│   ├── IAttendanceService.cs
│   └── ILeaveService.cs
└── Specifications/
├── EmployeeSpecification.cs
└── AttendanceSpecification.cs

**Services:**
UnicornHRMS.Services/
├── EmployeeService.cs
├── AttendanceService.cs
├── LeaveService.cs
├── PerformanceService.cs
├── NotificationService.cs
└── ReportService.cs

### 5. Data Access Layer

**Repository Pattern:**
UnicornHRMS.Data/
├── Repositories/
│   ├── EmployeeRepository.cs
│   ├── AttendanceRepository.cs
│   └── LeaveRepository.cs
├── Context/
│   └── ApplicationDbContext.cs
└── Migrations/
└── InitialCreate.cs

## Database Architecture

### Database Schema

**Core Tables:**
- Employees
- Departments
- Positions
- AttendanceRecords
- LeaveRequests
- LeaveTypes
- PerformanceEvaluations
- KPIs
- Users
- Roles
- Permissions

**Relationships:**
- One-to-Many: Department → Employees
- Many-to-One: Employees → Manager
- One-to-Many: Employee → AttendanceRecords
- Many-to-Many: Users → Roles

### Data Partitioning

**Attendance Records:**
- Partitioned by month
- Improves query performance
- Simplifies archiving

**Performance Data:**
- Partitioned by evaluation cycle
- Reduces table size
- Faster access to current data

## Security Architecture

### Authentication Flow

User Login Request
↓
Validate Credentials
↓
Generate JWT Token
↓
Return Token + Refresh Token
↓
Client Stores Tokens
↓
Include Token in API Requests
↓
Validate Token Middleware
↓
Process Request


### Authorization

**Role-Based Access Control (RBAC):**
- Predefined roles
- Custom permissions
- Resource-level access control

**Permission Hierarchy:**
Administrator
├── Full System Access
└── All Operations
HR Manager
├── Employee Management
├── Leave Administration
└── Reports
Department Manager
├── Team Attendance
├── Leave Approval
└── Team Reports
Employee
├── Self-Service
└── View Own Data

### Data Encryption

**At Rest:**
- Database encryption (TDE)
- Encrypted file storage
- Secure key management

**In Transit:**
- TLS 1.2/1.3
- Certificate pinning (mobile)
- Secure WebSockets

## Caching Strategy

### Cache Layers

**1. Application Cache (Memory):**
- Frequently accessed data
- User sessions
- Application configuration

**2. Distributed Cache (Redis):**
- Shared across instances
- User authentication tokens
- Temporary data
- Real-time analytics

**3. HTTP Cache:**
- Static resources
- API responses (GET)
- CDN integration

### Cache Invalidation

- Time-based expiration
- Event-based invalidation
- Manual cache clearing

## Scalability

### Horizontal Scaling

**Load Balancing:**
       ┌─── API Instance 1
       │
Client ─── Load Balancer ─── API Instance 2
│
└─── API Instance 3

**Session Management:**
- Stateless API design
- JWT tokens (no server-side sessions)
- Redis for shared state

### Vertical Scaling

- Increase server resources
- Database optimization
- Query performance tuning

### Database Scaling

**Read Replicas:**
- Separate read and write operations
- Multiple read replicas
- Async replication

**Sharding:**
- Company-based sharding
- Geographic sharding
- Improves performance at scale

## Integration Architecture

### External System Integration

**API Integration:**
Unicorn HRMS ←→ REST API ←→ External System
↓
Webhook ←→ External System

**Supported Integrations:**
- CRM systems (Salesforce, HubSpot)
- Payroll systems
- Email services (SMTP)
- SMS gateways
- Cloud storage (S3, Azure Blob)

### Webhook System

**Event-Driven Architecture:**
1. Event occurs in system
2. Event processor triggered
3. Webhook payload created
4. HTTP POST to configured URL
5. Retry on failure
6. Log result

## Monitoring and Logging

### Application Monitoring

**Metrics Collected:**
- API response times
- Request rates
- Error rates
- Resource utilization
- Database performance

**Tools:**
- Application Insights
- Prometheus + Grafana
- ELK Stack

### Logging Strategy

**Log Levels:**
- DEBUG - Development details
- INFO - General information
- WARNING - Potential issues
- ERROR - Errors requiring attention
- CRITICAL - System failures

**Log Storage:**
- File-based logging
- Centralized logging (ELK)
- Log rotation and archiving

## Backup and Disaster Recovery

### Backup Strategy

**Database Backups:**
- Full backup: Daily
- Incremental backup: Hourly
- Transaction log backup: Every 15 minutes

**File Backups:**
- User uploads: Daily
- Application files: After deployments

**Retention Policy:**
- Daily backups: 30 days
- Weekly backups: 3 months
- Monthly backups: 1 year

### Disaster Recovery

**RTO (Recovery Time Objective):** 4 hours
**RPO (Recovery Point Objective):** 15 minutes

**Recovery Procedures:**
1. Assess damage
2. Restore from backup
3. Verify data integrity
4. Resume operations
5. Post-mortem analysis

## Performance Optimization

### Database Optimization

- Proper indexing
- Query optimization
- Connection pooling
- Prepared statements
- Stored procedures for complex operations

### API Optimization

- Response compression
- Pagination for large datasets
- Selective field loading
- Async operations
- Batch operations

### Frontend Optimization

- Code splitting
- Lazy loading
- Asset compression
- CDN for static assets
- Service workers for PWA

## Technology Stack Summary

**Backend:**
- Language: C# (.NET 8.0)
- Framework: ASP.NET Core
- ORM: Entity Framework Core
- Authentication: JWT
- API Documentation: Swagger/OpenAPI

**Database:**
- Primary: PostgreSQL 14+ or MySQL 8.0+
- Cache: Redis 6+
- Search: (Optional) Elasticsearch

**Frontend:**
- Web: Modern JavaScript framework
- Mobile iOS: Swift
- Mobile Android: Kotlin/Java

**Infrastructure:**
- Web Server: Nginx/IIS
- Container: Docker (optional)
- CI/CD: GitHub Actions
- Cloud: Cloud-agnostic design

**Monitoring:**
- APM: Application Insights
- Metrics: Prometheus
- Visualization: Grafana
- Logging: Serilog + ELK

---

For database schema details, see [database-schema.md](database-schema.md)
For security architecture, see [security.md](security.md)
